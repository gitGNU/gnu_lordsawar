//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

#include "heromap.h"

#include "gui/image-helpers.h"
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

    Glib::RefPtr<Gdk::Pixbuf> heropic = gc->getSmallHeroPic (true);
    surface->draw_pixbuf(heropic, 0, 0, 
			 start.x - (heropic->get_width()/2), 
			 start.y - (heropic->get_height()/2), 
			 heropic->get_width(), heropic->get_height(),
			 Gdk::RGB_DITHER_NONE, 0, 0);
    map_changed.emit(surface);
}

