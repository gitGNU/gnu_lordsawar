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

#include "temple.h"
#include "GameMap.h"
#include "QuestsManager.h"
#include "stack.h"

Temple::Temple(Vector<int> pos, std::string name, int type)
  :Location(name, pos),d_type(type)
{
    //mark the location on the game map as occupied by a temple
    GameMap::getInstance()->getTile(d_pos)->setBuilding(Maptile::TEMPLE);
}

Temple::Temple(XML_Helper* helper)
    :Location(helper)
{
    //mark the location on the game map as occupied by a temple
    helper->getData(d_type, "type");
    GameMap::getInstance()->getTile(d_pos)->setBuilding(Maptile::TEMPLE);
}

Temple::Temple(const Temple& t)
  :Location(t), d_type(t.d_type)
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
    retval &= helper->saveData("name", d_name);
    retval &= helper->saveData("type", d_type);
    retval &= helper->saveData("x", d_pos.x);
    retval &= helper->saveData("y", d_pos.y);
    retval &= helper->closeTag();
    
    return retval;
}
