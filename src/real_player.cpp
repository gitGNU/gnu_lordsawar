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

#include <fstream>
#include <algorithm>
#include <stdlib.h>
#include <SDL_timer.h>

#include "real_player.h"
#include "action.h"
#include "xmlhelper.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

RealPlayer::RealPlayer(string name, Uint32 armyset, SDL_Color color, int width,
		       int height, Player::Type type, int player_no)
    :Player(name, armyset, color, width, height, type, player_no)
{
}

RealPlayer::RealPlayer(const Player& player)
    :Player(player)
{
    d_type = HUMAN;
}

RealPlayer::RealPlayer(XML_Helper* helper)
    :Player(helper)
{
}

RealPlayer::~RealPlayer()
{
}

bool RealPlayer::save(XML_Helper* helper) const
{
    // This may seem a bit dumb, but allows derived players (especially
    // AI's) to save additional data, such as character types or so.
    bool retval = true;
    retval &= helper->openTag("player");
    retval &= Player::save(helper);
    retval &= helper->closeTag();

    return retval;
}

bool RealPlayer::startTurn()
{
    return false;
}

void RealPlayer::endTurn()
{
  Action *action = new Action_EndTurn;
  addAction(action);
}

void RealPlayer::invadeCity(City* c)
{
    // For the realplayer, this function doesn't do a lot. However, an AI
    // player has to decide here what to do (occupy, raze, pillage)
}

bool RealPlayer::recruitHero(Hero* hero, City *city, int cost)
{
    // for the realplayer, this function also just raises a signal and looks
    // what to do next.

    return srecruitingHero.emit(hero, city, cost);
}

void RealPlayer::levelArmy(Army* a)
{
    // the standard human player just asks the GUI what to do
    Army::Stat stat = snewLevelArmy.emit(a);
    doLevelArmy(a, stat);

    Action_Level* item = new Action_Level();
    item->fillData(a, stat);
    addAction(item);
}

// End of file
