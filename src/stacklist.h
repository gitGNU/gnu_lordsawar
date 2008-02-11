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

#ifndef STACKLIST_H
#define STACKLIST_H

#include <list>
#include <vector>
#include <sigc++/trackable.h>
#include <SDL_types.h>
#include "vector.h"
#include <sstream>

class City;
class Stack;
class XML_Helper;

/** List of stacks of a single player
  *
  * All stacks of a player are contained in his stacklist. There is not really
  * much to do besides saving and loading, so the stacklist contains a lot of
  * useful functions. It can check if two stacks can join, return the defenders
  * of a city etc.
  */

class Stacklist : public std::list<Stack*>, public sigc::trackable
{
    public:
        Stacklist();
        Stacklist(Stacklist *stacklist);
        Stacklist(XML_Helper* helper);
        ~Stacklist();


        //! Return the stack at position (x,y) or 0 if there is none
        static Stack* getObjectAt(int x, int y);

        //! Return stack at position pos or 0 if there is none
        static Stack* getObjectAt(Vector<int> point);

        //! Return position of an army
        static Vector<int> getPosition(Uint32 id);

        /** This function finds stacks which occupy the same tile.
          * For internal and logical reasons, we always assume that a tile is
          * occupied by at most one stack as strange bugs may happen when this
          * is violated. However, in some cases, it does happen that stacks
          * temporarily occupy the same tile. Reasons may be that, on its
          * route, a stack crosses another stacks tile and can't move further.
          *
          * @param s        the stack which we search an ambiguity for
          */
        static Stack* getAmbiguity(Stack* s);

        //! Searches through the player's lists and deletes the stack
        static void deleteStack(Stack* s);

        /** Returns stacks defending a city
          *
          * If a city is attacked, all stacks which occupy a city tile are
          * regarded as defenders.
          *
          * @param c        the city under attack
          * @return a list of all stacks defending the city
          */
        static std::vector<Stack*> defendersInCity(City* c);

        //! Returns the number of stacks owned by all players
        static unsigned int getNoOfStacks();

        //! Returns the number of armies in the list
        unsigned int countArmies();

        /** Sets the activestack. The purpose of this pointer is that the
          * activestack is assumed to be one the player is currently "touching".
          * Several functions use this feature for internal purposes, so don't
          * forget to set the activestack
          *
          * @param activestack      the stack currently moved by the player
          */
        void setActivestack(Stack* activestack) {d_activestack = activestack;}

        //! Get the next non-defending stack that can move.
        Stack* getNextMovable();

        //! Returns true if s and d can form one stack (esp. regarding size)
        bool canJoin(Stack* s, Stack* d) const;

        //! Save the data. See XML_Helper for details
        bool save(XML_Helper* helper) const;

        //! Calls nextTurn of each stack in the list (for healing, upkeep etc.)
        void nextTurn();

        //! Returns true if _any_ of the stacks has enough moves for its next step
        bool enoughMoves() const;

        //! Returns the designated activestack
        Stack* getActivestack() const {return d_activestack;}


        //! Behaves like std::list::clear(), but frees pointers as well
        void flClear();

        //! Behaves like std::list::erase(), but frees pointers as well
        iterator flErase(iterator object);

        //! Behaves like std::list::remove(), but frees pointers as well
        bool flRemove(Stack* object);

        //! Return the stack at position (x,y) or 0 if there is none
	//! only operates on this stacklist, and not all players stacklists.
        Stack* getOwnObjectAt(int x, int y);

    private:
        //! Callback function for loading
        bool load(std::string tag, XML_Helper* helper);

        Stack* d_activestack;
};

#endif // STACKLIST_H

// End of file
