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

#ifndef EARMYKILLED_H
#define EARMYKILLED_H

#include "Event.h"

class Army;

/** This event occurs if a specific army is killed (Note: army, not stack!).
  *
  * @note There is a certain condition where this event may cause trouble. If
  * a player dies, all his armies are killed. The more sane behaviour here is
  * not to raise the events associated with his armies which is why these events
  * are not raised if a player has already been killed (all his cities have been
  * conquered). Keep this in mind when constructing an event chain.
  *
  * More details about events can be found in Event.h
  */

class EArmyKilled : public Event
{
    public:
        //! Standard constructor waiting for army with id "army" to be killed
        EArmyKilled(Uint32 army);

        //! Loading constructor
        EArmyKilled(XML_Helper* helper);
        ~EArmyKilled();

        //! Saves the data to the savefile specified by helper
        bool save(XML_Helper* helper) const;

        //! Initialises the event
        void init();

        
        //! Returns the army whose death the event waits for
        Army* getArmy() const;

        //! Returns only the id of the army. Sometimes neccessary
        Uint32 getArmyId() const {return d_army;}

        //! Sets the army whose death we await
        void setArmy(Uint32 army) {d_army = army;}

    private:
        Uint32 d_army;
        
        //! Callback for triggering the event
        void trigger(Army*);
};

#endif //EARMYKILLED_H
