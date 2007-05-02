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

#ifndef ESTACKMOVE_H
#define ESTACKMOVE_H

#include "Event.h"

#include "../vector.h"

class Stack;

/** Event that is triggered when units cross one or more fields
  * 
  * Using this event, you can specify a point. Whenever a unit crosses
  * this field, the event is triggered. To handle only units of one player,
  * use the player condition!
  */

class EStackMove : public Event
{
    public:
        //! standard constructor with a set of points
        EStackMove(Vector<int> pos);

        //! Loading constructor
        EStackMove(XML_Helper* helper);
        ~EStackMove();

        //! Saves the event data
        bool save(XML_Helper* helper) const;

        //! Initialises the event at the beginning of a game
        void init();

        //! Returns the point this event looks for
        const Vector<int> getPos() const {return d_pos;}

        //! Changes the point this event looks for
        void setPos(Vector<int> pos) {d_pos = pos;}

    private:
        Vector<int> d_pos;

        //! Callback for loading
        bool loadPoint(std::string tag, XML_Helper* helper);

        //! Triggers the event
        void trigger(Stack* s);
};

#endif //ESTACKMOVE_H
