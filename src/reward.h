//  Copyright (C) 2007, 2008 Ben Asselstine
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

#ifndef __REWARD_H
#define __REWARD_H
#include <SDL_types.h>
#include "vector.h"
#include "ruinlist.h"
#include <string>
class Player;
class Army;
class Location;
class Item;
class XML_Helper;
class Ruin;

//! A little something nice for the Player.
/**
 * Reward objects are given to the Player upon completion of a difficult
 * task.  Rewards are awarded when a ruin is successfully searched, or when a
 * Hero completes a Quest, or visits a sage.
 *
 * Rewards come in 5 flavours (Reward::Type): an amount of gold pieces, a
 * number of powerful allies, a useful item, a map that exposes part of a
 * hidden map when playing with fog-of-war, and also a hidden ruin that only
 * that Player who is awarded the Reward can search.
 *
 * This is the base class for each of the different kinds of rewards.  It
 * holds the kind of reward and the name of the reward.
 *
 */
class Reward
{
    public:

	//! The different kinds of Reward objects.
        enum Type {
	  //! A number of gold pieces.
	  GOLD = 1, 
	  //! A number of powerful allies.
	  ALLIES= 2, 
	  //! A useful item.
	  ITEM = 3, 
	  //! A hidden ruin that only the rewarded Player can see.
	  RUIN = 4, 
	  //! A portion of the hidden map to expose to the rewarded player.
	  MAP = 5
	};

        //! Default constructor.
	/**
	 * Make a new constructor of the given type and name.
	 *
	 * @param type  The kind of reward.
	 * @param name  The name of the reward.
	 *
	 * @note This constructor is only used in the constructors of other 
	 *       Reward objects, and shouldn't be called directly.
	 */
        Reward(Type type, std::string name = "");

	//! Loading constructor.
	/**
	 * Make a new Reward by reading it in from the opened saved-game file.
	 *
	 * @param helper  The opened saved-game file to read the Reward from.
	 *
	 * @note This constructor is only used within the constructors of
	 *       other Reward objects, and shouldn't be called directly.
	 *       It only loads the parts common to all Reward objects.
	 */
        Reward(XML_Helper* helper);

	//! Copy constructor.
	/**
	 * Make a new Reward by copying it from another Reward object.
	 *
	 * @param orig  The Reward object to copy it from.
	 *
	 * @note This constructor is only used within the constructors of
	 *       other Reward objects, and shouldn't be called directly.
	 *       It only copies the parts common to all Reward objects.
	 */
        Reward (const Reward& orig);

	//! Destructor.
        virtual ~Reward();

        //! Get the type of the reward.
        Type getType() const { return d_type; }

	//! Sets the name of the reward.
	void setName(std::string name) {d_name = name;}

        //! Returns the name of the reward.
        std::string getName() const {return d_name;}

	//! Generates a description of this reward.
	/**
	 * This method inspects the underlying reward and generates an
	 * appropriate description.
	 */
	std::string getDescription();

	//! Saves the data elements common to all rewards.
        /**
         * @note This function is called by the actual reward and only saves
         * the common data. It does NOT open/close tags etc. This has to be
         * done by the derived classes.
	 *
	 * @param helper  The opened saved-game file to save the Reward object
	 *                to.
         */
        virtual bool save(XML_Helper* helper) const;

	//! Assist in the loading of Rewards of all kinds.
        /**
         * Whenever an reward item is loaded, this function is called. It
         * examines the stored id and calls the constructor of the appropriate
         * reward class.
         *
         * @param helper   The opened saved-game file to load the Reward from.
         */
        static Reward* handle_load(XML_Helper* helper);

    protected:

        //! Type of the reward.
        Type d_type;

	//! The name of the reward.
	std::string d_name;

};

//! A Reward of some gold pieces.
/**
 * Gold pieces are added to the Player's treasury.
 */
class Reward_Gold : public Reward
{
    public:
	//! Default constructor. 
	/**
	 * @param gold  The number of gold pieces to award the Player.
	 */
        Reward_Gold(Uint32 gold);
	//! Loading constructor.
	Reward_Gold(XML_Helper *helper);
	//! Copy constructor.
	Reward_Gold(const Reward_Gold& orig);
	//! Destructor.
        ~Reward_Gold();

	//! Return a random number of gold pieces.
	/**
	 * This method provides a random number of gold pieces suitable for a
	 * reward in the game.
	 */
	static Uint32 getRandomGoldPieces();

	//! Save the gold reward to the opened saved-game file.
        bool save(XML_Helper* helper) const;

	//! Return the number of gold pieces associated with this reward.
	Uint32 getGold() const {return d_gold;}

    private:
	//! The number of gold pieces to award the player.
        Uint32 d_gold;
};

