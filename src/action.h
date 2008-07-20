// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2007, 2008 Ben Asselstine
// Copyright (C) 2008 Ole Laursen
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

#ifndef ACTION_H
#define ACTION_H

#include <string>
#include "vector.h"
#include <sigc++/trackable.h>

#include "fight.h"
#include "army.h"
#include "reward.h"
#include "player.h"
#include "Ownable.h"

class Quest;
class Stack;
class City;
class Signpost;
class Ruin;
class Temple;
class XML_Helper;

//! A temporary record of an event during gameplay.
/** 
 * The purpose of the action classes is to keep track of what a player has
 * done. This information can be sent over the network, so that a networked 
 * player then just has to decode and repeat the remote player's actions so
 * that the game state is synchronised.
 * 
 * Each action item is derived from the abstract Action class. It must
 * contain three functions; A loading constructor (which takes an 
 * XML_Helper parameter), a save function which saves the data to the
 * XML file, and a fillData function which takes some parameters and 
 * stores the pertinent data about what happened.
 *
 * Each Player has an Actionlist to which these actions belong.
 */

class Action
{
    public:
	//! An Action can be one of the following kinds.
        enum Type {
	        /** A stack has moved. */
                STACK_MOVE = 1,
		/** A stack has separated into two parts. */
                STACK_SPLIT = 2,
		/** A stack is fighting a city or another stack. */
                STACK_FIGHT = 3,
		/** A stack has merged with another stack. */
                STACK_JOIN = 4,
		/** A stack containing a hero has examined a ruin. */
                RUIN_SEARCH = 5,
		/** A stack has examined a temple. */
                TEMPLE_SEARCH = 6,
		/** A stack has defeated a city and occupied it. */
                CITY_OCCUPY = 7,
		/** A stack has defeated a city and pillaged it. */
                CITY_PILLAGE = 8,
		/** A stack has defeated a city and razed it. */
                CITY_RAZE = 9,
		/** A player has improved the defenses of a city. (Not used) */
                CITY_UPGRADE = 10,
		/** A player has purchased a new Army unit to be produced 
		 * in a city. */
                CITY_BUY = 11,
		/** A player has changed production in a city to another 
		 * Army unit.*/
                CITY_PROD = 12,
		/** A stack has received a reward. */
                REWARD = 13,
		/** A hero has received a new quest. */
                QUEST = 14,
		/** A hero has picked up or dropped an item. */
                HERO_EQUIP = 15,
		/** An Army unit has advanced to a new level (Only used for 
		 * heroes). */
                UNIT_ADVANCE = 16,
		/** A stack has defeated a city and sacked it. */
                CITY_SACK = 17,
		/** A player has removed a stack. */
                STACK_DISBAND = 18,
		/** A player has changed what a signpost says. */
                MODIFY_SIGNPOST = 19,
		/** A player has changed the name of a city. */
                CITY_RENAME = 20,
		/** A player has vectored Army units from one city to
		 * another. */
                CITY_VECTOR = 21,
		/** A player has changed the order in which Army units do
		 * battle. */
                FIGHT_ORDER = 22,
		/** A player has surrendered. */
		RESIGN = 23,
		/** A hero has planted an item in the ground. */
		ITEM_PLANT = 24,
		/** A newly produced Army unit arrives on the map. */
		PRODUCE_UNIT = 25,
		/** A new vectored Army unit has shown up at a location. */
		PRODUCE_VECTORED_UNIT = 26,
		/** The player's diplomatic relations with respect to another
		 * player has changed. */
		DIPLOMATIC_STATE = 27,
		/** The player's diplomatic proposal with respect to another
		 * player has changed. */
		DIPLOMATIC_PROPOSAL = 28,
		/** The player's diplomatic score with respect to another
		 * player has changed. */
		DIPLOMATIC_SCORE = 29,
                END_TURN = 30,
                CITY_CONQUER = 31,
                RECRUIT_HERO = 32,
                PLAYER_RENAME = 33,
		CITY_DESTITUTE = 34
        };
	static std::string actionTypeToString(Action::Type type);
	static Action::Type actionTypeFromString(std::string str);

	//! Default constructor.
        Action(Type type);

	//! Copy constructor (shallow).
	Action(const Action &action);

	//! Loading constructor.
        Action(XML_Helper *helper);

	//! Destructor.
        virtual ~Action();

        //! Returns debug information. Needs to be overwritten by derivatives.
        virtual std::string dump() const = 0;

        //! Save function. See XML_Helper for information about saving.
        bool save(XML_Helper* helper) const;
	bool saveContents(XML_Helper* helper) const;
        
        /** 
	 * static load function (see XML_Helper)
         * 
         * Whenever an action item is loaded, this function is called. It
         * examines the stored id and calls the constructor of the appropriate
         * action class.
         *
         * @param helper       the XML_Helper instance for the savegame
         */
	//! Load the action from an opened saved-game file.
        static Action* handle_load(XML_Helper* helper);

        //! Make a new action from an existing one.
        static Action* copy(const Action* a);

        //! Returns the Action::Type for this action.
        Type getType() const {return d_type;}

