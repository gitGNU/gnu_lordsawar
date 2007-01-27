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

#ifndef ERUINSEARCH_H
#define ERUINSEARCH_H

#include "Event.h"

class Ruin;
class Stack;
class Player;

/** This event is raised when a special ruin is searched.
  */

class ERuinSearch : public Event
{
    public:
        //! Constructor with the id of the ruin to be searched
        ERuinSearch(Uint32 ruin);

        //! Constructor for loading a game from the savegame given by helper
        ERuinSearch(XML_Helper* helper);
        ~ERuinSearch();

        //! Saves the event data to a savegame
        bool save(XML_Helper* helper) const;

        //! Initialises the event
        void init();

        
        //! Returns the ruin whose searching the event waits for
        Uint32 getRuin() const {return d_ruin;};

        //! Sets the ruin to be searched
        void setRuin(Uint32 ruin) {d_ruin = ruin;}

    private:
        Uint32 d_ruin;

        //! Callback which is triggered whenever a ruin is searched
        void trigger(Ruin* r, Stack* s);
};

#endif //ERUINSEARCH_H