//! A number of powerful allies.
/**
 * Up to 8 allies are awarded to the Player's stack being given the Reward.
 * Allies are Army prototypes that have `Awardable' ability set.
 */
class Reward_Allies: public Reward
{
    public:
	//! Default constructor.  Make a new reward of allies.
	/**
	 * @param army  The Army prototype to create allies from.
	 * @param count The number of Army units to create from the prototype.
	 */
        Reward_Allies(const Army *army, Uint32 count);

	//! Secondary constructor.  Make a new reward of allies.
	/**
	 * @param army_type  The Id of the Army prototype to create allies from.
	 * @param army_set   The Id of the Armyset that the type belongs to.
	 * @param count      The number of Armies to create from the prototype.
	 */
        Reward_Allies(Uint32 army_type, Uint32 army_set, Uint32 count);

	//! Make a new reward of allies from another one.
	Reward_Allies(const Reward_Allies& orig);

	//! Loading constructor.  Load the allies reward from a saved-game file.
	Reward_Allies(XML_Helper *helper);

	//! Destructor.
        ~Reward_Allies();

	//! Save the allies reward to the opened saved-game file.
        bool save(XML_Helper* helper) const;

	//! Return the army prototype of the allies associated with this reward.
	const Army * getArmy() const {return d_army;}

	//! Return the number allies that this reward will create.
	Uint32 getNoOfAllies() const {return d_count;}

	//! A static method that returns a random awardable Army prototype.
        static const Army* randomArmyAlly();

	//! A static method that returns a number of allies between 1 and 8.
	static const Uint32 getRandomAmountOfAllies();

	//! A static method for adding allies to the game map.
	/**
	 * Place the given number of the given allies onto the map at the given
	 * position.
	 *
	 * @param p           The player to make the new stack for, if the 
	 *                    stack can't take all of the allies.
	 * @param pos         The place on the map to add the allies.
	 * @param army        The Army prototype that defines the allies.
	 * @param alliesCount The number of allies to add.
	 *
	 * @return True if the armies could successfully be added to the game
	 *         map.  Returns false otherwise.
	 */
        static bool addAllies(Player *p, Vector<int> pos, const Army *army, Uint32 alliesCount);

	//! A static method for adding allies to the game map.
	/**
	 * Place the given number of the given allies onto the map at the given
	 * location.
	 *
	 * @param p           The player to make the new stack for, if the 
	 *                    stack can't take all of the allies.
	 * @param loc         The place on the map to add the allies.
	 * @param army        The Army prototype that defines the allies.
	 * @param alliesCount The number of allies to add.
	 *
	 * @note This method tries to add the armies to the various tiles of
	 *       the location first, before placing it outside of the location.
	 *
	 * @return True if the armies could successfully be added to the game
	 *         map.  Returns false otherwise.
	 */
        static bool addAllies(Player *p, Location *l, const Army *army, Uint32 alliesCount);

    private:
	//! The Army prototype that represents the allies to give the Player.
        const Army *d_army;
	//! The army type of the given prototype.
	Uint32 d_army_type;
	//! The army set of the given prototype.
	Uint32 d_army_set;
	//! The number of allies to give the Player.
        Uint32 d_count;
};

class Reward_Item: public Reward
{
    public:
        Reward_Item (Item *item);
	Reward_Item(XML_Helper *helper);
	Reward_Item(const Reward_Item& orig);
        ~Reward_Item();

	static Item *getRandomItem();

        bool save(XML_Helper* helper) const;
	Item *getItem() const {return d_item;}

    private:
        bool loadItem(std::string tag, XML_Helper* helper);
        Item *d_item;
};

class Reward_Ruin: public Reward
{
    public:
        Reward_Ruin(Ruin *ruin);
	Reward_Ruin(XML_Helper *helper);
	Reward_Ruin(const Reward_Ruin& orig);
        ~Reward_Ruin();
	static Ruin *getRandomHiddenRuin();

        bool save(XML_Helper* helper) const;
	Ruin* getRuin() const {return Ruinlist::getInstance()->getObjectAt(d_ruin_pos);}

    private:
	Vector<int> d_ruin_pos;
};

class Reward_Map: public Reward, public Location
{
    public:
        Reward_Map(Vector<int> pos, std::string name, 
		   Uint32 height, Uint32 width);
	Reward_Map(XML_Helper *helper);
	Reward_Map(const Reward_Map& orig);
        ~Reward_Map();

        bool save(XML_Helper* helper) const;
	Uint32 getHeight() const {return d_height;}
	Uint32 getWidth() const {return d_width;}
	static void getRandomMap(int *x, int *y, int *width, int *height);

    private:
	Uint32 d_height;
	Uint32 d_width;
};

#endif
