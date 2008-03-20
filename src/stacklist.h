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

//! A list of Stack objects for a single player.
/** 
 * All stacks owned by a Player are contained in a Stacklist.  This class
 * covers the loading and saving of stack lists, and also some methods for
 * getting and managing groups of stacks.
 */

class Stacklist : public std::list<Stack*>, public sigc::trackable
{
    public:
	//! Default constructor.
        Stacklist();
	//! Copy constructor.
        Stacklist(Stacklist *stacklist);
	//! Loading constructor.
        Stacklist(XML_Helper* helper);
	//! Destructor.
        ~Stacklist();

	/**
	 * Scan through every player's Stacklist, for a stack that is located 
	 * at the given position on the game map.
	 *
	 * @param x   The number of tiles down in the vertical axis from the
	 *            topmost edge of the map.
	 * @param y   The number of tiles right in the horizontal axis from
	 *            the leftmost edge of the map.
	 *
	 * @return A pointer to a stack at the given position, or NULL if no
	 *         Stack could be found.
	 */
        //! Return the stack at position (x,y) or 0 if there is none.
        static Stack* getObjectAt(int x, int y);

	/**
	 * Scan through every player's Stacklist, for a stack that is located 
	 * at the given position on the game map.
	 *
	 * @param point    The point on the map that we're looking to see
	 *                 if there is a Stack on.
	 *
	 * @return A pointer to a stack at the given position, or NULL if no
	 *         Stack could be found.
	 */
        //! Return stack at position pos or 0 if there is none.
        static Stack* getObjectAt(Vector<int> point)
	  { return getObjectAt(point.x, point.y);}

	/**
	 * Scan through all stacks in the list, and then through each Army 
	 * unit of every Stack for an Army unit with a particular Id.
	 *
	 * @param id     The Id of the Army unit that we're looking for.
	 *
	 * @return The position of the Army unit.  If no Army unit could be
	 *         found with the given Id, the position of (-1,-1) is
	 *         returned.
	 */
        //! Return position of an Army.
        static Vector<int> getPosition(Uint32 id);

        /** 
	 * This method finds stacks which occupy the same tile.
         * For internal and logical reasons, we always assume that a tile is
         * occupied by at most one stack as strange bugs may happen when this
         * is violated. However, in some cases, it does happen that stacks
         * temporarily occupy the same tile. Reasons may be that, on its
         * route, a stack crosses another stacks tile and can't move further.
         *
	 * We only expect one ambiguity at a time with stacks of the same 
	 * player. This never happens except when a stack comes to halt on 
	 * another stack during long movements.
	 *
         * @param stack        The stack which we search an ambiguity for.
	 *
	 * @return The other stack on the same tile occupied by stack.
         */
	//! Get the other stack on a tile that has more than one stack on it.
        static Stack* getAmbiguity(Stack* stack);

        //! Searches through the all players Stacklists and deletes the stack.
        static void deleteStack(Stack* stack);

        /** 
	 * Scan each tile occupied by the given city and return a list of
	 * stacks who are in the city.
         *
         * When a city is attacked, all stacks which occupy a city tile are
         * regarded as defenders.  The purpose of this function is to 
	 * enumerate the defending stacks when a stack has attacked a city.
         *
         * @param city        The city to search for stacks in.
	 *
         * @return A list of all stacks defending the city.
         */
	//! Return a list of stacks defending a city.
        static std::vector<Stack*> defendersInCity(City* city);

        //! Returns the total number of stacks owned by all players.
        static unsigned int getNoOfStacks();

        //! Returns the total number of armies in the list.
        unsigned int countArmies();

        /** 
	 * Sets the currently selected stack. The purpose of this method is 
         * to designate a stack to be the one the player is currently touching.
         * It is important to use this method because several functions
	 * expect that there is an active stack.
         *
         * @param activestack      The stack currently selected by the player.
         */
        void setActivestack(Stack* activestack) {d_activestack = activestack;}

	/**
	 * Scan through the list of stacks to find one that is not defending, 
	 * and not parked, and can move to another tile.
	 *
	 * @return A pointer to the next moveable stack or NULL if no more
	 *         stacks can move.
	 */
        //! Return the next moveable stack in the list.
        Stack* getNextMovable();

        //! Save the data to an opened saved-game file.
        bool save(XML_Helper* helper) const;

        //! Callback method executed at the end of every turn.
        void nextTurn();

	/**
         * @return True if any stacks in the list have enough moves for 
	 * it's next step along it's Path.  Otherwise, false.
	 */
	//! Returns whether or not any stacks can move.
        bool enoughMoves() const;

        //! Returns the currently selected stack.
        Stack* getActivestack() const {return d_activestack;}

        //! Erase all stacks from the list, and their contents too.
        void flClear();

        /** 
	 * Erase a Stack from the list, and free the contents of the Stack.
	 *
	 * @param it   The place in the Stacklist to erase.
	 *
	 * @return The place in the list that was erased.
         */
	//! Erase a stack from the list.
        iterator flErase(iterator object);

        /** 
	 * Scan the list of stacks for a particular stack.  If it is found,
	 * remove it from the list of stacks and free it's contents.
	 *
	 * @param stack  The stack in the Stacklist to remove.
	 *
	 * @return Whether or not the stack was found and deleted.
         */
        //! Erase a stack from the list.
        bool flRemove(Stack* stack);

	/**
	 * Scan through the Stacklist, for a stack that is located at the 
	 * given position on the game map.
	 *
	 * @note This method works only on this Stacklist, rather than all
	 *       of the players Stacklists as in Stacklist::getObjectAt.
	 *
	 * @param x   The number of tiles down in the vertical axis from the
	 *            topmost edge of the map.
	 * @param y   The number of tiles right in the horizontal axis from
	 *            the leftmost edge of the map.
	 *
	 * @return A pointer to a stack at the given position, or NULL if no
	 *         Stack could be found.
	 */
        //! Return the stack at position (x,y) or 0 if there is none.
        Stack* getOwnObjectAt(int x, int y);

        Stack *getStackById(Uint32 id);
        Stack *getArmyStackById(Uint32 army);

    private:
        //! Callback function for loading.
        bool load(std::string tag, XML_Helper* helper);

	//! A pointer to the currently selected Stack.
        Stack* d_activestack;
};

#endif // STACKLIST_H

// End of file
