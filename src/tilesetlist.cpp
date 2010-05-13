//  Copyright (C) 2007, 2008, 2009, 2010 Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#include <iostream>
#include <algorithm>
#include <expat.h>
#include "rectangle.h"
#include "ucompose.hpp"
#include <sigc++/functors/mem_fun.h>

#include "tilesetlist.h"
#include "File.h"
#include "defs.h"
#include "tileset.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

Tilesetlist* Tilesetlist::s_instance = 0;

Tilesetlist* Tilesetlist::getInstance()
{
    if (!s_instance)
        s_instance = new Tilesetlist();

    return s_instance;
}

void Tilesetlist::deleteInstance()
{
    if (s_instance)
      delete s_instance;

    s_instance = 0;
}

Tilesetlist::Tilesetlist()
	:SetList()
{
    // load all tilesets
    loadTilesets(Tileset::scanSystemCollection());
    loadTilesets(Tileset::scanUserCollection());
}

Tilesetlist::~Tilesetlist()
{
  for (iterator it = begin(); it != end(); it++)
    delete (*it);
}

void Tilesetlist::getSizes(std::list<guint32> &sizes) const
{
  for (const_iterator i = begin(); i != end(); i++)
    {
      if (find (sizes.begin(), sizes.end(), (*i)->getTileSize()) == sizes.end())
	sizes.push_back((*i)->getTileSize());
    }
}

std::list<std::string> Tilesetlist::getNames() const
{
  std::list<std::string> names;
  for (const_iterator it = begin(); it != end(); it++)
    names.push_back((*it)->getName());
  return names;
}

std::list<std::string> Tilesetlist::getNames(guint32 tilesize) const
{
  std::list<std::string> names;
  for (const_iterator it = begin(); it != end(); it++)
    if ((*it)->getTileSize() == tilesize)
      names.push_back((*it)->getName());
  return names;
}

Tileset *Tilesetlist::loadTileset(std::string filename)
{
  debug("Loading tileset " <<File::get_basename(File::get_dirname(filename)));
  Tileset *tileset = Tileset::create(filename);
  if (tileset == NULL)
    return NULL;
  if (tileset->validate() == false)
    {
      cerr<< "Error!  Tileset: `" << tileset->getConfigurationFile() <<
	"' fails validation.  Skipping.\n",
      delete tileset;
      return NULL;
    }
  if (d_tilesetids.find(tileset->getId()) != d_tilesetids.end())
    {
      Tileset *t = (*d_tilesetids.find(tileset->getId())).second;
      cerr << "Error!  tileset: `" << tileset->getName() << 
	"' shares a duplicate tileset id with `" << 
	t->getConfigurationFile() << "'.  Skipping." << endl;
      delete tileset;
      return NULL;
    }
  return tileset;
}

void Tilesetlist::add(Tileset *tileset)
{
  std::string subdir = File::get_basename(tileset->getDirectory());
  push_back(tileset); 
  tileset->setSubDir(subdir);
  d_dirs[String::ucompose("%1 %2", tileset->getName(), tileset->getTileSize())] = subdir;
  d_tilesets[subdir] = tileset;
  d_tilesetids[tileset->getId()] = tileset;
}

std::string Tilesetlist::getTilesetDir(std::string name, guint32 tilesize) const
{
  DirMap::const_iterator it = d_dirs.find(String::ucompose("%1 %2", name, tilesize));
  if (it == d_dirs.end())
    return "";
  else
    return (*it).second;
}

void Tilesetlist::loadTilesets(std::list<std::string> tilesets)
{
  for (std::list<std::string>::const_iterator i = tilesets.begin(); 
       i != tilesets.end(); i++)
    {
      Tileset *tileset = loadTileset(*i);
      if (!tileset)
	continue;
      add(tileset);
    }
}

