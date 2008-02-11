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
#include <algorithm>

#include "Threatlist.h"
#include "stack.h"
#include "ruin.h"
#include "player.h"
#include "AICityInfo.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl<<flush;}
#define debug(x)

Threatlist::Threatlist()
{
}

Threatlist::~Threatlist()
{
}

void Threatlist::sortByValue()
{
    // bubble sort is the easiest thing I remember
    bool sorted = false;

    while (!sorted)
    {
        sorted = true;

        // setup
        iterator it = begin();
        iterator nextit = it;
        nextit++;

        // now loop through the list
        for (; nextit != end(); it++, nextit++)
            if ((*it)->value() < (*nextit)->value())
            {
                // exchange the two threats
                sorted = false;
                Threat* tmp = (*nextit);
                erase(nextit);
                nextit = it;

                it = insert(nextit, tmp);
            }
    }
}

void Threatlist::sortByDistance(Vector<int> pos)
{
    // A simple bubble sort is propably too computationally expensive here.
    // To reduce the overhead, we first calculate all distances, store them
    // in another list and sort both lists together.

    std::list<int> distances;

    for (iterator it = begin(); it != end(); it++)
    {
        int dist = abs((*it)->getClosestPoint(pos).x - pos.x);
        if (dist < abs((*it)->getClosestPoint(pos).y - pos.y))
            dist = abs((*it)->getClosestPoint(pos).y - pos.y);
        distances.push_back(dist);
    }

    // now again a bubble sort :)
    bool sorted = false;

    while (!sorted)
    {
        sorted = true;

        // setup
        std::list<int>::iterator dit = distances.begin();
        std::list<int>::iterator dnextit = distances.begin();
        dnextit++;

        iterator it = begin();
        iterator nextit = it;
        nextit++;

        for (; nextit != end(); it++, nextit++, dit++, dnextit++)
            if ((*dit) > (*dnextit))
            {
                // exchange the items in both lists
                sorted = false;
                
                Threat* tmp = (*nextit);
                erase(nextit);
                nextit = it;
                it = insert(nextit, tmp);

                int val = (*dnextit);
                distances.erase(dnextit);
                dnextit = dit;
                dit = distances.insert(dnextit, val);
            }
    }
}

void Threatlist::addStack(Stack *stack)
{
    for (iterator it = begin(); it != end(); it++)
    {
        Threat *threat = *it;
        if (threat->Near(stack->getPos(), stack->getPlayer()))
        {
            threat->addStack(stack);
            return;
        }
    }

    Threat *t = new Threat(stack);
    push_back(t);
}

void Threatlist::addRuin(Ruin *ruin)
{
    //if the ruin is abandoned, it is not a threat nor a valuable target
    if (ruin->isSearched())
        return;

    Threat *t = new Threat(ruin);
    push_back(t);
}

void Threatlist::findThreats(AICityInfo *info) const
{
    //shortcut
    Vector<int> location = info->getPos();

    for (const_iterator it = begin(); it != end(); it++)
    {
        Threat *threat = *it;
        Vector<int> closestPoint = threat->getClosestPoint(location);

        //This happens only if a threat doesn't contain any stacks any longer.
        if (closestPoint.x == -1)
            continue;

        // UL: stacks can move diagonally
        int distToThreat = abs(closestPoint.x - location.x);
        int disty = abs(closestPoint.y - location.y);
        if (distToThreat < disty)
            distToThreat = disty;

        float movesToThreat = ((float) distToThreat + 6.0) / 7.0;
        debug("moves to " << threat->toString() << " is " << movesToThreat)

        //Ignore threats too far away
        if (movesToThreat > 3.0)
            continue;

        if (movesToThreat == 0.0)
            movesToThreat++;

        float strength = threat->strength();
        if (strength == 0.0)
            continue;

        debug("strength of " << threat->toString() << " is " << strength)
        float dangerFromThisThreat = strength / movesToThreat;
        info->addThreat(dangerFromThisThreat, threat);

        // a side-effect of this calculation is that we calculate the overall
        // danger from each threat. If a threat threatens multiple cities, it
        // is considered especially dangerous, so it is okay that we add the
        // danger multiple times.
        threat->addDanger(dangerFromThisThreat);
    }
}

void Threatlist::deleteStack(Stack* s)
{
    for (Threatlist::iterator it = begin(); it != end(); it++)
        (*it)->deleteStack(s);
}

string Threatlist::toString() const
{
    string result = "{";
    bool first = true;
    for (const_iterator it = begin(); it != end(); it++)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            result += ",";
        }
        result = result + " " + (*it)->toString();
    }
    result += " }";
    return result;
}

void Threatlist::flClear()
{
    for (iterator it = begin(); it != end(); it++)
        delete (*it);

    clear();
}

Threatlist::iterator Threatlist::flErase(iterator object)
{
    delete (*object);
    return erase(object);
}

bool Threatlist::flRemove(Threat* object)
{
    iterator threatit = find(begin(), end(), object);
    if (threatit != end())
    {
        delete object;
        erase(threatit);
        return true;
    }
    return false;
}

// End of file
