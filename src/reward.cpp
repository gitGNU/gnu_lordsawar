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

#include <stdlib.h>
#include <sstream>
#include <sigc++/functors/mem_fun.h>

#include "reward.h"
using namespace std;

Reward::Reward(Type type)
    :d_type(type)
{
}

Reward::~Reward()
{
}

Reward_Gold::Reward_Gold(Uint32 gold)
    :Reward(Reward::GOLD), d_gold(gold)
{
}

Reward_Gold::~Reward_Gold()
{
}

Reward_Allies::Reward_Allies(Uint32 armytype, Uint32 count)
    :Reward(Reward::ALLIES), d_armytype(armytype), d_count(count)
{
}

Reward_Allies::~Reward_Allies()
{
}

Reward_Item::Reward_Item(Uint32 itemtype)
    :Reward(Reward::ITEM), d_itemtype(itemtype)
{
}

Reward_Item::~Reward_Item()
{
}
