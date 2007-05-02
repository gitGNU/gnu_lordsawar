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

#include "EStackMove.h"
#include "../playerlist.h"
#include "../stack.h"

EStackMove::EStackMove(Vector<int> pos)
    :Event(STACKMOVE), d_pos(pos)
{
}

EStackMove::EStackMove(XML_Helper* helper)
    :Event(helper)
{
    d_type = STACKMOVE;

    int data;
    helper->getData(data, "x");
    d_pos.x = data;
    helper->getData(data, "y");
    d_pos.y = data;
}

EStackMove::~EStackMove()
{
}

bool EStackMove::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("event");
    retval &= Event::save(helper);
    retval &= helper->saveData("x", d_pos.x);
    retval &= helper->saveData("y", d_pos.y);
    retval &= helper->closeTag();
    
    return retval;
}

void EStackMove::init()
{
    // connect to the smovingStack signals of all players
    Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::iterator it = pl->begin(); it != pl->end(); it++)
        (*it)->smovingStack.connect(sigc::mem_fun(*this, &EStackMove::trigger));
}

void EStackMove::trigger(Stack* s)
{
    if (s->getPos().x != d_pos.x || s->getPos().y != d_pos.y)
        return;

    raise();
}
