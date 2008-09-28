// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2007, 2008 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>

#include "tileset.h"

#include "File.h"
#include "SmallTile.h"
#include "xmlhelper.h"

using namespace std;

#include <iostream>
//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

#define DEFAULT_TILE_SIZE 40
	
std::string Tileset::d_tag = "tileset";

Tileset::Tileset(std::string name)
	: d_name(name), d_tileSize(DEFAULT_TILE_SIZE), d_dir("")
{
  d_info = "";
  d_large_selector = "misc/selector.png";
  d_small_selector = "misc/small_selector.png";
}

Tileset::Tileset(XML_Helper *helper)
{
    helper->getData(d_name, "name"); 
    helper->getData(d_info, "info");
    helper->getData(d_tileSize, "tilesize");
    helper->getData(d_large_selector, "large_selector");
    helper->getData(d_small_selector, "small_selector");
    helper->registerTag(Tile::d_tag, sigc::mem_fun((*this), &Tileset::loadTile));
    helper->registerTag(SmallTile::d_tag, sigc::mem_fun((*this), &Tileset::loadTile));
    helper->registerTag(TileStyle::d_tag, sigc::mem_fun((*this), &Tileset::loadTile));
    helper->registerTag(TileStyleSet::d_tag, sigc::mem_fun((*this), &Tileset::loadTile));
}

Tileset::~Tileset()
{
    for (unsigned int i=0; i < size(); i++)
        delete (*this)[i];
}

Uint32 Tileset::getIndex(Tile::Type type) const
{
    for (Uint32 i = 0; i < size(); i++)
        if (type == (*this)[i]->getType())
            return i;

    // catch errors?
    return 0;
}

bool Tileset::loadTile(string tag, XML_Helper* helper)
{
    debug("loadTile()")

    if (tag == Tile::d_tag)
      {
	// create a new tile with the information we got
	Tile* tile = new Tile(helper);
	this->push_back(tile);

	return true;
      }

    if (tag == SmallTile::d_tag)
      {
	Tile *tile = this->back();
	SmallTile* smalltile = new SmallTile(helper);
	tile->setSmallTile(smalltile);
	return true;
      }

    if (tag == TileStyle::d_tag)
      {
	Tile *tile = this->back();
	TileStyleSet *tilestyleset = tile->back();
	// create a new tile style with the information we got
	// put it on the latest tilestyleset
	TileStyle* tilestyle = new TileStyle(helper);
	tilestyleset->push_back(tilestyle);
	d_tilestyles[tilestyle->getId()] = tilestyle;

	return true;
      }

    if (tag == TileStyleSet::d_tag)
      {
	Tile *tile = this->back();
	// create a new tile style set with the information we got
	// put it on the latest tile
	TileStyleSet* tilestyleset = new TileStyleSet(helper);
	tilestyleset->setSubDir(getSubDir());
	tile->push_back(tilestyleset);
	return true;
      }

    return false;
}

TileStyle *Tileset::getRandomTileStyle(Uint32 index, TileStyle::Type style)
{
  Tile *tile = (*this)[index];
  if (tile)
    return tile->getRandomTileStyle (style);
  else
    return NULL;
}

bool Tileset::save(XML_Helper *helper)
{
  bool retval = true;

  retval &= helper->openTag(d_tag);
  retval &= helper->saveData("name", d_name);
  retval &= helper->saveData("info", d_info);
  retval &= helper->saveData("tilesize", d_tileSize);
  retval &= helper->saveData("large_selector", d_large_selector);
  retval &= helper->saveData("small_selector", d_small_selector);
  for (Tileset::iterator i = begin(); i != end(); ++i)
    retval &= (*i)->save(helper);
  retval &= helper->closeTag();

  return retval;
}
	
Tile *Tileset::lookupTileByName(std::string name)
{
  for (Tileset::iterator i = begin(); i != end(); ++i)
    if ((*i)->getName() == name)
      return *i;
  return NULL;
}
	
int Tileset::getFreeTileStyleId()
{
  int ids[65535];
  memset (ids, 0, sizeof (ids));
  for (Tileset::iterator i = begin(); i != end(); ++i)
    {
      for (std::list<TileStyleSet*>::iterator j = (*i)->begin(); j != (*i)->end(); j++)
	{
	  for (std::vector<TileStyle*>::iterator k = (*j)->begin(); k != (*j)->end(); k++)
	    {
	      ids[(*k)->getId()] = 1;
	    }
	}
    }
  //these ids range from 0 to 65535.
  for (unsigned int i = 0; i <= 65535; i++)
    {
      if (ids[i] == 0)
	return i;
    }
  return -1;
}

int Tileset::getLargestTileStyleId()
{
  int largest = 0;
  for (Tileset::iterator i = begin(); i != end(); ++i)
    {
      for (std::list<TileStyleSet*>::iterator j = (*i)->begin(); j != (*i)->end(); j++)
	{
	  for (std::vector<TileStyle*>::iterator k = (*j)->begin(); k != (*j)->end(); k++)
	    {
	      if ((*k)->getId() > largest)
		largest = (*k)->getId();
	    }
	}
    }
  return largest;
}

void Tileset::setSubDir(std::string dir)
{
  d_dir = dir;
  for (Tileset::iterator i = begin(); i != end(); ++i)
    for (Tile::iterator j = (*i)->begin(); j != (*i)->end(); j++)
      (*j)->setSubDir(dir);
}

Uint32 Tileset::getDefaultTileSize()
{
  return DEFAULT_TILE_SIZE;
}

bool Tileset::validate()
{
  for (Tileset::iterator i = begin(); i != end(); ++i)
    {
      if ((*i)->validate() == false)
	return false;
    }
  return true;
}
// End of file
