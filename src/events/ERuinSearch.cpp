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

#include "ERuinSearch.h"
#include "../ruinlist.h"
#include "../playerlist.h"
#include "../player.h"

ERuinSearch::ERuinSearch(Uint32 ruin)
    :Event(RUINSEARCH), d_ruin(ruin)
{
}

ERuinSearch::ERuinSearch(XML_Helper* helper)
    :Event(helper)
{
    helper->getData(d_ruin, "ruin");
    d_type = RUINSEARCH;
}

ERuinSearch::~ERuinSearch()
{
}

bool ERuinSearch::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("event");
    retval &= helper->saveData("ruin", d_ruin);

    retval &= Event::save(helper);

    retval &= helper->closeTag();

    return retval;
}

void ERuinSearch::init()
{
    Playerlist* plist = Playerlist::getInstance();
    for (Playerlist::iterator it = plist->begin(); it != plist->end(); it++)
        (*it)->ssearchingRuin.connect(sigc::mem_fun(*this, &ERuinSearch::trigger));
}

void ERuinSearch::trigger(Ruin* r, Stack* s)
{
    if (r->getId() != d_ruin)
        return;

    raise();
}
