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

#include "xmlhelper.h"

class Player;
class Path;
class Army;

/** Group of up to eight armies
  * 
  * While armies are the actual troops you command, they always belong to a
  * stack. The stack holds these armies together in one object. The player
  * usually doesn't command the armies but the stack, so all functionality and
  * data which affects player's controls is bundled in the stack class. Among
  * this is the location of the units, the movement etc.
  */

class Stack : public std::list<Army*>, public sigc::trackable
{
    public:
        /** Default constructor
          * 
          * @param player       the owning player or 0 if e.g. ruin keeper
          * @param pos          the position where the stack is created
          */
        Stack(Player* player, Vector<int> pos);

        //! Copy constructor, it does a deep copy of the other stack's armies!
        Stack(Stack& s);

        //! Loading constructor. See XML_Helper for details
        Stack(XML_Helper* helper);
        ~Stack();

        
        //! In rare cases it may be wanted to change the stack's loyality...
        void setPlayer(Player* p);

        //! ...or its position (for testing reasons)
        void setPosition(Vector<int> pos){d_pos = pos;}

        /** Sets the defending value. Defending means that this stack is ignored
          * when a player cycles through his list of stacks with Stack::setNext().
          */
        void setDefending(bool defending){d_defending = defending;}


        //! Save the stack's data. See XML_Helper for more details.
        bool save(XML_Helper* helper) const;

        //! Heals the armies of the stack etc.
        void nextTurn();

        //! Reduces movement points of the armies.
        void decrementMoves(Uint32 moves);

        //! Sets the stack's position to the next item of the internal path
        bool moveOneStep();

        //! Resets the moves of all armies to their maximum
        void resetMoves();

        //! Blesses all armies of the stack (strength +1)
	//! Returns the number of blessed armies.
        int bless();

        //! Returns whether the stack has enough moves for the next step
        bool enoughMoves() const;

	// returns whether the stack can move in any direction
	bool canMove() const;
        
        //! Returns the unique id of the stack
        Uint32 getId() const {return d_id;}

        //! Returns the owning player
        Player* getPlayer() const {return d_player;}

        //! Returns the location of the stack
        Vector<int> getPos() const {return d_pos;}

        //! Returns the internal path object of the stack
        Path* getPath() const {return d_path;}

        //! Returns the minimum number of MP of all armies
        Uint32 getGroupMoves() const;

        //! Returns the minimum number of MP of all tiles around the stack, or
        // -1 if the stack can't move
        int getMinTileMoves() const;

        //! Get the next item of the stack's path
        Vector<int> nextStep();

        //! Get the strongest army (most strength) for displaying
        Army* getStrongestArmy() const;

        //! Used for splitting stacks. See Player::stackSplit how it works.
        Army* getFirstUngroupedArmy() const;

        //! True if the stack has a hero. They add strength to the other armies.
        bool hasHero() const;

        //! Return the first hero in the stack
        Army* getFirstHero() const;

        //! Returns the ids of all (living) heroes in the stack in the dst reference
        void getHeroes(std::vector<Uint32>& dst) const;

        //! Return true if the stack belongs to player or his allies
        bool isFriend(Player* player) const;

        //! Return the defending status of the stack (see setDefending)
        bool getDefending() const {return d_defending;}

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


        sigc::signal<void, Stack*> sdying;

	void selectAll();
        
    private:    
        //! Callback for loading the stack
        bool load(std::string tag, XML_Helper* helper);
    
        // DATA
        Uint32 d_id;
        Player* d_player;
        Path* d_path;
        bool d_defending;
        Vector<int> d_pos;
        
        // true if the stack is currently being deleted. This is neccessary as
        // some things may happen in the destructor of the contained armies and
        // we don't want bigmap to draw the stack when it is being removed.
        bool d_deleting;
};

#endif // STACK_H

// End of file
