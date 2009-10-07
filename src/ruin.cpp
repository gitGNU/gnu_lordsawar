// Copyright (C) 2001, 2003 Michael Bartl
// Copyright (C) 2002, 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2007, 2008, 2009 Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
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

#include "ruin.h"
#include "playerlist.h"
#include "GameMap.h"
#include "rewardlist.h"
#include <stdlib.h>

std::string Ruin::d_tag = "ruin";

Ruin::Ruin(Vector<int> pos, std::string name, int type, Stack* occupant, bool searched, bool hidden, Player *owner, bool sage)
:NamedLocation(pos, Ruin::getWidth(), name,
	        name + _(" is inhabited by monsters and full of treasure!")), 
    d_searched(searched), 
    d_type(type), d_occupant(occupant), d_hidden(hidden), d_owner(owner), 
    d_sage(sage)
{
    d_owner = NULL;
    d_reward = NULL;
    //mark the location as being occupied by a ruin on the map
    for (unsigned int i = 0; i < getSize(); i++)
      for (unsigned int j = 0; j < getSize(); j++)
	{
	  Vector<int> pos = getPos() + Vector<int>(i, j);
	  GameMap::getInstance()->getTile(pos)->setBuilding(Maptile::RUIN);
	}
}

Ruin::Ruin(const Ruin& ruin)
    :NamedLocation(ruin), d_searched(ruin.d_searched), 
    d_type(ruin.d_type), d_occupant(ruin.d_occupant), d_hidden(ruin.d_hidden), 
    d_owner(ruin.d_owner), d_sage(ruin.d_sage), d_reward(ruin.d_reward)
{
}

Ruin::Ruin(XML_Helper* helper)
    :NamedLocation(helper, Ruin::getWidth()), d_type(0), d_occupant(0), 
    d_hidden(0), d_owner(0), d_sage(0), d_reward(0)
{
    guint32 ui;
    std::string type_str;
    helper->getData(type_str, "type");
    d_type = ruinTypeFromString(type_str);
    helper->getData(d_searched, "searched");
    helper->getData(d_sage, "sage");
    helper->getData(d_hidden, "hidden");
    if (d_hidden)
      {
        helper->getData(ui, "owner");
	if (ui != MAX_PLAYERS)
          d_owner = Playerlist::getInstance()->getPlayer(ui);
	else
	  d_owner = NULL;
      }
    else
      d_owner = NULL;

    //mark the location as being occupied by a ruin on the map
    for (unsigned int i = 0; i < getSize(); i++)
      for (unsigned int j = 0; j < getSize(); j++)
	{
	  Vector<int> pos = getPos() + Vector<int>(i, j);
	  GameMap::getInstance()->getTile(pos)->setBuilding(Maptile::RUIN);
	}
}

Ruin::~Ruin()
{
    if (d_reward)
        delete d_reward;
    if (d_occupant)
        delete d_occupant;
}

bool Ruin::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag(Ruin::d_tag);
  retval &= helper->saveData("id", d_id);
  retval &= helper->saveData("x", getPos().x);
  retval &= helper->saveData("y", getPos().y);
  retval &= helper->saveData("name", getName(false));
  retval &= helper->saveData("description", getDescription());
  std::string type_str = ruinTypeToString(Ruin::Type(d_type));
  retval &= helper->saveData("type", type_str);
  retval &= helper->saveData("searched", d_searched);
  retval &= helper->saveData("sage", d_sage);
  retval &= helper->saveData("hidden", d_hidden);
  if (d_owner != NULL)
    retval &= helper->saveData("owner", d_owner->getId());
  else
    retval &= helper->saveData("owner", MAX_PLAYERS);
  if (d_occupant)
    retval &= d_occupant->save(helper);
  if (d_sage == false && d_reward)
    retval &= d_reward->save(helper);
  retval &= helper->closeTag();

  return retval;
}

bool Ruin::load(std::string tag, XML_Helper* helper)
{
  if (tag == Reward::d_tag)
    {
	guint32 t;
	std::string type_str;
	helper->getData(type_str, "type");
	t = Reward::rewardTypeFromString(type_str);
	switch (t)
	  {
	  case  Reward::GOLD:
	    d_reward = new Reward_Gold(helper); break;
	  case  Reward::ALLIES:
	    d_reward = new Reward_Allies(helper); break;
	  case Reward::ITEM:
	    d_reward = new Reward_Item(helper); break;
	  case Reward::RUIN:
	    d_reward = new Reward_Ruin(helper); break;
	  case Reward::MAP:
	    d_reward = new Reward_Map(helper); break;
	  }
	return true;
    }

  if (tag == Stack::d_tag)
    {
      Stack* s = new Stack(helper);
      d_occupant = s;
      return true;
    }

  return false;
}

void Ruin::populateWithRandomReward()
{
  int num;
  if (getType() == Ruin::STRONGHOLD)
    num = 1 + (rand() % 2);
  else
    num = rand() % 3;
  if (num == 0)
    setReward(new Reward_Gold(Reward_Gold::getRandomGoldPieces()));
  else if (num == 1)
    {
      const ArmyProto *a = Reward_Allies::randomArmyAlly();
      setReward(new Reward_Allies(a, Reward_Allies::getRandomAmountOfAllies()));
    }
  else if (num == 2)
    {
      Reward *reward = Rewardlist::getInstance()->popRandomItemReward();
      if (reward)
	setReward(reward);
      else //no items left to give!
	setReward(new Reward_Gold(Reward_Gold::getRandomGoldPieces()));
    }
}

std::string Ruin::ruinTypeToString(const Ruin::Type type)
{
  switch (type)
    {
      case Ruin::RUIN:
	return "Ruin::RUIN";
	break;
      case Ruin::STRONGHOLD:
	return "Ruin::STRONGHOLD";
	break;
    }
  return "Ruin::RUIN";
}

Ruin::Type Ruin::ruinTypeFromString(const std::string str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Ruin::Type(atoi(str.c_str()));
  if (str == "Ruin::RUIN")
    return Ruin::RUIN;
  else if (str == "Ruin::STRONGHOLD")
    return Ruin::STRONGHOLD;
  return Ruin::RUIN;
}
// End of file
