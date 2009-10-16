//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
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
{
    // load all tilesets
    loadTilesets(File::scanTilesets(), false);
    loadTilesets(File::scanUserTilesets(), true);
}

Tilesetlist::~Tilesetlist()
{
  for (iterator it = begin(); it != end(); it++)
    delete (*it);
}

void Tilesetlist::getSizes(std::list<guint32> &sizes)
{
  for (iterator i = begin(); i != end(); i++)
    {
      if (find (sizes.begin(), sizes.end(), (*i)->getTileSize()) == sizes.end())
	sizes.push_back((*i)->getTileSize());
    }
}

std::list<std::string> Tilesetlist::getNames()
{
  std::list<std::string> names;
  for (iterator it = begin(); it != end(); it++)
    names.push_back((*it)->getName());
  return names;
}

std::list<std::string> Tilesetlist::getNames(guint32 tilesize)
{
  std::list<std::string> names;
  for (iterator it = begin(); it != end(); it++)
    if ((*it)->getTileSize() == tilesize)
      names.push_back((*it)->getName());
  return names;
}

bool Tilesetlist::load(std::string tag, XML_Helper *helper)
{
  if (tag == Tileset::d_tag)
    {
      Tileset *tileset = new Tileset(helper);
	push_back(tileset); 
    }
  return true;
}

bool Tilesetlist::loadTileset(std::string name, bool from_private_collection)
{
  debug("Loading tileset " <<name);
  Tileset *tileset = Tileset::create(name, from_private_collection);
  if (tileset == NULL)
    return false;
  if (tileset->validate() == false)
    {
      cerr<< "Error!  Tileset: `" << File::getTileset(tileset) <<
	"' fails validation.  Skipping.\n",
      delete tileset;
      return false;
    }
  if (d_tilesetids.find(tileset->getId()) != d_tilesetids.end())
    {
      Tileset *t = (*d_tilesetids.find(tileset->getId())).second;
      cerr << "Error!  tileset: `" << tileset->getName() << 
	"' shares a duplicate tileset id with `" << File::getTileset(t) << 
	"'.  Skipping." << endl;
      delete tileset;
      return false;
    }
  push_back(tileset); 
  return true;
}

std::string Tilesetlist::getTilesetDir(std::string name, guint32 tilesize)
{
  return d_dirs[String::ucompose("%1 %2", name, tilesize)];
}

void Tilesetlist::loadTilesets(std::list<std::string> tilesets, bool priv)
{
  for (std::list<std::string>::const_iterator i = tilesets.begin(); 
       i != tilesets.end(); i++)
    {
      if (loadTileset(*i, priv) == true)
	{
	  iterator it = end();
	  it--;
	  (*it)->setSubDir(*i);
	  d_dirs[String::ucompose("%1 %2", (*it)->getName(), (*it)->getTileSize())] = *i;
	  d_tilesets[*i] = *it;
	  d_tilesetids[(*it)->getId()] = *it;
	}
    }
}

int Tilesetlist::getNextAvailableId(int after)
{
  std::list<guint32> ids;
  std::list<std::string> tilesets = File::scanTilesets();
  //there might be IDs in invalid tilesets.
  for (std::list<std::string>::const_iterator i = tilesets.begin(); 
       i != tilesets.end(); i++)
    {
      Tileset *tileset = Tileset::create(*i, false);
      if (tileset != NULL)
	{
	  ids.push_back(tileset->getId());
	  delete tileset;
	}
    }
  tilesets = File::scanUserTilesets();
  for (std::list<std::string>::const_iterator i = tilesets.begin(); 
       i != tilesets.end(); i++)
    {
      Tileset *tileset = Tileset::create(*i, true);
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
