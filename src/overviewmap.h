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

#ifndef OVERVIEWMAP_H
#define OVERVIEWMAP_H

#include <pgwidget.h>

/** Draw a map of the whole game.
  * 
  * This class actually cares for drawing a small map with colors representing
  * terrain and symbols representing cities, ruins etc.
  *
  * It provides the basis for actual implementations, namely SmallMap and
  * VectorMap.
  */

class OverviewMap : public PG_Widget
{
    public:
        /** Constructor
          * 
          * @param parent       the parent widget
          * @param rect         the rectangle for the widget
          *
          * Other data (namely the map size) is taken from GameMap.
          */
        OverviewMap(PG_Widget* parent, PG_Rect rect);
        ~OverviewMap();

    protected:
        /* creates the underlying map showing all the terrain. If the parameters
         * are both != 0, use them for width and height, else take the internal
         * coordinates.
         */
        void createStaticMap(Uint16 xsize = 0, Uint16 ysize = 0);

        // EVENT HANDLERS
        virtual void eventDraw(SDL_Surface* surface, const PG_Rect& rect);
        virtual void eventSizeWidget(Uint16 w, Uint16 h);
        
        //! Maps the given point in absolute screen coordinates to a map coordinate
        PG_Point mapFromScreen(PG_Point pos);
        
        //! And (almost) the other way round. Map a map coordinate to a surface pixel.
        PG_Point mapToSurface(PG_Point pos);
        
        // DATA
        SDL_Surface* d_staticMap;
        
        double d_xpixels, d_ypixels;
};

#endif // OVERVIEWMAP_H
