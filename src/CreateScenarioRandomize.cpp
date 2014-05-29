//  Copyright (C) 2008, 2009, 2014 Ben Asselstine
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

#include <sstream>
#include <iostream>
#include <fstream>
#include "ucompose.hpp"

#include "CreateScenarioRandomize.h"

#include "File.h"
#include "citylist.h"
#include "city.h"
#include "ruin.h"
#include "temple.h"
#include "signpost.h"
#include "armysetlist.h"
#include "playerlist.h"
#include "SightMap.h"
#include "reward.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

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

Glib::ustring CreateScenarioRandomize::popRandomCityName()
{
  Glib::ustring name = d_citynames->popRandomName().c_str();
  if (name == "")
    return City::getDefaultName();
  return name;
}

void CreateScenarioRandomize::pushRandomCityName(Glib::ustring name)
{
  d_citynames->push_back(name);
}

Glib::ustring CreateScenarioRandomize::popRandomRuinName()
{
  Glib::ustring name = d_ruinnames->popRandomName().c_str();
  if (name == "")
    return Ruin::getDefaultName();
  return name;
}

void CreateScenarioRandomize::pushRandomRuinName(Glib::ustring name)
{
  d_ruinnames->push_back(name);
}

Glib::ustring CreateScenarioRandomize::popRandomTempleName()
{
  Glib::ustring name = d_templenames->popRandomName().c_str();
  if (name == "")
    return Temple::getDefaultName();
  return name;
}

void CreateScenarioRandomize::pushRandomTempleName(Glib::ustring name)
{
  d_templenames->push_back(name);
}

Glib::ustring CreateScenarioRandomize::popRandomSignpost()
{
  return d_signposts->popRandomName().c_str();
}

void CreateScenarioRandomize::pushRandomSignpost(Glib::ustring name)
{
  d_signposts->push_back(name);
}

guint32 CreateScenarioRandomize::getRandomCityIncome(bool capital)
{
  if (capital)
    return 33 + (rand() % 8);
  else
    return 15 + (rand() % 12);
}

Army * CreateScenarioRandomize::getRandomRuinKeeper(Player *p)
{
  Armysetlist *al = Armysetlist::getInstance();
  const ArmyProto *a= al->getArmyset(p->getArmyset())->getRandomRuinKeeper();
  if (a)
    return (new Army(*a, p));

  return NULL;
}

Glib::ustring CreateScenarioRandomize::get_direction(int xdir, int ydir)
{
  if (xdir >= 1 && ydir >= 1)
    return _("southeast");
  else if (xdir >= 1 && ydir == 0)
    return _("east");
  else if (xdir >= 1 && ydir <= -1)
    return _("northeast");
  else if (xdir == 0 && ydir >= 1)
    return _("south");
  else if (xdir == 0 && ydir <= -1)
    return _("north");
  else if (xdir <= -1 && ydir >= 1)
    return _("southwest");
  else if (xdir <= -1 && ydir == 0)
    return _("west");
  else if (xdir <= -1 && ydir <= -1)
    return _("northwest");
  return _("nowhere");
}

Glib::ustring CreateScenarioRandomize::getDynamicSignpost(Signpost *signpost)
{
  int xdir, ydir;
  Vector<int> signpostPos = signpost->getPos();
  City *nearCity = Citylist::getInstance()->getNearestCity(signpostPos);
  if (nearCity == NULL)
    return _("nowhere");

  Vector<int> cityPos = nearCity->getPos();
  xdir = cityPos.x - signpostPos.x;
  ydir = cityPos.y - signpostPos.y;
  Glib::ustring dir = get_direction(xdir, ydir);
  return String::ucompose(_("%1 lies to the %2"), nearCity->getName(), dir);
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

int CreateScenarioRandomize::adjustBaseGold (int base_gold)
{
  int gold = base_gold + ((rand() % 7) - 4);
  if (gold < 0)
    gold = 0;
  return gold;
}

void CreateScenarioRandomize::getBaseGold (int difficulty, int *base_gold)
{
  if (difficulty < 50)
    *base_gold = 131;
  else if (difficulty < 60)
    *base_gold = 129;
  else if (difficulty < 70)
    *base_gold = 127;
  else if (difficulty < 80)
    *base_gold = 125;
  else if (difficulty < 90)
    *base_gold = 123;
  else
    *base_gold = 121;
}

Glib::ustring CreateScenarioRandomize::getPlayerName(Shield::Colour id)
{
  Glib::ustring name = "";
  switch (id)
    {
    case Shield::WHITE: 
      name = _("The Sirians");
      break;
    case Shield::GREEN: 
      name = _("Elvallie");
      break;
    case Shield::YELLOW: 
      name = _("Storm Giants");
      break;
    case Shield::DARK_BLUE: 
      name = _("Horse Lords");
      break;
    case Shield::ORANGE: 
      name = _("Grey Dwarves");
      break;
    case Shield::LIGHT_BLUE: 
      name = _("The Selentines");
      break;
    case Shield::RED:
      name = _("Orcs of Kor");
      break;
    case Shield::BLACK:
      name = _("Lord Bane");
      break;
    case Shield::NEUTRAL:
      name = _("Neutrals");
      break;
    }
  return name;
}
