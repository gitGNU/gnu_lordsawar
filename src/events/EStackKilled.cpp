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

#include "EStackKilled.h"
#include "../playerlist.h"
#include "../stacklist.h"
#include "../stack.h"

EStackKilled::EStackKilled(Uint32 stack)
    :Event(STACKKILLED), d_stack(stack)
{
}

EStackKilled::EStackKilled(XML_Helper* helper)
    :Event(helper)
{
    helper->getData(d_stack, "stack");
    d_type = STACKKILLED;
}

EStackKilled::~EStackKilled()
{
}

bool EStackKilled::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("event");
    retval &= helper->saveData("stack", d_stack);
    retval &= Event::save(helper);
    retval &= helper->closeTag();

    return retval;
}

void EStackKilled::init()
{
    Stack* s = getStack();

    if (!s)
    {
        std::cerr <<"EStackKilled: stack with id " <<d_stack;
        std::cerr <<" does not exist?!?!?!\n" <<std::flush;
        return;
    }

    s->sdying.connect(SigC::slot(*this, &EStackKilled::trigger));
}

void EStackKilled::trigger(Stack* s)
{
    // Don't raise the signal if the player owning the stack has been killed.
    // If a player is killed, all his stacks are freed; however, this is not
    // what we intend with this event.
    if (s->getPlayer()->isDead())
        return;

    raise();
}

Stack* EStackKilled::getStack() const
{
    Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::iterator pit = pl->begin(); pit != pl->end(); pit++)
    {
        Stacklist* sl = (*pit)->getStacklist();
        for (Stacklist::iterator it = sl->begin(); it != sl->end(); it++)
            if ((*it)->getId() == d_stack)
                return (*it);
    }

    return 0;
}
