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

#include "armymap.h"

#include "sdl-draw.h"
#include "city.h"
#include "citylist.h"
#include "playerlist.h"
#include "stacklist.h"
#include "stack.h"
#include "GraphicsCache.h"
#include "GameMap.h"
#include "FogMap.h"
#include <assert.h>

ArmyMap::ArmyMap()
{
}

void ArmyMap::draw_stacks()
{
    
    // minimum size for typical features is 1
    int size = int(pixels_per_tile) > 1 ? int(pixels_per_tile) : 1;


    // Draw stacks as crosses using the player color
    for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
        pit != Playerlist::getInstance()->end(); pit++)
    {
        Stacklist* mylist = (*pit)->getStacklist();
        SDL_Color c = (*pit)->getColor();
	guint32 outline = SDL_MapRGB(surface->format, c.r, c.g, c.b);
        
        for (Stacklist::iterator it= mylist->begin(); it != mylist->end(); it++)
        {
            Vector<int> pos = (*it)->getPos();

            // don't draw stacks in cities, they could hardly be identified
            Maptile* mytile = GameMap::getInstance()->getTile(pos.x, pos.y);
            if (mytile->getBuilding() == Maptile::CITY)
                continue;

            // don't draw stacks on tiles we can't see
            if (FogMap::isFogged (pos, Playerlist::getActiveplayer()) == true)
                continue;

            pos = mapToSurface(pos);
            draw_hline(surface, pos.x - size, pos.x + size, pos.y, outline);
            draw_vline(surface, pos.x, pos.y - size, pos.y + size, outline);
        }
    }
}

void ArmyMap::after_draw()
{
    assert(surface);
    draw_cities(false);
    draw_stacks();
    map_changed.emit(surface);
}

