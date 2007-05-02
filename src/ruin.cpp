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
#include "GameMap.h"
#include <stdlib.h>

Ruin::Ruin(Vector<int> pos, std::string name, Stack* owner, bool searched)
    :Location(name, pos), d_searched(searched), d_occupant(owner)
{
    //mark the location as being occupied by a ruin on the map
    GameMap::getInstance()->getTile(d_pos.x, d_pos.y)->setBuilding(Maptile::RUIN);
}

Ruin::Ruin(const Ruin& ruin)
    :Location(ruin), d_searched(ruin.d_searched), d_occupant(ruin.d_occupant)
{
}

Ruin::Ruin(XML_Helper* helper)
    :Location(helper), d_occupant(0)
{
    helper->getData(d_searched, "searched");

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
