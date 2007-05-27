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
class Player;
class Army;
class Location;

/** Base class for rewards
  *
  */
class Reward
{
    public:

        enum Type {GOLD = 1, ALLIES= 2, ITEM = 3};

        //! Standard constructor
        Reward(Type type);
        
        virtual ~Reward();

        //! Get the type of the reward
        Type getType() const { return d_type; }

    protected:

        //! Type of the reward
        Type d_type;

};

class Reward_Gold : public Reward
{
    public:
        Reward_Gold(Uint32 gold);
        ~Reward_Gold();

	Uint32 getGold() const {return d_gold;}

    private:
        Uint32 d_gold;
};

class Reward_Allies: public Reward
{
    public:
        Reward_Allies(const Army *army, Uint32 count);
        ~Reward_Allies();

	const Army * getArmy() const {return d_army;}
	Uint32 getNoOfAllies() const {return d_count;}
        static const Army* randomArmyAlly();
        static bool addAllies(Player *p, Vector<int> pos, const Army *army, Uint32 alliesCount);
        static bool addAllies(Player *p, Location *l, const Army *army, Uint32 alliesCount);

    private:
        const Army *d_army;
        Uint32 d_count;
};

class Reward_Item: public Reward
{
    public:
        Reward_Item (Uint32 itemtype);
        ~Reward_Item();

	Uint32 getItemtype() const {return d_itemtype;}

    private:
        Uint32 d_itemtype;
};
#endif
