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

#include "ETempleSearch.h"
#include "../playerlist.h"
#include "../player.h"
#include "../templelist.h"

ETempleSearch::ETempleSearch(Uint32 id)
    :Event(TEMPLESEARCH), d_temple(id)
{
    init();
}

ETempleSearch::ETempleSearch(XML_Helper* helper)
    :Event(helper)
{
    helper->getData(d_temple, "temple");
    d_type = TEMPLESEARCH;
}

ETempleSearch::~ETempleSearch()
{
}

bool ETempleSearch::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("event");
    retval &= helper->saveData("temple", d_temple);

    retval &= Event::save(helper);
    retval &= helper->closeTag();

    return retval;
}

void ETempleSearch::init()
{
    Playerlist* plist = Playerlist::getInstance();
    for (Playerlist::iterator it = plist->begin(); it != plist->end(); it++)
        (*it)->svisitingTemple.connect(sigc::mem_fun(*this, &ETempleSearch::trigger));
}

void ETempleSearch::trigger(Temple* t, Stack* s)
{
    if (t->getId() != d_temple)
        return;
    
    raise();
}
