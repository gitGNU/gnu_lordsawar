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

#ifndef ENEXTTURN_H
#define ENEXTTURN_H

#include "Event.h"

class Player;

/** This event is called at the next possible occasion.
  * 
  * The ENextTurn event is quite similar to the ERound event, but in difference
  * to that it is not raised at the next round but rather at the next turn. The
  * idea behind this is to delay e.g. messages until a specific player starts
  * his turn. The ENextTurn event is then only activated when the reason for the
  * message has been triggered.
  */

class ENextTurn : public Event
{
    public:
        ENextTurn();
        ENextTurn(XML_Helper* helper);
        ~ENextTurn();

        //! Saves the event data
        bool save(XML_Helper* helper) const;

        //! triggers the event, the player is not used
        void trigger(Player* p);
};

#endif //ENEXTTURN_H
