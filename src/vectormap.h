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
  * freshly produced units automatically go to.  They show up at the destination
  * in 2 turns.
  */

class VectorMap : public OverviewMap
{
 public:
  enum ShowVectoring
    {
      SHOW_NO_VECTORING,
      SHOW_ORIGIN_CITY_VECTORING,
      SHOW_ALL_VECTORING,
    };
  enum ClickAction
    {
      CLICK_SELECTS,
      CLICK_VECTORS,
    };
    VectorMap(City *city, enum ShowVectoring vector);
    VectorMap(City *city);

    void mouse_button_event(MouseButtonEvent e);

    // emits the tile chosen, (-1, -1) if tile deselected
    sigc::signal<void, Vector<int> > destination_chosen;
     
    // emitted when the map surface has changed
    sigc::signal<void, SDL_Surface *> map_changed;

    void setShowVectoring (enum ShowVectoring v) { show_vectoring = v;}

    City* getCity() {return city;}

    void setClickAction (enum ClickAction a) { click_action = a;}
    enum ClickAction getClickAction () { return click_action;}
    
 private:
    City *city;
    Vector<int> planted_standard;
    enum ShowVectoring show_vectoring;
    enum ClickAction click_action;
    
    // hook from base class
    virtual void after_draw();
    void draw_city (City *c, Uint32 &type, bool &prod);
    void draw_cities (std::list<City*> citylist, Uint32 type);
    void draw_lines (std::list<City*> citylist);
    void draw_planted_standard(Vector<int> pos);
};

#endif
