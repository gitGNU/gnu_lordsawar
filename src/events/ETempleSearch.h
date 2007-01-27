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

#ifndef ETEMPLE_H
#define ETEMPLE_H

#include "Event.h"

class Temple;
class Stack;
class Player;

/** This event is raised when a temple is searched.
  * 
  * See event.h for more information about events.
  */

class ETempleSearch : public Event
{
    public:
        //! Default constructor with id of the temple
        ETempleSearch(Uint32 id);

        //! Loading constructor
        ETempleSearch(XML_Helper* helper);
        ~ETempleSearch();

        //! saves the data to the savegame helper
        bool save(XML_Helper* helper) const;

        //! Initialises the event.
        void init();

        
        //! Returns the id of the temple
        Uint32 getTemple() const {return d_temple;}

        //! Sets the id of the temple
        void setTemple(Uint32 temple) {d_temple = temple;}

    protected:
        Uint32 d_temple;
        
        //! Callback for triggering the event
        void trigger(Temple* t, Stack* s);
};

#endif //ETEMPLE_H
