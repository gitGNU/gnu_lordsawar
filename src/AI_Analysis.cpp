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

#include <iostream>
#include "AI_Analysis.h"
#include "citylist.h"
#include "Threatlist.h"
#include "Threat.h"
#include "playerlist.h"
#include "stacklist.h"
#include "ruinlist.h"
#include "army.h"
#include "AICityInfo.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
//#define debug(x)

//this instance is just needed in case one of the observed stacks dies during
//the analysis (and the following actions).
AI_Analysis* AI_Analysis::instance = 0;

AI_Analysis::AI_Analysis(Player *owner)
    :d_owner(owner)
{
    d_stacks = 0;
    d_threats = new Threatlist();
    d_stacks = new Stacklist(owner->getStacklist());

    examineCities();
    examineRuins();
    examineStacks();
    calculateDanger();

    instance = this;
}

AI_Analysis::~AI_Analysis()
{
    instance = 0;

    d_threats->flClear();
    delete d_threats;
    delete d_stacks;

    while (!d_cityInfo.empty())
    {
        delete (*d_cityInfo.begin()).second;
        d_cityInfo.erase(d_cityInfo.begin());
    }
}

void AI_Analysis::deleteStack(Stack* s)
{
    if (instance)
    {
        debug("delete stack from ai_analysis")
        instance->d_threats->deleteStack(s);
        instance->d_stacks->remove(s);
        debug("stack " << s << " died")
    }
}

float AI_Analysis::assessStackStrength(const Stack *stack)
{
    debug("assessing stack with id "<<stack->getId())
    
    if (!instance)
        return stack->size() * 5.0;

    if (stack->getPlayer() == instance->d_owner)
    {
        // our stack, so we can look inside it
        float total = 0.0;
        for (Stack::const_iterator it = stack->begin(); it != stack->end(); it++)
            total += (*it)->getStat(Army::STRENGTH);

        return total;
    }
    else
    {
        // enemy stack, no cheating!
        // if we were smarter, we would remember all stacks we had seen before and return a better number here.
        // We don't assume a too high average strength
        if (stack->getStrongestArmy()->getStat(Army::STRENGTH) < 5)
            return stack->size() * stack->getStrongestArmy()->getStat(Army::STRENGTH);
        return stack->size() * 5.0;
    }
}

const Threatlist* AI_Analysis::getThreatsInOrder()
{
    d_threats->sortByValue();
    return d_threats;
}

const Threatlist* AI_Analysis::getThreatsInOrder(PG_Point pos)
{
    d_threats->sortByDistance(pos);
    return d_threats;
}
    

void AI_Analysis::getCityWorstDangers(float dangers[3])
{
    std::map<string, AICityInfo *>::iterator it;


    // i wanto to have a result array with the first worst dangers
    for (int i=0;i<3;i++)
    {
        float tmp=0.0;

        for (it=d_cityInfo.begin();it!=d_cityInfo.end();it++)
	{
            tmp=(*it->second).getDanger();

            if (dangers[i] < tmp) 
	    {
	         if (i>0) // If The iteration is not the first we do want to avoid to store 
                          // already stored worst dangers
		 {
                     if(tmp < dangers[i-1]) 
                         dangers[i]=tmp;
		 }
                 else 
                 {
		     dangers[i]=tmp; // The first iteration we get the real worst Danger
		 }
	    }
	}
    }

    return;
}

float AI_Analysis::getCityDanger(City *city)
{
    // city does not exist in the map
    if (d_cityInfo.find(city->getName()) == d_cityInfo.end())
        return 0.0;
    
    debug("Threats to " << city->getName() << " are " << d_cityInfo[city->getName()]->getThreats()->toString())
    return d_cityInfo[city->getName()]->getDanger();
}

void AI_Analysis::reinforce(City *city, Stack *stack, int movesToArrive)
{
    if (d_cityInfo.find(city->getName()) == d_cityInfo.end())
        return;

    AICityInfo *info = d_cityInfo[city->getName()];
    info->addReinforcements(assessStackStrength(stack) / (float) movesToArrive);
}

float AI_Analysis::reinforcementsNeeded(City *city)
{
    if (d_cityInfo.find(city->getName()) == d_cityInfo.end())
        return -1000.0;

    const AICityInfo* info = d_cityInfo[city->getName()];
    return info->getDanger() - info->getReinforcements();
}

void AI_Analysis::examineCities()
{
    Citylist* cl = Citylist::getInstance();
    for (Citylist::iterator it = cl->begin(); it != cl->end(); ++it)
    {
        City *city = &(*it);
        if (!city->isFriend(d_owner) && !city->isBurnt())
        {
            d_threats->push_back(new Threat(city));
        }
    }
}

void AI_Analysis::examineStacks()
{
    // add all enemy stacks to the list of threats
    Playerlist *players = Playerlist::getInstance();
    for (Playerlist::iterator it = players->begin(); it != players->end(); it++)
    {
        Player *player = (*it);
        if (player == d_owner)
            continue;

        Stacklist *sl = player->getStacklist();
        for (Stacklist::iterator sit = sl->begin(); sit != sl->end(); ++sit)
        {
            d_threats->addStack(*sit);
        }
    }
}

void AI_Analysis::examineRuins()
{
    // enable the searching of ruins before you start to regard them as threats
/*    Ruinlist *ruins = Ruinlist::getInstance();
    for (Ruinlist::iterator it = ruins->begin(); it != ruins->end(); ++it)
    {
        Ruin ruin = *it;
        if (!ruin.isSearched())
        {
            d_threats->push_back(new Threat(&ruin));
        }
    }
*/
}

void AI_Analysis::calculateDanger()
{
    Citylist* cl = Citylist::getInstance();
    for (Citylist::iterator it = cl->begin(); it != cl->end(); ++it)
    {
        City *city = &(*it);
        if (city->isFriend(d_owner))
        {
            AICityInfo *info = new AICityInfo(city);
            d_threats->findThreats(info);
            d_cityInfo[city->getName()] = info;
        }
    }
}

// End of file
