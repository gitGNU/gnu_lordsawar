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

#ifndef CARMY_H
#define CARMY_H

#include "Condition.h"


/** Condition that checks for an army being in the currently active stack
  * 
  * This conditions returns true, if the active stack (i.e. the stack that is
  * currently selected/acting) contains the army with a given id. This can be
  * useful e.g. if you want to create a scenario that ends if a certain unit
  * reaches a certain position, then you only want to raise the event if the
  * unit is among the moving armies.
  */

class CArmy : public Condition
{
    public:
        //! Standard constructor with army id
        CArmy(Uint32 army);

        //! Loading constructor
        CArmy(XML_Helper* helper);
        ~CArmy();

        //! Checks if condition is fulfilled
        bool check();

        //! Saves the condition data
        bool save(XML_Helper* helper) const;

        //! Returns the army id
        Uint32 getArmy() const {return d_army;}

        //! Sets the id of the army
        void setArmy(Uint32 army) {d_army = army;}

    private:
        Uint32 d_army;
};

#endif //CARMY_H
