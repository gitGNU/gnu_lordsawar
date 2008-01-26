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
#include "AI_Allocation.h"
#include "player.h"
#include "playerlist.h"
#include "citylist.h"
#include "stacklist.h"
#include "stack.h"
#include "city.h"
#include "Threat.h"
#include "ruin.h"
#include "path.h"
#include "ruinlist.h"
#include "GameMap.h"
#include "Threatlist.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
//#define debug(x)

AI_Allocation* AI_Allocation::s_instance = 0;


AI_Allocation::AI_Allocation(AI_Analysis *analysis, Player *owner)
    :d_owner(owner), d_analysis(analysis)
{
    s_instance = this;
}

AI_Allocation::~AI_Allocation()
{
    s_instance = 0;
}

void AI_Allocation::deleteStack(Stack* s)
{
    // we need to remove collaterally eradicated stacks before they make
    // trouble
    if (s_instance)
        s_instance->d_stacks->remove(s);
}

int AI_Allocation::move()
{
    Citylist *allCities = Citylist::getInstance();

    // move stacks
    d_stacks = new Stacklist(d_owner->getStacklist());

    debug(d_owner->getName() << " starts with " << d_stacks->size() << " stacks to do something with")

    int count = allocateDefensiveStacks(allCities);
    debug(d_owner->getName() << " has " << d_stacks->size() << " stacks after assigning defenders")

    // stacks which are assigned to defence are no longer in 'stacks',
    // so we can tell the rest to do something
    count += allocateStacksToThreats();
    debug(d_owner->getName() << " still has " << d_stacks->size() << " stacks after allocating stacks to threats")

    count += defaultStackMovements();

    d_stacks->clear();
    delete d_stacks;
    d_stacks = 0;

    // return 0 (leads to an end in the loop in ai_smart) if the game has ended
    if (Playerlist::isFinished())
        return 0;

    return count;
}

// for each city, if it is in danger at all, allocate a stack to be its
// defender. These stacks sit in the NW corner and don't move out of the city.
int AI_Allocation::allocateDefensiveStacks(Citylist *allCities)
{
    int count = 0;
    for (Citylist::iterator cit = allCities->begin(); cit != allCities->end(); ++cit)
    {
        City *city = &(*cit);
        if (!city->isFriend(d_owner) || city->isBurnt())
            continue;

        float cityDanger = d_analysis->getCityDanger(city);
        if (cityDanger <= 0.0) continue; // city is not in danger, so allocate no defenders

        // Look how many defenders the city already has
        std::vector<Stack*> defenders = Stacklist::defendersInCity(city);
        vector<Stack*>::iterator it;
        float totalDefenderStrength = 0.0;
        for (it = defenders.begin(); it != defenders.end(); it++)
        {
            Stack *defender = *it;
            d_stacks->remove(defender);
            float stackStrength = d_analysis->assessStackStrength(defender);
            debug("assign stack " << defender->getId() << " with strength " << stackStrength
                  << " to " << city->getName() << " because its danger is " << cityDanger)

            totalDefenderStrength += stackStrength;
            if (totalDefenderStrength > cityDanger)
                break;
            // if we get here, we have assigned defenders but not enough to
            // counter the danger that the city is in
        }

        // allocate nearby stacks to come back to the city,
        // because we don't have enough defence
        while (totalDefenderStrength < cityDanger)
        {
            Stack *s = findClosestStack(city->getPos(), 2);
            if (!s) break;
            debug("Stack " << s->getId() << " should return to " << city->getName() << " to defend")
            Vector<int> *dest = getFreeSpotInCity(city, s->size());
            if (!dest) break;
            
            d_stacks->remove(s);
            d_owner->getStacklist()->setActivestack(s);
            MoveResult *result = moveStack(s, *dest);
            if (result && result->didSomething())
                count++;
            if (result)
                delete result;
            delete dest;
        }

        if (totalDefenderStrength < cityDanger)
        {
            debug(city->getName() << " cannot be adequately defended")
        }
    }
    return count;
}

int AI_Allocation::allocateStacksToThreats()
{
    int count = 0;
    const Threatlist *threats = d_analysis->getThreatsInOrder();
    //debug("Threats to " << d_owner->getName() << " are " << threats->toString())
    debug("We start with " <<threats->size() <<" threats.")

    for (Threatlist::const_iterator it = threats->begin(); it != threats->end(); ++it)
    {
        if (d_stacks->size() == 0)
            break;

        Threat *threat = (*it);
        while (true)
        {
            // make this short when we have no more players
            if (Playerlist::isFinished())
                return 0;
            
            Stack *attacker = findBestAttackerFor(threat);
            // if there is nobody to attack the threat, go onto the next one
            if (!attacker) break;

            d_owner->getStacklist()->setActivestack(attacker);
            Vector<int> dest = threat->getClosestPoint(attacker->getPos());
            float myStr = d_analysis->assessStackStrength(attacker);
            float opponentStr = 0.0;
            if (threat->isCity())
                opponentStr = threat->strength();
            else
            {
                Stack *opponent = Stacklist::getObjectAt(dest);
                opponentStr = (opponent == 0) ? 0.0 : d_analysis->assessStackStrength(opponent);
            }

            // do we think we will lose? Stacks of size 8 have to attack, that's their job
            if (myStr < opponentStr && attacker->size() < 8)
            {
                debug("we are not strong enough to fight " << threat->toString())
                break;
            }

            debug("assign stack " << attacker->getId() << " with strength " << myStr
                  << " to attack " << threat->toString()
                  << " which we think has strength " << opponentStr)
            d_stacks->remove(attacker);
            MoveResult *result = moveStack(attacker, dest);
            if (result && result->didSomething())
                count++;

            if (result && result->getFightResult() != Fight::DEFENDER_WON)
            {
                // if the threat has been removed, go onto the next one
                if (threat->strength() == 0.0)
                {
                    delete result;
                    result = 0;
                    break;
                }
            }

            if (result)
                delete result;
            result = 0;
        }
    }
    return count;
}

