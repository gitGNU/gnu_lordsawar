// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>
#include <string.h>

#include "tileset.h"

#include "defs.h"
#include "File.h"
#include "SmallTile.h"
#include "xmlhelper.h"

using namespace std;

#include <iostream>
//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

#define DEFAULT_TILE_SIZE 40
	
std::string Tileset::d_tag = "tileset";
std::string Tileset::d_road_smallmap_tag = "road_smallmap";

Tileset::Tileset(guint32 id, std::string name)
	: d_name(name), d_id(id), d_tileSize(DEFAULT_TILE_SIZE), d_dir(""), 
	private_collection(true)
{
  d_info = "";
  d_large_selector = "";
  d_small_selector = "";
  d_fog = "";
  d_roads = "";
  d_bridges = "";
  d_flags = "";
  d_road_color.set_rgb_p(0,0,0);
  for (unsigned int i = 0; i < ROAD_TYPES; i++)
    roadpic[i] = NULL;
  for (unsigned int i = 0; i < BRIDGE_TYPES; i++)
    bridgepic[i] = NULL;
  for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
    flagpic[i] = NULL;
  for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
    flagmask[i] = NULL;
  number_of_selector_frames = 0;
  selector.clear();
  selectormask.clear();
  number_of_small_selector_frames = 0;
  smallselector.clear();
  smallselectormask.clear();
  explosion = NULL;
  for (unsigned int i = 0; i < FOG_TYPES; i++)
    fogpic[i] = NULL;
}

Tileset::Tileset(XML_Helper *helper, bool from_private_collection)
{
    private_collection = from_private_collection;
    helper->getData(d_id, "id"); 
    helper->getData(d_name, "name"); 
    helper->getData(d_info, "info");
    helper->getData(d_tileSize, "tilesize");
    helper->getData(d_large_selector, "large_selector");
    helper->getData(d_small_selector, "small_selector");
    helper->getData(d_explosion, "explosion");
    helper->getData(d_roads, "roads");
    helper->getData(d_bridges, "bridges");
    helper->getData(d_fog, "fog");
    helper->getData(d_flags, "flags");
    helper->registerTag(Tile::d_tag, sigc::mem_fun((*this), &Tileset::loadTile));
    helper->registerTag(Tileset::d_road_smallmap_tag, sigc::mem_fun((*this), &Tileset::loadTile));
    helper->registerTag(SmallTile::d_tag, sigc::mem_fun((*this), &Tileset::loadTile));
    helper->registerTag(TileStyle::d_tag, sigc::mem_fun((*this), &Tileset::loadTile));
    helper->registerTag(TileStyleSet::d_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  for (unsigned int i = 0; i < ROAD_TYPES; i++)
    roadpic[i] = NULL;
  for (unsigned int i = 0; i < BRIDGE_TYPES; i++)
    bridgepic[i] = NULL;
  for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
    flagpic[i] = NULL;
  for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
    flagmask[i] = NULL;
  number_of_selector_frames = 0;
  selector.clear();
  selectormask.clear();
  number_of_small_selector_frames = 0;
  smallselector.clear();
  smallselectormask.clear();
  explosion = NULL;
  for (unsigned int i = 0; i < FOG_TYPES; i++)
    fogpic[i] = NULL;
}

Tileset::~Tileset()
{
    for (unsigned int i=0; i < size(); i++)
        delete (*this)[i];
    if (explosion != NULL)
      delete explosion;
    for (unsigned int i = 0; i < ROAD_TYPES; i++)
      if (roadpic[i] != NULL)
	delete roadpic[i];
    for (unsigned int i = 0; i < BRIDGE_TYPES; i++)
      if (bridgepic[i] != NULL)
	delete bridgepic[i];
    for (unsigned int i = 0; i < FOG_TYPES; i++)
      if (fogpic[i] != NULL)
	delete fogpic[i];
    for (unsigned int i = 0; i < selector.size(); i++)
      if (selector[i] != NULL)
	delete selector[i];
    for (unsigned int i = 0; i < selectormask.size();i++)
      if (selectormask[i] != NULL)
	delete selectormask[i];
    for (unsigned int i = 0; i < smallselector.size(); i++)
      if (smallselector[i] != NULL)
	delete smallselector[i];
    for (unsigned int i = 0; i < smallselectormask.size(); i++)
      if (smallselectormask[i] != NULL)
	delete smallselectormask[i];
    for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
      if (flagpic[i] != NULL)
	delete flagpic[i];
    for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
      if (flagmask[i] != NULL)
	delete flagmask[i];
}

guint32 Tileset::getIndex(Tile::Type type) const
{
    for (guint32 i = 0; i < size(); i++)
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

    if (tag == Tileset::d_road_smallmap_tag)
      {
	guint32 r, g, b;
	helper->getData(r, "red");
	helper->getData(g, "green");
	helper->getData(b, "blue");
	d_road_color.set_rgb_p((float)r/255.0,(float)g/255.0, (float)b/255.0);
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

TileStyle *Tileset::getRandomTileStyle(guint32 index, TileStyle::Type style)
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
  retval &= helper->saveData("explosion", d_explosion);
  retval &= helper->saveData("roads", d_roads);
  retval &= helper->saveData("bridges", d_bridges);
  retval &= helper->saveData("fog", d_fog);
  retval &= helper->saveData("flags", d_flags);
  retval &= helper->openTag(d_road_smallmap_tag);
  retval &= helper->saveData("red", int(d_road_color.get_red_p() *255));
  retval &= helper->saveData("green", int(d_road_color.get_green_p()*255));
  retval &= helper->saveData("blue", int(d_road_color.get_blue_p()*255));
  retval &= helper->closeTag();
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
  unsigned int largest = 0;
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

guint32 Tileset::getDefaultTileSize()
{
  return DEFAULT_TILE_SIZE;
}

bool Tileset::validate()
{
  if (size() == 0)
    return false;
  for (Tileset::iterator i = begin(); i != end(); i++)
    {
      if ((*i)->validate() == false)
	{
	  fprintf (stderr, "`%s' fails validation\n", 
		   (*i)->getName().c_str());
	  return false;
	}
    }
  return true;
}
// End of file
