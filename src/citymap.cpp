// Copyright (C) 2007 Ben Asselstine
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

#include "citymap.h"

#include "city.h"
#include "citylist.h"
#include "playerlist.h"
#include "stacklist.h"
#include "stack.h"
#include "GraphicsCache.h"
#include "GameMap.h"
#include <assert.h>

CityMap::CityMap()
{
}

void CityMap::after_draw()
{
    assert(surface);
    draw_cities(false);
    map_changed.emit(surface);
}

