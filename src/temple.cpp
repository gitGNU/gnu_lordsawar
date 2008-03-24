// Copyright (C) 2001, 2003 Michael Bartl
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2006 Andrea Paternesi
// Copyright (C) 2007, 2008 Ben Asselstine
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

#include "temple.h"
#include "GameMap.h"
#include "QuestsManager.h"
#include "stack.h"

Temple::Temple(Vector<int> pos, std::string name, int type)
  :NamedLocation(pos, name),d_type(type)
{
    //mark the location on the game map as occupied by a temple
    GameMap::getInstance()->getTile(getPos())->setBuilding(Maptile::TEMPLE);
}

Temple::Temple(XML_Helper* helper)
    :NamedLocation(helper)
{
    //mark the location on the game map as occupied by a temple
    helper->getData(d_type, "type");
    GameMap::getInstance()->getTile(getPos())->setBuilding(Maptile::TEMPLE);
}

Temple::Temple(const Temple& t)
  :NamedLocation(t), d_type(t.d_type)
{
}

Temple::~Temple()
{
}

bool Temple::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("temple");
    retval &= helper->saveData("id", d_id);
    retval &= helper->saveData("x", getPos().x);
    retval &= helper->saveData("y", getPos().y);
    retval &= helper->saveData("name", getName());
    retval &= helper->saveData("type", d_type);
    retval &= helper->closeTag();
    
    return retval;
}
