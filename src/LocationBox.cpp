// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2000, 2001, 2002, 2004, 2005 Ulf Lorenz
// Copyright (C) 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
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

#include "LocationBox.h"
#include "army.h"
#include "player.h"
#include "playerlist.h"
#include "stacklist.h"
#include "stack.h"
#include "FogMap.h"

#include "xmlhelper.h"

LocationBox::LocationBox(Vector<int> pos, Uint32 size)
    :Immovable(pos), d_size(size)
{
}

LocationBox::LocationBox(const LocationBox& loc)
  : Immovable(loc), d_size(loc.d_size)
{
}

LocationBox::LocationBox(XML_Helper* helper, Uint32 size)
    :Immovable(helper)
{
    d_size = size;
}

LocationBox::~LocationBox()
{
}

Stack *LocationBox::addArmy(Army *a) const
{
    Stack* stack = getFreeStack(a->getOwner());

    // No stack found in the entire location
    if (!stack)
      return NULL;

    // add army to stack
    stack->push_back(a);
    if (stack->countGroupedArmies() == 0)
      stack->ungroup();
    stack->sortForViewing(true);
    stack->setDefending(false);
    stack->setParked(false);
    return stack;
}

Stack* LocationBox::getFreeStack(Player *p) const
{
    for (unsigned int i = 0; i < d_size; i++)
        for (unsigned int j = 0; j < d_size; j++)
        {
            Stack* stack = Stacklist::getObjectAt(getPos().x + j, 
						  getPos().y + i);

            if (stack == NULL)
            {
                Vector<int> temp;
                temp.x = getPos().x + j;
                temp.y = getPos().y + i;
                stack = new Stack(p, temp);
                p->addStack(stack);
                return stack;
            }
            else if (stack->size() < MAX_STACK_SIZE) return stack;
        }
    return NULL;
}

bool LocationBox::isFogged()
{
  for (unsigned int i = 0; i < d_size; i++)
    for (unsigned int j = 0; j < d_size; j++)
      {
        Vector<int> pos;
        pos.x = getPos().x + i;
        pos.y = getPos().y + j;
	if (FogMap::isFogged(pos))
          return true;
      }
  return false;
}

void LocationBox::deFog()
{
  Player *p = Playerlist::getActiveplayer();
  if (!p)
    return;
  FogMap *fogmap = p->getFogMap();
  fogmap->alterFogRadius (getPos(), 3, FogMap::OPEN);
}

void LocationBox::deFog(Player *p)
{
  if (!p)
    return;
  FogMap *fogmap = p->getFogMap();
  if (!fogmap)
    return;
  fogmap->alterFogRadius (getPos(), 3, FogMap::OPEN);
}

bool LocationBox::contains(Vector<int> pos) const
{
    return (pos.x >= getPos().x) && (pos.x < getPos().x + (int) d_size) 
      && (pos.y >= getPos().y) && (pos.y < getPos().y + (int) d_size);
}