    protected:
        virtual bool doSave(XML_Helper* helper) const = 0;
        
        Type d_type;
};

//-----------------------------------------------------------------------------

//! A temporary record of a Stack moving.
/**
 * The purpose of the Action_Move class is to record when a stack has
 * moved to a new position on the map.
 */
class Action_Move : public Action
{
    public:
	//! Make a new move action.
        Action_Move();
	//! Copy constructor
        Action_Move(const Action_Move &action);
	//! Load a new move action from an opened saved-game file.
        Action_Move(XML_Helper* helper);
	//! Destroy a move action.
        ~Action_Move();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this move action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the move action with the stack and it's new position.
        bool fillData(Stack* s, Vector<int> dest);
    
	Uint32 getStackId() const {return d_stack;};
	Vector<int> getEndingPosition() const {return d_dest;};

        private:
        Uint32 d_stack;
        Vector<int> d_dest;
};

//! A temporary record of a Stack being disbanded.
/**
 * The purpose of the Action_Disband class is to record when a stack has
 * removed from the game.  Disbanding is done to primarily to save the 
 * gold pieces paid out every turn as upkeep for the Army units in the Stack.
 */
class Action_Disband: public Action
{
    public:
	//! Make a new disband action.
        Action_Disband();
	//! Copy constructor
	Action_Disband(const Action_Disband &action);
	//! Load a new disband action from an opened saved-game file.
        Action_Disband(XML_Helper* helper);
	//! Destroy a disband action.
        ~Action_Disband();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this disband action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the action with the Stack being removed.
        bool fillData(Stack* s);
    
	Uint32 getStackId() const {return d_stack;};

        private:
        Uint32 d_stack;
};

//-----------------------------------------------------------------------------

//! A temporary record of a Stack being split into two.
/**
 * The purpose of the Action_Split class is to record when a Stack has been
 * separated into two parts.  This happens when the Player groups only some
 * of the Army units in the stack (not all), and then moves them to a new 
 * position on the map.  When a split is completed there is the original
 * stack (the remaining Army units in the stack that didn't change position), 
 * and a new stack added to the game (the stack containing the grouped army 
 * units).
 */
class Action_Split : public Action
{
    public:
	//! Make a new stack split action.
        Action_Split();
	//! Copy constructor
	Action_Split(const Action_Split &action);
	//! Load a new stack split action from an opened saved-game file.
        Action_Split(XML_Helper* helper);
	//! Destroy a stack split action.
        ~Action_Split();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this split action to an opened saved-game file.
        bool doSave (XML_Helper* helper) const;

        /** 
	 * Populate the Action_Split class with the original Stack, and the 
	 * new stack that has been added to the game.  Please note that the 
	 * stacks have to be already split before this call.
	 */
	//! Populate the action with pertinent data.
        bool fillData(Stack* orig, Stack* added);
    
	Uint32 getStackId() const {return d_orig;};
	Uint32 getNewStackId() const {return d_added;};
	Uint32 getGroupedArmyId(int idx) const {return d_armies_moved[idx];};
        private:
        Uint32 d_orig, d_added;
        Uint32 d_armies_moved[MAX_STACK_SIZE];
};

//-----------------------------------------------------------------------------

//! A temporary record of a fight between opposing Stack objects.
/**
 * The purpose of the Action_Fight class is to record the results of a
 * fight between two Players.
 */
class Action_Fight : public Action, public sigc::trackable
{
    public:
	//! Make a new fight action.
        Action_Fight();
	//! Copy constructor
	Action_Fight(const Action_Fight &action);
	//! Load a new fight action from an opened saved-game file.
        Action_Fight(XML_Helper* helper);
	//! Destroy a fight action.
        ~Action_Fight();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this fight action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	/**
	 * Populate the action with the Fight.  Please note that the
	 * Fight must have already been faught.
	 */
	//! Fill the action with pertinent data.
        bool fillData(const Fight* f);


	std::list<FightItem> getBattleHistory() const {return d_history;};
	std::list<Uint32> getAttackerArmyIds() const {return d_attackers;};
	std::list<Uint32> getDefenderArmyIds() const {return d_defenders;};

        private:
        
        std::list<FightItem> d_history;
        std::list<Uint32> d_attackers;
        std::list<Uint32> d_defenders;

        bool loadItem(std::string tag, XML_Helper* helper);
};

//-----------------------------------------------------------------------------

//! A temporary record of two Stack objects merging into one.
/**
 * The purpose of the Action_Join class is to record a Stack has had 
 * another Stack merged into it.
 */
class Action_Join : public Action
{
    public:
	//! Make a new stack join action.
        Action_Join();
	//! Copy constructor
	Action_Join(const Action_Join &action);
	//! Load a new stack join action from an opened saved-game file.
        Action_Join(XML_Helper* helper);
	//! Destroy a stack join action.
        ~Action_Join();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this stack join action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

        /** 
	 * Populate the Action_Join class with the original Stack, and the 
	 * stack that is merging into the original one.  Please note that
	 * this method must be called before the merge takes place.
	 */
	//! Populate the action with pertinent data.
        bool fillData(Stack* orig, Stack* joining);
    
