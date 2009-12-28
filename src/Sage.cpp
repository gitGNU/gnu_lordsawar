// Copyright (C) 2009 Ben Asselstine
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

#include "Sage.h"
#include "rewardlist.h"

using namespace std;

Sage::Sage()
{
  reward = NULL;
  Rewardlist::iterator iter = Rewardlist::getInstance()->begin();
  for (;iter != Rewardlist::getInstance()->end(); iter++)
    {
      if ((*iter)->getType() == Reward::ITEM)
        continue;
      // we don't want items here, but we do want locations
      // of items, within hidden ruins.

      push_back(*iter);
    }
  //this covers, the one-time rewards of items and maps
  //but now we put in gold too
  push_back(new Reward_Gold(500 + rand() % 1000));
}

Sage::~Sage()
{
}

