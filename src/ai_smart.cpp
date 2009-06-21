// Copyright (C) 2004 John Farrell
// Copyright (C) 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2007, 2008 Ben Asselstine
// Copyright (C) 2007, 2008 Ole Laursen
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
#include <algorithm>
#include <map>

#include "ai_smart.h"
#include "playerlist.h"
#include "armysetlist.h"
#include "stacklist.h"
#include "citylist.h"
#include "path.h"
#include "AI_Analysis.h"
#include "AI_Allocation.h"
#include "AI_Diplomacy.h"
#include "action.h"
#include "xmlhelper.h"
#include "armyprodbase.h"
#include "armyproto.h"
#include "history.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
//#define debug(x)

AI_Smart::AI_Smart(string name, unsigned int armyset, SDL_Color color, int width, int height, int player_no)
  :RealPlayer(name, armyset, color, width, height, Player::AI_SMART, player_no),
   d_mustmakemoney(0), d_abort_requested(false)
{
}

AI_Smart::AI_Smart(const Player& player)
    :RealPlayer(player),d_mustmakemoney(0)
{
    d_type = AI_SMART;
    d_abort_requested = false;
}

AI_Smart::AI_Smart(XML_Helper* helper)
    :RealPlayer(helper),d_mustmakemoney(0), d_abort_requested(false)
{
}

AI_Smart::~AI_Smart()
{
}

bool AI_Smart::startTurn()
{
  sbusy.emit();
  AI_maybeBuyScout();
  maybeRecruitHero();
  
    debug(getName() << " start_turn")

    AI_Diplomacy diplomacy (this);

    diplomacy.considerCuspOfWar();

    if (getGold() < 500 && getUpkeep() > getIncome())
      d_mustmakemoney = 1;
    else
      d_mustmakemoney = 0;

    // the real stuff
    examineCities();

    AI_setupVectoring(18, 2, 30);

    sbusy.emit();
    //int loopCount = 0;
    while (true)
    {
        sbusy.emit();
        // if the code is working, this loop will eventually terminate.
        // However, in case there is a bug somewhere, we don't want the AI players to move forever,
        // so loopCount is a safeguard to prevent that.
        //loopCount++;
        //if (loopCount >= 5)
            //break;
        
        AI_Analysis *analysis = new AI_Analysis(this);
        AI_Allocation *allocation = new AI_Allocation(analysis, this);
        int moveCount = allocation->move();
        
        // tidying up
        delete analysis;
        delete allocation;
        
        // stop when no more stacks move
        if (moveCount == 0)
            break;
	if (d_abort_requested)
	  break;
    }
    d_stacklist->setActivestack(0);

    diplomacy.makeProposals();
    
    if (d_abort_requested)
      aborted_turn.emit();
    return true;
}

void AI_Smart::abortTurn()
{
  d_abort_requested = true;
  aborted_turn.emit();
}

void AI_Smart::invadeCity(City* c)
{
    // always occupy an enemy city
    cityOccupy(c);

    // Update its production
    maybeBuyProduction(c);
    setBestProduction(c);
}

void AI_Smart::levelArmy(Army* a)
{
    Army::Stat stat = Army::STRENGTH;
    doLevelArmy(a, stat);

    Action_Level* item = new Action_Level();
    item->fillData(a, stat);
    addAction(item);
}

int AI_Smart::maybeBuyProduction(City *c)
{
  Armysetlist *al = Armysetlist::getInstance();
    int freebasicslot = -1;
    int freeslot= -1;

    int armytype = -1;

    freebasicslot=c->getFreeBasicSlot();

    debug("AI gold " << d_gold);
    debug("AI cityname " << c->getName());
    debug("basic slot index " << freebasicslot);

    if (freebasicslot==-1)
    {
        debug("AI cannot buy a new production no slot available."); 
        return -1;
    }

    freeslot=freebasicslot; 
    armytype=chooseArmyTypeToBuy(c);
    //does this armytype beat the one we're currently producing?
    //is the one we're buying any better ?
    if (armytype == -1)
      return -1;

    bool buy = false;
    int slot = c->getActiveProductionSlot();
    if (slot == -1)
      buy = true;
    else if (scoreArmyType(al->getArmy(getArmyset(), armytype)) > 
	     scoreArmyType(c->getProductionBase(slot)))
      buy = true;

    if (buy)
      {
	debug("armytype i want to produce " << armytype)

	bool couldbuy=cityBuyProduction(c, freeslot, armytype);

	if (armytype >= 0 && couldbuy)
	  {
	    debug("YES I COULD BUY! type=" << armytype) 
	    return armytype;
	  }
      }
    return -1;
}

