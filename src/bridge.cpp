//  Copyright (C) 2007, 2008 Ben Asselstine
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

#include "bridge.h"
#include "GameMap.h"

Bridge::Bridge(Vector<int> pos, std::string name, int type)
  :Location(name, pos), d_type(type)
{
    //mark the location on the game map as occupied by a bridge
    GameMap::getInstance()->getTile(d_pos.x, d_pos.y)->setBuilding(Maptile::BRIDGE);
}

Bridge::Bridge(XML_Helper* helper)
    :Location(helper)
{
    //mark the location on the game map as occupied by a bridge
    helper->getData(d_type, "type");
    GameMap::getInstance()->getTile(d_pos.x, d_pos.y)->setBuilding(Maptile::BRIDGE);
}

Bridge::Bridge(const Bridge& s)
  :Location(s), d_type(s.d_type)
{
}

Bridge::~Bridge()
{
}

bool Bridge::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("bridge");
    retval &= helper->saveData("id", d_id);
    retval &= helper->saveData("name", d_name);
    retval &= helper->saveData("type", d_type);
    retval &= helper->saveData("x", d_pos.x);
    retval &= helper->saveData("y", d_pos.y);
    retval &= helper->closeTag();
    
    return retval;
}
