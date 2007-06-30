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

#include "ruin.h"
#include "playerlist.h"
#include "GameMap.h"
#include <stdlib.h>

Ruin::Ruin(Vector<int> pos, std::string name, Stack* occupant, bool searched, bool hidden, Player *owner, bool sage)
    :Location(name, pos), d_searched(searched), d_occupant(occupant), d_hidden(hidden), d_owner(owner), d_sage(sage)
{
    d_owner = Playerlist::getInstance()->getNeutral();
    //mark the location as being occupied by a ruin on the map
    GameMap::getInstance()->getTile(d_pos.x, d_pos.y)->setBuilding(Maptile::RUIN);
}

Ruin::Ruin(const Ruin& ruin)
    :Location(ruin), d_searched(ruin.d_searched), d_occupant(ruin.d_occupant),
    d_hidden(ruin.d_hidden), d_owner(ruin.d_owner), d_sage(ruin.d_sage)
{
}

Ruin::Ruin(XML_Helper* helper)
    :Location(helper), d_occupant(0), d_hidden(0), d_owner(0), d_sage(0)
{
    Uint32 ui;
    helper->getData(d_searched, "searched");
    helper->getData(d_sage, "sage");
    helper->getData(d_hidden, "hidden");
    if (d_hidden)
      {
        helper->getData(ui, "owner");
        d_owner = Playerlist::getInstance()->getPlayer(ui);
      }
    else
      d_owner = Playerlist::getInstance()->getNeutral();

    //mark the location as being occupied by a ruin on the map
    GameMap::getInstance()->getTile(d_pos.x, d_pos.y)->setBuilding(Maptile::RUIN);
}

Ruin::~Ruin()
{
    if (d_occupant)
        delete d_occupant;
}

bool Ruin::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("ruin");
    retval &= helper->saveData("id", d_id);
    retval &= helper->saveData("name", d_name);
    retval &= helper->saveData("x", d_pos.x);
    retval &= helper->saveData("y", d_pos.y);
    retval &= helper->saveData("searched", d_searched);
    retval &= helper->saveData("sage", d_sage);
    retval &= helper->saveData("hidden", d_hidden);
    if (d_owner != Playerlist::getInstance()->getNeutral())
      retval &= helper->saveData("owner", d_owner->getId());
    if (d_occupant)
        retval &= d_occupant->save(helper);
    retval &= helper->closeTag();

    return retval;
}

bool Ruin::load(std::string tag, XML_Helper* helper)
{
    if (tag != "stack")
        return false;
    
    Stack* s = new Stack(helper);
    d_occupant = s;

    return true;
}

// End of file
