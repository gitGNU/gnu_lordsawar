//  Copyright (C) 2008, Ben Asselstine
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

#include "CreateScenarioRandomize.h"

#include "File.h"
#include "citylist.h"
#include "signpost.h"
#include "armysetlist.h"
#include "playerlist.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

CreateScenarioRandomize::CreateScenarioRandomize()
{
    // Fill the namelists 
    bool success = true;
    
    std::ifstream file(File::getMiscFile("citynames").c_str());
    success &= loadNames(d_citynames, file);
    file.close();

    file.open(File::getMiscFile("templenames").c_str());
    success &= loadNames(d_templenames, file);
    file.close();

    file.open(File::getMiscFile("ruinnames").c_str());
    success &= loadNames(d_ruinnames, file);
    file.close();

    file.open(File::getMiscFile("signposts").c_str());
    success &= loadNames(d_signposts, file);
    file.close();

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


bool CreateScenarioRandomize::loadNames(std::vector<std::string>& list, std::ifstream& namefile)
{
    debug("CreateScenarioRandomize::loadNames")

    int counter;
    char buffer[101];
    buffer[100] = '\0';
    
    if (!namefile)
    {
        std::cerr << "Critical Error: Couldn't open names data file\n";
        return false;
    }

    namefile >> counter;
    list.resize(counter);

    //with getline, the first call will get the first line to the end, i.e.
    //a newline character. Thus, we throw away the result of the first call.
    namefile.getline(buffer, 100);

    for (counter--; counter >= 0; counter--)
    {
        namefile.getline(buffer, 100);
        list[counter] = std::string(buffer);
    }

    return true;
}

std::string CreateScenarioRandomize::popRandomListName(std::vector<std::string> &list)
{
  std::string name;
  if (list.empty())
    return "";
  int randno = rand() % list.size();
  name = list[randno];
  return name;
}

std::string CreateScenarioRandomize::popRandomCityName()
{
  return popRandomListName(d_citynames);
}

void CreateScenarioRandomize::pushRandomCityName(std::string name)
{
  d_citynames.push_back(name);
}

std::string CreateScenarioRandomize::popRandomRuinName()
{
  return popRandomListName(d_ruinnames);
}

void CreateScenarioRandomize::pushRandomRuinName(std::string name)
{
  d_ruinnames.push_back(name);
}

std::string CreateScenarioRandomize::popRandomTempleName()
{
  return popRandomListName(d_templenames);
}

void CreateScenarioRandomize::pushRandomTempleName(std::string name)
{
  d_templenames.push_back(name);
}

std::string CreateScenarioRandomize::popRandomSignpost()
{
  return popRandomListName(d_signposts);
}

void CreateScenarioRandomize::pushRandomSignpost(std::string name)
{
  d_signposts.push_back(name);
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
  std::vector<const Army*> occupants;
  Armysetlist *al = Armysetlist::getInstance();
  for (unsigned int j = 0; j < al->getSize(p->getArmyset()); j++)
    {
      const Army *a = al->getArmy (p->getArmyset(), j);
      if (a->getDefendsRuins())
	occupants.push_back(a);
    }
            
  if (!occupants.empty())
    {
      const Army *a = occupants[rand() % occupants.size()];
      return (new Army(*a, p));
    }
  return NULL;
}

std::string CreateScenarioRandomize::getDynamicSignpost(Signpost *signpost)
{
  char *dir = NULL;
  int xdir, ydir;
  char buf[101]; buf[100] = '\0';
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
  snprintf(buf,100,_("%s lies to the %s"), 
	   nearCity->getName().c_str(), dir);
  return buf;
}
  
Reward *CreateScenarioRandomize::getNewRandomReward(bool hidden_ruins)
{
  int max_reward_types = 5;
  if (!hidden_ruins)
    max_reward_types--;
  int num = rand() % max_reward_types;
  Reward *reward;

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
	  const Army *a = Reward_Allies::randomArmyAlly();
	  if (a)
	    reward = new Reward_Allies 
	      (new Army (*a), Reward_Allies::getRandomAmountOfAllies());
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
