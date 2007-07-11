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

#ifndef __REWARD_H
#define __REWARD_H
#include <SDL_types.h>
#include "vector.h"
#include <string>
class Player;
class Army;
class Location;
class Item;
class XML_Helper;
class Ruin;

/** Base class for rewards
  *
  */
class Reward
{
    public:

        enum Type {GOLD = 1, ALLIES= 2, ITEM = 3, RUIN = 4};

        //! Standard constructor
        Reward(Type type, std::string name = "");
	//! XML constructor
        Reward(XML_Helper* helper);
        
        virtual ~Reward();

        //! Get the type of the reward
        Type getType() const { return d_type; }

        //! Returns the name of the reward
        std::string getName() const {return d_name;}

        /** Saves the reward data
          * 
          * @note This function is called by the actual reward and only saves
          * the common data. It does NOT open/close tags etc. This has to be
          * done by the derived classes.
          */
        virtual bool save(XML_Helper* helper) const;

        /** static load function (see XML_Helper)
          * 
          * Whenever an reward item is loaded, this function is called. It
          * examines the stored id and calls the constructor of the appropriate
          * reward class.
          *
          * @param helper       the XML_Helper instance for the savegame
          */
        static Reward* handle_load(XML_Helper* helper);

    protected:

        //! Type of the reward
        Type d_type;
	std::string d_name;

};

class Reward_Gold : public Reward
{
    public:
        Reward_Gold(Uint32 gold);
	Reward_Gold(XML_Helper *helper);
        ~Reward_Gold();

        bool save(XML_Helper* helper) const;
	Uint32 getGold() const {return d_gold;}

    private:
        Uint32 d_gold;
};

class Reward_Allies: public Reward
{
    public:
        Reward_Allies(const Army *army, Uint32 count);
        Reward_Allies(Uint32 army_type, Uint32 army_set, Uint32 count);
	Reward_Allies(XML_Helper *helper);
        ~Reward_Allies();

        bool save(XML_Helper* helper) const;
	const Army * getArmy() const {return d_army;}
	Uint32 getNoOfAllies() const {return d_count;}
        static const Army* randomArmyAlly();
        static bool addAllies(Player *p, Vector<int> pos, const Army *army, Uint32 alliesCount);
        static bool addAllies(Player *p, Location *l, const Army *army, Uint32 alliesCount);

    private:
        const Army *d_army;
	Uint32 d_army_type;
	Uint32 d_army_set;
        Uint32 d_count;
};

class Reward_Item: public Reward
{
    public:
        Reward_Item (Item *item);
	Reward_Item(XML_Helper *helper);
        ~Reward_Item();

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
        ~Reward_Ruin();

        bool save(XML_Helper* helper) const;
	Ruin* getRuin() const {return d_ruin;}

    private:
        Ruin *d_ruin;
};

#endif