	Uint32 getReceivingStackId() const {return d_orig_id;};
	Uint32 getJoiningStackId() const {return d_joining_id;};
        private:
        Uint32 d_orig_id, d_joining_id;
};

//-----------------------------------------------------------------------------

//! A temporary record of what happened when a Stack searched a Ruin.
/**
 * The purpose of the Action_Ruin class is to record what happens when a
 * Stack containing a Hero attempts to search a Ruin.
 */
class Action_Ruin : public Action
{
    public:
	//! Make a new ruin search attempted action.
        Action_Ruin();
	//! Copy constructor
	Action_Ruin(const Action_Ruin&action);
	//! Load a new ruin search attempted action from a saved-game file.
        Action_Ruin(XML_Helper* helper);
	//! Destroy a ruin search attempted action.
        ~Action_Ruin();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this ruin search attempted action to a saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	/**
	 * Populate the Action_Ruin class with the Stack containing the
	 * Hero Army unit, and the Ruin being searched.
	 */
	//! Populate the action with pertinent data.
        bool fillData(Ruin* r, Stack* explorers);

	//! Set whether or not the Stack was successful in searching the ruin.
        void setSearched(bool searched) {d_searched = searched;}
    
	Uint32 getRuinId() const {return d_ruin;};
	Uint32 getStackId() const {return d_stack;};
	bool getSearchSuccessful() const {return d_searched;};

        private:
        Uint32 d_ruin;
        Uint32 d_stack;
        bool d_searched;
};

//-----------------------------------------------------------------------------

//! A temporary record of what happened when a Stack visited a Temple.
/**
 * The purpose of the Action_Temple class is to record what happens when
 * a Stack visits a Temple.  The Stack may be getting blessed, or instead it
 * might be a Stack containing a Hero who is obtaining a new Quest.
 */
class Action_Temple : public Action
{
    public:
	//! Make a new temple search action.
        Action_Temple();
	//! Copy constructor
	Action_Temple(const Action_Temple &action);
	//! Load a new temple search action from a saved-game file.
        Action_Temple(XML_Helper* helper);
	//! Destroy a temple search action.
        ~Action_Temple();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this temple search attempted action to a saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	/**
	 * Populate the Action_Temple class with the Stack and the Temple
	 * being searched.
	 */
	//! Populate the action with pertinent data.
        bool fillData(Temple* t, Stack* s);
    
	Uint32 getTempleId() const {return d_temple;};
	Uint32 getStackId() const {return d_stack;};

        private:
        Uint32 d_temple;
        Uint32 d_stack;
};


//-----------------------------------------------------------------------------

//! A temporary record of what happened when a Player occupied a City.
/**
 * The purpose of the Action_Occupy class is to record when a Player has
 * defeated a City and has occupied it.  Ocuppying differs from sacking
 * and pillaging in that none of the existing Army units in the City are
 * exchanged for gold pieces.
 */
class Action_Occupy : public Action
{
    public:
	//! Make a new city occupy action.
        Action_Occupy();
	//! Copy constructor.
	Action_Occupy(const Action_Occupy &action);
	//! Load a new city occupied action from an opened saved-game file.
        Action_Occupy(XML_Helper* helper);
	//! Destroy a city occupy action.
        ~Action_Occupy();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this city occupied action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the action with the City being occupied.
        bool fillData (City* c);
    
	Uint32 getCityId() const {return d_city;};

        private:
        Uint32 d_city;
};

//-----------------------------------------------------------------------------

//! A temporary record of what happened when a Player pillaged a City.
/**
 * The purpose of the Action_Pillage class is to record when a Player has
 * defeated a City and has pillaged it.  Pillaging a city results in the
 * strongest Army unit being produced in that city being exchanged for an 
 * amount of gold pieces.
 */
class Action_Pillage : public Action
{
    public:
	//! Make a new city pillaged action.
        Action_Pillage();
	//! Copy constructor
	Action_Pillage(const Action_Pillage &action);
	//! Load a new city pillaged action from an opened saved-game file.
        Action_Pillage(XML_Helper* helper);
	//! Destroy a city pillaged action.
        ~Action_Pillage();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this city pillaged action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the action with the City that has been pillaged.
        bool fillData(City* c);

	Uint32 getCityId() const {return d_city;};

        private:
        Uint32 d_city;
};

//-----------------------------------------------------------------------------

//! A temporary record of what happened when a Player sacked a City.
/**
 * The purpose of the Action_Sack class is to record when a Player has
 * defeated a City and has sacked it.  Sacking a city results in all 
 * Army units, except the weakest being exchanged for an amount of gold 
 * pieces.
 */
class Action_Sack : public Action
{
    public:
	//! Make a new city sacked action.
        Action_Sack();
	//! Copy constructor
	Action_Sack(const Action_Sack &action);
	//! Load a new city sacked action from an opened saved-game file.
        Action_Sack(XML_Helper* helper);
	//! Destroy a city sacked action.
        ~Action_Sack();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this city sacked action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the action with the City that has been sacked.
        bool fillData(City* c);

	Uint32 getCityId() const {return d_city;};

        private:
        Uint32 d_city;
};

//-----------------------------------------------------------------------------

