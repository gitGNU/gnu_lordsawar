//  Copyright (C) 2007, 2008, 2009, 2010, 2011, 2014 Ben Asselstine
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

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
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
    loadTilesets(Set::scanCollection(Tileset::file_extension));
    loadTilesets(Set::scanCollection(Tileset::file_extension, false));
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

std::list<Glib::ustring> Tilesetlist::getValidNames(guint32 tilesize) const
{
  std::list<Glib::ustring> names;
  for (const_iterator it = begin(); it != end(); it++)
    if ((*it)->getTileSize() == tilesize && (*it)->validate() == true)
      names.push_back((*it)->getName());
  names.sort(case_insensitive);
  return names;
}

Tileset *Tilesetlist::loadTileset(Glib::ustring name)
{
  debug("Loading tileset " <<File::get_basename(name));
  bool unsupported_version = false;
  Tileset *tileset = Tileset::create(name, unsupported_version);
  if (tileset == NULL)
    {
      std::cerr << String::ucompose(_("Error!  Tileset: `%1' is malformed.  Skipping."), File::get_basename(name, true)) << std::endl;
        return NULL;
    }
  if (d_tilesets.find(tileset->getBaseName()) != d_tilesets.end())
    {
      Tileset *t = (*d_tilesets.find(tileset->getBaseName())).second;
      std::cerr << String::ucompose(_("Error!  tileset: `%1' shares a duplicate tileset subdir `%2' with `%3'. Skipping."), tileset->getConfigurationFile(), t->getBaseName(), t->getConfigurationFile()) << std::endl;
      delete tileset;
      return NULL;
    }
    
  if (d_tilesetids.find(tileset->getId()) != d_tilesetids.end())
    {
      Tileset *t = (*d_tilesetids.find(tileset->getId())).second;
      std::cerr << String::ucompose(_("Error!  tileset: `%1' shares a duplicate tileset id with `%2'.  Skipping."), tileset->getConfigurationFile(), t->getConfigurationFile()) << std::endl;
      delete tileset;
      return NULL;
    }
  return tileset;
}

void Tilesetlist::add(Tileset *tileset, Glib::ustring file)
{
  Glib::ustring basename = File::get_basename(file);
  push_back(tileset); 
  tileset->setBaseName(basename);
  d_dirs[String::ucompose("%1 %2", tileset->getName(), tileset->getTileSize())] = basename;
  d_tilesets[basename] = tileset;
  d_tilesetids[tileset->getId()] = tileset;
}

Glib::ustring Tilesetlist::getTilesetDir(Glib::ustring name, guint32 tilesize) const
{
  DirMap::const_iterator it = d_dirs.find(String::ucompose("%1 %2", name, tilesize));
  if (it == d_dirs.end())
    return "";
  else
    return (*it).second;
}

void Tilesetlist::loadTilesets(std::list<Glib::ustring> tilesets)
{
  for (std::list<Glib::ustring>::const_iterator i = tilesets.begin(); 
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
  std::list<Glib::ustring> tilesets = Set::scanCollection(Tileset::file_extension);
  //there might be IDs in invalid tilesets.
  for (std::list<Glib::ustring>::const_iterator i = tilesets.begin(); 
       i != tilesets.end(); i++)
    {
      Tileset *tileset = Tileset::create(*i, unsupported_version);
      if (tileset != NULL)
	{
	  ids.push_back(tileset->getId());
	  delete tileset;
	}
    }
  tilesets = Set::scanCollection(Tileset::file_extension, false);
  for (std::list<Glib::ustring>::const_iterator i = tilesets.begin(); 
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
	
Tileset *Tilesetlist::getTileset(Glib::ustring dir) const
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

Tileset *Tilesetlist::import(Tar_Helper *t, Glib::ustring f, bool &broken)
{
  bool unsupported_version = false;
  Glib::ustring filename = t->getFile(f, broken);
  if (broken)
    return NULL;
  Tileset *tileset = Tileset::create(filename, unsupported_version);
  assert (tileset != NULL);
  tileset->setBaseName(File::get_basename(f));

  Glib::ustring basename = "";
  guint32 id = 0;
  addToPersonalCollection(tileset, basename, id);

  return tileset;
}

Glib::ustring Tilesetlist::findFreeBaseName(Glib::ustring basename, guint32 max, guint32 &num) const
{
  Glib::ustring new_basename;
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

bool Tilesetlist::addToPersonalCollection(Tileset *tileset, Glib::ustring &new_basename, guint32 &new_id)
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
          Glib::ustring new_basename = findFreeBaseName(tileset->getBaseName(), 100, num);
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
  Glib::ustring file = File::getSetDir(Tileset::file_extension, false) + new_basename + Tileset::file_extension;

  tileset->save(file, Tileset::file_extension);

  if (new_basename != tileset->getBaseName())
    tileset->setBaseName(new_basename);
  tileset->setDirectory(File::get_dirname(file));
  add (tileset, file);
  return true;
}

bool Tilesetlist::contains(Glib::ustring name) const
{
  for (const_iterator it = begin(); it != end(); it++)
    if ((*it)->getName() == name)
      return true;

  return false;
}

guint32 Tilesetlist::getTilesetId(Glib::ustring basename) const
{
  Tileset *ts = getTileset(basename);
  if (ts)
    return ts->getId();
  else
    return 0;
}

SmallTile *Tilesetlist::getSmallTile(Glib::ustring basename, Tile::Type type) const
{
  Tileset *ts = getTileset(basename);
  if (!ts)
    return NULL;
  int idx = ts->getIndex(type);
  return (*ts)[idx]->getSmallTile();
}

Gdk::RGBA Tilesetlist::getColor(Glib::ustring basename, Tile::Type type) const
{
  SmallTile *smalltile = getSmallTile(basename, type);
  if (!smalltile)
    return Gdk::RGBA("black");
  else
    return smalltile->getColor();
}

bool Tilesetlist::reload(guint32 id) 
{
  Tileset *tileset = getTileset(id);
  if (!tileset)
    return false;
  bool broken = false;
  tileset->reload(broken);
  if (broken)
    return false;
  Glib::ustring basename = tileset->getBaseName();
  d_dirs[String::ucompose("%1 %2", tileset->getName(), tileset->getTileSize())] = basename;
  d_tilesets[basename] = tileset;
  d_tilesetids[tileset->getId()] = tileset;
  return true;
}
