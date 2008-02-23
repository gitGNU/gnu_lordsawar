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

#ifndef STACK_H
#define STACK_H

#include <list>
#include <vector>
#include "vector.h"
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <sstream>

#include "Object.h"
#include "Ownable.h"

class Player;
class Path;
class Army;
class XML_Helper;

/** 
 * A set of up to eight Army units that move around as a single entity on the
 * game map.
 * 
 * While Army units are the actual troops you command, they always belong to a
 * stack. The stack holds these armies together in one object. The player
 * usually doesn't command the armies but the stack, so all functionality and
 * data which affects player's controls is bundled in the stack class. Among
 * this is the location of the units, the intended movement path, and more.
 */

class Stack : public ::Object, public Ownable, public std::list<Army*>, public sigc::trackable
{
    public:
        /** 
	 * Make a new stack.
         * 
         * @param player       A pointer to the owning player.
         * @param pos          The position on the map where the stack is to 
	 *                     be created.
         */
	//! Default constructor.
        Stack(Player* player, Vector<int> pos);

	/**
	 * Copy the whole stack into a new stack.  This method performs a 
	 * deep copy of the stack's armies.
	 */
        //! Copy constructor.
        Stack(Stack& s);

        //! Loading constructor.
        Stack(XML_Helper* helper);

	//! Destructor.
        ~Stack();

        //! Change the loyalty of the stack.
        void setPlayer(Player* p);

        //! Change the position of the stack.
        void setPosition(Vector<int> pos){d_pos = pos;}

        /** 
	 * Sets the defending value.  Defending entails that this stack is 
	 * ignored when a user cycles through the list of stacks with 
	 * Stacklist::getNextMovable().  If a stack stays defending to the
	 * next round, it gets a fortify bonus in battle.
	 */
	//! Set the defending status of the stack.
        void setDefending(bool defending){d_defending = defending;}

        /** 
	 * Sets the parked value. Parking entails that this stack is ignored
         * when a player cycles through his list of stacks with 
	 * Stacklist::getNextMovable().  This value behaves just like 
	 * defending, but there's no bonus conferred if a stack remains in 
	 * this state.
         */
	//! Set the parked status of the stack.
        void setParked(bool parked){d_parked = parked;}

        //! Save the stack to an opened saved-game file.
        bool save(XML_Helper* helper) const;

	/**
	 * When the end of a turn occurs, this callback is used to calculate
	 * stack bonuses, moves, paths, and it also charges the player the
	 * upkeep fee for every Army unit in the Stack.
	 */
        //! Callback when the end of a turn happens.
        void nextTurn();

        //! Reduces movement points of the Army units in the Stack.
        void decrementMoves(Uint32 moves);

        //! Sets the stack's position to the next point in it's Path.
        bool moveOneStep();

	/**
	 * Adds one to the strength of each Army unit in the stack.
	 * If the Army unit has already visited the temple co-located with
	 * the stack's position, no strength bonus will be added.
	 *
	 * @return The number of Army units blessed.
	 */
	//! Bless the Army units in the stack.
        int bless();

	/**
         * @return True if the stack has enough moves to traverse to
	 * the next step in it's Path.  Otherwise, false.
	 */
	//! Returns whether or not the stack can move.
        bool enoughMoves() const;

	/**
	 * @return Whether or not the stack has enough moves to travel to
	 *         an adjacent tile.  The adjacent tile does not have to be
	 *         in the stack's Path.
	 */
	//! Returns whether the stack can move in any direction.
	bool canMove() const;
        
        //! Returns the Path object of the stack.
        Path* getPath() const {return d_path;}

        //! Returns the minimum number of movement points of all armies.
        Uint32 getGroupMoves() const;

	//! Returns true if all armies in the stack are grouped.
	bool isGrouped();

	/**
	 * Scan all adjacent tiles relative to the stack's position and 
	 * see how much a move would cost in terms of movement points.
	 * Determine the minimum amount of movement points to make a move.
	 * 
	 * @return The minimum number of movement points to travel to the
	 *         cheapest adjacent tile that the stack can afford to
	 *         move to.  If the stack cannot afford to move there, this
	 *         method returns -1.
	 */
        int getMinTileMoves() const;

        //! Get the next position in the stack's intended Path.
        Vector<int> nextStep();

        //! Return the Army unit in the Stack that has the best strength value.
        Army* getStrongestArmy() const;

        //! Return the Hero unit in the Stack that has the best strength value.
        Army* getStrongestHero() const;

	/**
	 * Scan through the Army units in the stack and return the first
	 * one that is ungrouped.  This method is used for splitting stacks.  
	 * See Player::stackSplit for more information.
	 *
	 * @return A pointer to the first ungrouped army in the stack or NULL
	 *         if an ungrouped Army unit could not be found.
	 */
        //! Get the first ungrouped Army unit in the Stack.
        Army* getFirstUngroupedArmy() const;

        //! True if the stack contains a Hero unit.  Otherwise, false.
        bool hasHero() const;

        //! Return the first Hero unit in the stack, or NULL if no Hero found.
        Army* getFirstHero() const;

        //! Returns the ids of all (living) heroes in the stack in the dst reference
	/**
	 * Scan the Army units in the Stack for heroes that have more than
	 * zero hitpoints.
	 *
	 * @param dst       Passed in as an empty or non-empty list, and
	 *                  filled up with the Ids belonging to Hero army 
	 *                  units in the stack.
	 */
	// Return the Ids of all of the Hero units in the Stack.
        void getHeroes(std::vector<Uint32>& dst) const;

        //! Return the defending status of the stack (see setDefending)
        bool getDefending() const {return d_defending;}
        //! Return the parked status of the stack (see setParked)
        bool getParked() const {return d_parked;}

        //! Returns whether the stack is being deleted (set to true in the destructor)
        bool getDeleting() const {return d_deleting;}

        //! Return the maximum sight of the stack
        Uint32 getMaxSight() const;


        //! The same as std::list::clear, but alse frees pointers
        void flClear();

        //! The same as std::list::erase, but also frees pointers
        iterator flErase(iterator object);

       /** Calculates group move bonuses.
          *
          */
        Uint32 calculateMoveBonus() const;
        bool isFlying () const;
        bool hasShip () const;

	//! how much does it cost in movement points for the stack
	//to move to move onto a tile that has the characteristics of the
	//tile located at POS. (this is not a distance calculation)
	Uint32 calculateTileMovementCost(Vector<int> pos) const;

        sigc::signal<void, Stack*> sdying;

	void group();
	void ungroup();
	void sortForViewing(bool reverse);
	void setFortified(bool fortified);
	bool getFortified();
	Uint32 getUpkeep();
        
	static bool armyCompareFightOrder (const Army *, const Army *);
	Uint32 getMovesExhaustedAtPoint() {return d_moves_exhausted_at_point;}
	void setMovesExhaustedAtPoint(Uint32 index) {d_moves_exhausted_at_point = index;}
    private:    
        //! Callback for loading the stack
        bool load(std::string tag, XML_Helper* helper);
    
	Army* getStrongestArmy(bool hero) const;

        // DATA
        Path* d_path;
        bool d_defending;
        bool d_parked;
        
        // true if the stack is currently being deleted. This is neccessary as
        // some things may happen in the destructor of the contained armies and
        // we don't want bigmap to draw the stack when it is being removed.
        bool d_deleting;
	Uint32 d_moves_exhausted_at_point;
};

#endif // STACK_H

// End of file