int Tilesetlist::getNextAvailableId(int after)
{
  std::list<guint32> ids;
  std::list<std::string> tilesets = Tileset::scanSystemCollection();
  //there might be IDs in invalid tilesets.
  for (std::list<std::string>::const_iterator i = tilesets.begin(); 
       i != tilesets.end(); i++)
    {
      Tileset *tileset = Tileset::create(*i);
      if (tileset != NULL)
	{
	  ids.push_back(tileset->getId());
	  delete tileset;
	}
    }
  tilesets = Tileset::scanUserCollection();
  for (std::list<std::string>::const_iterator i = tilesets.begin(); 
       i != tilesets.end(); i++)
    {
      Tileset *tileset = Tileset::create(*i);
      if (tileset != NULL)
	{
	  ids.push_back(tileset->getId());
	  delete tileset;
	}
    }
  for (guint32 i = after + 1; i < 1000000; i++)
    {
      if (find(ids.begin(), ids.end(), i) == ids.end())
	return i;
    }
  return -1;
}

void Tilesetlist::uninstantiateImages()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->uninstantiateImages();
}
void Tilesetlist::instantiateImages()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->instantiateImages();
}
	
Tileset *Tilesetlist::getTileset(std::string dir) const
{ 
  TilesetMap::const_iterator it = d_tilesets.find(dir);
  if (it == d_tilesets.end())
    return NULL;
  return (*it).second;
}
	
Tileset *Tilesetlist::getTileset(guint32 id) const
{ 
  TilesetIdMap::const_iterator it = d_tilesetids.find(id);
  if (it == d_tilesetids.end())
    return NULL;
  return (*it).second;
}

bool Tilesetlist::addToPersonalCollection(Tileset *tileset, std::string &new_subdir, guint32 &new_id)
{
  //do we already have this one?
      
  if (getTileset(tileset->getSubDir()) == getTileset(tileset->getId()) &&
      getTileset(tileset->getSubDir()) != NULL)
    {
      tileset->setDirectory(getTileset(tileset->getId())->getDirectory());
      return true;
    }

  //if the subdir conflicts with any other subdir, then change it.
  if (getTileset(tileset->getSubDir()) != NULL)
    {
      if (new_subdir != "" && getTileset(new_subdir) == NULL)
        tileset->setSubDir(new_subdir);
      else
        {
          bool found = false;
          for (int count = 0; count < 100; count++)
            {
              new_subdir = String::ucompose("%1%2", tileset->getSubDir(), 
                                            count);
              if (getTileset(new_subdir) == NULL)
                {
                  found = true;
                  break;
                }
            }
          if (found == false)
            return false;
          tileset->setSubDir(new_subdir);
        }
    }
  else
    new_subdir = tileset->getSubDir();

  //if the id conflicts with any other id, then change it
  if (getTileset(tileset->getId()))
    {
      if (new_id != 0 && getTileset(new_id) == NULL)
        tileset->setId(new_id);
      else
        {
          new_id = Tilesetlist::getNextAvailableId(tileset->getId());
          tileset->setId(new_id);
        }
    }
  else
    new_id = tileset->getId();

  //make the directory where the tileset is going to live.
  std::string directory = 
    File::getUserTilesetDir() + tileset->getSubDir() + "/";

  if (File::create_dir(directory) == false)
    return false;

  //okay now we copy the image files into the new directory 
  std::list<std::string> files;
  tileset->getFilenames(files);
  for (std::list<std::string>::iterator it = files.begin(); it != files.end();
       it++)
    File::copy(tileset->getFile(*it), directory + *it + ".png");

  //save out the tileset file
  tileset->setDirectory(directory);
  XML_Helper helper(tileset->getConfigurationFile(), std::ios::out, false);
  tileset->save(&helper);
  helper.close();
  add (tileset);
  return true;
}

bool Tilesetlist::contains(std::string name) const
{
  std::list<std::string> n = getNames();
  for (std::list<std::string>::iterator it = n.begin(); it != n.end(); it++)
    {
      if (*it == name)
        return true;
    }
  return false;
}
