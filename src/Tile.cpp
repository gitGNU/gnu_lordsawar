// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2002, 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2007, 2008, 2010 Ben Asselstine
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

#include "Tile.h"
#include "SmallTile.h"
#include <iostream>
#include <algorithm>
#include "File.h"
#include "tileset.h"
#include "tarhelper.h"

std::string Tile::d_tag = "tile";

using namespace std;


Tile::Tile()
{
  d_type = Tile::GRASS;
  d_moves = 0;
  d_smalltile = new SmallTile();
}

Tile::Tile(XML_Helper* helper)
{
    helper->getData(d_name, "name");
    helper->getData(d_moves, "moves");
    std::string type_str;
    helper->getData(type_str, "type");
    d_type = tileTypeFromString(type_str);
}

bool Tile::save(XML_Helper *helper) const
{
  bool retval = true;

  retval &= helper->openTag(d_tag);
  retval &= helper->saveData("name", d_name);
  retval &= helper->saveData("moves", d_moves);
  std::string type_str = tileTypeToString(Tile::Type(d_type));
  retval &= helper->saveData("type", type_str);
  retval &= d_smalltile->save(helper);
  for (Tile::const_iterator i = begin(); i != end(); ++i)
    retval &= (*i)->save(helper);
  retval &= helper->closeTag();

  return retval;
}

Tile::~Tile()
{
  for (iterator it = begin(); it != end(); it++)
      delete *it;
  if (d_smalltile)
    delete d_smalltile;
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
    case 6: setType(VOID); break;
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
    case VOID: return 6; break;
    }
  return 0;
}

TileStyle *Tile::getRandomTileStyle (TileStyle::Type style) const
{
  std::vector<TileStyle*> tilestyles;
  for (const_iterator it = begin(); it != end(); ++it)
    {
      TileStyleSet *tilestyleset = *it;
      for (guint32 k = 0; k < tilestyleset->size(); k++)
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
    case Tile::VOID:
      return "Tile::VOID";
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
  else if (str == "Tile::VOID")
    return Tile::VOID;
  return Tile::GRASS;
}

      
bool Tile::validateGrass(std::list<TileStyle::Type> types) const
{
  //grass tiles only have lone styles and other styles.
  for (std::list<TileStyle::Type>::iterator it = types.begin(); 
       it != types.end(); it++)
    {
      if ((*it) != TileStyle::LONE && (*it) != TileStyle::OTHER)
	return false;
    }
  return true;
}

bool Tile::validateFeature(std::list<TileStyle::Type> types) const
{
  //forest, water and hill tiles have a full suite of styles
  //"other" styles are optional.
  if (types.size() == TileStyle::OTHER)
    return true;
  if (types.size() == TileStyle::OTHER - 1 &&
      find (types.begin(), types.end(), TileStyle::OTHER) == types.end())
    return true;
  return false;
}

bool Tile::consistsOnlyOfLoneAndOtherStyles() const
{
  std::list<TileStyle::Type> types;
  for (Tile::const_iterator i = begin(); i != end(); ++i)
    (*i)->getUniqueTileStyleTypes(types);
      return validateGrass(types);
}
bool Tile::validate() const
{
  if (size() == 0)
    return false;

  for (Tile::const_iterator i = begin(); i != end(); ++i)
    if ((*i)->validate() == false)
      return false;

  std::list<TileStyle::Type> types;
  for (Tile::const_iterator i = begin(); i != end(); ++i)
    (*i)->getUniqueTileStyleTypes(types);

  if (types.empty())
    return false;

  switch (getType())
    {
    case Tile::GRASS:
      if (validateGrass(types) == false)
	return false;
      break;
    case Tile::FOREST: case Tile::WATER: case Tile::HILLS: 
    case Tile::SWAMP: case Tile::VOID: case Tile::MOUNTAIN:
      if (validateFeature(types) == false)
	{
	  if (validateGrass(types) == false)
	    return false;
	  else
	    return true;
	}
      break;
    }
  return true;
}

void Tile::uninstantiateImages()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->uninstantiateImages();
}

void Tile::instantiateImages(int tilesize, Tileset *ts)
{
  bool broken = false;
  Tar_Helper t(ts->getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return;
  for (iterator it = begin(); it != end(); it++)
    {
      std::string file = "";
      if ((*it)->getName().empty() == false)
	file = t.getFile((*it)->getName() + ".png", broken);
      if (!broken)
        (*it)->instantiateImages(tilesize, file);
      if (file.empty() == false)
        File::erase(file);
    }
  t.Close();

}
// End of file