Vector<int> *AI_Allocation::getFreeSpotInCity(City *city, int stackSize)
{
    // northwest
    Vector<int> *result = new Vector<int>(city->getPos());
    Stack *s = Stacklist::getObjectAt(*result);
    if (!s)
        return result;
    else
        if (s->size() + stackSize <= 8) return result;
    
    // northeast
    result->x++;
    s = Stacklist::getObjectAt(*result);
    if (!s)
        return result;
    else
        if (s->size() + stackSize <= 8) return result;
    
    // southeast
    result->y++;
    s = Stacklist::getObjectAt(*result);
    if (!s)
        return result;
    else
        if (s->size() + stackSize <= 8) return result;
    
    // southwest
    result->x--;
    s = Stacklist::getObjectAt(*result);
    if (!s)
        return result;
    else
        if (s->size() + stackSize <= 8) return result;
    
    // no room
    delete result;
    return 0;
}

Stack *AI_Allocation::findClosestStack(Vector<int> pos, int limitInMoves)
{
    Stack *best = 0;
    float bestScore = 1000.0;
    for (Stacklist::iterator it = d_stacks->begin(); it != d_stacks->end(); ++it)
    {
        Stack* s = *it;
        Vector<int> spos = s->getPos();
        // UL: stacks can move diagonally
        int distToThreat = abs(pos.x - spos.x);
        int disty = abs(pos.y - spos.y);
        if (distToThreat < disty)
            distToThreat = disty;

        float movesToThreat = (distToThreat + 6.0) / 7.0;
        if (movesToThreat > (float) limitInMoves) continue;
        if (movesToThreat < bestScore)
        {
            best = s;
            bestScore = movesToThreat;
        }
    }
    return best;
}

Stack *AI_Allocation::findBestAttackerFor(Threat *threat)
{
    Stack *best = 0;
    float bestScore = 0.0;
    for (Stacklist::iterator it = d_stacks->begin(); it != d_stacks->end(); ++it)
    {
        Stack* s = *it;
        Vector<int> closestPoint = threat->getClosestPoint(s->getPos());
        // threat has been destroyed anyway
        if (closestPoint.x == -1)
            return 0;
        Vector<int> spos = s->getPos();

        // UL: consider diagonal movement of stacks
        int distToThreat = abs(closestPoint.x - spos.x);
        int disty = abs(closestPoint.y - spos.y);
        if (distToThreat < disty)
            distToThreat = disty;

        int movesToThreat = (distToThreat + 6) / 7;
        // don't bother moving more than three moves to kill anything
        if (movesToThreat > 3) continue;
        float score = d_analysis->assessStackStrength(s) / (float) movesToThreat;
        if (score > bestScore)
        {
            best = s;
            bestScore = score;
        }
    }
    return best;
}

int AI_Allocation::defaultStackMovements()
{
    int count = 0;
    Citylist *allCities = Citylist::getInstance();
    debug("Default movement for " <<d_stacks->size() <<" stacks")

    for (Stacklist::iterator it = d_stacks->begin(); it != d_stacks->end(); ++it)
    {
        if (Playerlist::isFinished())
            return 0;
        
        Stack* s = *it;
        debug("thinking about stack " << s->getId() <<" at ("<<s->getPos().x<<","<<s->getPos().y<<")")
        d_owner->getStacklist()->setActivestack(s);
        MoveResult *result = 0;
        if (s->size() >= 8)
        {
            City* enemyCity = allCities->getNearestEnemyCity(s->getPos());
            if (enemyCity)
            {
                debug("let's attack " <<enemyCity->getName())
                result = moveStack(s, enemyCity->getPos());
                if (result && result->didSomething()) count++;
            }
	    else
	    {
	      enemyCity = allCities->getNearestForeignCity(s->getPos());
	      if (enemyCity)
		{
		  s->getPlayer()->proposeDiplomacy(Player::PROPOSE_WAR,
						   enemyCity->getPlayer());
		  debug("let's attack " <<enemyCity->getName())
		  result = moveStack(s, enemyCity->getPos());
		  if (result && result->didSomething()) count++;
		}
	    }

            if (!result || !result->didSomething())
            {
                // for some reason (islands are one bet), we could not attack the
                // enemy city. Let's iterator through all cities and attack the first
                // one we can lay our hands on.
                debug("Mmmh, did not work.")
                for (Citylist::iterator cit = allCities->begin(); cit != allCities->end(); cit++)
                    if ((*cit).getPlayer() != d_owner)
                    {
                        debug("Let's try "<<(*cit).getName() <<" instead.")
                        result = moveStack(s, (*cit).getPos());
                        if (result && result->moveSucceeded())
                        {
                            debug("Worked")
                            count++;
                            break;
                        }
                    }
            }
        }
        else
        {
            result = stackReinforce(s);
            if (result && result->didSomething()) count++;
        }
        // a stack has died -> further iterating leads to a crash so we
        // start again (recursive programming, therefore the break
        // afterwards is _VERY_ important)
        if (result) {
            if (result->stackVanished())
            {
                d_stacks->remove(s);
                delete result;
                result = 0;
                break;
            }
        }
        delete result;
        result = 0;
    }
    return count;
}

