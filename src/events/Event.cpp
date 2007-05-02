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

#include <sigc++/functors/mem_fun.h>

#include "Event.h"
#include "Reaction.h"
#include "Condition.h"
#include "EPlayerDead.h"
#include "EKillAll.h"
#include "ECityConq.h"
#include "EArmyKilled.h"
#include "ERound.h"
#include "ERuinSearch.h"
#include "ETempleSearch.h"
#include "EDummy.h"
#include "ENextTurn.h"
#include "EStackKilled.h"
#include "EStackMove.h"
#include "../counter.h"


Event::Event(Type type)
    :d_type(type), d_active(true), d_comment("")
{
    d_id = fl_counter->getNextId();
}

Event::Event(XML_Helper* helper)
{
    helper->getData(d_active, "active");
    helper->getData(d_id, "id");
    helper->getData(d_comment, "comment");

    helper->registerTag("reaction", sigc::mem_fun((*this), &Event::loadData));
    helper->registerTag("condition", sigc::mem_fun((*this), &Event::loadData));

    // Note: When loading, the init function is called automatically by
    // GameScenario when all has been set up, so no need to call init here.
}

Event::~Event()
{
    for (Uint32 i = 0; i < d_reactions.size(); i++)
        delete d_reactions[i];

    for (Uint32 i = 0; i < d_conditions.size(); i++)
        delete d_conditions[i];
}

Event* Event::loadEvent(XML_Helper* helper)
{
    Uint32 type;
    helper->getData(type, "type");


    switch(type)
    {
        case PLAYERDEAD:
            return new EPlayerDead(helper);
        case KILLALL:
            return new EKillAll(helper);
        case CITYCONQUERED:
            return new ECityConq(helper);
        case ARMYKILLED:
            return new EArmyKilled(helper);
        case ROUND:
            return new ERound(helper);
        case RUINSEARCH:
            return new ERuinSearch(helper);
        case TEMPLESEARCH:
            return new ETempleSearch(helper);
        case DUMMY:
            return new EDummy(helper);
        case NEXTTURN:
            return new ENextTurn(helper);
        case STACKKILLED:
            return new EStackKilled(helper);
        case STACKMOVE:
            return new EStackMove(helper);
    }
    
    return 0;
}

bool Event::loadData(std::string tag, XML_Helper* helper)
{
    if (tag == "reaction")
    {
        d_reactions.push_back(Reaction::loadReaction(helper));
        return true;
    }
    else if (tag == "condition")
    {
        d_conditions.push_back(Condition::loadCondition(helper));
        return true;
    }

    return false;
}

bool Event::save(XML_Helper* helper) const
{
    //Note that this function doesn't open or close tags, this has to be done
    //by the derived classes!
    bool retval = true;

    retval &= helper->saveData("id", d_id);
    retval &= helper->saveData("active", d_active);
    retval &= helper->saveData("type", d_type);
    retval &= helper->saveData("comment", d_comment);

    //! save the conditions first, else the reactions will misinterpret it as
    //one of their conditions!!
    for (Uint32 i = 0; i < d_conditions.size(); i++)
        d_conditions[i]->save(helper);
    
    //save the reactions
    for (Uint32 i = 0; i < d_reactions.size(); i++)
        d_reactions[i]->save(helper);

    return retval;
}

bool Event::addReaction(Reaction* r, int index)
{
    if (index == -1)
    {
        d_reactions.push_back(r);
        return true;
    }

    if (index < 0 || index > static_cast<int>(d_reactions.size()))
        return false;

    std::vector<Reaction*>::iterator it;
    for (it = d_reactions.begin(); index > 0; it++, index--);

    d_reactions.insert(it, r);
    return true;
}

bool Event::removeReaction(Uint32 index)
{
    std::vector<Reaction*>::iterator it = d_reactions.begin();

    while (index > 0)
    {
        it++;
        if (it == d_reactions.end())
            return false;
        index--;
    }

    delete (*it);
    d_reactions.erase(it);
    return true;
}

Reaction* Event::getReaction(Uint32 index)
{
    if (index >= d_reactions.size())
        return 0;

    return d_reactions[index];
}

bool Event::addCondition(Condition* c, int index)
{
    if (index == -1)
    {
        d_conditions.push_back(c);
        return true;
    }

    if (index < 0 || index > static_cast<int>(d_conditions.size()))
        return false;

    std::vector<Condition*>::iterator it;
    for (it = d_conditions.begin(); index > 0; it++, index--);

    d_conditions.insert(it, c);
    return true;
}

bool Event::removeCondition(Uint32 index)
{
    std::vector<Condition*>::iterator it = d_conditions.begin();

    while (index > 0)
    {
        it++;
        if (it == d_conditions.end())
            return false;
        index--;
    }

    delete (*it);
    d_conditions.erase(it);
    return true;
}

Condition* Event::getCondition(Uint32 index)
{
    if (index >= d_conditions.size())
        return 0;

    return d_conditions[index];
}

void Event::raise()
{
    if (!d_active)
        return;
    
    // first, check if all conditions are fullfilled; if not, we don't raise the
    // event
    for (Uint32 i = 0; i < d_conditions.size(); i++)
        if (!d_conditions[i]->check())
            return;

    // deactivate the event _before_ the reactions are triggered. Using the reaction
    // to activate events, we can therefore create eternal-living events
    d_active = false;
    
    for (Uint32 i = 0; i < d_reactions.size(); i++)
        d_reactions[i]->trigger();
}
