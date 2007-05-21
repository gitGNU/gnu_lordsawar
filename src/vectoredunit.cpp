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

#include "vectoredunit.h"
#include <stdlib.h>
#include <string.h>
#include <xmlhelper.h>

#include "citylist.h"
#include "armysetlist.h"
#include "army.h"
#include "city.h"

VectoredUnit::VectoredUnit(Vector<int> pos, Vector<int> dest, int armytype, int duration)
    :Location("", pos), d_destination(dest), d_armytype(armytype), 
     d_duration(duration)
{
}

VectoredUnit::VectoredUnit(const VectoredUnit& v)
    :Location(v), d_destination(v.d_destination), d_armytype(v.d_armytype), 
     d_duration(v.d_duration)
{
}

VectoredUnit::VectoredUnit(XML_Helper* helper)
    :Location(helper), d_armytype(-1), d_duration(0)
{
    helper->getData(d_armytype, "armytype");
    helper->getData(d_duration, "duration");
    helper->getData(d_destination.x, "dest_x");
    helper->getData(d_destination.y, "dest_y");
}

VectoredUnit::~VectoredUnit()
{
}

bool VectoredUnit::save(XML_Helper* helper) const
{
    bool retval = true;
    std::string name = "";

    retval &= helper->openTag("vectoredunit");
    retval &= helper->saveData("id", d_id);
    retval &= helper->saveData("name", name);
    retval &= helper->saveData("x", d_pos.x);
    retval &= helper->saveData("y", d_pos.y);
    retval &= helper->saveData("armytype", d_armytype);
    retval &= helper->saveData("duration", d_duration);
    retval &= helper->saveData("dest_x", d_destination.x);
    retval &= helper->saveData("dest_y", d_destination.y);
    retval &= helper->closeTag();

    return retval;
}

bool VectoredUnit::nextTurn()
{
  const Armysetlist* al = Armysetlist::getInstance();
  Uint32 set = al->getStandardId();
  Citylist *cl = Citylist::getInstance();
  d_duration--;
  if (d_duration == 0)
    {
      City *dest;
      // drop it in the destination city!
      dest = cl->getObjectAt(d_destination);
      dest->addArmy(new Army(*(al->getArmy(set, d_armytype))));
      return true;
    }
  return false;
}

// End of file
