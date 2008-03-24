// Copyright (C) 2001, 2003 Michael Bartl
// Copyright (C) 2002, 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2007, 2008 Ben Asselstine
//
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#ifndef RUIN_H
#define RUIN_H

#define DEFAULT_RUIN_NAME  ""
#include <string>
#include <sigc++/trackable.h>
#include "NamedLocation.h"
#include "stack.h"
#include "Namable.h"

class Stack;
class Reward;

//! A ruin on the game map.
/** 
 * A ruin is a simple feature on the map which contains an id, a flag whether 
 * it has already been searched and optionally an occupant (called "keeper").
 * If a ruin is searched, the player starts a fight with the keeper. If the
 * player wins, the ruin becomes searched and the player gets a reward.
 *
 * Sometimes a ruin contains a sage which lets the player choose from a 
 * variety of rewards.
 *
 * Sometimes a ruin is hidden to all players except one player.
 */
class Ruin : public NamedLocation, public sigc::trackable
{
    public:
	//! The kind of ruin.
        enum Type {
	  //! A normal ruin.
	  RUIN = 0, 
	  //! A stronghold ruin is a little stronger.
	  STRONGHOLD = 1
	};
	//! Default constructor.
        /** 
          * @param pos          The location of the ruin.
          * @param name         The name of the ruin.
          * @param occupant     The monster occupying the ruin.
          * @param searched     Sets the searchedness flag of the ruin.
	  * @param hidden       Sets the hidden flag of the ruin.
	  * @param owner        Who can see this hidden ruin.
	  * @param sage         if this ruin contains a sage or not.
          */
        Ruin(Vector<int> pos, std::string name = DEFAULT_RUIN_NAME, 
	     int type = Ruin::RUIN, Stack* occupant = 0, 
	     bool searched = false, bool hidden = false, Player *owner = 0, 
	     bool sage = false);

        //! Copy constructor.
        Ruin(const Ruin&);

        //! Loading constructor.
	/**
	 * @param helper  The opened saved-game file to load the ruin from.
	 */
        Ruin(XML_Helper* helper);
	//!Destructor.
        ~Ruin();

        //! Returns the type of the ruin.
        int getType() {return d_type;};

        //! Sets the type of the ruin.
        void setType(int type) {d_type = type;};

        //! Change whether or not the ruin has been successfully searched.
        void setSearched(bool searched) {d_searched = searched; 
	  d_reward = NULL;}
        
        //! Set the keeper of the ruin.
        void setOccupant(Stack* occupant) {d_occupant = occupant;}
        
        //! Return whether or not the ruin has been searched already.
        bool isSearched() const {return d_searched;}

        //! Returns the keeper that guards the ruin from Hero units.
        Stack* getOccupant() const {return d_occupant;}

	//! Returns whether or not this is a "hidden" ruin.
	bool isHidden() const {return d_hidden;}

        //! Change the "hidden" flag of the ruin.
        void setHidden (bool hidden) {d_hidden = hidden;}

	//! Returns whether or not this ruin has a sage.
	bool hasSage() const {return d_sage;}

	//! Sets whether or not this ruin has a sage.
	void setSage(bool sage) {d_sage = sage;}

	//! Returns the player that owns this hidden ruin.
	Player *getOwner() const {return d_owner;}

	//! Sets the player that owns this hidden ruin.
	void setOwner(Player *owner) {d_owner = owner;}

        //! Callback for loading the ruin data.
        bool load(std::string tag, XML_Helper* helper);

	//! Returns the reward for this ruin.
	Reward *getReward() const {return d_reward;}

	//! Sets the reward for this ruin.
	void setReward(Reward *r) {d_reward = r;}

        //! Saves the ruin data to an opened saved-game file.
        bool save(XML_Helper* helper) const;

	//! Put a random reward in this ruin.
	/**
	 * @note This method does not remove an existing reward before putting
	 *       a new one in it.
	 */
	void populateWithRandomReward();

    private:
        // DATA
	//! Whether or not the Ruin has already successfully been searched.
        bool d_searched;

	//! The type of the ruin.
        bool d_type;

	//! The keeper of the ruin.
	/**
	 * The Hero unit fights this stack when it is searched.  The stack
	 * consists of a single Army unit that is cabable of defending ruins.
	 */
        Stack* d_occupant;

	//! Whether or not the ruin is a hidden ruin.
	/**
	 * Hidden ruins are rewards.  Only a certain player can see the Ruin.
	 */
	bool d_hidden;

	//! The player who can see the hidden ruin.
	/**
	 * Owning a ruin only makes sense if it is a hidden ruin.
	 * Only this player can see the hidden ruin, although other players
	 * may occupy the same tile.
	 */
	Player *d_owner;

	//! The ruin contains a sage.
	/**
	 * A Sage offers the Hero the choice of many rewards.
	 */
	bool d_sage;

	//! The reward to give if the Hero is successful in beating the keeper.
	Reward *d_reward;
};

#endif // RUIN_H

