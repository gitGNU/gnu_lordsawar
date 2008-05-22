// Copyright (C) 2004 John Farrell
// Copyright (C) 2004, 2005 Ulf Lorenz
// Copyright (C) 2005 Andrea Paternesi
// Copyright (C) 2007, 2008 Ben Asselstine
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

#include "Threat.h"
#include "defs.h"
#include "stacklist.h"
#include <iostream>
#include "city.h"
#include "ruin.h"
#include "stack.h"
#include "playerlist.h"
#include "AI_Analysis.h"

using namespace std;

Threat::Threat(City *c)
    :Ownable(*c), d_city(c), d_ruin(0), d_danger(0)
{
    d_stacks = new Stacklist();
}

Threat::Threat(Stack *s)
    :Ownable(*s), d_city(0), d_ruin(0), d_danger(0)
{
    d_stacks = new Stacklist();
    d_stacks->push_front(s);
}

Threat::Threat(Ruin *r)
    :Ownable((Player *)0), d_city(0), d_ruin(r), d_danger(0)
{
    d_stacks = new Stacklist();
}

Threat::~Threat()
{
    delete d_stacks;
}

std::string Threat::toString() const
{
    if (d_city)
    {
        return d_city->getName() + " owned by " + d_owner->getName();
    }
    else if (d_ruin)
    {
        return d_ruin->getName();
    }
    else
    {
        return "stack owned by " + d_owner->getName();
    }
}

bool Threat::Near(Vector<int> pos, Player *p) const
{
    if (p != d_owner)
        return false;

    if (d_city)
    {
        return d_city->contains(pos);
    }
    else if (d_ruin)
    {
        return d_ruin->contains(pos);
    }
    else
        for (Stacklist::const_iterator it = d_stacks->begin();
            it != d_stacks->end(); ++it)
        {
            Vector<int> spos = (*it)->getPos();
            if (abs(pos.x - spos.x) <= 1 && abs(pos.y - spos.y <= 1))
                return true;
        }

    return false;
}

void Threat::addStack(Stack *stack)
{
    d_stacks->push_back(stack);
}

// this is the strength of the threat to us
float Threat::strength() const
{
    // neutral cities pose no threat
    if (d_city)
        if (d_city->getOwner() == Playerlist::getInstance()->getNeutral())
            return 0.0;
    float score = 0.0;
    for (Stacklist::const_iterator it = d_stacks->begin();
        it != d_stacks->end(); ++it)
    {
        score += AI_Analysis::assessStackStrength(*it);
    }

    return score;
}

float Threat::value() const
{
    float score = 0.0;
    // if city has turned friendly, it is no longer a valuable target
    if (d_city && d_city->getOwner() == d_owner && !d_city->isBurnt())
            score += 10.0;
    
    // if ruin has become searched, it is no longer valuable
    if (d_ruin && !d_ruin->isSearched())
            score += 5.0;

    score += d_danger;
    return score;
}

Vector<int> Threat::getClosestPoint(Vector<int> location) const
{
    Vector<int> result(-1,-1);
    if (d_city)
    {
        result.x = d_city->getPos().x;
        result.y = d_city->getPos().y;
        if (location.x > result.x)
            result.x++;
        if (location.y > result.y)
            result.y++;
    }
    else if (d_ruin)
    {
        result.x = d_ruin->getPos().x;
        result.y = d_ruin->getPos().y;
    }
    else
    {
        int distToClosest = 1000000;
        for (Stacklist::const_iterator it = d_stacks->begin();
            it != d_stacks->end(); ++it)
        {
            Vector<int> spos = (*it)->getPos();
            
            //UL: remember, stacks can move diagonally
            int dist = abs(spos.x - location.x);
            int disty = abs(spos.y - location.y);
            if (dist > disty)
                dist = disty;
            if (dist < distToClosest)
            {
                distToClosest = dist;
                result.x = spos.x;
                result.y = spos.y;
            }
        }
    }

    return result;
}

void Threat::deleteStack(Stack* s)
{
    d_stacks->remove(s);
}
