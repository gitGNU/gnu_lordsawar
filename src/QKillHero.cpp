// Copyright (C) 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2004 Andrea Paternesi
// Copyright (C) 2007, 2008 Ben Asselstine
// Copyright (C) 2008 Ole Laursen
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

#include <iostream>
#include <sstream>
#include <assert.h>
#include <sigc++/functors/mem_fun.h>
#include "ucompose.hpp"

#include "history.h"
#include "QKillHero.h"
#include "QuestsManager.h"
#include "playerlist.h"
#include "stacklist.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

//=======================================================================
QuestKillHero::QuestKillHero(QuestsManager& mgr, Uint32 hero) 
    : Quest(mgr, hero, Quest::KILLHERO)
{
    // find a suitable hero for us
    Hero *hunted = chooseToKill();
    assert(hunted);         // should never fail, since isFeasible is checked

    d_victim = hunted->getId();
    d_targets.push_back(hunted->getOwner()->getStacklist()->getPosition(d_victim));
    initDescription();
}
//=======================================================================
QuestKillHero::QuestKillHero(QuestsManager& q_mgr, XML_Helper* helper) 
    : Quest(q_mgr, helper)
{
    helper->getData(d_victim, "to_kill");
    debug("load: hero_to_kill = " << d_victim);

    // double and triple check :)
    Hero *hero = Quest::getHeroById(d_victim);
    assert(hero);
    d_targets.push_back(hero->getOwner()->getStacklist()->getPosition(d_victim));

    initDescription();
}
//=======================================================================
QuestKillHero::QuestKillHero(QuestsManager& q_mgr, Uint32 hero, Uint32 victim) 
    : Quest(q_mgr, hero, Quest::KILLHERO)
{
    d_victim = victim;

    // double and triple check :)
    Hero *h = Quest::getHeroById(d_victim);
    assert(h);
    d_targets.push_back(h->getOwner()->getStacklist()->getPosition(d_victim));

    initDescription();
}
//=======================================================================
bool QuestKillHero::isFeasible(Uint32 heroId)
{
    // chooseToKill returns 0 if no enemy heroes exist
    return (chooseToKill() != 0);
}
//=======================================================================
bool QuestKillHero::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(Quest::d_tag);
    retval &= Quest::save(helper);
    retval &= helper->saveData("to_kill", d_victim);
    retval &= helper->closeTag();

    return retval;
}
//=======================================================================
std::string QuestKillHero::getProgress() const
{
    std::stringstream ss;

    // add info about the location of the hunted:
    Stack* s = 0;
    Quest::getHeroById(d_victim, &s);

    ss <<_("You're still searching for him...");
    ss << _("Seen lately near (") << s->getPos().x << ", " << s->getPos().y
       << ")";
    return ss.str();
}
//=======================================================================
void QuestKillHero::getSuccessMsg(std::queue<std::string>& msgs) const
{
    std::string name = getHeroNameForDeadHero(d_victim);
    msgs.push(String::ucompose(_("You have slain the wicked hero %1."), name));
}
//=======================================================================
void QuestKillHero::getExpiredMsg(std::queue<std::string>& msgs) const
{
    msgs.push(String::ucompose(_("You could not slay the wicked hero %1"),
            getHeroNameForDeadHero(d_victim)));
    msgs.push(_("The hero was slain by other fellows!"));
}
//=======================================================================
void QuestKillHero::initDescription()
{
    Hero* v = Quest::getHeroById(d_victim);
    d_description = String::ucompose(_("Kill the hero named %1, servant of player %2."),
            v->getName(), v->getOwner()->getName());
}
//=======================================================================
Hero* QuestKillHero::chooseToKill()
{
    std::vector<Hero*> heroes;
    
    // Collect all enemy heroes in the vector
    const Playerlist* pl = Playerlist::getInstance();
    Player* active = Playerlist::getActiveplayer();

    for (Playerlist::const_iterator pit = pl->begin(); pit != pl->end(); pit++)
    {
        if ((*pit) == active)
            continue;

        const Stacklist* sl = (*pit)->getStacklist();
        for (Stacklist::const_iterator it = sl->begin(); it != sl->end(); it++)
            for (Stack::iterator sit = (*it)->begin(); sit != (*it)->end(); sit++)
                if ((*sit)->isHero()) 
                    heroes.push_back(dynamic_cast<Hero*>(*sit));
    }
     
    // isFeasible() should depend on this behaviour...
    if (heroes.empty())
        return NULL;
    
    // Now pick a hero:
    return heroes[rand() % heroes.size()];
}
	
void QuestKillHero::armyDied(Army *a, bool heroIsCulprit)
{
  if (!isActive())
    return;

  // is it a hero we were hunting for?
  if (a->getId() != d_victim)
    return;

  // Answer: yes; now check if he was killed by our hero
  if (heroIsCulprit == false)
    {
      /*The Hero was killed by a stack without heroes so the quest expires*/
      //debug("SORRY: YOUR QUEST 'KILL HERO' HS EXPIRED BECAUSE THE HERO TO KILL WAS KILLED BY ANOTHER ONE");
      d_q_mgr.questExpired(d_hero);
      //hopefully this is handled by questsmanager, and not here!
      return;
    }
  else
    {
      debug("CONGRATULATIONS: QUEST 'KILL HERO' IS COMPLETED!");
      d_q_mgr.questCompleted(d_hero);
      return;
    }

  return;
}
	
void QuestKillHero::cityAction(City *c, CityDefeatedAction action, 
			       bool heroIsCulprit, int gold)
{
  ;//this quest doesn't care what happens to cities
}
