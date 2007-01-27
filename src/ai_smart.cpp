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
#include <algorithm>
#include <fstream>
#include <map>

#include "ai_smart.h"
#include "playerlist.h"
#include "armysetlist.h"
#include "FightDialog.h"
#include "stacklist.h"
#include "citylist.h"
#include "path.h"
#include "AI_Analysis.h"
#include "AI_Allocation.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
//#define debug(x)

AI_Smart::AI_Smart(string name, unsigned int armyset, SDL_Color color)
  :RealPlayer(name, armyset, color, Player::AI_SMART),d_mustmakemoney(0),d_isadvanced(false)
{
}

AI_Smart::AI_Smart(const Player& player)
    :RealPlayer(player),d_mustmakemoney(0),d_isadvanced(false)
{
    d_type = AI_SMART;
}

AI_Smart::AI_Smart(XML_Helper* helper)
    :RealPlayer(helper),d_mustmakemoney(0),d_isadvanced(false)
{
}

AI_Smart::~AI_Smart()
{
}

bool AI_Smart::startTurn()
{
    debug(getName() << " start_turn")
    clearActionlist();

    // the real stuff
    examineCities();
    int loopCount = 0;
    while (true)
    {
        // if the code is working, this loop will eventually terminate.
        // However, in case there is a bug somewhere, we don't want the AI players to move forever,
        // so loopCount is a safeguard to prevent that.
        loopCount++;
        if (loopCount >= 5)
            break;
        
        AI_Analysis *analysis = new AI_Analysis(this);
        AI_Allocation *allocation = new AI_Allocation(analysis, this);
        int moveCount = allocation->move();
        
        // tidying up
        delete analysis;
        delete allocation;
        
        // stop when no more stacks move
        if (moveCount == 0)
            break;
    }
    d_stacklist->setActivestack(0);
    return true;
}

bool AI_Smart::invadeCity(City* c)
{
    // always occupy an enemy city
    bool retval = cityOccupy(c);
    sinvadingCity.emit(c);
    soccupyingCity.emit(c);

    // Update its production
    maybeBuyProduction(c);
    setBestProduction(c);
    return retval;
}

bool AI_Smart::recruitHero(Hero* hero, int cost)
{
    debug("AI_Smart: hero recruited")
    return true;    //always recruit heroes
}

bool AI_Smart::levelArmy(Army* a)
{
    if (!a->canGainLevel())
        return false;
    
    Army::Stat stat = Army::RANGED;
    if (a->getStat(Army::SHOTS) == 0)
        stat = Army::STRENGTH;
    a->gainLevel(stat);

    Action_Level* item = new Action_Level();
    item->fillData(a->getId(), stat);
    d_actions.push_back(item);
    
    return true;
}

bool AI_Smart::chooseIfAdvanced(int freebasicslot, int freeadvancedslot)
{
    // Andrea: here obviously we can improve the decision
    //         for now AI chooses to buy advanced only if it cannot buy basic 
    if (freebasicslot==-1) return true;
    if (freeadvancedslot==-1) return false;

    //else we choose to buy a basic production
    return false;   
}

int AI_Smart::maybeBuyProduction(City *c)
{
    int freebasicslot = -1;
    int freeadvancedslot = -1;
    int freeslot= -1;

    bool advanced=false;
   
    int armytype = -1;

    freebasicslot=c->getFreeBasicSlot();
    freeadvancedslot=c->getFreeAdvancedSlot();

    debug("AI gold " << d_gold);
    debug("AI cityname " << c->getName());
    debug("basic slot index " << freebasicslot);
    debug("advanced slot index " << freeadvancedslot);

    // if freebasicslot == -1  and freeadvancedslot == -1 we do not have a free slot to add a new production
    if (freebasicslot==-1 && freeadvancedslot==-1) 
    {
        debug("AI cannot buy a new production no slot available."); 
        return -1;
    }

    advanced=chooseIfAdvanced(freebasicslot,freeadvancedslot);
    
    if (advanced) 
    {
        freeslot=freeadvancedslot;
        armytype=chooseArmyTypeToBuy(c,true);
        debug("AI choosed to buy advanced."); 
    }
    else 
    {
        freeslot=freebasicslot; 
        armytype=chooseArmyTypeToBuy(c,false);
        debug("AI choosed to buy basic."); 
    }

    debug("armytype i want to produce " << armytype)

    bool couldbuy=cityBuyProduction(c, freeslot, armytype, advanced);

    if (armytype >= 0 && couldbuy)
    {
        debug("YES I COULD BUY! type=" << armytype) 
        return armytype;
    }
    return -1;
}

