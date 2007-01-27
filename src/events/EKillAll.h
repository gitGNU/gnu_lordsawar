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

#ifndef EKILLALL_H
#define EKILLALL_H

#include "Event.h"

class Player;

/** This event is raised in the case of "last man standing", i.e. if there is
  * only one player left.
  */

class EKillAll : public Event
{
    public:
        EKillAll();

        //! Loading constructor which gets the data from a savegame
        EKillAll(XML_Helper* helper);
        ~EKillAll();

        //! Saves the event data
        bool save(XML_Helper* helper) const;

        //! Initialises the event
        void init();

    private:
        //! Callback which triggers the event.
        void trigger(Player* p);
};


#endif //EKILLALL_H
