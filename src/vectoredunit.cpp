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
#include "playerlist.h"
#include "army.h"
#include "city.h"
#include "GameMap.h"
#include "action.h"

VectoredUnit::VectoredUnit(Vector<int> pos, Vector<int> dest, Army *army, int duration, Player *player)
    :Location("", pos), d_destination(dest), d_army(army), 
     d_duration(duration)
{
  d_player = player;
}

VectoredUnit::VectoredUnit(const VectoredUnit& v)
    :Location(v), d_destination(v.d_destination), d_army(v.d_army), 
     d_duration(v.d_duration), d_player(v.d_player)
{
}

VectoredUnit::VectoredUnit(XML_Helper* helper)
    :Location(helper), d_army(NULL), d_duration(0)
{
    helper->getData(d_duration, "duration");
    helper->getData(d_destination.x, "dest_x");
    helper->getData(d_destination.y, "dest_y");
    int i;
    helper->getData(i, "player");
    if (i == -1)
	d_player = 0;
    else
	d_player = Playerlist::getInstance()->getPlayer(i);
    //army is loaded via callback in vectoredunitlist
}

VectoredUnit::~VectoredUnit()
{
  if (d_army)
    delete d_army;
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
    retval &= helper->saveData("duration", d_duration);
    retval &= helper->saveData("dest_x", d_destination.x);
    retval &= helper->saveData("dest_y", d_destination.y);
    if (d_player)
        retval &= helper->saveData("player", d_player->getId());
    else
        retval &= helper->saveData("player", -1);
    retval &= d_army->save(helper, Army::PRODUCTION_BASE);
    retval &= helper->closeTag();

    return retval;
}

bool VectoredUnit::nextTurn()
{
  d_duration--;
  if (d_duration == 0)
    {
      if (d_player->getGold() <= 0)
	{
	  //don't bring this army in because we can't afford it
	  return false;
	}

      Citylist *cl = Citylist::getInstance();
      Army *a = new Army(*d_army, d_player);
      //FIXME: this action should be in player somehow
      Action_ProduceVectored *item = new Action_ProduceVectored();
      item->fillData(a->getType(), d_destination);
      d_player->getActionlist()->push_back(item);

      City *dest;
      // drop it in the destination city!
      dest = cl->getObjectAt(d_destination);
      if (!dest)
        {
          std::list<Item*> items;
          items = GameMap::getInstance()->getTile(d_destination)->getItems();
          for (std::list<Item*>::iterator it = items.begin(); 
               it != items.end(); it++)
            {
              if ((*it)->getPlanted() == true &&
                  (*it)->getPlantableOwner() == d_player)
                {
                  Location loc = Location("planted standard", d_destination, 1);
                  loc.addArmy(a);
                  break;
                }
             
            }
        }
      else
        {
          if (!dest->isBurnt() && dest->getPlayer() == d_player)
            dest->addArmy(a);
        }
      return true;
    }
  return false;
}

// End of file
