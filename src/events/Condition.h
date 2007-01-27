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

#ifndef CONDITION_H
#define CONDITION_H

#include <SDL_types.h>

#include "../xmlhelper.h"

/** Defines a condition.
  * 
  * The third part in the event system (besides events and reactions) are the
  * conditions. Basically, each event and reaction can have one or multiple
  * conditions associated with it and is only triggered if the conditions are
  * satisfied.
  *
  * In case of events, the event is only triggered if _all_ conditions are
  * satisfied. Else it is _not_ triggered and _not_ disabled!!!
  *
  * In case of reactions, the reaction is only triggered if the condition is
  * satisfied.
  *
  * Conditions are:
  *
  * CPlayer:        returns true if a specific player is active
  * CCounter:       returns true if the event has been triggered a number of times
  * CLiving:        returns true if a specific player still lives
  * CDead:          returns true if a specific player is dead
  * CArmy:          returns true if a specific army is in the currently active stack
  */

class Condition
{
    public:
        enum Type {PLAYER = 0, COUNTER = 1, LIVING = 2, DEAD = 3, ARMY = 4};
        
        Condition(Type type);
        virtual ~Condition();

        //! loads the appropriate condition and returns it
        static Condition* loadCondition(XML_Helper* helper);

        //! returns true if the condition is satisfied, false otherwise
        virtual bool check() = 0;

        //! saves the data
        virtual bool save(XML_Helper* helper) const = 0;

        Type getType() const {return d_type;}

    protected:
        Type d_type;
};


#endif //CONDITION_H
