//  Copyright (C) 2007, 2008 Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
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
#include <SDL_image.h>
#include <SDL.h>
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
    std::list<std::string> tilesets = File::scanTilesets();

    for (std::list<std::string>::const_iterator i = tilesets.begin(); 
	 i != tilesets.end(); i++)
      {
        loadTileset(*i);
	iterator it = end();
	it--;
	(*it)->setSubDir(*i);
	d_dirs[String::ucompose("%1 %2", (*it)->getName(), (*it)->getTileSize())] = *i;
	d_tilesets[*i] = *it;
      }
}

Tilesetlist::~Tilesetlist()
{
  for (iterator it = begin(); it != end(); it++)
    delete (*it);
}

void Tilesetlist::getSizes(std::list<Uint32> &sizes)
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

std::list<std::string> Tilesetlist::getNames(Uint32 tilesize)
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
      if (tileset->validate() == true)
	push_back(tileset); 
      else
	{
	  fprintf (stderr, "tileset `%s' fails validation.\n", 
		   tileset->getName().c_str());
	  delete tileset;
	}
    }
  return true;
}

bool Tilesetlist::loadTileset(std::string name)
{
  debug("Loading tileset " <<name);

  XML_Helper helper(File::getTileset(name), ios::in, false);

  helper.registerTag(Tileset::d_tag, sigc::mem_fun((*this), &Tilesetlist::load));

  if (!helper.parse())
    {
      std::cerr <<_("Error, while loading a tileset. Tileset Name: ");
      std::cerr <<name <<std::endl <<std::flush;
      exit(-1);
    }

  return true;
}

std::string Tilesetlist::getTilesetDir(std::string name, Uint32 tilesize)
{
  return d_dirs[String::ucompose("%1 %2", name, tilesize)];
}
