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

#ifndef RDELUNIT_H
#define RDELUNIT_H

#include "Reaction.h"

class Army;

/** This reaction removes a specific army (if it exists).
  * If it can't find the army, it doesn't care
  *
  * @note be careful with assigning the army another event, this may result
  * in a complex nested chain of reactions.
  *
  * For more information about reactions see Reaction.h
  */

class RDelUnit : public Reaction
{
    public:
        //! Initialises the reaction with an army's id
        RDelUnit(Uint32 army);

        //! Loading constructor. See XML_Helper for details
        RDelUnit(XML_Helper* helper);

        ~RDelUnit();

        //! Saves the data
        bool save(XML_Helper* helper) const;

        //! Triggers the reaction
        bool trigger() const;

        
        //! Returns the army we want to delete
        Army* getArmy();

        //! Returns the id of the army we want to delete
        Uint32 getArmyId() const {return d_army;}

        //! Sets the army we want to delete
        void setArmy(Uint32 army) {d_army = army;}

    private:
        Uint32 d_army;
};

#endif //RDELUNIT_H
