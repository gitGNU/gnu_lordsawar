// Copyright (C) 2007, 2008 Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
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

#include "port.h"
#include "GameMap.h"

std::string Port::d_tag = "port";

Port::Port(Vector<int> pos)
  :Location(pos)
{
    //mark the location on the game map as occupied by a port
    GameMap::getInstance()->getTile(getPos())->setBuilding(Maptile::PORT);
}

Port::Port(XML_Helper* helper)
    :Location(helper)
{
    //mark the location on the game map as occupied by a port
    GameMap::getInstance()->getTile(getPos())->setBuilding(Maptile::PORT);
}

Port::Port(const Port& s)
  :Location(s)
{
}

Port::Port(const Port& s, Vector<int> pos)
  :Location(s, pos)
{
}

Port::~Port()
{
}

bool Port::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(Port::d_tag);
    retval &= helper->saveData("id", d_id);
    retval &= helper->saveData("x", getPos().x);
    retval &= helper->saveData("y", getPos().y);
    retval &= helper->closeTag();
    
    return retval;
}
