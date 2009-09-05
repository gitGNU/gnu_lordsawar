//  Copyright (C) 2008, 2009 Ben Asselstine
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

#include <sstream>
#include <iostream>
#include <fstream>
#include "ucompose.hpp"

#include "CreateScenarioRandomize.h"

#include "File.h"
#include "citylist.h"
#include "signpost.h"
#include "armysetlist.h"
#include "playerlist.h"
#include "reward.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

CreateScenarioRandomize::CreateScenarioRandomize()
{
    // Fill the namelists 
    bool success = true;
    
    d_citynames = new NameList("citynames.xml", "city");
    d_templenames = new NameList("templenames.xml", "temple");
    d_ruinnames = new NameList("ruinnames.xml", "ruin");
    d_signposts = new NameList("signposts.xml", "signpost");

    if (!success)
    {
        std::cerr <<"CreateScenarioRandomize: Didn't succeed in reading object names. Aborting!\n";
        exit(-1);
    }
}

CreateScenarioRandomize::~CreateScenarioRandomize()
{
    debug("CreateScenarioRandomize::~CreateScenarioRandomize")
}

std::string CreateScenarioRandomize::popRandomCityName()
{
  return _(d_citynames->popRandomName().c_str());
}

void CreateScenarioRandomize::pushRandomCityName(std::string name)
{
  d_citynames->push_back(name);
}

std::string CreateScenarioRandomize::popRandomRuinName()
{
  return _(d_ruinnames->popRandomName().c_str());
}

void CreateScenarioRandomize::pushRandomRuinName(std::string name)
{
  d_ruinnames->push_back(name);
}

std::string CreateScenarioRandomize::popRandomTempleName()
{
  return _(d_templenames->popRandomName().c_str());
}

void CreateScenarioRandomize::pushRandomTempleName(std::string name)
{
  d_templenames->push_back(name);
}

std::string CreateScenarioRandomize::popRandomSignpost()
{
  return _(d_signposts->popRandomName().c_str());
}

void CreateScenarioRandomize::pushRandomSignpost(std::string name)
{
  d_signposts->push_back(name);
}

Uint32 CreateScenarioRandomize::getRandomCityIncome(bool capital)
{
  if (capital)
    return 33 + (rand() % 8);
  else
    return 15 + (rand() % 12);
}

Army * CreateScenarioRandomize::getRandomRuinKeeper(Player *p)
{
  // list all the army types that can be a sentinel.
  std::vector<const ArmyProto*> occupants;
  Armysetlist *al = Armysetlist::getInstance();
  for (unsigned int j = 0; j < al->getSize(p->getArmyset()); j++)
    {
      const ArmyProto *a = al->getArmy (p->getArmyset(), j);
      if (a->getDefendsRuins())
	occupants.push_back(a);
    }
            
  if (!occupants.empty())
    {
      const ArmyProto *a = occupants[rand() % occupants.size()];
      return (new Army(*a, p));
    }
  return NULL;
}

std::string CreateScenarioRandomize::getDynamicSignpost(Signpost *signpost)
{
  char *dir = NULL;
  int xdir, ydir;
  Vector<int> signpostPos = signpost->getPos();
  City *nearCity = Citylist::getInstance()->getNearestCity(signpostPos);
  if (nearCity == NULL)
    return "nowhere";

  Vector<int> cityPos = nearCity->getPos();
  xdir = cityPos.x - signpostPos.x;
  ydir = cityPos.y - signpostPos.y;
  if (xdir >= 1 && ydir >= 1)
    dir = _("southeast");
  else if (xdir >= 1 && ydir == 0)
    dir = _("east");
  else if (xdir >= 1 && ydir <= -1)
    dir = _("northeast");
  else if (xdir == 0 && ydir >= 1)
    dir = _("south");
  else if (xdir == 0 && ydir <= -1)
    dir = _("north");
  else if (xdir <= -1 && ydir >= 1)
    dir = _("southwest");
  else if (xdir <= -1 && ydir == 0)
    dir = _("west");
  else if (xdir <= -1 && ydir <= -1)
    dir = _("northwest");
  return String::ucompose("%1 lies to the %2", nearCity->getName(), dir);
}
  
Reward *CreateScenarioRandomize::getNewRandomReward(bool hidden_ruins)
{
  int max_reward_types = 5;
  if (!hidden_ruins)
    max_reward_types--;
  int num = rand() % max_reward_types;
  Reward *reward = 0;

  switch (num)
    {
    case 0: //Gold
      reward = new Reward_Gold(Reward_Gold::getRandomGoldPieces());
      break;
    case 1: //Item
      reward = new Reward_Item(Reward_Item::getRandomItem());
      break;
    case 2: //Allies
	{
	  const ArmyProto *a = Reward_Allies::randomArmyAlly();
	  if (a)
	    reward = new Reward_Allies 
	      (a, Reward_Allies::getRandomAmountOfAllies());
	  else
	    reward = new Reward_Gold(Reward_Gold::getRandomGoldPieces());
	}
      break;
    case 3: //Map
	{
	  int x, y, width, height;
	  Reward_Map::getRandomMap(&x, &y, &width, &height);
	  reward = new Reward_Map (Vector<int>(x, y), "", height, width);
	  reward->setName(reward->getDescription());
	}
      break;
    case 4: //Hidden Ruin
	{
	  Ruin *r = Reward_Ruin::getRandomHiddenRuin();
	  reward = new Reward_Ruin (r);
	  reward->setName(reward->getDescription());
	}
      break;
    }
      
  if (reward)
    reward->setName(reward->getDescription());
  return reward;
}
