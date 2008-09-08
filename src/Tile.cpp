// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2002, 2003, 2004, 2005 Ulf Lorenz
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

#include "Tile.h"
#include <iostream>

using namespace std;

Tile::Tile()
{
  d_type = Tile::GRASS;
  d_pattern = Tile::SOLID;
  d_moves = 0;
  d_color.r = 80;
  d_color.g = 172;
  d_color.b = 28;
}

Tile::Tile(XML_Helper* helper)
{
    helper->getData(d_name, "name");
    helper->getData(d_moves, "moves");
    std::string type_str;
    helper->getData(type_str, "type");
    d_type = tileTypeFromString(type_str);
}

bool Tile::save(XML_Helper *helper)
{
  bool retval = true;

  retval &= helper->openTag("tile");
  retval &= helper->saveData("name", d_name);
  retval &= helper->saveData("moves", d_moves);
  std::string type_str = tileTypeToString(Tile::Type(d_type));
  retval &= helper->saveData("type", type_str);
  retval &= helper->openTag("smallmap");
  switch (d_pattern)
    {
      //patterns with a single colour
    case SOLID:
      retval &= helper->saveData("red", d_color.r);
      retval &= helper->saveData("green", d_color.g);
      retval &= helper->saveData("blue", d_color.b);
      break;
      //patterns with two colours
    case STIPPLED: case SUNKEN:
      retval &= helper->saveData("red", d_color.r);
      retval &= helper->saveData("green", d_color.g);
      retval &= helper->saveData("blue", d_color.b);
      retval &= helper->saveData("2nd_red", d_second_color.r);
      retval &= helper->saveData("2nd_green", d_second_color.g);
      retval &= helper->saveData("2nd_blue", d_second_color.b);
      break;
      //patterns with three colours
    case RANDOMIZED: case TABLECLOTH: case DIAGONAL: case CROSSHATCH:
      retval &= helper->saveData("red", d_color.r);
      retval &= helper->saveData("green", d_color.g);
      retval &= helper->saveData("blue", d_color.b);
      retval &= helper->saveData("2nd_red", d_second_color.r);
      retval &= helper->saveData("2nd_green", d_second_color.g);
      retval &= helper->saveData("2nd_blue", d_second_color.b);
      retval &= helper->saveData("3rd_red", d_third_color.r);
      retval &= helper->saveData("3rd_green", d_third_color.g);
      retval &= helper->saveData("3rd_blue", d_third_color.b);
      break;
    }
  retval &= helper->closeTag();
  for (Tile::iterator i = begin(); i != end(); ++i)
    retval &= (*i)->save(helper);
  retval &= helper->closeTag();

  return retval;
}
    
Tile::~Tile()
{
  for (iterator it = begin(); it != end(); it++)
      delete *it;
}

void Tile::setTypeByIndex(int idx)
{
  switch (idx)
    {
    case 0: setType(GRASS); break;
    case 1: setType(WATER); break;
    case 2: setType(FOREST); break;
    case 3: setType(HILLS); break;
    case 4: setType(MOUNTAIN); break;
    case 5: setType(SWAMP); break;
    }
}
int Tile::getTypeIndexForType(Tile::Type type)
{
  switch (type)
    {
    case GRASS: return 0; break;
    case WATER: return 1; break;
    case FOREST: return 2; break;
    case HILLS: return 3; break;
    case MOUNTAIN: return 4; break;
    case SWAMP: return 5; break;
    }
  return 0;
}
TileStyle *Tile::getRandomTileStyle (TileStyle::Type style)
{
  std::vector<TileStyle*> tilestyles;
  for (iterator it = begin(); it != end(); ++it)
    {
      TileStyleSet *tilestyleset = *it;
      for (Uint32 k = 0; k < tilestyleset->size(); k++)
	{
	  TileStyle *tilestyle = (*tilestyleset)[k];
	  if (tilestyle->getType() == style)
	    tilestyles.push_back(tilestyle);
	}
    }

  if (tilestyles.empty() == true)
    return NULL;
  return tilestyles[rand() % tilestyles.size()];
}

std::string Tile::tileTypeToString(const Tile::Type type)
{
  switch (type)
    {
    case Tile::GRASS:
      return "Tile::GRASS";
    case Tile::WATER:
      return "Tile::WATER";
    case Tile::FOREST:
      return "Tile::FOREST";
    case Tile::HILLS:
      return "Tile::HILLS";
    case Tile::MOUNTAIN:
      return "Tile::MOUNTAIN";
    case Tile::SWAMP:
      return "Tile::SWAMP";
    }
  return "Tile::GRASS";
}

Tile::Type Tile::tileTypeFromString(const std::string str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Tile::Type(atoi(str.c_str()));
  if (str == "Tile::GRASS")
    return Tile::GRASS;
  else if (str == "Tile::WATER")
    return Tile::WATER;
  else if (str == "Tile::FOREST")
    return Tile::FOREST;
  else if (str == "Tile::HILLS")
    return Tile::HILLS;
  else if (str == "Tile::MOUNTAIN")
    return Tile::MOUNTAIN;
  else if (str == "Tile::SWAMP")
    return Tile::SWAMP;
  return Tile::GRASS;
}

// End of file
