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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include "maptile.h"
#include <stdlib.h>
#include <iostream>
#include "tileset.h"

Maptile::Maptile(Tileset* tileSet, int x, int y, Uint32 type, TileStyle *tileStyle)
    :d_index(type), d_building(NONE) 
{
    d_tileSet = tileSet;
    d_tileStyle = tileStyle;
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
}

Maptile::~Maptile()
{
    while (!d_items.empty())
    {
        delete (*d_items.begin());
        d_items.erase(d_items.begin());
    }
}

Uint32 Maptile::getMoves() const
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

void Maptile::addItem(Item* item, int position)
{
    if (position < 0)
    {
        d_items.push_back(item);
        return;
    }

    std::list<Item*>::iterator it;
    for (it = d_items.begin(); position > 0; position--, it++);
    d_items.insert(it, item);
}

void Maptile::removeItem(Item* item)
{
    std::list<Item*>::iterator it;
    for (it = d_items.begin(); it != d_items.end(); it++)
        if ((*it) == item)
        {
            d_items.erase(it);
            return;
        }
}

std::list<Item*> Maptile::getItems() const
{
    return d_items;
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
// End of file
