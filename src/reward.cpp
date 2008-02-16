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
#include "Itemlist.h"
#include "GameMap.h"
#include "ruin.h"
#include "ucompose.hpp"
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
        
Reward::Reward (const Reward& orig)
	:d_type(orig.d_type), d_name(orig.d_name)
{
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

Reward_Gold::Reward_Gold (const Reward_Gold & orig)
	:Reward(orig), d_gold(orig.d_gold)
{
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

Uint32 Reward_Gold::getRandomGoldPieces()
{
  return 310 + (rand() % 1000);
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

Reward_Allies::Reward_Allies (const Reward_Allies& orig)
	:Reward(orig), d_army_type(orig.d_army_type), 
	d_army_set(orig.d_army_set), d_army(orig.d_army), d_count(orig.d_count)
{
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
	
const Uint32 Reward_Allies::getRandomAmountOfAllies()
{
  return (rand() % MAX_STACK_SIZE) + 1;
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

Reward_Item::Reward_Item (const Reward_Item& orig)
	:Reward(orig), d_item(orig.d_item)
{
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

Item *Reward_Item::getRandomItem()
{
  Itemlist *il = Itemlist::getInstance();
  Item *i = (*il)[rand() % il->size()];
  return new Item(*i);
}

Reward_Item::~Reward_Item()
{
  if (d_item)
    delete d_item;
}

Reward_Ruin::Reward_Ruin(Ruin *ruin)
    :Reward(Reward::RUIN), d_ruin_pos(ruin->getPos())
{
}

Reward_Ruin::Reward_Ruin(XML_Helper* helper)
    :Reward(helper)
{
  Uint32 x;
  Uint32 y;
  helper->getData(x, "x");
  helper->getData(y, "y");
  d_ruin_pos = Vector<int>(x,y);
}

Reward_Ruin::Reward_Ruin (const Reward_Ruin& orig)
	:Reward(orig), d_ruin_pos(orig.d_ruin_pos)
{
}
bool Reward_Ruin::save(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->openTag("reward");
  retval &= Reward::save(helper);
  retval &= helper->saveData("x", getRuin()->getPos().x);
  retval &= helper->saveData("y", getRuin()->getPos().y);
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

Reward_Map::Reward_Map (const Reward_Map& orig)
	:Reward(orig), d_loc(orig.d_loc), d_height(orig.d_height), 
	d_width(orig.d_width)
{
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

void Reward_Map::getRandomMap(int *x, int *y, int *width, int *height)
{
  int map_width = GameMap::getInstance()->getWidth();
  *x = rand() % (map_width - (map_width / 10));
  int map_height = GameMap::getInstance()->getHeight();
  *y = rand() % (map_height - (map_height / 10));
  *width = ((rand() % (map_width - *x)) + (map_width / 10));
  *height = ((rand() % (map_height - *y)) + (map_height / 10));
}

Reward_Map::~Reward_Map()
{
}

std::string Reward::getDescription()
{
  Glib::ustring s = "";
  switch (getType())
    {
    case Reward::GOLD:
	{
	  Reward_Gold *g = dynamic_cast<Reward_Gold*>(this);
	  s += String::ucompose(ngettext("%1 Gold Piece", "%1 Gold Pieces", 
					 g->getGold()), g->getGold());
	  return s;
	}
    case Reward::ALLIES:
	{
	  Reward_Allies *a = dynamic_cast<Reward_Allies *>(this);
	  if (a->getArmy())
	    s += String::ucompose(_("Allies: %1 x %2"), a->getArmy()->getName(),
				  a->getNoOfAllies());
	  return s;
	}
    case Reward::ITEM:
	{
	  Reward_Item *i = dynamic_cast<Reward_Item *>(this);
	  if (i->getItem())
	    s += String::ucompose(_("Item: %1"), i->getItem()->getName());
	  return s;
	}
    case Reward::RUIN:
	{
	  Reward_Ruin *r = dynamic_cast<Reward_Ruin *>(this);
	  if (r->getRuin())
	    s += String::ucompose(_("Site: %1"), r->getRuin()->getName());
	  return s;
	}
    case Reward::MAP:
	{
	  Reward_Map *m = dynamic_cast<Reward_Map *>(this);
	  if (m->getLocation())
	    s += String::ucompose(_("Map: %1,%2 %3x%4"), 
				  m->getLocation()->getPos().x,
				  m->getLocation()->getPos().y,
				  m->getHeight(),
				  m->getWidth());
	  return s;
	}
    }
  return s;
}
