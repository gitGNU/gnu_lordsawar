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

#ifndef RUINMAP_H
#define RUINMAP_H

#include <sigc++/signal.h>

#include "overviewmap.h"
#include "input-events.h"
#include "player.h"

class Ruin;

/** Display of the whole game map.
  * 
  * This is a map where you can see ruins and temples.
  */

class RuinMap : public OverviewMap
{
 public:
    RuinMap(Location *r); //r is the selected ruin or temple

    //! change what ruin or temple is selected
    void setLocation (Location *r) {ruin = r;}

    Location * getLocation () const {return ruin;}

    void mouse_button_event(MouseButtonEvent e);

    // emits the location chosen
    sigc::signal<void, Location *> location_changed;

    // emitted when the map surface has changed
    sigc::signal<void, SDL_Surface *> map_changed;
    
 private:
    Location *ruin;
    void draw_ruins (bool show_selected);
    void draw_temples (bool show_selected);
    //void draw_cities(bool all_razed);
    
    // hook from base class
    virtual void after_draw();
};

#endif
