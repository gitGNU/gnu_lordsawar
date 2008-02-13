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

#include "signpost.h"
#include "GameMap.h"

Signpost::Signpost(Vector<int> pos, std::string name)
  :Location(name, pos)
{
    //mark the location on the game map as occupied by a signpost
    GameMap::getInstance()->getTile(d_pos.x, d_pos.y)->setBuilding(Maptile::SIGNPOST);
}

Signpost::Signpost(XML_Helper* helper)
    :Location(helper)
{
    //mark the location on the game map as occupied by a signpost
    GameMap::getInstance()->getTile(d_pos.x, d_pos.y)->setBuilding(Maptile::SIGNPOST);
}

Signpost::Signpost(const Signpost& s)
  :Location(s)
{
}

Signpost::~Signpost()
{
}

bool Signpost::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("signpost");
    retval &= helper->saveData("id", d_id);
    retval &= helper->saveData("name", d_name);
    retval &= helper->saveData("x", d_pos.x);
    retval &= helper->saveData("y", d_pos.y);
    retval &= helper->closeTag();
    
    return retval;
}
