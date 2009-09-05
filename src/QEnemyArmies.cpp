// Copyright (C) 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2004 Andrea Paternesi
// Copyright (C) 2007, 2008 Ben Asselstine
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

#include <sstream>
#include <sigc++/functors/mem_fun.h>
#include "ucompose.hpp"

#include "QEnemyArmies.h"
#include "QuestsManager.h"
#include "playerlist.h"
#include "stacklist.h"
#include "citylist.h"

using namespace std;

//go get an existing alive player,
//with the stipluation that player P is not taken into consideration
Player* getVictimPlayer(Player *p)
{
  std::vector<Player*> players;
  const Playerlist* pl = Playerlist::getInstance();
  for (Playerlist::const_iterator it = pl->begin(); it != pl->end(); it++)
    {
      if ((*it) != p && (*it)->isDead() == false && (*it) != pl->getNeutral())
        players.push_back((*it));
    }
  if (players.size() == 0)
    return NULL;
  else
    return players[rand() % players.size()];
}

void QuestEnemyArmies::update_targets()
{
  Stacklist::const_iterator sit ;
  Stacklist *sl = d_victim_player->getStacklist();
  Citylist *cl = Citylist::getInstance();
  d_targets.clear();
  for (sit = sl->begin(); sit != sl->end(); sit++)
    {
      //is this not a city location?  no?  then it's a target.
      if (cl->getObjectAt((*sit)->getPos()) == NULL)
        d_targets.push_back((*sit)->getPos());
    }
}

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)
//=======================================================================
QuestEnemyArmies::QuestEnemyArmies(QuestsManager& q_mgr, Uint32 hero)
    : Quest(q_mgr, hero, Quest::KILLARMIES), d_killed(0)
{
    // have us be informed when hostilities break out
    d_victim_player = getVictimPlayer(getHero()->getOwner());
    
    /** we have to kill 14-20 units: 14 + rand(0..6) */
    d_to_kill = 14 + (rand() % 7);

    update_targets();
    initDescription();
}
//=======================================================================
QuestEnemyArmies::QuestEnemyArmies(QuestsManager& q_mgr, XML_Helper* helper) 
    : Quest(q_mgr, helper)
{
    Uint32 ui;
    
    helper->getData(d_to_kill, "to_kill");
    helper->getData(d_killed,  "killed");
    helper->getData(ui, "victim_player");

    d_victim_player = Playerlist::getInstance()->getPlayer(ui);

    update_targets();
    initDescription();
}
//=======================================================================
QuestEnemyArmies::QuestEnemyArmies(QuestsManager& q_mgr, Uint32 hero,
                                   Uint32 armies_to_kill, Uint32 victim_player)
    : Quest(q_mgr, hero, Quest::KILLARMIES), d_killed(0)
{
    // have us be informed when hostilities break out
    d_victim_player = Playerlist::getInstance()->getPlayer(victim_player);
    d_to_kill = armies_to_kill;

    update_targets();
    initDescription();
}
//=======================================================================
bool QuestEnemyArmies::save(XML_Helper *helper) const
{
    bool retval = true;

    retval &= helper->openTag(Quest::d_tag);
    retval &= Quest::save(helper);
    retval &= helper->saveData("to_kill", d_to_kill);
    retval &= helper->saveData("killed",  d_killed);
    retval &= helper->saveData("victim_player", d_victim_player->getId());
    retval &= helper->closeTag();

    return retval;
}
//=======================================================================
std::string QuestEnemyArmies::getProgress() const
{
    return String::ucompose (_("You have killed %1 so far."), d_killed);
}
//=======================================================================
void QuestEnemyArmies::getSuccessMsg(std::queue<std::string>& msgs) const
{
    msgs.push(String::ucompose(_("You have managed to slaughter %1 armies."), d_killed));
    msgs.push(_("Well done!"));
}
//=======================================================================
void QuestEnemyArmies::getExpiredMsg(std::queue<std::string>& msgs) const
{
    // This quest should never expire, so this is just a dummy function
}
//=======================================================================
void QuestEnemyArmies::initDescription()
{
    d_description = String::ucompose(_("You shall slaughter %1 armies of the treacherous %2."),
				     d_to_kill, d_victim_player->getName());
}

bool QuestEnemyArmies::isFeasible(Uint32 heroId)
{
  if (getVictimPlayer(getHeroById(heroId)->getOwner()))
    return true;
  return false;
}

void QuestEnemyArmies::armyDied(Army *a, bool heroIsCulprit)
{
  if (!isActive())
    return;

  if (heroIsCulprit == true && a->getOwner() == d_victim_player)
    {
      d_killed++;
      if (d_killed >= d_to_kill)
	{
	  debug("CONGRATULATIONS: QUEST 'ENEMY ARMIES' IS COMPLETED!");
	  d_q_mgr.questCompleted(d_hero);
	}
    }
}

void QuestEnemyArmies::cityAction(City *c, CityDefeatedAction action, 
				  bool heroIsCulprit, int gold)
{
  ;//this quest doesn't care what happens to cities
}
