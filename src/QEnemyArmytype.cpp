//  Copyright (C) 2007, 2008 Ben Asselstine
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

#include "QEnemyArmytype.h"
#include "QuestsManager.h"
#include "playerlist.h"
#include "stacklist.h"
#include "citylist.h"
#include "defs.h"
#include "armysetlist.h"

using namespace std;

//go get an existing army type,
//with the stipluation that player P's armies are not taken into consideration
int getVictimArmytype(Player *p, std::list<Vector<int> >&targets)
{
  std::vector<Army*> specials;
  Stacklist::const_iterator sit ;
  Stack::iterator it ;
  Stacklist *sl;
  Citylist *cl = Citylist::getInstance();
  const Playerlist* pl = Playerlist::getInstance();
  for (Playerlist::const_iterator pit = pl->begin(); pit != pl->end(); pit++)
    {
      if ((*pit) == p)
        continue;
      sl = (*pit)->getStacklist();
      for (sit = sl->begin(); sit != sl->end(); sit++)
        {
          //is this stack not in a city?  no?  it's a target.
          if (cl->getObjectAt((*sit)->getPos()) == NULL)
            targets.push_back((*sit)->getPos());
          for (it = (*sit)->begin(); it != (*sit)->end(); it++)
            {
              if ((*it)->getAwardable())
                {
                  specials.push_back((*it));
                }
	    }
        }
    }
  if (specials.size() == 0)
    return -1;
  else
    return specials[rand() % specials.size()]->getType();
}

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)
//=======================================================================
QuestEnemyArmytype::QuestEnemyArmytype(QuestsManager& q_mgr, Uint32 hero)
    : Quest(q_mgr, hero, Quest::KILLARMYTYPE)
{
    // have us be informed when hostilities break out
    Player *p = getHero()->getPlayer();
    
    // pick a victim
    d_type_to_kill = getVictimArmytype (p, d_targets);
    
    initDescription();
}
//=======================================================================
QuestEnemyArmytype::QuestEnemyArmytype(QuestsManager& q_mgr, XML_Helper* helper) 
    : Quest(q_mgr, helper)
{
    helper->getData(d_type_to_kill, "type_to_kill");

    initDescription();
}
//=======================================================================
bool QuestEnemyArmytype::save(XML_Helper *helper) const
{
    bool retval = true;

    retval &= helper->openTag("quest");
    retval &= Quest::save(helper);
    retval &= helper->saveData("type_to_kill", d_type_to_kill);
    retval &= helper->closeTag();

    return retval;
}
//=======================================================================
std::string QuestEnemyArmytype::getProgress() const
{
    Armysetlist *al = Armysetlist::getInstance();
    //FIXME: we're picking a type that *we* own, not what our enemy owns
    Uint32 set = Playerlist::getInstance()->getActiveplayer()->getArmyset();
    const Army *a = al->getArmy(set, d_type_to_kill);
    char buffer[101]; buffer[100]='\0';
    snprintf(buffer, 100, _("You have not killed a unit of enemy %s yet."), 
	     a->getName().c_str());
    return std::string(buffer);
}
//=======================================================================
void QuestEnemyArmytype::getSuccessMsg(std::queue<std::string>& msgs) const
{
    Armysetlist *al = Armysetlist::getInstance();
    Uint32 set = Playerlist::getInstance()->getActiveplayer()->getArmyset();
    const Army *a = al->getArmy(set, d_type_to_kill);
    char buffer[101]; buffer[100]='\0';
    snprintf(buffer, 100, _("You have killed a unit of enemy %s."), a->getName().c_str());
    
    msgs.push(std::string(buffer));
    msgs.push(_("Well done!"));
}
//=======================================================================
void QuestEnemyArmytype::getExpiredMsg(std::queue<std::string>& msgs) const
{
    // This quest should never expire, so this is just a dummy function
}
//=======================================================================
void QuestEnemyArmytype::initDescription()
{
    Armysetlist *al = Armysetlist::getInstance();
    Uint32 set = Playerlist::getInstance()->getActiveplayer()->getArmyset();
    const Army *a = al->getArmy(set, d_type_to_kill);
    char buffer[101]; buffer[100]='\0';
    snprintf(buffer, 100, _("You must destroy a unit of enemy %s."), 
	     a->getName().c_str());

    d_description = std::string(buffer);
}
//=======================================================================
bool QuestEnemyArmytype::isFeasible(Uint32 heroId)
{
  std::list< Vector<int> >targets;
  int type = getVictimArmytype(getHeroById(heroId)->getPlayer(), targets);
  if (type >= 0)
    return true;
  return false;
}
void QuestEnemyArmytype::armyDied(Army *a, bool heroIsCulprit)
{
  //was it the army type we were after?
    
  debug("QuestEnemyArmytype: armyDied - pending = " << (int)d_pending);
    
  if (!isActive())
    return;
    
  if (a->getType() == d_type_to_kill)
    {
      if (heroIsCulprit)
	{
	  debug("CONGRATULATIONS: QUEST 'KILL ENEMY ARMYTYPE' IS COMPLETED!");
	  d_q_mgr.questCompleted(d_hero);
	}
      else
	{
	  ; 
	  //hopefully there are more armies of this type for hero to kill
	}
    }
}
void QuestEnemyArmytype::cityAction(City *c, CityDefeatedAction action, 
				    bool heroIsCulprit, int gold)
{
  ;//this quest doesn't care what happens to cities
}
