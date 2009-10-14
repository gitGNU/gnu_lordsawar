//  Copyright (C) 2007, 2008 Ben Asselstine
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
  TilesetLoader loader(name, from_private_collection);
  if (loader.tileset == NULL)
    return false;
  if (loader.tileset->validate() == false)
    return false;
  push_back(loader.tileset); 
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
	}
      else
	{
	  //we failed validation
	  iterator it = end();
	  it--;
	  fprintf (stderr, "tileset `%s' fails validation. skipping.\n",
		   (*it)->getName().c_str());
	  delete *it;
	  erase (it);
	}
    }
}

