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

#ifndef HISTORYMAP_H
#define HISTORYMAP_H

#include <sigc++/signal.h>
#include <sigc++/connection.h>
#include <sigc++/trackable.h>

#include "overviewmap.h"
#include "LocationList.h"

//! Draw the given cities on the map.
/** 
 * Draw a set of cities onto the miniature map graphic.
 *
 * @note This is called HistoryMap because it is used for the HistoryDialog.
 *
 */
class HistoryMap: public OverviewMap
{
 public:
     //! Default constructor.  Make a new HistoryMap.
     /**
      * @param clist  The list of the City objects to draw on the miniature map.
      */
     HistoryMap(LocationList<City*> *clist);
 
     //! Emitted when the cities are finished being drawn on the map surface.
     /**
      * Classes that use HistoryMap must catch this signal to display the map.
      */
     sigc::signal<void, Glib::RefPtr<Gdk::Pixmap> > map_changed;
        
     //! Change which cities are shown on the miniature map graphic.
     /**
      * This method erases the cities that were previously drawn and shows
      * a new set of City objects.
      *
      * @param clist  The new list of City objects to draw onto the miniature
      *               map graphic.
      */
     void updateCities (LocationList<City*> *clist);

 private:

     //! The set of city objects to show on the miniature map graphic.
     LocationList<City*> *d_clist;

     //! Draw the City objects onto the miniature map graphic.
     /**
      * This method is automatically called by the HistoryMap::draw method.
      */
     virtual void after_draw();

     //! Draw the cities.
     /**
      * This method iterates over the members of HistoryMap::d_clist and draws
      * each city onto the minature map graphic.
      *
      * We can't use the OverviewMap::draw_cities method because it draws the
      * City objects of Citylist, and not the set of cities defined here.
      */
     void drawCities ();
};

#endif
