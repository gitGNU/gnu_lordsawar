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

#ifndef CITYMAP_H
#define CITYMAP_H

#include <sigc++/signal.h>

#include "overviewmap.h"
#include "input-events.h"

//! Draw all of the City objects onto a miniature map graphic.
/** 
 * 
 */

class CityMap : public OverviewMap
{
 public:
     //! Default constructor.  Make a new CityMap.
    CityMap();

    //! Emitted when the cities are finished being drawn on the map surface.
    /**
     * Classes that use CityMap must catch this signal to display the map.
     */
    sigc::signal<void, SDL_Surface *> map_changed;
    
 private:
    
    //! Draw the City objects onto the miniature map graphic.
    /**
     * This method is automatically called by the CityMap::draw method.
     */
    virtual void after_draw();

};

#endif
