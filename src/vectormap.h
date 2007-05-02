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

#include <pgtimerobject.h>
#include <sigc++/sigc++.h>

#include "overviewmap.h"
#include "city.h"

/** Display of the whole game map.
  * 
  * This is a map where you can select a city's vector, i.e. the position where
  * freshly produced units automatically go to (at least the path is set, the
  * units have to be moved manually).
  */

class VectorMap : public OverviewMap
{
    public:
        /** Constructor
          * 
          * @param city         the city whose vectoring we deal with
          * @param parent       the parent widget
          * @param rect         the rectangle for the vector map
          */
        VectorMap(City * city, PG_Widget* parent, Rectangle rect);
        ~VectorMap();

        /** Sets the city that we concentrate on. This city is drawn differently
          * and its vector is set on mouse clicks
          */
        void setCity(City* c) {d_city = c;}
       
        //! Returns said city.
        City* getCity() const {return d_city;}

        //! emitted whenever the user moves the mouse to a new tile
        sigc::signal<void, Vector<int>> smovVectMouse;

        //! emitted whenever the user click the mouse button
        sigc::signal<void, Vector<int>> sclickVectMouse;
        
    private:
        // EVENT HANDLERS
        void eventDraw(SDL_Surface* surface, const Rectangle& rect);
        bool eventMouseButtonDown(const SDL_MouseButtonEvent* event);
        bool eventMouseMotion(const SDL_MouseMotionEvent* event);
        void eventMouseLeave(); 
     
        // DATA
        Vector<int> d_pos;
        City * d_city;
};

#endif // VECTORMAP_H
