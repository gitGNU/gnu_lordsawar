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
#include "TileSet.h"

Maptile::Maptile(TileSet* tileSet, int x, int y, Uint32 type)
    :d_index(type), d_building(NONE) 
{
    d_tileSet = tileSet;
}

Maptile::~Maptile()
{
    while (!d_items.empty())
    {
        delete (*d_items.begin());
        d_items.erase(d_items.begin());
    }
}

void Maptile::setCorners(int c1, int c2, int c3, int c4)
{
    d_corner[0] = c1;
    d_corner[1] = c2;
    d_corner[2] = c3;
    d_corner[3] = c4;
}

Tile::Type Maptile::getMaptileType() const
{
    return (*d_tileSet)[d_index]->getType();
}

Uint32 Maptile::getMoves() const
{
    if (d_building == Maptile::CITY)
        return 1;
    else if (d_building == Maptile::ROAD)
        return 1;

    return (*d_tileSet)[d_index]->getMoves();
}

SDL_Color Maptile::getColor() const
{
    return (*d_tileSet)[d_index]->getColor();
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


// End of file
