//  Copyright (C) 2007, 2008, 2009, 2010, 2011 Ben Asselstine
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
#include <assert.h>
#include "rectangle.h"
#include "ucompose.hpp"
#include <sigc++/functors/mem_fun.h>

#include "tilesetlist.h"
#include "File.h"
#include "defs.h"
#include "tileset.h"
#include "tarhelper.h"
#include "SmallTile.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

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

std::list<std::string> Tilesetlist::getValidNames() const
{
  std::list<std::string> names;
  for (const_iterator it = begin(); it != end(); it++)
    if ((*it)->validate() == true)
      names.push_back((*it)->getName());
  return names;
}

std::list<std::string> Tilesetlist::getValidNames(guint32 tilesize) const
{
  std::list<std::string> names;
  for (const_iterator it = begin(); it != end(); it++)
    if ((*it)->getTileSize() == tilesize && (*it)->validate() == true)
      names.push_back((*it)->getName());
  return names;
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

Tileset *Tilesetlist::loadTileset(std::string name)
{
  debug("Loading tileset " <<File::get_basename(name));
  bool unsupported_version = false;
  Tileset *tileset = Tileset::create(name, unsupported_version);
  if (tileset == NULL)
    {
      cerr<< "Error!  Tileset: `" << File::get_basename(name, true) <<
	"' is malformed.  Skipping.\n";
        return NULL;
    }
  if (d_tilesets.find(tileset->getBaseName()) != d_tilesets.end())
    {
      Tileset *t = (*d_tilesets.find(tileset->getBaseName())).second;
      cerr << "Error!  tileset: `" << tileset->getConfigurationFile() << 
	"' shares a duplicate tileset subdir `" << t->getBaseName() << "' with `" << t->getConfigurationFile() 
	<< "'.  Skipping." << endl;
      delete tileset;
      return NULL;
    }
    
  if (d_tilesetids.find(tileset->getId()) != d_tilesetids.end())
    {
      Tileset *t = (*d_tilesetids.find(tileset->getId())).second;
      cerr << "Error!  tileset: `" << tileset->getConfigurationFile() << 
	"' shares a duplicate tileset id with `" << 
	t->getConfigurationFile() << "'.  Skipping." << endl;
      delete tileset;
      return NULL;
    }
  return tileset;
}

void Tilesetlist::add(Tileset *tileset, std::string file)
{
  std::string basename = File::get_basename(file);
  push_back(tileset); 
  tileset->setBaseName(basename);
  d_dirs[String::ucompose("%1 %2", tileset->getName(), tileset->getTileSize())] = basename;
  d_tilesets[basename] = tileset;
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
      add(tileset, *i);
    }
}

int Tilesetlist::getNextAvailableId(int after)
{
  bool unsupported_version = false;
  std::list<guint32> ids;
  std::list<std::string> tilesets = Tileset::scanSystemCollection();
  //there might be IDs in invalid tilesets.
  for (std::list<std::string>::const_iterator i = tilesets.begin(); 
       i != tilesets.end(); i++)
    {
      Tileset *tileset = Tileset::create(*i, unsupported_version);
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
      Tileset *tileset = Tileset::create(*i, unsupported_version);
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

void Tilesetlist::instantiateImages(bool &broken)
{
  broken = false;
  for (iterator it = begin(); it != end(); it++)
    {
      if (!broken)
        (*it)->instantiateImages(broken);
    }
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

Tileset *Tilesetlist::import(Tar_Helper *t, std::string f, bool &broken)
{
  bool unsupported_version = false;
  std::string filename = t->getFile(f, broken);
  if (broken)
    return NULL;
  Tileset *tileset = Tileset::create(filename, unsupported_version);
  assert (tileset != NULL);
  tileset->setBaseName(File::get_basename(f));

  std::string basename = "";
  guint32 id = 0;
  addToPersonalCollection(tileset, basename, id);

  return tileset;
}

std::string Tilesetlist::findFreeBaseName(std::string basename, guint32 max, guint32 &num) const
{
  std::string new_basename;
  for (unsigned int count = 1; count < max; count++)
    {
      new_basename = String::ucompose("%1%2", basename, count);
      if (getTileset(new_basename) == NULL)
        {
          num = count;
          break;
        }
      else
        new_basename = "";
    }
  return new_basename;
}

bool Tilesetlist::addToPersonalCollection(Tileset *tileset, std::string &new_basename, guint32 &new_id)
{
  //do we already have this one?
      
  if (getTileset(tileset->getBaseName()) == getTileset(tileset->getId()) 
      && getTileset(tileset->getBaseName()) != NULL)
    {
      tileset->setDirectory(getTileset(tileset->getId())->getDirectory());
      return true;
    }

  //if the basename conflicts with any other basename, then change it.
  if (getTileset(tileset->getBaseName()) != NULL)
    {
      if (new_basename != "" && getTileset(new_basename) == NULL)
        ;
      else
        {
          guint32 num = 0;
          std::string new_basename = findFreeBaseName(tileset->getBaseName(), 100, num);
          if (new_basename == "")
            return false;
        }
    }
  else if (new_basename == "")
    new_basename = tileset->getBaseName();

  //if the id conflicts with any other id, then change it
  if (getTileset(tileset->getId()) != NULL)
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
  std::string file = File::getUserTilesetDir() + new_basename + Tileset::file_extension;

  tileset->save(file, Tileset::file_extension);

  if (new_basename != tileset->getBaseName())
    tileset->setBaseName(new_basename);
  tileset->setDirectory(File::get_dirname(file));
  add (tileset, file);
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

guint32 Tilesetlist::getTilesetId(std::string basename) const
{
  Tileset *ts = getTileset(basename);
  if (ts)
    return ts->getId();
  else
    return 0;
}

SmallTile *Tilesetlist::getSmallTile(std::string basename, Tile::Type type) const
{
  Tileset *ts = getTileset(basename);
  if (!ts)
    return NULL;
  int idx = ts->getIndex(type);
  return (*ts)[idx]->getSmallTile();
}

Gdk::Color Tilesetlist::getColor(std::string basename, Tile::Type type) const
{
  SmallTile *smalltile = getSmallTile(basename, type);
  if (!smalltile)
    return Gdk::Color("black");
  else
    return smalltile->getColor();
}
