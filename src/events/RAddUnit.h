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

#ifndef RADDUNIT_H
#define RADDUNIT_H

#include <string>
#include <sigc++/sigc++.h>
#include "Reaction.h"
#include "../stack.h"

/** Adds a specific army to a given player when triggered.
  * 
  * @note Don't reycle this reaction or the army it contains under any
  * circumstances unless you want to get into trouble. The army this reaction
  * has must have a unique id.
  * 
  * @note The position where the army is placed has to be free of other stacks.
  * If this cannot be guaranteed, be aware that the stack will then be placed
  * on the next tile where this is possible. If there are several tiles, no order
  * is guaranteed. So be careful if you place a new stack on a tiny island. :)
  *
  * TODO: This reaction currently tries to place the army in a separate stack at
  * the specified position or at one of the eight tiles around it. If they are
  * blocked, this reaction fails.
  * 
  * See Reaction.h for a more detailed explanation of reactions.
  */


class RAddUnit : public Reaction
{
    public:
        /** default constructor
          * 
          * @param army     the army to add (note: shallow copy! Don't use this army
          *                 any more)
          * @param player   the player who gets the army
          */
        RAddUnit(Stack* unit, Uint32 player, PG_Point pos);

        //! Loads the data from a savegame
        RAddUnit(XML_Helper* helper);
        ~RAddUnit();

        //! Saves the data to a savegame. See XML_Helper for more information
        bool save(XML_Helper* helper) const;

        //! Triggers the reaction
        bool trigger() const;


        //! Returns the specialized army
        Stack* getStack() {return d_stack;}

        //! Sets the specialized army; returns the old stack
        Stack* setStack(Stack* s);
        
        //! Returns the player
        Uint32 getPlayer() const {return d_player;}

        //! Sets the player id
        void setPlayer(Uint32 player) {d_player = player;}

        //! Returns the position of the placement
        PG_Point getPos() const {return d_pos;}

        //! Sets the position of the placement
        void setPos(PG_Point pos) {d_pos = pos;}

        
    private:
        Stack* d_stack;
        Uint32 d_player;
        PG_Point d_pos;

        //! Callback for loading, useless otherwise
        bool loadStack(std::string tag, XML_Helper* helper);
};

#endif //RADDUNIT_H
