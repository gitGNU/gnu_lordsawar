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

#include <stdlib.h>
#include <sstream>
#include <vector>
#include <sigc++/functors/mem_fun.h>

#include "reward.h"
#include "army.h"
#include "armysetlist.h"
#include "playerlist.h"
#include "ruinlist.h"
#include "GameMap.h"
#include "ruin.h"
using namespace std;

Reward::Reward(Type type, std::string name)
    :d_type(type), d_name(name)
{
}

Reward::Reward(XML_Helper *helper)
{
  Uint32 t;
  helper->getData(t, "type");
  d_type = static_cast<Reward::Type>(t);
  helper->getData(d_name, "name");
}

bool Reward::save(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->saveData("type", d_type);
  retval &= helper->saveData("name", d_name);
  return retval;
}

Reward::~Reward()
{
}

Reward* Reward::handle_load(XML_Helper* helper)
{
    Uint32 t;
    helper->getData(t, "type");

    switch (t)
    {
        case Reward::GOLD:
            return (new Reward_Gold(helper));
        case Reward::ALLIES:
            return (new Reward_Allies(helper));
        case Reward::ITEM:
            return (new Reward_Item(helper));
        case Reward::RUIN:
            return (new Reward_Ruin(helper));
        case Reward::MAP:
            return (new Reward_Map(helper));
    }

    return 0;
}

Reward_Gold::Reward_Gold(Uint32 gold)
    :Reward(Reward::GOLD), d_gold(gold)
{
}

Reward_Gold::Reward_Gold(XML_Helper* helper)
    :Reward(helper)
{
  helper->getData(d_gold, "gold");
}

bool Reward_Gold::save(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->openTag("reward");
  retval &= Reward::save(helper);
  retval &= helper->saveData("gold", d_gold);
  retval &= helper->closeTag();
  return retval;
}

Reward_Gold::~Reward_Gold()
{
}


Reward_Allies::Reward_Allies(Uint32 army_type, Uint32 army_set, Uint32 count)
    :Reward(Reward::ALLIES), d_count(count)
{
  Armysetlist *al = Armysetlist::getInstance();
  d_army_type = army_type;
  d_army_set = army_set;
  d_army  = al->getArmy (army_set, army_type);
}

Reward_Allies::Reward_Allies(const Army *army, Uint32 count)
    :Reward(Reward::ALLIES), d_count(count)
{
  d_army_type = army->getType();
  d_army_set = army->getArmyset();
  d_army = army;
}

Reward_Allies::Reward_Allies(XML_Helper* helper)
    :Reward(helper)
{
  Armysetlist *al = Armysetlist::getInstance();
  helper->getData(d_count, "num_allies");
  helper->getData(d_army_type, "ally_type");
  helper->getData(d_army_set, "ally_armyset");
  d_army = al->getArmy (d_army_set, d_army_type);
}

bool Reward_Allies::save(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->openTag("reward");
  retval &= Reward::save(helper);
  retval &= helper->saveData("num_allies", d_count);
  retval &= helper->saveData("ally_type", d_army_type);
  retval &= helper->saveData("ally_armyset", d_army_set);
  retval &= helper->closeTag();
  return retval;
}
const Army* Reward_Allies::randomArmyAlly()
{
  Uint32 allytype;
  // list all the army types that can be allies.
  std::vector<const Army*> allytypes;
  Armysetlist *al = Armysetlist::getInstance();
  Player *p = Playerlist::getInstance()->getActiveplayer();
  if (!p)
    p = Playerlist::getInstance()->getNeutral();
  for (unsigned int j = 0; j < al->getSize(p->getArmyset()); j++)
    {
      const Army *a = al->getArmy (p->getArmyset(), j);
      if (a->getAwardable())
        allytypes.push_back(a);
    }
  if (!allytypes.empty())
    allytype = rand() % allytypes.size();
  else 
    return NULL;

  return allytypes[allytype];
}

bool Reward_Allies::addAllies(Player *p, Vector<int> pos, const Army *army, Uint32 alliesCount)
{
  for (unsigned int i = 0; i < alliesCount; i++)
    {
      Army* ally = new Army(*army, p);
      if (GameMap::getInstance()->addArmy(pos, ally) == NULL)
        return false;
    }
  return true;
}

bool Reward_Allies::addAllies(Player *p, Location *l, const Army *army, Uint32 alliesCount)
{
  for (unsigned int i = 0; i < alliesCount; i++)
    {
      Army* ally = new Army(*army, p);
      if (GameMap::getInstance()->addArmy(l, ally) == NULL)
        return false;
    }
  return true;
}


Reward_Allies::~Reward_Allies()
{
}

Reward_Item::Reward_Item(Item *item)
    :Reward(Reward::ITEM), d_item(item)
{
}

bool Reward_Item::loadItem(std::string tag, XML_Helper* helper)
{
  if (tag == "item")
    {
      d_item = new Item(helper);
      return true;
    }
    
  return false;
}

Reward_Item::Reward_Item(XML_Helper* helper)
    :Reward(helper)
{
  helper->registerTag("item", sigc::mem_fun(this, &Reward_Item::loadItem));
}

bool Reward_Item::save(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->openTag("reward");
  retval &= Reward::save(helper);
  retval &= d_item->save(helper);
  retval &= helper->closeTag();
  return retval;
}


Reward_Item::~Reward_Item()
{
  if (d_item)
    delete d_item;
}

Reward_Ruin::Reward_Ruin(Ruin *ruin)
    :Reward(Reward::RUIN), d_ruin(ruin)
{
}

Reward_Ruin::Reward_Ruin(XML_Helper* helper)
    :Reward(helper)
{
  Uint32 x;
  Uint32 y;
  helper->getData(x, "x");
  helper->getData(y, "y");
  d_ruin = Ruinlist::getInstance()->getObjectAt(x, y);
}

bool Reward_Ruin::save(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->openTag("reward");
  retval &= Reward::save(helper);
  retval &= helper->saveData("x", d_ruin->getPos().x);
  retval &= helper->saveData("y", d_ruin->getPos().y);
  retval &= helper->closeTag();
  return retval;
}

Reward_Ruin::~Reward_Ruin()
{
}

Reward_Map::Reward_Map(Location *loc, Uint32 height, Uint32 width)
    :Reward(Reward::MAP), d_loc(loc), d_height(height), d_width(width)
{
}

bool Reward_Map::loadLocation(std::string tag, XML_Helper* helper)
{
  if (tag == "location")
    {
      d_loc = new Location(helper);
      return true;
    }
    
  return false;
}

Reward_Map::Reward_Map(XML_Helper* helper)
    :Reward(helper)
{
  helper->registerTag("location", sigc::mem_fun(this, 
						&Reward_Map::loadLocation));
  helper->getData(d_height, "height");
  helper->getData(d_width, "width");
}

bool Reward_Map::save(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->openTag("reward");
  retval &= Reward::save(helper);
  retval &= helper->saveData("height", d_height);
  retval &= helper->saveData("width", d_width);
  retval &= helper->openTag("location");
  retval &= helper->saveData("id", d_loc->getId());
  retval &= helper->saveData("name", d_loc->getName());
  Vector<int> pos = d_loc->getPos();
  retval &= helper->saveData("x", pos.x);
  retval &= helper->saveData("y", pos.y);
  retval &= helper->closeTag();
  retval &= helper->closeTag();
  return retval;
}


Reward_Map::~Reward_Map()
{
}

