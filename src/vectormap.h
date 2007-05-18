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

#ifndef VECTORMAP_H
#define VECTORMAP_H

#include <sigc++/signal.h>

#include "overviewmap.h"
#include "input-events.h"

class City;

/** Display of the whole game map.
  * 
  * This is a map where you can select a city's vector, i.e. the position where
  * freshly produced units automatically go to (at least the path is set, the
  * units have to be moved manually).
  */

class VectorMap : public OverviewMap
{
 public:
    VectorMap(City *city);

    void mouse_button_event(MouseButtonEvent e);

    // emits the tile chosen, (-1, -1) if tile deselected
    sigc::signal<void, Vector<int> > destination_chosen;
     
    // emitted when the map surface has changed
    sigc::signal<void, SDL_Surface *> map_changed;
    
 private:
    City *city;
    
    // hook from base class
    virtual void after_draw();
    void draw_city (City *c, Uint32 &type, bool &prod);
    void draw_cities (std::list<City*> citylist, Uint32 type);
    void draw_lines (std::list<City*> citylist);
};

#endif
