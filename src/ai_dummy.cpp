// Copyright (C) 2002, 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2006 Andrea Patton
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

#include <stdlib.h>
#include <algorithm>
#include "ai_dummy.h"
#include "playerlist.h"
#include "armysetlist.h"
#include "stacklist.h"
#include "citylist.h"
#include <fstream>
#include "path.h"
#include "action.h"
#include "xmlhelper.h"
#include "history.h"

#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

AI_Dummy::AI_Dummy(std::string name, Uint32 armyset, SDL_Color color, int width, int height, int player_no)
    :RealPlayer(name, armyset, color, width, height, Player::AI_DUMMY, player_no)
{
}

AI_Dummy::AI_Dummy(const Player& player)
    :RealPlayer(player)
{
    d_type = AI_DUMMY;
}

AI_Dummy::AI_Dummy(XML_Helper* helper)
    :RealPlayer(helper)
{
}

AI_Dummy::~AI_Dummy()
{
}

bool AI_Dummy::startTurn()
{
    //this is a dummy AI (neutral player) so there is not much point in
    //doing anything
    return true;
}

void AI_Dummy::invadeCity(City* c)
{
    //dummy ai player should never invade an enemy city, but if it happens, we
    //make sure there is no inconsistency
    cityOccupy(c);
}

void AI_Dummy::levelArmy(Army* a)
{
    Army::Stat stat = Army::STRENGTH;
    doLevelArmy(a, stat);

    Action_Level* item = new Action_Level();
    item->fillData(a, stat);
    addAction(item);
}

// End of file
