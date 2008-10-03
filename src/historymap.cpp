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

#include <config.h>

#include <algorithm>
#include <assert.h>

#include "vector.h"

#include "historymap.h"
#include "sdl-draw.h"
#include "GameMap.h"
#include "GraphicsCache.h"

HistoryMap::HistoryMap(LocationList<City*> *clist)
{
  d_clist = clist;
}

void HistoryMap::after_draw()
{
    OverviewMap::after_draw();
    drawCities();
    map_changed.emit(get_surface());
}

void HistoryMap::drawCities()
{
  GraphicsCache *gc = GraphicsCache::getInstance();

  // Draw all cities as shields over the city location, in the colors of
  // the players.
  LocationList<City*>::iterator it = d_clist->begin();
  for (; it != d_clist->end(); it++)
  {
      SDL_Surface *tmp;
      if ((*it)->isFogged())
        continue;
      if ((*it)->isBurnt() == true)
        tmp = gc->getSmallRuinedCityPic();
      else
        tmp = gc->getShieldPic(0, (*it)->getOwner());
  
      Vector<int> pos = (*it)->getPos();
      pos = mapToSurface(pos);
      SDL_Rect r;
      r.x = pos.x - (tmp->w/2);
      r.y = pos.y - (tmp->h/2);
      r.w = tmp->w;
      r.h = tmp->h;
      SDL_BlitSurface(tmp, 0, surface, &r);
  }
}

void HistoryMap::updateCities (LocationList<City*> *clist)
{
  d_clist = clist;
  draw();
  after_draw();
}