//! A temporary record of what happened when a Player razed a City.
/**
 * The purpose of the Action_Raze class is to record when a Player has
 * defeated a City and has razed it.  Razing a city results in that
 * city becoming uninhabitable, and it ceases to produce new Army units.
 */
class Action_Raze : public Action
{
    public:
	//! Make a new city razed action.
        Action_Raze();
	//! Copy constructor
	Action_Raze(const Action_Raze &action);
	//! Load a new city razed action from an opened saved-game file.
        Action_Raze(XML_Helper* helper);
	//! Destroy a city razed action.
        ~Action_Raze();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this city razed action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the action with the City that has been razed.
        bool fillData (City* c);
    
	Uint32 getCityId() const {return d_city;};

        private:
        Uint32 d_city;
};

//-----------------------------------------------------------------------------

//! A temporary record of what happened when a City's defenses were increased.
/**
 * The purpose of the Action_Upgrade class is to record when a Player has
 * improved the City's defenses.  This action is not currently used by 
 * LordsAWar.
 */
class Action_Upgrade : public Action
{
    public:
	//! Make a new city upgraded action.
        Action_Upgrade();
	//! Copy constructor
	Action_Upgrade(const Action_Upgrade &action);
	//! Load a new city upgraded action from an opened saved-game file.
        Action_Upgrade(XML_Helper* helper);
	//! Destroy a city upgraded action.
        ~Action_Upgrade();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this city upgraded action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the action with the City that has been upgraded.
        bool fillData(City* c);

	Uint32 getCityId() const {return d_city;};

        private:
        Uint32 d_city;
};

//-----------------------------------------------------------------------------

//! A temporary record of more production being added to a City.
/**
 * The purpose of the Action_Buy class is to record when a Player purchases
 * a new Army unit for production in a City.  When this happens the player
 * specifies a production slot to hold the new Army unit type.  Any existing
 * Army unit type in that slot before the buy is removed.
 * The idea here is that the city may produce one Army unit type from a set 
 * of available Army units, and this action records what happens when we make 
 * a new kind of Army unit available for production in the City.
 * The army unit is taken from the Player's Armyset.
 */
class Action_Buy : public Action
{
    public:
	//! Make a new city buy production action.
        Action_Buy();
	//! Copy constructor
	Action_Buy(const Action_Buy &action);
	//! Load a new city buy production action from a saved-game file.
        Action_Buy(XML_Helper* helper);
	//! Destroy a city buy production action.
        ~Action_Buy();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this city buy production action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	/**
	 * Populate the Action_Buy with City where the buy has happened.
	 * Also populate it with the City's production slot that is now
	 * producing the new Army unit type.  Lastly, populate the Action_Buy 
	 * with the new Army unit type being produced.
	 */
	//! Populate the action with pertinent data.
        bool fillData(City* c, int slot, const Army *prod);

	Uint32 getCityId() const {return d_city;};
	int getProductionSlot() const {return d_slot;};
	int getBoughtArmyTypeId() const {return d_prod;};
        private:
        Uint32 d_city;
        int d_slot, d_prod;
};

//-----------------------------------------------------------------------------

//! A temporary record of a change in production strategy in a City.
/**
 * The purpose of the Action_Production class is to record when the Player
 * changes the production of new Army units within a City.  The idea here
 * is that the City has a set of available Army units that it may produce,
 * and the Player has selected a different Army unit to produce, or has
 * stopped production of new Army units altogether.
 */
class Action_Production : public Action
{
    public:
	//! Make a new city change production action.
        Action_Production();
	//! Copy constructor
	Action_Production (const Action_Production &action);
	//! Load a new city change production action from a saved-game file.
        Action_Production(XML_Helper* helper);
	//! Destroy a city change production action.
        ~Action_Production();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this city change production action to a saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	/**
	 * Populate the Action_Production with City where the change
	 * in production has taken place.  Also populate it with the newly
	 * active production slot (-1 means stopped).
	 */
	//! Populate the action with pertinent data.
        bool fillData(City* c, int slot);

	Uint32 getCityId() const {return d_city;};
	int getSlot() const {return d_prod;};

        private:
        Uint32 d_city;
        int d_prod;
};

//-----------------------------------------------------------------------------

//! A temporary record of a Player or Stack getting a Reward.
/**
 * The purpose of the Action_Reward class is to record when the Player
 * has been given a Reward.
 * It could be that the player has been given: Some gold pieces, a map that 
 * makes more of the map visible or information about the location of a new
 * ruin.  It could also be that the player's active stack has been given
 * a number of allies.  It could also be that the Player's active Stack
 * contains a Hero, and the Reward is an Item for the Hero to carry.
 */
class Action_Reward : public Action
{
    public:
	//! Make a new player rewarded action.
        Action_Reward();
	//! Copy constructor
	Action_Reward (const Action_Reward &action);
	//! Load a new player rewarded action from a saved-game file.
        Action_Reward(XML_Helper* helper);
	//! Destroy a player rewarded action.
        ~Action_Reward();
        
	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this player rewarded action to a saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the Action_Reward with a Reward.
        bool fillData (Stack *stack, Reward *r);
    

