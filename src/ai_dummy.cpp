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

#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

AI_Dummy::AI_Dummy(std::string name, Uint32 armyset, SDL_Color color, int player_no)
    :RealPlayer(name, armyset, color, Player::AI_DUMMY, player_no)
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
    clearActionlist();

    return true;
}

bool AI_Dummy::invadeCity(City* c)
{
    //dummy ai player should never invade an enmy city, but if it happens, we
    //make sure there is no inconsistency
    bool retval = cityOccupy(c);
    sinvadingCity.emit(c);
    soccupyingCity.emit(c, getActivestack());

    return retval;
}

bool AI_Dummy::recruitHero(Hero* hero, City *city, int cost)
{
    return false;   //never recruit a hero
}

bool AI_Dummy::levelArmy(Army* a)
{
    if (!a->canGainLevel())
        return false;

    a->gainLevel(Army::STRENGTH);

    Action_Level* item=0;
    item->fillData(a->getId(), Army::STRENGTH);
    d_actions.push_back(item);

    return true;
}

// End of file
