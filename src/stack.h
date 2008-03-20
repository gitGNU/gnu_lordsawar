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

#include "UniquelyIdentified.h"
#include "Ownable.h"
#include "Movable.h"

class Player;
class Path;
class Army;
class XML_Helper;

//! A set of up to eight Army units that move as a single entity on the map.
/** 
 * While Army units are the actual troops you command, they always belong to a
 * stack. The stack holds these armies together in one object. The player
 * usually doesn't command the armies but the stack, so all functionality and
 * data which affects player's controls is bundled in the stack class. Among
 * this is the location of the units, the intended movement path, and more.
 */

class Stack : public ::UniquelyIdentified, public Movable, public Ownable, public std::list<Army*>, public sigc::trackable
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
	 * deep copy of the stack's Army units.
	 */
        //! Copy constructor.
        Stack(Stack& s);

        //! Loading constructor.
        Stack(XML_Helper* helper);

	//! Destructor.
        ~Stack();

        //! Change the loyalty of the stack.
        void setPlayer(Player* p);

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
        void moveOneStep();

        void moveToDest(Vector<int> dest);

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

        //! Returns the minimum number of movement points of all Army units.
        Uint32 getGroupMoves() const;

	//! Returns true if all Army units in the stack are grouped.
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

        Army* getArmyById(Uint32 id) const;
        
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

        //! Return the defending status of the stack.
        bool getDefending() const {return d_defending;}

        //! Return the parked status of the stack.
        bool getParked() const {return d_parked;}

        //! Returns whether the stack is being deleted.
        bool getDeleting() const {return d_deleting;}

        //! Return the maximum sight of the stack.
        Uint32 getMaxSight() const;

        //! Erase the stack, deleting the Army units too.
        void flClear();

        /** 
	 * Erase an Army unit from the Stack, and free the contents of 
	 * the Army unit too (e.g. Items a Hero might be carrying).
	 *
	 * @param it   The place in the Stack to erase.
	 *
	 * @return The place in the stack that was erased.
         */
	//! Erase an Army unit from the list.
        iterator flErase(iterator object);

       /** 
	* Determine which terrain kinds (Tile::Type) the Stack can travel 
	* efficiently on.  When one Army unit is good at traveling through 
	* the forest, and another in the same stack is good at traveling 
	* through the hills, the movement capabilities of each individual 
	* army is given to the other Army units in the Stack.  This means 
	* the whole stack can move well through hills and forest.
	* Traveling efficently on a tile means it takes 2 movement points
	* to traverse.
	*
	* The calculation also takes into account a movement-changing Item
	* that the Hero may be carrying (e.g. `Wings of Flying', or 
	* `Swamp Boots').
	*
	* This calculation also lets Hero units `ride' flying Army units;
	* meaning that the Hero doesn't have the ability to fly, but it
	* has the special ability to ride on the back of another flying
	* Army unit.
	*
	* @return A bitwise OR-ing of the values in Tile::Type.
        */
	//! Calculate the move bonus for the Stack.
        Uint32 calculateMoveBonus() const;

	//! Calculate if the Stack has the gift of flight.
        bool isFlying () const;

	//! Calculate if the Stack is in a boat.
        bool hasShip () const;

	/**
	 * Calculate the number of movement points it costs for the Stack
	 * to move to an adjacent tile.
	 *
	 * @note This is not a distance calculation.
	 *
	 * @param pos    The adjacent tile to calculate the movement points for.
	 *
	 * @return The number of movement points, or -1 if moving to the
	 *         adjacent tile is impossible.
	 */
	//! Return the movement points it costs to travel to an adjacent tile.
	Uint32 calculateTileMovementCost(Vector<int> pos) const;

	//! Set each Army unit in the Stack to a grouped state.
	void group();

	//! Set all armies in the Stack except the first one to be ungrouped.
	void ungroup();

	/**
	 * Alter the order of the Army units in the stack according to each
	 * unit's groupedness, and fight order.
	 *
	 * The purpose of this sorting is to show the units in the stack
	 * info window.
	 *
	 * @param reverse     Invert the sort.
	 */
	//! Sort the Army units in the stack.
	void sortForViewing(bool reverse);

	//! Set all Army units in the stack to have this fortified state.
	void setFortified(bool fortified);

	//! Return true if any of the Army units in the stack are fortified.
	bool getFortified();
	
	//! Calculate the number of gold pieces this stack costs this turn.
	Uint32 getUpkeep();
        
	/**
	 * This comparator function compares the fight order of two Army units.
	 *
	 * @param left    An army that we want to sort by fight order.
	 * @param right   An army that we want to sort by fight order.
	 *
	 * @return True if the fight order of the left army is more than
	 *         the fight order of the right army.
	 */
	//! Comparator function to assist in sorting the armies in the stack.
	static bool armyCompareFightOrder (const Army *left, const Army *right);

	//! Emitted when a stack dies.
        sigc::signal<void, Stack*> sdying;

    private:    

        //! Callback for loading the stack
        bool load(std::string tag, XML_Helper* helper);
    
	//! Helper method for returning strongest army.
	Army* getStrongestArmy(bool hero) const;

        // DATA
	//! The stack's intended path.
        Path* d_path;
	//! Whether or not the stack is defending.
        bool d_defending;
	//! Whether or not the stack is parked.
        bool d_parked;
        
	/**
	 * True if the stack is currently being deleted. This is neccessary as
	 * some things may happen in the destructor of the contained armies and
	 * we don't want bigmap to draw the stack when it is being removed.
	 */
	//! Whether or not this stack is in the midst of being deleted.
        bool d_deleting;
};

#endif // STACK_H

// End of file
