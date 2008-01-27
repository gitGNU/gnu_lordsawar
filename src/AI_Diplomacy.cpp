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

#include <iostream>
#include "AI_Diplomacy.h"
#include "player.h"
#include "playerlist.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
//#define debug(x)

AI_Diplomacy* AI_Diplomacy::instance = 0;

AI_Diplomacy::AI_Diplomacy(Player *owner)
:d_owner(owner)
{
  instance = this;
}

AI_Diplomacy::~AI_Diplomacy()
{
  instance = 0;
}

void AI_Diplomacy::makeProposals()
{
  // Declare war with enemies, make peace with friends
  Playerlist *pl = Playerlist::getInstance();
  for (Playerlist::iterator it = pl->begin(); it != pl->end(); it++)
    {
      if (pl->getNeutral() == (*it))
	continue;
      if ((*it)->isDead())
	continue;
      if ((*it) == d_owner)
	continue;
      if (d_owner->getDiplomaticState(*it) != Player::AT_WAR)
	{
	  if (d_owner->getDiplomaticScore (*it) < DIPLOMACY_MIN_SCORE + 2)
	    d_owner->proposeDiplomacy (Player::PROPOSE_WAR , *it);
	}
      else if (d_owner->getDiplomaticState(*it) != Player::AT_PEACE)
	{
	  if (d_owner->getDiplomaticScore (*it) > DIPLOMACY_MAX_SCORE - 2)
	    d_owner->proposeDiplomacy (Player::PROPOSE_PEACE, *it);
	}
    }
}
// End of file
