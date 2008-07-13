//  Copyright (C) 2008, Ben Asselstine
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

#include "Ownable.h"
#include "player.h"
#include "playerlist.h"

#include "xmlhelper.h"

Ownable::Ownable(Player *owner)
  :d_owner(owner)
{
}

Ownable::Ownable(const Ownable& own)
  :d_owner(own.d_owner)
{
}

Ownable Ownable::load(XML_Helper *helper)
{
  Ownable result((Player*)0);
  int i = -1;
  helper->getData(i, "owner");
  if (i == -1)
    result.setOwner(0);
  else
    result.setOwner(Playerlist::getInstance()->getPlayer(i));
  return result;
}

Ownable::Ownable(XML_Helper* helper)
{
  if (!helper)
    return;
  Ownable result((Player*)0);
  result = load(helper);
  d_owner = result.d_owner;
}

Ownable::~Ownable()
{
}

bool Ownable::isFriend(Player *opponent)
{
  return d_owner == opponent;
}
