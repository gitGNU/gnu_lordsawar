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

#include "heromap.h"

#include "sdl-draw.h"
#include "city.h"
#include "citylist.h"
#include "playerlist.h"
#include "GraphicsCache.h"

HeroMap::HeroMap(City *c)
{
    city = c;
}

void HeroMap::after_draw()
{
    GraphicsCache *gc = GraphicsCache::getInstance();
    OverviewMap::after_draw();
    draw_cities(false);
    // draw the hero picture over top of the host city
    Vector<int> start = city->getPos();

    start = mapToSurface(start);

    start += Vector<int>(int(pixels_per_tile/2), int(pixels_per_tile/2));

    SDL_Surface *tmp = gc->getSmallHeroPic();
    
    SDL_Rect r;
    r.x = start.x - (tmp->w/2);
    r.y = start.y - (tmp->h/2);
    r.w = tmp->w;
    r.h = tmp->h;
    SDL_BlitSurface(tmp, 0, surface, &r);
    SDL_FreeSurface(tmp);
    map_changed.emit(surface);
}

