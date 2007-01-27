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

#ifndef CCOUNTER_H
#define CCOUNTER_H

#include "Condition.h"

/** This class is initialised with a counter value and decreases it with every call
  * of check(). When the counter has reached zero, it returns true, else false.
  * This condition can produce quite interesting results in connection with a dummy
  * event and triggering reactions. See EDummy.h for a scenario when this becomes
  * handy.
  */

class CCounter: public Condition
{
    public:
        //! Initialise condition with a counter
        CCounter(Uint32 number);

        //! Loading constructor. Details to be found in xmlhelper.h
        CCounter(XML_Helper* helper);
        ~CCounter();

        //! Returns true when the counter has reached zero, false otherwise
        bool check();

        //! Saves the game data. See again xmlhelper.h
        bool save(XML_Helper* helper) const;

        
        //! Return the current counter value
        Uint32 getCounter() const {return d_counter;}

        //! Change the current counter value
        void setCounter(Uint32 number) {d_counter = number;}

    private:
        Uint32 d_counter;
};

#endif //CCOUNTER_H