	Reward *getReward() const {return d_reward;};
	Uint32 getStackId() const {return d_stack;};

        private:
	Reward *d_reward;
        Uint32 d_stack;

        bool load(std::string tag, XML_Helper *helper);
	
};

//-----------------------------------------------------------------------------

//! A temporary record of a Hero initiating a new Quest.
/**
 * The purpose of the Action_Quest class is to record when a Player's
 * Hero has gone to a Temple and initiated a new Quest.
 */
class Action_Quest : public Action
{
    public:
	//! Make a new hero quest assigned action.
        Action_Quest();
	//! Copy constructor
	Action_Quest (const Action_Quest &action);
	//! Load a new hero quest assigned action from a saved-game file.
        Action_Quest(XML_Helper* helper);
	//! Destroy a hero quest assigned action.
        ~Action_Quest();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this hero quest assigned action to a saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the Action_Quest with a Quest.
        bool fillData(Quest* q);

	Uint32 getHeroId() const {return d_hero;};
	Uint32 getQuestType() const {return d_questtype;};
	Uint32 getData() const {return d_data;};
	Uint32 getVictimPlayerId() const {return d_victim_player;};

        private:
        Uint32 d_hero;
        Uint32 d_questtype;
        Uint32 d_data;
	Uint32 d_victim_player; //victim player, only KILLARMIES uses this
};

//-----------------------------------------------------------------------------

//! A temporary record of a Hero picking up or dropping an Item.
/**
 * The purpose of the Action_Equip class is to record when a Player's Hero
 * has picked up an Item or dropped it onto the ground.  Heroes pick up
 * and drop Items one at a time.
 */
class Action_Equip : public Action
{
    public:
        enum Slot {
	    // The Item is going neither to the ground or the backpack.
            NONE = 0,
	    //! The Item has gone into the Hero's Backpack.
            BACKPACK = 1,
	    //! The Item has been dropped onto the ground.
            GROUND = 2};
        
	//! Make a new item equipped action.
        Action_Equip();
	//! Copy constructor
	Action_Equip (const Action_Equip &action);
	//! Load a new item equipped action from an opened saved-game file.
        Action_Equip(XML_Helper* helper);
	//! Destroy an item equipped action.
        ~Action_Equip();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this item equipped action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	/**
	 * Populate the Action_Equip class with the Id of the Hero,
	 * the Id of the Item and the destination of the Item in terms of
	 * it's Action_Equip::Slot.
	 */
	//! Populate the action with pertinent data.
        bool fillData(Hero *hero, Item *item, Slot slot);

	Uint32 getHeroId() const {return d_hero;};
	Uint32 getItemId() const {return d_item;};
	Uint32 getToBackpackOrToGround() const {return d_slot;};

        private:
        Uint32 d_hero;
        Uint32 d_item;
        Uint32 d_slot;
};

//-----------------------------------------------------------------------------

//! A temporary record of a Hero gaining a new level.
/**
 * The purpose of the Action_Level class is to record when a Player's Hero
 * advances a level and subsequently gains a stat.
 * Stats that may get increased are: Strength, Moves, and Sight (for use on 
 * a hidden map).
 */
class Action_Level : public Action
{
    public:
	//! Make a new level advancement action.
        Action_Level();
	//! Copy constructor
	Action_Level (const Action_Level &action);
	//! Load a new level advancement action from an opened saved-game file.
        Action_Level(XML_Helper* helper);
	//! Destroy a level advancement action.
        ~Action_Level();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this level advancement action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	/**
	 * Populate the Action_Level class with the the Id of the Hero
	 * Army unit, and also the Hero's stat that has been raised as a
	 * result of the level advancement.
	 */
	//! Populate the action with pertinent data.
        bool fillData(Army *unit, Army::Stat raised);

	Uint32 getArmyId() const {return d_army;};
	Uint32 getStatToIncrease() const {return d_stat;};

        private:
        Uint32 d_army;
        Uint32 d_stat;
};

//-----------------------------------------------------------------------------

//! A temporary record of a Player changing the contents of a Signpost.
/**
 * The purpose of the Action_ModifySignpost is to record when a Signpost
 * has been altered by a player to have a different message on it.  The
 * idea here is that we're playing on a hidden map and a Player wants to
 * thwart an opponent by changing what signs say before he can read them.
 */
class Action_ModifySignpost: public Action
{
    public:
	//! Make a new change signpost action.
        Action_ModifySignpost();
	//! Copy constructor
	Action_ModifySignpost(const Action_ModifySignpost &action);
	//! Load a new change signpost action from an opened saved-game file.
        Action_ModifySignpost(XML_Helper* helper);
	//! Destroy a change signpost action.
        ~Action_ModifySignpost();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this change signpost action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the action with the signpost and the new message.
        bool fillData(Signpost * s, std::string message);
    
	Uint32 getSignpostId() const {return d_signpost;};
	std::string getSignContents() const {return d_message;};

        private:
        Uint32 d_signpost;
	std::string d_message;
};

//-----------------------------------------------------------------------------

