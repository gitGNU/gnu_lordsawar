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

#ifndef ACTION_H
#define ACTION_H

#include <string>
#include "vector.h"
#include <sigc++/trackable.h>

#include "fight.h"
#include "army.h"
#include "reward.h"

class Quest;
class Stack;
class City;
class Ruin;
class Temple;
class XML_Helper;

/** The purpose of the action classes is to keep track about what a player has
  * done. This information can e.g. then be sent over the network, so that a
  * networked player then just has to decode and repeat the actions which the
  * remote player has done.
  * 
  * Each action item is derived from the abstract Action class. It has to
  * contain three functions:
  *
  * - A loading constructor (which takes an XML_Helper parameter)
  * - a save function which saves the data
  * - a fillData function which takes some parameters and with these stores the
  *   data about what happened.
  */


class Action
{
    public:
        enum Type {
                STACK_MOVE = 1,
                STACK_SPLIT = 2,
                STACK_FIGHT = 3,
                STACK_JOIN = 4,
                RUIN_SEARCH = 5,
                TEMPLE_SEARCH = 6,
                CITY_OCCUPY = 7,
                CITY_PILLAGE = 8,
                CITY_RAZE = 9,
                CITY_UPGRADE = 10,
                CITY_BUY = 11,
                CITY_PROD = 12,
                REWARD = 13,
                QUEST = 14,
                HERO_EQUIP = 15,
                UNIT_ADVANCE = 16,
                CITY_SACK = 17,
        };
                
        
        Action(Type type);
        virtual ~Action();

        //! Returns debug information. Needs to be overwritten by derivatives
        virtual std::string dump() const = 0;

        //! Save function. See XML_Helper for information about saving.
        virtual bool save(XML_Helper* helper) const = 0;
        
        /** static load function (see XML_Helper)
          * 
          * Whenever an action item is loaded, this function is called. It
          * examines the stored id and calls the constructor of the appropriate
          * action class.
          *
          * @param helper       the XML_Helper instance for the savegame
          */
        static Action* handle_load(XML_Helper* helper);

        //! Copies an action to a new one
        static Action* copy(const Action* a);

        //! Returns the id which identifies the type of the action
        Type getType() const {return d_type;}
        
    protected:
        Type d_type;
};

//-----------------------------------------------------------------------------

class Action_Move : public Action
{
    public:
        Action_Move();
        Action_Move(XML_Helper* helper);
        ~Action_Move();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData(Stack* s, Vector<int> dest);
    
    private:
        Uint32 d_stack;
        Vector<int> d_dest;
};

//-----------------------------------------------------------------------------

class Action_Split : public Action
{
    public:
        Action_Split();
        Action_Split(XML_Helper* helper);
        ~Action_Split();

        std::string dump() const;
        bool save (XML_Helper* helper) const;

        //! Both stacks have to be split already!!
        bool fillData(Stack* orig, Stack* added);
    
    private:
        Uint32 d_orig, d_added;
        Uint32 d_armies_moved[8];
};

//-----------------------------------------------------------------------------

class Action_Fight : public Action, public sigc::trackable
{
    public:
        Action_Fight();
        Action_Fight(XML_Helper* helper);
        ~Action_Fight();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData(const Fight* f);

    private:
        bool loadItem(std::string tag, XML_Helper* helper);
        
        std::list<FightItem> d_history;
        std::list<Uint32> d_attackers;
        std::list<Uint32> d_defenders;
};

//-----------------------------------------------------------------------------

class Action_Join : public Action
{
    public:
        Action_Join();
        Action_Join(XML_Helper* helper);
        ~Action_Join();

        std::string dump() const;
        bool save(XML_Helper* helper) const;
        bool fillData(Stack* orig, Stack* joining);
    
    private:
        Uint32 d_orig_id, d_joining_id;
};

//-----------------------------------------------------------------------------

class Action_Ruin : public Action
{
    public:
        Action_Ruin();
        Action_Ruin(XML_Helper* helper);
        ~Action_Ruin();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData(Ruin* r, Stack* explorers);