int AI_Smart::setBestProduction(City *c)
{
    int selectbasic = -1;
    int select = -1;
    int scorebasic = -1;

    // we try to determine the most attractive basic production
    for (Uint32 i = 0; i < c->getMaxNoOfProductionBases(); i++)
    {
        if (c->getArmytype(i) == -1)    // no production in this slot
            continue;

        const ArmyProdBase *proto = c->getProductionBase(i);
        if (scoreArmyType(proto) > scorebasic)
        {
            selectbasic = i;
            scorebasic = scoreArmyType(proto);
        }
    }


    select=selectbasic;

    if (select != c->getActiveProductionSlot())
    {
        cityChangeProduction(c, select);
        debug(getName() << " Set production to BASIC" << select << " in " << c->getName())
    }

    return c->getActiveProductionSlot();
}

int AI_Smart::chooseArmyTypeToBuy(City *c)
{
    int bestScore, bestIndex;
    Uint32 size = 0;

    const Armysetlist* al = Armysetlist::getInstance();

    size = al->getSize(getArmyset());
        
    bestScore = -1;
    bestIndex = -1;
    
    debug("size " << size)

    for (unsigned int i = 0; i < size; i++)
    {
        const ArmyProto *proto = NULL;

        proto=al->getArmy(getArmyset(), i);

	if (proto->getProductionCost() == 0)
	  continue;

        if ((int)proto->getProductionCost() > d_gold)
	{ 
   	    debug("The production cost is too high for army of index=" << i)   
            continue;
	}
        
       if (c->hasProductionBase(proto->getTypeId(), getArmyset())==false)
       {
         debug("I can buy it " << i)
         int score = scoreArmyType(proto);
         if (score >= bestScore)
         {
            bestIndex = i;
            bestScore = score;
         }
       }
       else debug("It was already bought " << i) 
    }

    debug("BEST INDEX=" << bestIndex)
    return bestIndex;
}

int AI_Smart::scoreArmyType(const ArmyProto *a)
{
  //this treats armies with turns of 7 or higher unfairly
  int max_strength = 60 / a->getProduction() * a->getStrength();

  int city_bonus = 0;
  switch (a->getArmyBonus())
    {
    case Army::ADD1STRINCITY: city_bonus += 5; break;
    case Army::ADD2STRINCITY: city_bonus += 10; break;
    }

  int any_other_bonus = 0;
  if (a->getArmyBonus() && city_bonus == 0)
    any_other_bonus += 2;

  int move_bonus = 0;
  if (a->getMaxMoves() >  10)
    move_bonus += 2;
  if (a->getMaxMoves() >=  20)
    move_bonus += 4;

  return max_strength + city_bonus + move_bonus + any_other_bonus;
}

int AI_Smart::scoreArmyType(const ArmyProdBase *a)
{
  //this treats armies with turns of 7 or higher unfairly
  int max_strength = 60 / a->getProduction() * a->getStrength();

  int city_bonus = 0;
  switch (a->getArmyBonus())
    {
    case Army::ADD1STRINCITY: city_bonus += 5; break;
    case Army::ADD2STRINCITY: city_bonus += 10; break;
    }

  int any_other_bonus = 0;
  if (a->getArmyBonus() && city_bonus == 0)
    any_other_bonus += 2;

  int move_bonus = 0;
  if (a->getMaxMoves() >  10)
    move_bonus += 2;
  if (a->getMaxMoves() >=  20)
    move_bonus += 4;

  return max_strength + city_bonus + move_bonus + any_other_bonus;
}

void AI_Smart::examineCities()
{
    debug("Examinating Cities to see what we can do")
    Citylist* cl = Citylist::getInstance();
    for (Citylist::iterator it = cl->begin(); it != cl->end(); ++it)
    {
        City *city = (*it);
        if ((city->isFriend(this)) && (city->isBurnt()==false))
        {
            // Here we wait to earn more money so we do not buy new productions
            if (d_mustmakemoney==0) maybeBuyProduction(city);
            setBestProduction(city);
        }
    }
}

// End of file
