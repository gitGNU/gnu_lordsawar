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

#ifndef HEROMAP_H
#define HEROMAP_H

#include <sigc++/signal.h>

#include "overviewmap.h"
#include "input-events.h"

class City;

/** Display of the whole game map.
  * 
  * This is a map where you can highlight a city with a hero icon.
  */

class HeroMap : public OverviewMap
{
 public:
    HeroMap(City *city);

    // emitted when the map surface has changed
    sigc::signal<void, SDL_Surface *> map_changed;
    
 private:
    City *city;
    
    // hook from base class
    virtual void after_draw();
};

#endif
