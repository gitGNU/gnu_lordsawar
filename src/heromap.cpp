//  Copyright (C) 2007, 2008, 2009, 2014 Ben Asselstine
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

#include "heromap.h"

#include "gui/image-helpers.h"
#include "city.h"
#include "citylist.h"
#include "playerlist.h"

HeroMap::HeroMap(City *c)
{
    city = c;
}

void HeroMap::after_draw()
{
    OverviewMap::after_draw();
    draw_cities(false);
    draw_hero(city->getPos(), true);
    map_changed.emit(surface);
}