//! A temporary record of a Player changing the name of a City.
/**
 * The purpose of the Action_RenameCity class is to record when a Player has
 * changed the name of a City.  The idea here is that a Player wants
 * to gloat by renaming a newly conquered City.
 */
class Action_RenameCity: public Action
{
    public:
	//! Make a new city rename action.
        Action_RenameCity();
	//! Copy constructor
	Action_RenameCity(const Action_RenameCity &action);
	//! Load a new city rename action from an opened saved-game file.
        Action_RenameCity(XML_Helper* helper);
	//! Destroy a city rename action.
        ~Action_RenameCity();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this city rename action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the action with the city being renamed and the new name.
        bool fillData(City *c, std::string name);
    
	Uint32 getCityId() const {return d_city;};
	std::string getNewCityName() const {return d_name;};
        private:
        Uint32 d_city;
	std::string d_name;
};

//-----------------------------------------------------------------------------

//! A temporary record of a Player changing vectoring strategies for a City.
/**
 * The purpose of the Action_Vector class is to record when a Player has
 * changed the vectoring policy of a City.  The City's Army units can
 * be vectored to another City or to a Hero's planted standard (Item).
 * When units are vectored they take 2 turns to appear at their destination.
 * While the vectored units are en route, they are invisible.
 */
class Action_Vector: public Action
{
    public:
	//! Make a new city vector action.
        Action_Vector();
	//! Copy constructor
	Action_Vector(const Action_Vector &action);
	//! Load a new city vector action from an opened saved-game file.
        Action_Vector(XML_Helper* helper);
	//! Destroy a city vector action.
        ~Action_Vector();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this city vector action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	/**
	 * Populate the Action_Vector class with the City being vectored
	 * from, and the destination position for the vectored units.
	 */
	//! Populate the action with pertinent data.
        bool fillData(City* src, Vector <int> dest);
    
	Uint32 getCityId() const {return d_city;};
	Vector<int> getVectoringDestination() const {return d_dest;};

        private:
        Uint32 d_city;
        Vector<int> d_dest;
};

//-----------------------------------------------------------------------------

//! A temporary record of a Player changing the fight order of an Armyset.
/**
 * The purpose of the Action_FightOrder action is to record when a Player
 * changes the order in which Army units fight in battle.
 */
class Action_FightOrder: public Action
{
    public:
	//! Make a new fight order action.
        Action_FightOrder();
	//! Copy constructor
	Action_FightOrder(const Action_FightOrder &action);
	//! Load a new fight order action from an opened saved-game file.
        Action_FightOrder(XML_Helper* helper);
	//! Destroy a fight order action.
        ~Action_FightOrder();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this fight order action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the action with a list of ranks, one per Army unit type.
        bool fillData(std::list<Uint32> order);
    
	std::list<Uint32> getFightOrder() const {return d_order;};

        private:
	std::list<Uint32> d_order;
};

//-----------------------------------------------------------------------------

//! A temporary record of a Player surrendering.
/**
 * The purpose of the Action_Resign class is to record when a Player has
 * resigned from the game.  Because these actions are held in a Player's
 * Actionlist, we do not have to store the player's Id.
 */
class Action_Resign: public Action
{
    public:
	//! Make a new player resignation action.
        Action_Resign();
	//! Copy constructor
	Action_Resign(const Action_Resign &action);
	//! Load a new player resignation action from an opened saved-game file.
        Action_Resign(XML_Helper* helper);
	//! Destroy a player resignation action.
        ~Action_Resign();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this player resignation action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! This method doesn't need to be called for Action_Resign actions.
        bool fillData();
};

//-----------------------------------------------------------------------------

//! A temporary record of a Hero planting a standard into the ground.
/**
 * The purpose of the Action_Plant class is to record when a Hero has 
 * planted a standard (e.g. a flag Item) into the ground, so that Army units
 * can be vectored there.
 */
class Action_Plant: public Action
{
    public:
	//! Make a new item planted action.
        Action_Plant();
	//! Copy constructor
	Action_Plant(const Action_Plant &action);
	//! Load a new item planted action from an opened saved-game file.
        Action_Plant(XML_Helper* helper);
	//! Destroy a item planted action.
        ~Action_Plant();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this item planted action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the action with the Id of the Hero and the Id of the Item.
        bool fillData(Hero *hero, Item *item);

	Uint32 getHeroId() const {return d_hero;};
	Uint32 getItemId() const {return d_item;};

        private:
        Uint32 d_hero;
        Uint32 d_item;
};

//-----------------------------------------------------------------------------

//! A temporary record of a new Army unit showing up at a city.
/**
 * The purpose of the Action_Produce class is to record when a new Army unit
 * is created.  The idea here is that a City has produced a new Army unit.
 * The City might be vectoring elsewhere, so the unit doesn't show up in the
 * City that produced it.  If vectoring is not enabled the Army unit shows
 * up right away in the host City.
 * This action is used primarily for reporting purposes.  The player's 
 * production report shows which army units were created in what cities.
 * The army unit is taken from the Player's Armyset.
 */
