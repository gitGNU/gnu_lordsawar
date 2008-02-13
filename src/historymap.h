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

#ifndef HISTORYMAP_H
#define HISTORYMAP_H

#include <sigc++/signal.h>
#include <sigc++/connection.h>
#include <sigc++/trackable.h>

#include "overviewmap.h"
#include "ObjectList.h"

/** 
  * 
  */
class HistoryMap: public OverviewMap, public sigc::trackable
{
 public:
    HistoryMap(ObjectList<City> *clist);
 
    // emitted when the map surface has changed
    sigc::signal<void, SDL_Surface *> map_changed;
        
    void updateCities (ObjectList<City> *clist);

 private:

    ObjectList<City> *d_clist;
    // hook from base class
    virtual void after_draw();
    void drawCities ();
};

#endif
