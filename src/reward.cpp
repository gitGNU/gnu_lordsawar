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
#include "rewardlist.h"
#include "Itemlist.h"
#include "GameMap.h"
#include "ruin.h"
#include "ucompose.hpp"
#include "SightMap.h"

std::string Reward::d_tag = "reward";

using namespace std;

Reward::Reward(Type type, std::string name)
    :d_type(type), d_name(name)
{
}

Reward::Reward(XML_Helper *helper)
{
  std::string type_str;
  helper->getData(type_str, "type");
  d_type = rewardTypeFromString(type_str);
  helper->getData(d_name, "name");
}

Reward::Reward (const Reward& orig)
	:d_type(orig.d_type), d_name(orig.d_name)
{
}

Reward::~Reward()
{
}

Reward* Reward::handle_load(XML_Helper* helper)
{
    Uint32 t;
    std::string type_str;
    helper->getData(type_str, "type");
    t = rewardTypeFromString(type_str);

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

bool Reward_Gold::save(XML_Helper* helper)
{
  bool retval = true;
  retval &= helper->openTag(Reward::d_tag);
  std::string type_str = rewardTypeToString(Reward::Type(d_type));
  retval &= helper->saveData("type", type_str);
  retval &= helper->saveData("name", d_name);
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

Reward_Allies::Reward_Allies(const ArmyProto *army, Uint32 count)
    :Reward(Reward::ALLIES), d_count(count)
{
  d_army_type = army->getTypeId();
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
	:Reward(orig), d_army(orig.d_army), d_army_type(orig.d_army_type), 
	d_army_set(orig.d_army_set), d_count(orig.d_count)
{
}

bool Reward_Allies::save(XML_Helper* helper)
{
  bool retval = true;
  retval &= helper->openTag(Reward::d_tag);
  std::string type_str = rewardTypeToString(Reward::Type(d_type));
  retval &= helper->saveData("type", type_str);
  retval &= helper->saveData("name", d_name);
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

const ArmyProto* Reward_Allies::randomArmyAlly()
{
  Uint32 allytype;
  // list all the army types that can be allies.
  std::vector<const ArmyProto*> allytypes;
  Armysetlist *al = Armysetlist::getInstance();
  Player *p = Playerlist::getInstance()->getActiveplayer();
  if (!p)
    p = Playerlist::getInstance()->getNeutral();
  for (unsigned int j = 0; j < al->getSize(p->getArmyset()); j++)
    {
      const ArmyProto *a = al->getArmy (p->getArmyset(), j);
      if (a->getAwardable())
        allytypes.push_back(a);
    }
  if (allytypes.empty())
    return NULL;

  allytype = rand() % allytypes.size();
  return allytypes[allytype];
}

bool Reward_Allies::addAllies(Player *p, Vector<int> pos, const ArmyProto *army, Uint32 alliesCount)
{
  for (unsigned int i = 0; i < alliesCount; i++)
    {
      Army* ally = new Army(*army, p);
      ally->setUpkeep(0);
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
      ally->setUpkeep(0);
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
  if (tag == Item::d_tag)
    {
      d_item = new Item(helper);
      return true;
    }
    
  return false;
}

Reward_Item::Reward_Item(XML_Helper* helper)
    :Reward(helper)
{
  helper->registerTag(Item::d_tag, sigc::mem_fun(this, &Reward_Item::loadItem));
}

Reward_Item::Reward_Item (const Reward_Item& orig)
	:Reward(orig), d_item(orig.d_item)
{
}

bool Reward_Item::save(XML_Helper* helper)
{
  bool retval = true;
  retval &= helper->openTag(Reward::d_tag);
  std::string type_str = rewardTypeToString(Reward::Type(d_type));
  retval &= helper->saveData("type", type_str);
  retval &= helper->saveData("name", d_name);
  retval &= d_item->save(helper);
  retval &= helper->closeTag();
  return retval;
}

Item *Reward_Item::getRandomItem()
{
  Itemlist *il = Itemlist::getInstance();
  Itemlist::iterator it = il->begin();
  std::advance(it, rand() % il->size());
  ItemProto *i = it->second;
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
bool Reward_Ruin::save(XML_Helper* helper)
{
  bool retval = true;
  retval &= helper->openTag(Reward::d_tag);
  std::string type_str = rewardTypeToString(Reward::Type(d_type));
  retval &= helper->saveData("type", type_str);
  retval &= helper->saveData("name", d_name);
  retval &= helper->saveData("x", getRuin()->getPos().x);
  retval &= helper->saveData("y", getRuin()->getPos().y);
  retval &= helper->closeTag();
  return retval;
}

Ruin *Reward_Ruin::getRandomHiddenRuin()
{
  std::vector<Ruin *>hidden_ruins;
  Ruinlist *rl = Ruinlist::getInstance();
  Rewardlist *rw = Rewardlist::getInstance();
  for (Ruinlist::iterator it = rl->begin(); it != rl->end(); it++)
    {
      if ((*it).isHidden())
	if ((*it).getOwner() == NULL || 
	    (*it).getOwner() == Playerlist::getInstance()->getNeutral())
	  {
	    //is it already being pointed to by a reward in the rewardlist?
	    bool found = false;
	    for (Rewardlist::iterator i = rw->begin(); i != rw->end(); i++)
	      {
		if ((*i)->getType() == Reward::RUIN)
		  {
		    Ruin *r = static_cast<Reward_Ruin*>(*i)->getRuin();
		    if (r)
		      {
			if (r->getPos() == (*it).getPos())
			  {
			    found = true;
			    break;
			  }
		      }
		  }
	      }
	    if (found == false)
	      hidden_ruins.push_back(&*it);
	  }
    }
 if (hidden_ruins.empty())
   return NULL;
 return hidden_ruins[rand() % hidden_ruins.size()];
}

Reward_Ruin::~Reward_Ruin()
{
}

Reward_Map::Reward_Map(Vector<int> pos, std::string name, 
		       Uint32 height, Uint32 width)
    :Reward(Reward::MAP, name)
{
  d_sightmap = new SightMap(name, pos, height, width);
}

bool Reward_Map::loadMap(std::string tag, XML_Helper* helper)
{
  if (tag == SightMap::d_tag)
    {
      d_sightmap = new SightMap(helper);
      return true;
    }
    
  return false;
}


Reward_Map::Reward_Map(XML_Helper* helper)
    :Reward(helper)
{
  helper->registerTag(SightMap::d_tag, sigc::mem_fun(this, &Reward_Map::loadMap));
}

Reward_Map::Reward_Map (const Reward_Map& orig)
	:Reward(orig)
{
  d_sightmap = new SightMap(*orig.d_sightmap);
}

bool Reward_Map::save(XML_Helper* helper)
{
  bool retval = true;
  retval &= helper->openTag(Reward::d_tag);
  std::string type_str = rewardTypeToString(Reward::Type(d_type));
  retval &= helper->saveData("type", type_str);
  retval &= helper->saveData("name", d_sightmap->getName());
  retval &= d_sightmap->save(helper);
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
  if (d_sightmap)
    delete d_sightmap;
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
	  s += String::ucompose(_("Map: %1,%2 %3x%4"), 
				  m->getLocation().x,
				  m->getLocation().y,
				  m->getHeight(),
				  m->getWidth());
	  return s;
	}
    }
  return s;
}

std::string Reward::rewardTypeToString(const Reward::Type type)
{
  switch (type)
    {
      case Reward::GOLD:
	return "Reward::GOLD";
	break;
      case Reward::ALLIES:
	return "Reward::ALLIES";
	break;
      case Reward::ITEM:
	return "Reward::ITEM";
	break;
      case Reward::RUIN:
	return "Reward::RUIN";
	break;
      case Reward::MAP:
	return "Reward::MAP";
	break;
    }
  return "Reward::GOLD";
}

Reward::Type Reward::rewardTypeFromString(const std::string str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Reward::Type(atoi(str.c_str()));
  if (str == "Reward::GOLD")
    return Reward::GOLD;
  else if (str == "Reward::ALLIES")
    return Reward::ALLIES;
  else if (str == "Reward::ITEM")
    return Reward::ITEM;
  else if (str == "Reward::RUIN")
    return Reward::RUIN;
  else if (str == "Reward::MAP")
    return Reward::MAP;
  return Reward::GOLD;
}

Reward* Reward::copy(const Reward* r)
{
  switch(r->getType())
    {
    case  Reward::GOLD:
      return (new Reward_Gold(*dynamic_cast<const Reward_Gold*>(r)));
    case  Reward::ALLIES:
      return (new Reward_Allies(*dynamic_cast<const Reward_Allies*>(r)));
    case Reward::ITEM:
      return (new Reward_Item(*dynamic_cast<const Reward_Item*>(r)));
    case Reward::RUIN:
      return (new Reward_Ruin(*dynamic_cast<const Reward_Ruin*>(r)));
    case Reward::MAP:
      return (new Reward_Map(*dynamic_cast<const Reward_Map*>(r)));
    }

  return 0;
}
