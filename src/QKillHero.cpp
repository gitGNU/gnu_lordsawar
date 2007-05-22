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
#include <sstream>
#include <assert.h>
#include <sigc++/functors/mem_fun.h>

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
    // we want to stay informed about killed armies...
    const Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::const_iterator it = pl->begin(); it != pl->end(); it++)
        (*it)->sdyingArmy.connect(sigc::mem_fun(*this, &QuestKillHero::dyingArmy));
    
    // find a suitable hero for us
    Hero *hunted = chooseToKill();
    assert(hunted);         // should never fail, since isFeasible is checked

    d_victim = hunted->getId();
    initDescription();
}
//=======================================================================
QuestKillHero::QuestKillHero(QuestsManager& q_mgr, XML_Helper* helper) 
    : Quest(q_mgr, helper)
{
    const Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::const_iterator it = pl->begin(); it != pl->end(); it++)
        (*it)->sdyingArmy.connect(sigc::mem_fun(*this, &QuestKillHero::dyingArmy));

    helper->getData(d_victim, "to_kill");
    debug("load: hero_to_kill = " << d_victim);

    // double and triple check :)
    Hero *hero = Quest::getHeroById(d_victim);
    assert(hero);

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

    retval &= helper->openTag("quest");
    retval &= Quest::save(helper);
    retval &= helper->saveData("to_kill", d_victim);
    retval &= helper->closeTag();

    return retval;
}
//=======================================================================
std::string QuestKillHero::getProgress() const
{
    std::stringstream ss;
    ss <<_("You're still searching for him...");

    // add info about the location of the hunted:
    Stack* s = 0;
    Quest::getHeroById(d_victim, &s);

    ss << _("Seen lately near (") << s->getPos().x << ", " << s->getPos().y
       << ")";
    return ss.str();
}
//=======================================================================
void QuestKillHero::getSuccessMsg(std::queue<std::string>& msgs) const
{
    std::string name = getHeroById(d_victim)->getName();
    char buffer[101]; buffer[100]='\0';
    snprintf(buffer, 100, _("You slayed the wicked hero %s. He deserved that!"), name.c_str());
    msgs.push(std::string(buffer));
}
//=======================================================================
void QuestKillHero::getExpiredMsg(std::queue<std::string>& msgs) const
{
    char buffer[101]; buffer[100]='\0';
    snprintf(buffer, 100, _("You could not slay the wicked hero %s"),
            getHeroById(d_victim)->getName().c_str());

    msgs.push(std::string(buffer));
    msgs.push(_("He was slain by other fellows!"));
}
//=======================================================================
void QuestKillHero::dyingArmy(Army *army, std::vector<Uint32> culprits)
{
    if (!isActive())
        return;
        
    // is it a hero we were hunting for?
    if (army->getId() != d_victim)
        return;

    // Answer: yes; now check if he was killed by our hero
    if (culprits.size()==0) 
    {
        /*The Hero was killed by a stack without heroes so the quest expires*/
        debug("SORRY: YOUR QUEST 'KILL HERO' HS EXPIRED BECAUSE THE HERO TO KILL WAS KILLED BY ANOTHER ONE");
        d_q_mgr.questExpired(d_hero);
        return;
    }

    for (unsigned int i = 0; i < culprits.size(); i++)
        if (culprits[i] == d_hero)
        {
            debug("CONGRATULATIONS: QUEST 'KILL HERO' IS COMPLETED!");
            d_q_mgr.questCompleted(d_hero);
            return;
        }

    debug("SORRY: YOUR QUEST 'KILL HERO' EXPIRED BECAUSE THE HERO TO KILL WAS KILLED BY ANOTHER ONE");
    d_q_mgr.questExpired(d_hero);
}
//=======================================================================
void QuestKillHero::initDescription()
{
    char buffer[101]; buffer[100]='\0';
    Hero* v = Quest::getHeroById(d_victim);
    
    snprintf(buffer, 100, _("Kill the hero named %s, servant of player %s"),
            v->getName().c_str(), v->getPlayer()->getName().c_str());
    
    d_description = std::string(buffer);
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
