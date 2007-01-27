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
#include "EArmyKilled.h"
#include "../playerlist.h"
#include "../stacklist.h"
#include "../stack.h"
#include "../army.h"
#include "../defs.h"


EArmyKilled::EArmyKilled(Uint32 army)
    :Event(ARMYKILLED), d_army(army)
{
}

EArmyKilled::EArmyKilled(XML_Helper* helper)
    :Event(helper)
{
    helper->getData(d_army, "army");
    d_type = ARMYKILLED;
}

EArmyKilled::~EArmyKilled()
{
}

bool EArmyKilled::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("event");
    retval &= helper->saveData("army", d_army);

    retval &= Event::save(helper);

    retval &= helper->closeTag();
    
    return retval;
}

void EArmyKilled::init()
{
    Army::sdying.connect(SigC::slot((*this), &EArmyKilled::trigger));
}

void EArmyKilled::trigger(Army* army)
{
    //Check if the player is dead. Background: when a player dies, all his
    //armies are deleted. That would cause all events associated with his
    //armies to be raised automatically, which can be very irritating.
    if (!army->getPlayer() || army->getPlayer()->isDead())
        return;

    if (army->getId() == d_army)
        raise();
}

Army* EArmyKilled::getArmy() const
{
    Playerlist* plist = Playerlist::getInstance();
    
    // We need to look for the army in all stack of each player's stacklist.
    for (Playerlist::iterator pit = plist->begin(); pit != plist->end(); pit++)
    {
        Stacklist* slist = (*pit)->getStacklist();
        for (Stacklist::iterator sit = slist->begin(); sit != slist->end(); sit++)
        {
            Stack* s = (*sit);
            for (Stack::iterator it = s->begin(); it != s->end(); it++)
                if ((*it)->getId() == d_army)
                {
                    return (*it);
                }
        }
    }

    return 0;
}