class Action_Produce: public Action
{
    public:
	//! Make a new unit produced action.
        Action_Produce();
	//! Copy constructor
	Action_Produce(const Action_Produce &action);
	//! Load a new unit produced action from an opened saved-game file.
        Action_Produce(XML_Helper* helper);
	//! Destroy a unit produced action.
        ~Action_Produce();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this unit produced action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	/**
	 * Populate the Action_Produce action with the army type being
	 * produced, the City in which it has arrived, and also whether
	 * or not this unit was prevented from showing up here because
	 * it is being vectored elsewhere.
	 */
	//! Populate the action with pertinent data.
        bool fillData(const Army *a, City *city, bool vectored);

	//! Get the Id of the army type of the Army unit that was produced.
	Uint32 getArmyType() const {return d_army_type;}

	//! Get the Id of the City that produced the Army unit.
	Uint32 getCityId() const {return d_city;}

	//! Get whether or not the Army unit is being vectored elsewhere.
	bool getVectored() const {return d_vectored;}
    private:
        Uint32 d_army_type;
        Uint32 d_city;
        bool d_vectored;
};

        
//-----------------------------------------------------------------------------

//! A temporary record of a vectored Army unit showing up at a city.
/**
 * The purpose of the Action_ProduceVectored class is to record when a new
 * vectored Army unit arrives at it's destination.  The idea here is that
 * two turns have passed since a unit was vectored, and now the unit has
 * shown up.
 * This action is used primarily for reporting purposes.  The player's 
 * production report shows which vectored army units arrived in what cities.
 * The army unit is taken from the Player's Armyset.
 */
class Action_ProduceVectored: public Action
{
    public:
	//! Make a new vector arrival action.
        Action_ProduceVectored();
	//! Copy constructor
	Action_ProduceVectored(const Action_ProduceVectored &action);
	//! Load a new vector arrival action from an opened saved-game file.
        Action_ProduceVectored(XML_Helper* helper);
	//! Destroy a vector arrival action.
        ~Action_ProduceVectored();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this vector arrival action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	/**
	 * Populate the Action_ProduceVectored with the Id of the army
	 * unit type being produced, and the position on the map where
	 * it has showing up.
	 */
	//! Populate the action with pertinent data.
        bool fillData(Uint32 army_type, Vector <int>dest);

	//! Get the army type Id that has shown up.
	Uint32 getArmyType() const {return d_army_type;}

	//! Get the position on the map where the army showed up.
	Vector<int> getDestination() const {return d_dest;}
    private:
        Uint32 d_army_type;
        Vector<int> d_dest;
};

//-----------------------------------------------------------------------------

//! A temporary record of the diplomatic state changing.
/**
 * The purpose of the Action_DiplomacyState action is to record our
 * diplomatic state with other players has it changes.  The idea here is 
 * that every player has a diplomatic status with every other player.
 * Although we might propose war on a given turn, we would achieve the
 * state of being at war on a later turn.
 */
class Action_DiplomacyState: public Action
{
    public:
	//! Make a new diplomatic state action.
        Action_DiplomacyState();
	//! Copy constructor
	Action_DiplomacyState(const Action_DiplomacyState &action);
	//! Load a new diplomatic state action from an opened saved-game file.
        Action_DiplomacyState(XML_Helper* helper);
	//! Destroy a diplomatic state action.
        ~Action_DiplomacyState();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this diplomatic state action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	/**
	 * Populate the Action_DiplomacyState class with the Player for
	 * which we are in a state with.  Also populate the action with
	 * the new diplomatic state.
	 */
	//! Populate action with pertinent data.
        bool fillData(Player *player, Player::DiplomaticState state);

	//! Get the Id of the Player that we have entered a new state for.
	Uint32 getOpponentId() const {return d_opponent_id;}

	//! Get the state that we're in with the other Player.
	Player::DiplomaticState getDiplomaticState() const
	  {return d_diplomatic_state;};
    private:
	Uint32 d_opponent_id;
	Player::DiplomaticState d_diplomatic_state;
};

//-----------------------------------------------------------------------------

//! A temporary record of a diplomatic proposal.
/**
 * The purpose of the Action_DiplomacyProposal action is to record our
 * diplomatic proposals to other players.  The idea here is that the player
 * wishes to go to war with another Player and so offers/proposes war to a 
 * prospective enemy.
 */
class Action_DiplomacyProposal: public Action
{
    public:
	//! Make a new diplomatic proposal action.
        Action_DiplomacyProposal();
	//! Copy constructor
	Action_DiplomacyProposal(const Action_DiplomacyProposal &action);
	//! Load a new diplomatic proposal action from a saved-game file.
        Action_DiplomacyProposal(XML_Helper* helper);
	//! Destroy a diplomatic proposal action.
        ~Action_DiplomacyProposal();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this diplomatic proposal action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	/**
	 * Populate the Action_DiplomacyProposal class with the Player for
	 * which we have a new proposal for.  Also populate the action with
	 * the new diplomatic proposal.
	 */
	//! Populate action with pertinent data.
        bool fillData(Player *player, Player::DiplomaticProposal proposal);

	//! Get the Id of the Player that our proposal is for.
	Uint32 getOpponentId() const {return d_opponent_id;}