        void setSearched(bool searched) {d_searched = searched;}
    
    private:
        Uint32 d_ruin;
        Uint32 d_stack;
        bool d_searched;
};

//-----------------------------------------------------------------------------

class Action_Temple : public Action
{
    public:
        Action_Temple();
        Action_Temple(XML_Helper* helper);
        ~Action_Temple();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData(Temple* t, Stack* s);
    
    private:
        Uint32 d_temple;
        Uint32 d_stack;
};


//-----------------------------------------------------------------------------

class Action_Occupy : public Action
{
    public:
        Action_Occupy();
        Action_Occupy(XML_Helper* helper);
        ~Action_Occupy();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData (City* c);
    
    private:
        Uint32 d_city;
};

//-----------------------------------------------------------------------------

class Action_Pillage : public Action
{
    public:
        Action_Pillage();
        Action_Pillage(XML_Helper* helper);
        ~Action_Pillage();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData(City* c);

    private:
        Uint32 d_city;
};

//-----------------------------------------------------------------------------

class Action_Sack : public Action
{
    public:
        Action_Sack();
        Action_Sack(XML_Helper* helper);
        ~Action_Sack();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData(City* c);

    private:
        Uint32 d_city;
};

//-----------------------------------------------------------------------------

class Action_Raze : public Action
{
    public:
        Action_Raze();
        Action_Raze(XML_Helper* helper);
        ~Action_Raze();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData (City* c);
    
    private:
        Uint32 d_city;
};

//-----------------------------------------------------------------------------

class Action_Upgrade : public Action
{
    public:
        Action_Upgrade();
        Action_Upgrade(XML_Helper* helper);
        ~Action_Upgrade();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData(City* c);

    private:
        Uint32 d_city;
};

//-----------------------------------------------------------------------------

class Action_Buy : public Action
{
    public:
        Action_Buy();
        Action_Buy(XML_Helper* helper);
        ~Action_Buy();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData(City* c, int slot, int prod);

    private:
        Uint32 d_city;
        int d_slot, d_prod;
};

//-----------------------------------------------------------------------------

class Action_Production : public Action
{
    public:
        Action_Production();
        Action_Production(XML_Helper* helper);
        ~Action_Production();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData(City* c, int prod);

    private:
        Uint32 d_city;
        int d_prod;
};

//-----------------------------------------------------------------------------

//this class will certainly have to be changed a lot, it doesn't handle anything
//else but gold, but I want at least a basic

class Action_Reward : public Action
{
    public:
        Action_Reward();
        Action_Reward(XML_Helper* helper);
        ~Action_Reward();
        
        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData (Reward *);
    
    private:
	Reward *d_reward;
	
};

//-----------------------------------------------------------------------------
// This class also may have some problems lateron, but should suffice for now.

class Action_Quest : public Action
{
    public:
        Action_Quest();
        Action_Quest(XML_Helper* helper);
        ~Action_Quest();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData(Quest* q);

    private:
        Uint32 d_hero;
        Uint32 d_questtype;
        Uint32 d_data;
	Uint32 d_victim_player; //victim player, only KILLARMIES uses this
};

//-----------------------------------------------------------------------------

class Action_Equip : public Action
{
    public:
        enum Slot {
            NONE = 0,
            BACKPACK = 1,
            GROUND = 2,
            BODY = 3};
        
        Action_Equip();
        Action_Equip(XML_Helper* helper);
        ~Action_Equip();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData(Uint32 hero, Uint32 item, Slot slot, int index);

    private:
        Uint32 d_hero;
        Uint32 d_item;
        Uint32 d_slot;
        int d_index;
};

//-----------------------------------------------------------------------------

class Action_Level : public Action
{
    public:
        Action_Level();
        Action_Level(XML_Helper* helper);
        ~Action_Level();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData(Uint32 unit, Army::Stat raised);

    private:
        Uint32 d_army;
        Uint32 d_stat;
};
        
#endif //ACTION_H