int AI_Smart::setBestProduction(City *c)
{
    int selectbasic = -1;
    int selectadvanced = -1;
    int select = -1;
    int scorebasic = -1;
    int scoreadvanced= -1;
    bool actualisadvanced=c->getAdvancedProd();
    bool produceadv= false;

    // we try to determine the most attractive basic production
    for (int i = 0; i < c->getNoOfBasicProd(); i++)
    {
        if (c->getArmytype(i, false) == -1)    // no production in this slot
            continue;

        const Army *proto = c->getArmy(i, false);
        if (scoreArmyType(proto) > scorebasic)
        {
            selectbasic = i;
            scorebasic = scoreArmyType(proto);
        }
    }

    // we try to determine the most attractive advanced production
    for (int i = 0; i < c->getNoOfAdvancedProd(); i++)
    {
        if (c->getArmytype(i, false) == -1)    // no production in this slot
            continue;

        const Army *proto = c->getArmy(i, false);
        if (scoreArmyType(proto) > scoreadvanced)
        {
            selectadvanced = i;
            scoreadvanced = scoreArmyType(proto);
        }
    }

    if (scoreadvanced >= scorebasic) 
    {
        select=selectadvanced;
        produceadv=true;
    }
    else 
    {
        select=selectbasic;
        produceadv=false;
    }

    // this should work in any case; if no production exists, we set it to none
    if (produceadv && actualisadvanced) 
    {
        if (select != c->getProductionIndex())
        {
            cityChangeProduction(c, select, true);
            debug(getName() << " Set production to ADVANCED" << select << " in " << c->getName())
        }
    }

    if (!produceadv && !actualisadvanced) 
    {
        if (select != c->getProductionIndex())
        {
            cityChangeProduction(c, select, false);
            debug(getName() << " Set production to BASIC" << select << " in " << c->getName())
        }
    }

    if (produceadv && !actualisadvanced) 
    {
        if (select != c->getProductionIndex())
        {
            cityChangeProduction(c, select, true);
            debug(getName() << " Change production to ADVANCED" << select << " in " << c->getName())
        }
    }

    if (!produceadv && actualisadvanced) 
    {
        if (select != c->getProductionIndex())
        {
            cityChangeProduction(c, select, false);
            debug(getName() << " Change production to BASIC" << select << " in " << c->getName())
        }
    }

    d_isadvanced=produceadv;
    return c->getProductionIndex();
}

int AI_Smart::chooseArmyTypeToBuy(City *c, bool advanced)
{
    int bestScore, bestIndex;
    Uint32 size = 0;

    // Andrea: cannot understand why was addes this piece of code
    //         i comment it
    //if (c->//getProductionIndex() > 0)
    //{
    //    return -1;
    //}
    
    const Armysetlist* al = Armysetlist::getInstance();

    if (advanced)
        size = al->getSize(getArmyset());
    else 
        size = al->getSize(al->getStandardId());
        
    bestScore = -1;
    bestIndex = -1;
    
    debug("size " << size)

    for (unsigned int i = 0; i < size; i++)
    {
        const Army *proto = NULL;

        if (advanced)
            proto=al->getArmy(getArmyset(), i);
        else 
            proto=al->getArmy(al->getStandardId(), i);

        if (proto->getStat(Army::ARMY_BONUS) & Army::SHIP)
	{ 
	    debug("skipping : The Army was a ship. index=" << i)   
            continue;
	}

        if ((int)proto->getProductionCost() > d_gold)
	{ 
   	    debug("The production cost is too high for army of index=" << i)   
            continue;
	}
        
        //if (c->isAlreadyBought(proto,false)==false)
        if (advanced)
	{
	    if (c->hasProduction(proto->getType(), getArmyset())==false)
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
	else
	{
	    if (c->hasProduction(proto->getType(), al->getStandardId())==false)
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
    }

    debug("BEST INDEX=" << bestIndex)
    return bestIndex;
}

int AI_Smart::scoreArmyType(const Army *a)
{
    // UL: I have spent some minutes calculating the score of the default
    // armyset. The algorithm looks reasonable.
    int strength = a->getStat(Army::STRENGTH) + a->getStat(Army::DEFENSE)
                   + a->getStat(Army::HP) / 3;
    return (strength - a->getProduction());
}

bool AI_Smart::maybeRaiseLevel(City * city)
{
    bool upgraded=false;
    float cityDanger = 0.0;
    float worstDangers[3];

    worstDangers[0]=0.0;
    worstDangers[1]=0.0;
    worstDangers[2]=0.0;

    AI_Analysis *analysis = new AI_Analysis(this);
    cityDanger = analysis->getCityDanger(city);
    analysis->getCityWorstDangers(worstDangers);

    delete(analysis);
    
    for (int i=0;i<3;i++)    
        debug("worst Danger["<< i <<"]=" << worstDangers[i])

    debug("City Danger=" << cityDanger)

    // Here we calculate with the 3 worst dangers
    // so that we can upgrade at maximum 3 cities per turn
    for (int i=0;i<3;i++)    
    {
        if ((cityDanger == worstDangers[i]) && (worstDangers[i] > 0.0) )
        {
            upgraded=cityUpgradeDefense(city);
            if (!upgraded) 
	    {
                debug("I could not upgrade the city -- " << city->getName() << " -- because i had no money")
                d_mustmakemoney++;
	    }
	    else 
            {
                debug("I could upgrade the city -- " << city->getName() << " -- to level " << city->getDefenseLevel());
                d_mustmakemoney=0;
	    }
            break; 
        }
    }
    return upgraded;
}

void AI_Smart::examineCities()
{
    debug("Examinating Cities to see what we can do")
    Citylist* cl = Citylist::getInstance();
    for (Citylist::iterator it = cl->begin(); it != cl->end(); ++it)
    {
        City *city = &(*it);
        if ((city->isFriend(this)) && (city->isBurnt()==false))
        {
	    maybeRaiseLevel(city);
            // Here we wait to earn more money so we do not buy new productions
            if (d_mustmakemoney==0) maybeBuyProduction(city);
            setBestProduction(city);
        }
    }
}

// End of file