	//! Get the proposal that we're offering.
	Player::DiplomaticProposal getDiplomaticProposal() const
	  {return d_diplomatic_proposal;};
    private:
	Uint32 d_opponent_id;
	Player::DiplomaticProposal d_diplomatic_proposal;
};

//-----------------------------------------------------------------------------

//! A temporary record of the diplomatic score.
/**
 * The purpose of the Action_DiplomacyScore is to record when a Player's
 * diplomatic opinion of another Player has changed.  The idea here is that
 * an enemy player has razed a city and now our opinion of that player
 * deteriorates.
 */
class Action_DiplomacyScore: public Action
{
    public:
	//! Make a new diplomatic score action.
        Action_DiplomacyScore();
	//! Copy constructor.
	Action_DiplomacyScore(const Action_DiplomacyScore &action);
	//! Load a new diplomatic score action from an opened saved-game file.
        Action_DiplomacyScore(XML_Helper* helper);
	//! Destroy a diplomatic score action.
        ~Action_DiplomacyScore();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this diplomatic score action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	/**
	 * Populate the Action_DiplomacyScore class with the Player for
	 * which we have changed our opinion of.  Also populate the action
	 * with the amount of change for that Player.  The change can be 
	 * negative, and is added to the existing score to get the new 
	 * score.
	 */
	//! Populate action with pertinent data.
        bool fillData(Player *player, int amount);

	//! Get the Id of the Player that our opinion has changed of.
	Uint32 getOpponentId() const {return d_opponent_id;}

	//! Get the amount of the opinion change.
	int getAmountChange() const {return d_amount;};
    private:
	Uint32 d_opponent_id;
	int d_amount;
};

//-----------------------------------------------------------------------------

class Action_EndTurn: public Action
{
    public:
	//! Make a new end turn action.
        Action_EndTurn();
	//! Copy constructor
	Action_EndTurn(const Action_EndTurn &action);
	//! Load a new end turn action from an opened saved-game file.
        Action_EndTurn(XML_Helper* helper);
	//! Destroy a end turn action.
        ~Action_EndTurn();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;
};

//-----------------------------------------------------------------------------

class Action_ConquerCity : public Action
{
    public:
	//! Make a new city conquer action.
        Action_ConquerCity();
	//! Copy constructor
	Action_ConquerCity(const Action_ConquerCity &action);
	//! Load a new city conquer action from an opened saved-game file.
        Action_ConquerCity(XML_Helper* helper);
	//! Destroy a city conquer action.
        ~Action_ConquerCity();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this city occupied action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the action with the City being conquered.
        bool fillData (City* c, Stack *s);
    
	Uint32 getCityId() const {return d_city;};
	Uint32 getStackId() const {return d_stack;};
        private:
        Uint32 d_city, d_stack;
};

//-----------------------------------------------------------------------------

class Action_RecruitHero : public Action
{
    public:
	//! Make a new recruit hero action.
        Action_RecruitHero();
	//! Copy a new recruit hero action
	Action_RecruitHero(const Action_RecruitHero &action);
	//! Load a new recruit hero action from an opened saved-game file.
        Action_RecruitHero(XML_Helper* helper);
	//! Destroy a recruit hero action.
        ~Action_RecruitHero();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this city occupied action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the action.
        bool fillData(Hero* hero, City *city, int cost, int alliesCount, const Army *ally);
    
	Hero* getHero() const {return d_hero;};
	Uint32 getCityId() const {return d_city;};
	Uint32 getCost() const {return d_cost;};
	Uint32 getNumAllies() const {return d_allies;};
	Uint32 getAllyArmyType() const {return d_ally_army_type;};

        private:
        Hero *d_hero;
        Uint32 d_city, d_cost, d_allies, d_ally_army_type;

        bool load(std::string tag, XML_Helper *helper);
};

//-----------------------------------------------------------------------------

class Action_RenamePlayer: public Action
{
    public:
	//! Make a new rename player action
        Action_RenamePlayer();
	//! Copy constructor
	Action_RenamePlayer(const Action_RenamePlayer &action);
	//! Load a new rename player action from an opened saved-game file.
        Action_RenamePlayer(XML_Helper* helper);
	//! Destroy a rename player action.
        ~Action_RenamePlayer();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this city occupied action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the action.
        bool fillData(std::string name);
    
	std::string getName() const {return d_name;};

        private:
	std::string d_name;
};

//-----------------------------------------------------------------------------

class Action_CityTooPoorToProduce: public Action
{
    public:
	//! Make a new city-too-poor action
        Action_CityTooPoorToProduce();
	//! Copy constructor
	Action_CityTooPoorToProduce(const Action_CityTooPoorToProduce &action);
	//! Load a new too-poor action from an opened saved-game file.
        Action_CityTooPoorToProduce(XML_Helper* helper);
	//! Destroy a too-poor action.
        ~Action_CityTooPoorToProduce();

	//! Return some debug information about this action.
        std::string dump() const;

	//! Save this city occupied action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the action.
        bool fillData(City *city, const Army *army);
    
	Uint32 getCityId() const {return d_city;}
	Uint32 getArmyType() const {return d_army_type;}

        private:
	Uint32 d_city;
	Uint32 d_army_type;
};
#endif //ACTION_H