MoveResult *AI_Allocation::stackReinforce(Stack *s)
{
    Citylist *allCities = Citylist::getInstance();
    float mostNeeded = -1000.0;
    City *cityNeeds = 0;
    int moves = 1000;
    for (Citylist::iterator it = allCities->begin(); it != allCities->end(); ++it)
    {
        City *city = &(*it);
        Vector<int> spos = s->getPos();
        Vector<int> cpos = city->getPos();
        // UL: stacks can move diagnonally
        int distToCity = abs(spos.x - cpos.x);
        int disty = abs(spos.y - cpos.y);
        if (distToCity < disty)
            distToCity = disty;

        int movesToCity = (distToCity + 6) / 7;
        if (movesToCity > 3) continue;
        float need = d_analysis->reinforcementsNeeded(city);
        if (need > mostNeeded)
        {
            cityNeeds = city;
            mostNeeded = need;
            moves = movesToCity;
        }
    }

    if (cityNeeds) {
        debug("stack is sent to reinforce " << cityNeeds->getName() <<" if possible")
        // don't forget to send the stack to a free field within the city
        Vector<int>* dest = getFreeSpotInCity(cityNeeds, s->size());
        if (dest)
        {
            d_analysis->reinforce(cityNeeds, s, moves);
            Vector<int> target = *dest;
            delete dest;
            return moveStack(s, target);
        }
    }

    City *target = allCities->getNearestFriendlyCity(s->getPos());
    if (!target) // no friendly city?
        return 0;
    if (target->contains(s->getPos()))
    {
        // already at that city
        return shuffleStacksWithinCity(target, s);
    }
    else
    {
        Vector<int> p = target->getPos();
        Vector<int> alt;
        debug(d_owner->getName() << " has decided to retreat to " << target->getName())
        MoveResult *result = moveStack(s, p);
        if (result->moveSucceeded())
        {
            debug(d_owner->getName() << " The best position in the city is available")
            return result;
        }
        alt.x = p.x + 1;
        alt.y = p.y;
        result = moveStack(s, alt);
        if (result->moveSucceeded())
        {
            debug(d_owner->getName() << " Moving into the northeast")
            return result;
        }
        alt.x = p.x;
        alt.y = p.y + 1;
        result = moveStack(s, alt);
        if (result->moveSucceeded())
        {
            debug(d_owner->getName() << " Moving into the southwest")
            return result;
        }
        alt.x = p.x + 1;
        alt.y = p.y + 1;
        result = moveStack(s, alt);
        if (result->moveSucceeded())
        {
            debug(d_owner->getName() << " Moving into the southeast")
            return result;
        }
    }
    return 0;
}

void AI_Allocation::searchRuin(Stack *stack, Ruin *ruin)
{
    d_owner->stackSearchRuin(stack, ruin);
    // what to do if the ruin search fails?
}

MoveResult *AI_Allocation::shuffleStacksWithinCity(City *city, Stack *stack)
{
    Stack *join;
    int cx = city->getPos().x;
    int cy = city->getPos().y;
    int sx = stack->getPos().x;
    int sy = stack->getPos().y;
    if (sx == cx && sy == cy)
        // already in the "primary" position
        return 0;
    join = Stacklist::getObjectAt(city->getPos());
    if (!join)
    {
        // move to the northwest unless there is nothing there
        if (!Stacklist::getObjectAt(city->getPos().x, city->getPos().y))
            return 0;
        return moveStack(stack, city->getPos());
    }
    else if (join->size() + stack->size() <= 8)
    {
        // join with the stack in the northwest
        return moveStack(stack, city->getPos());
    }
    // To do any more we would have to split the stack, which sounds hard.
    return 0;
}

MoveResult *AI_Allocation::moveStack(Stack *stack, Vector<int> pos)
{
    return d_owner->stackMove(stack, pos, false);
}
// End of file
