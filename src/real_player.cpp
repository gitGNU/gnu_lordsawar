// Copyright (C) 2002, 2003 Michael Bartl
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2006 Andrea Paternesi
// Copyright (C) 2004 John Farrell
// Copyright (C) 2004 Bryan Duff
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
// Copyright (C) 2007, 2008 Ole Laursen
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

#include <fstream>
#include <algorithm>
#include <stdlib.h>
#include <SDL_timer.h>

#include "real_player.h"
#include "action.h"
#include "playerlist.h"
#include "stacklist.h"
#include "citylist.h"
#include "herotemplates.h"
#include "game.h"
#include "GameScenario.h"
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
  maybeRecruitHero();
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

void RealPlayer::levelArmy(Army* a)
{
    // the standard human player just asks the GUI what to do
    Army::Stat stat = snewLevelArmy.emit(a);
    doLevelArmy(a, stat);

    Action_Level* item = new Action_Level();
    item->fillData(a, stat);
    addAction(item);
}

/*
 *
 * what are the chances of a hero showing up?
 *
 * 1 in 6 if you have enough gold, where "enough gold" is...
 *
 * ... 1500 if the player already has a hero, then:  1500 is generally 
 * enough to buy all the heroes.  I forget the exact distribution of 
 * hero prices but memory says from 1000 to 1500.  (But, if you don't 
 * have 1500 gold, and the price is less, you still get the offer...  
 * So, calculate price, compare to available gold, then decided whether 
 * or not to offer...)
 *
 * ...500 if all your heroes are dead: then prices are cut by about 
 * a factor of 3.
 */
bool RealPlayer::maybeRecruitHero ()
{
  bool accepted = false;
  if (this == Playerlist::getInstance()->getNeutral())
    return false;
  
  City *city;
  int gold_needed = 0;
  if (Citylist::getInstance()->countCities(this) == 0)
    return false;
  //give the player a hero if it's the first round.
  //otherwise we get a hero based on chance
  //a hero costs a random number of gold pieces
  if (Game::getScenario()->getRound() == 0)
    gold_needed = 0;
  else
    {
      bool exists = false;
      for (Stacklist::iterator it = d_stacklist->begin(); 
	   it != d_stacklist->end(); it++)
	if ((*it)->hasHero())
	  exists = true; 

      gold_needed = (rand() % 500) + 1000;
      if (exists == false)
	gold_needed /= 2;
    }

  if (((((rand() % 6) == 0) && (gold_needed < getGold())) 
       || gold_needed == 0))
    {
      Hero *herotemplate =HeroTemplates::getInstance()->getRandomHero(getId());
#if 0
      const Armysetlist* al = Armysetlist::getInstance();
      const Army *heroType = al->getArmy (p->getArmyset(), 
					  templateHero->getType());
      Hero* newhero = new Hero(*heroType, templateHero->getName(), p);
      newhero->setGender(Army::Gender(templateHero->getGender()));
#endif
      if (gold_needed == 0)
	city = Citylist::getInstance()->getFirstCity(this);
      else
	{
	  std::vector<City*> cities;
	  Citylist* cl = Citylist::getInstance();
	  for (Citylist::iterator it = cl->begin(); it != cl->end(); ++it)
	    if (!(*it).isBurnt() && (*it).getOwner() == this)
	      cities.push_back(&(*it));
	  if (cities.empty())
	    return false;
	  city = cities[rand() % cities.size()];
	}

      if (srecruitingHero.empty())
        accepted = true;
      else
        accepted = srecruitingHero.emit(herotemplate, city, gold_needed);

      if (accepted) {
        /* now maybe add a few allies */
        int alliesCount;
        if (gold_needed > 1300)
          alliesCount = 3;
        else if (gold_needed > 1000)
          alliesCount = 2;
        else if (gold_needed > 800)
          alliesCount = 1;
        else
          alliesCount = 0;

        const Army *ally = 0;
        if (alliesCount > 0)
        {
          ally = Reward_Allies::randomArmyAlly();
          if (!ally)
            alliesCount = 0;
        }
        
        recruitHero(herotemplate, city, gold_needed, alliesCount, ally);
      }
    }
  return accepted;
}


// End of file
