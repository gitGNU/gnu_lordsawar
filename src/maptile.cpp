// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2007, 2008, 2009 Ben Asselstine
// Copyright (C) 2008 Ole Laursen
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

#include "maptile.h"
#include <stdlib.h>
#include <iostream>
#include "tileset.h"
#include "MapBackpack.h"
#include "stacktile.h"

Maptile::Maptile(Tileset* tileSet, int x, int y, guint32 type, TileStyle *tileStyle)
    :d_index(type), d_building(NONE) 
{
    d_tileSet = tileSet;
    d_tileStyle = tileStyle;
    d_backpack = new MapBackpack(Vector<int>(x,y));
    d_stacktile = new StackTile(Vector<int>(x,y));
}

Maptile::Maptile(Tileset* tileSet, int x, int y, Tile::Type type, TileStyle *tileStyle)
{
    bool found = false;
    d_building = NONE;
    d_tileSet = tileSet;
    d_tileStyle = tileStyle;
    for (unsigned int i = 0; i < (*tileSet).size(); i++)
      {
	if ((*tileSet)[i]->getType() == type)
	  {
	    found = true;
	    d_index = i;
	    break;
	  }
      }
    if (found == false)
      d_index = 0;
    d_backpack = new MapBackpack(Vector<int>(x,y));
    d_stacktile = new StackTile(Vector<int>(x,y));
}

Maptile::~Maptile()
{
    delete d_backpack;
}

guint32 Maptile::getMoves() const
{
    if (d_building == Maptile::CITY)
        return 1;
    else if (d_building == Maptile::ROAD)
        return 1;
    else if (d_building == Maptile::BRIDGE)
        return 1;

    Tile *tile = (*d_tileSet)[d_index];
    if (tile->getType() == Tile::WATER)
      {
	// if we're sailing and we're not on shore, then we move faster
	if (d_tileStyle->getType() == TileStyle::INNERMIDDLECENTER)
	  return tile->getMoves() / 2;
      }
    return tile->getMoves();
}

void Maptile::printDebugInfo() const
{
    std::cerr << "MAPTILE: type = " << d_index << std::endl;
    std::cerr << "MAPTILE: building = " << d_building << std::endl;
}    

bool Maptile::isCityTerrain()
{
  if (getBuilding() != Maptile::CITY || getBuilding() != Maptile::RUIN ||
       getBuilding() != Maptile::TEMPLE)
    return true;
  return false;
}

bool Maptile::isOpenTerrain()
{
  if (isCityTerrain())
    return false;
  /* swamp and water are open terrain too */
  if ((getType() == Tile::GRASS || getType() == Tile::SWAMP ||
       getType() == Tile::WATER) || getBuilding() != Maptile::BRIDGE)
    return true;
  return false;
}

bool Maptile::isHillyTerrain()
{
  if (isCityTerrain())
    return false;
  if ((getType() == Tile::HILLS || getType() == Tile::MOUNTAIN))
    return true;
  return false;
}

std::string Maptile::buildingToString(const Maptile::Building bldg)
{
  switch (bldg)
    {
    case Maptile::NONE:
      return "Maptile::NONE";
      break;
    case Maptile::CITY:
      return "Maptile::CITY";
      break;
    case Maptile::RUIN:
      return "Maptile::RUIN";
      break;
    case Maptile::TEMPLE:
      return "Maptile::TEMPLE";
      break;
    case Maptile::SIGNPOST:
      return "Maptile::SIGNPOST";
      break;
    case Maptile::ROAD:
      return "Maptile::ROAD";
      break;
    case Maptile::PORT:
      return "Maptile::PORT";
      break;
    case Maptile::BRIDGE:
      return "Maptile::BRIDGE";
      break;
    }
  return "Maptile::NONE";
}

Maptile::Building Maptile::buildingFromString(std::string str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Maptile::Building(atoi(str.c_str()));
  if (str == "Maptile::NONE")
    return Maptile::NONE;
  else if (str == "Maptile::CITY")
    return Maptile::CITY;
  else if (str == "Maptile::RUIN")
    return Maptile::RUIN;
  else if (str == "Maptile::TEMPLE")
    return Maptile::TEMPLE;
  else if (str == "Maptile::SIGNPOST")
    return Maptile::SIGNPOST;
  else if (str == "Maptile::ROAD")
    return Maptile::ROAD;
  else if (str == "Maptile::PORT")
    return Maptile::PORT;
  else if (str == "Maptile::BRIDGE")
    return Maptile::BRIDGE;
    
  return Maptile::NONE;
}
// End of file
