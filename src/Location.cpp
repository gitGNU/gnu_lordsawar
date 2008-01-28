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

#include "Location.h"
#include "army.h"
#include "player.h"
#include "playerlist.h"
#include "stacklist.h"
#include "FogMap.h"

#include "xmlhelper.h"

Location::Location(std::string name, Vector<int> pos, Uint32 size)
    :Object(pos, size), d_name(name)
{
}

Location::Location(const Location& loc)
  :Object(loc), d_name(loc.d_name)
{
}

Location::Location(XML_Helper* helper, Uint32 size)
    :Object(helper, size)
{
    helper->getData(d_name, "name");
}

Location::~Location()
{
}

Stack *Location::addArmy(Army *a) const
{
    Stack* stack = getFreeStack(a->getPlayer());

    // No stack found in the entire location
    if (!stack)
      return NULL;

    // add army to stack
    stack->push_back(a);
    stack->sortForViewing(true);
    return stack;
}

Stack* Location::getFreeStack(Player *p) const
{
    for (unsigned int i = 0; i < d_size; i++)
        for (unsigned int j = 0; j < d_size; j++)
        {
            Stack* stack = Stacklist::getObjectAt(d_pos.x + j, d_pos.y+ i);

            if (stack == NULL)
            {
                Vector<int> temp;
                temp.x = d_pos.x + j;
                temp.y = d_pos.y + i;
                stack = new Stack(p, temp);
                p->addStack(stack);
                return stack;
            }
            else if (stack->size() < 8) return stack;
        }
    return NULL;
}

bool Location::isFogged()
{
  FogMap *fogmap = Playerlist::getActiveplayer()->getFogMap();
  for (unsigned int i = 0; i < d_size; i++)
    for (unsigned int j = 0; j < d_size; j++)
      {
        Vector<int> pos;
        pos.x = d_pos.x + i;
        pos.y = d_pos.y + j;
        if (fogmap->getFogTile(pos) == FogMap::CLOSED)
          return true;
      }
  return false;
}

void Location::deFog()
{
  Player *p = Playerlist::getActiveplayer();
  if (!p)
    return;
  FogMap *fogmap = p->getFogMap();
  fogmap->alterFogRadius (d_pos, 3, FogMap::OPEN);
}
void Location::deFog(Player *p)
{
  if (!p)
    return;
  FogMap *fogmap = p->getFogMap();
  if (!fogmap)
    return;
  fogmap->alterFogRadius (d_pos, 3, FogMap::OPEN);
}
