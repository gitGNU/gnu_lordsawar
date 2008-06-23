// Copyright (C) 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
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

#include <sigc++/functors/mem_fun.h>

#include "QuestsManager.h"

#include "Quest.h"
#include "QKillHero.h"
#include "QEnemyArmies.h"
#include "QCitySack.h"
#include "QCityRaze.h"
#include "QCityOccupy.h"
#include "QEnemyArmytype.h"
#include "QPillageGold.h"
#include "stacklist.h"
#include "rewardlist.h"
#include "army.h"
#include "xmlhelper.h"
#include "history.h"

QuestsManager* QuestsManager::s_instance = NULL;

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)


//======================================================================
QuestsManager* QuestsManager::getInstance()
{
    if (s_instance == 0)
        s_instance = new QuestsManager();
    return s_instance;
}
//======================================================================
QuestsManager* QuestsManager::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new QuestsManager(helper);
    return s_instance;
}
//=======================================================================
void QuestsManager::deleteInstance()
{
    debug("QuestsManager: deleteInstance")
    delete s_instance;
    s_instance = 0;
}
//======================================================================
QuestsManager::QuestsManager()
{
    sharedInit();
}
//======================================================================
QuestsManager::QuestsManager(XML_Helper* helper)
{
    sharedInit();
    debug("QuestsManager: registerTag!");
    helper->registerTag("quest", sigc::mem_fun((*this), &QuestsManager::load));
}
//======================================================================
QuestsManager::~QuestsManager()
{
    std::map<Uint32,Quest*>::iterator it;

    for (it = d_quests.begin(); it != d_quests.end(); it++) 
        delete (*it).second;
    cleanup();
}
//======================================================================
Quest* QuestsManager::createNewQuest(Uint32 heroId, bool razing_possible)
{
    // don't let a hero have more than one quest
    if (d_quests.count(heroId))
        return NULL;

    int which = 0;
    while (!which)
    {
        which = 1 + rand() % 7;
        // if this quest is not feasible - try again with another
        // quest:
        if ((*(d_questsFeasible[which-1]))(heroId) == 0)
            which = 0;
    }
    // ok - this quest can be completed

    Quest *quest = NULL;
    switch (which)
    {
        case 1:
            quest = new QuestKillHero(*this, heroId);
            break;
        case 2:
            quest = new QuestEnemyArmies(*this, heroId);
            break;
        case 3:
            quest = new QuestCitySack(*this, heroId);
            break;
        case 4:
	    if (razing_possible)
	      quest = new QuestCityRaze(*this, heroId);
	    else
	      quest = new QuestCitySack(*this, heroId);
            break;
        case 5:
            quest = new QuestCityOccupy(*this, heroId);
            break;
        case 6:
            quest = new QuestEnemyArmytype(*this, heroId);
            break;
        case 7:
            quest = new QuestPillageGold(*this, heroId);
            break;
    }
    
    if (quest)
    {
        d_quests[heroId] = quest;
    }
    
    return quest;
}
//========================================================================
Quest* QuestsManager::createNewQuest(Uint32 hero, Uint32 questtype,
                                     Uint32 data, Uint32 victim_player)
{
  Quest *quest = NULL;
  switch (Quest::Type(questtype))
  {
  case Quest::KILLHERO:
    quest = new QuestKillHero(*this, hero, data);
    break;
  case Quest::KILLARMIES:
    quest = new QuestEnemyArmies(*this, hero, data, victim_player);
    break;
  case Quest::CITYSACK:
    quest = new QuestCitySack(*this, hero, data);
    break;
  case Quest::CITYRAZE:
    quest = new QuestCityRaze(*this, hero, data);
    break;
  case Quest::CITYOCCUPY:
    quest = new QuestCityOccupy(*this, hero, data);
    break;
  case Quest::KILLARMYTYPE:
    quest = new QuestEnemyArmytype(*this, hero, data);
    break;
  case Quest::PILLAGEGOLD:
    quest = new QuestPillageGold(*this, hero, data);
    break;
  }
    
  if (quest)
  {
    d_quests[hero] = quest;
  }
  
  return quest;
}
//========================================================================
void QuestsManager::questCompleted(Uint32 heroId)
{
    Quest *quest = d_quests[heroId];
    Player *p = quest->getHero()->getOwner();

    p->heroCompletesQuest(quest->getHero());

    int num = rand() % 4;
    if (num == 0)
      {
	int gold = Reward_Gold::getRandomGoldPieces();
        Reward_Gold reward(gold);
        p->giveReward(p->getActivestack(), &reward);
        quest_completed.emit(quest, &reward);
      }
    else if (num == 1)
      {
        int num = (rand() % 8) + 1;
        const Army *a = Reward_Allies::randomArmyAlly();
        Reward_Allies reward(a, num);
        p->giveReward(p->getActivestack(), &reward);
        quest_completed.emit(quest, &reward);
	    
	History_HeroFindsAllies* item = new History_HeroFindsAllies();
	item->fillData(quest->getHero());
	p->getHistorylist()->push_back(item);
      }
    else if (num == 2)
      {
        Reward *itemReward = Rewardlist::getInstance()->popRandomItemReward();
        if (itemReward)
          {
            p->giveReward(p->getActivestack(), itemReward);
            quest_completed.emit(quest, itemReward);
          }
        else //no items left to give!
          {
	    int gold = Reward_Gold::getRandomGoldPieces();
            Reward_Gold reward(gold);
            p->giveReward(p->getActivestack(), &reward);
            quest_completed.emit(quest, &reward);
          }
      }
    else if (num == 3)
      {
        Reward *ruinReward = Rewardlist::getInstance()->popRandomRuinReward();
        if (ruinReward)
          {
            p->giveReward(p->getActivestack(), ruinReward);
            quest_completed.emit(quest, ruinReward);
          }
        else //no ruins left to give!
          {
            int gold = Reward_Gold::getRandomGoldPieces();
            Reward_Gold reward(gold);
            p->giveReward(p->getActivestack(), &reward);
            quest_completed.emit(quest, &reward);
          }
      }

    //debug("deactivate quest");
    //deactivateQuest(heroId);
    d_quests.erase(heroId);
    //debug("quest deactivated");
}
//========================================================================
void QuestsManager::questExpired(Uint32 heroId)
{
    Quest *quest = d_quests[heroId];

    if (quest == 0)
        return;
    
    //quest_expired.emit(quest);
    
    debug("deactivate quest");
    deactivateQuest(heroId);
    debug("quest deactivated");
}
//=========================================================================
std::vector<Quest*> QuestsManager::getPlayerQuests(Player *player)
{
    std::vector<Quest*> res;
    // loop through the player's units
    // for every hero check any pending quests
    const Stacklist* sl = player->getStacklist();
    for (Stacklist::const_iterator it = sl->begin(); it != sl->end(); it++)
    {
        for (Stack::const_iterator sit = (*it)->begin();
            sit != (*it)->end(); sit++)
	    {
            Uint32 heroId = (*sit)->getId();
            if (((*sit)->isHero()) && (d_quests.count(heroId)))
            {
                debug("heroId = " << heroId << " - has quest: " 
                      << d_quests[heroId]);
                res.push_back(d_quests[heroId]);
            }
        }
    }
    return res;
}
//======================================================================
bool QuestsManager::save(XML_Helper* helper) const
{
    debug("Saving quests\n");

	bool retval = true;
	retval &= helper->openTag("questlist");
    
    for (std::map<Uint32,Quest*>::const_iterator it = d_quests.begin(); 
	 it != d_quests.end(); it++) 
      {
	if ((*it).second == NULL)
	  continue;
	retval &= ((*it).second)->save(helper);
      }
    for (std::list<Quest *>::const_iterator it = d_inactive_quests.begin();
	 it != d_inactive_quests.end(); it++)
      retval &= (*it)->save(helper);

    debug("Quests saved\n");
	retval &= helper->closeTag();
	return retval;
}
//===========================================================================
bool QuestsManager::load(string tag, XML_Helper* helper)
{
    debug("QuestsManager: load tag = " << tag);

    if (tag == "quest")
    {
        Uint32  questType, hero;
        helper->getData(questType, "type");
        helper->getData(hero, "hero");

        debug("quest load: type = " << questType << ", heroId = " << hero);

        Quest *quest=0;
        switch (static_cast<Quest::Type>(questType)) {
            case Quest::KILLHERO:
                quest = new QuestKillHero(*this, helper);
                break;
            case Quest::KILLARMIES:
                quest = new QuestEnemyArmies(*this, helper);
                break;
            case Quest::CITYSACK:
                quest = new QuestCitySack(*this, helper);
                break;
            case Quest::CITYRAZE:
                quest = new QuestCityRaze(*this, helper);
                break;
            case Quest::CITYOCCUPY:
                quest = new QuestCityOccupy(*this, helper);
                break;
            case Quest::KILLARMYTYPE:
                quest = new QuestEnemyArmytype(*this, helper);
                break;
            case Quest::PILLAGEGOLD:
                quest = new QuestPillageGold(*this, helper);
                break;
        }
        
        debug("quest created: q = " << quest);
        if (quest)
	  {
	    if (quest->isPendingDeletion())
	      d_inactive_quests.push_back(quest);
	    else
	      d_quests[hero] = quest;
	  }
        
        return true;
    }

    return false;
}
//======================================================================
void QuestsManager::sharedInit()
{
    debug("QuestsManager constructor")

    // now prepare the vector of pointers to the
    // functions (class static members) checking feasibility
    // for every quest
    d_questsFeasible.push_back(&(QuestKillHero::isFeasible));
    d_questsFeasible.push_back(&(QuestEnemyArmies::isFeasible));
    d_questsFeasible.push_back(&(QuestCitySack::isFeasible));
    d_questsFeasible.push_back(&(QuestCityRaze::isFeasible));
    d_questsFeasible.push_back(&(QuestCityOccupy::isFeasible));
    d_questsFeasible.push_back(&(QuestEnemyArmytype::isFeasible));
    d_questsFeasible.push_back(&(QuestPillageGold::isFeasible));
}
//========================================================================
void QuestsManager::deactivateQuest(Uint32 heroId)
{
    Quest *q = d_quests[heroId];
    q->deactivate();
    d_inactive_quests.push_back(q);
    // delete it from hash of active quests
    d_quests.erase(heroId);
}
//======================================================================
void QuestsManager::cleanup(Player::Type type)
{
    debug("QuestsManager: cleanup!");

    std::list<Quest *>::iterator it = d_inactive_quests.begin();
    while(it != d_inactive_quests.end())
      {
	Quest *q =(*it);
	it = d_inactive_quests.erase(it);
	if (q)
	  delete q;
      }
}

void QuestsManager::armyDied(Army *a, std::vector<Uint32>& culprits)
{
  //tell all quests that an army died
  //each quest takes care of what happens when an army dies
  std::map<Uint32,Quest*>::iterator it;
  for (it = d_quests.begin(); it != d_quests.end(); it++) 
    {
      if ((*it).second == NULL)
	continue;
      if ((*it).second->isPendingDeletion() == true)
	continue;
      //was this hero a perpetrator?
      bool heroIsCulprit = false;
      for (unsigned int i = 0; i <culprits.size(); i++)
	{
	  if (culprits[i] == (*it).second->getHeroId())
	    {
	      heroIsCulprit = true;
	      break;
	    }
	}
      (*it).second->armyDied(a, heroIsCulprit);
    }

  //is it a hero that has an outstanding quest?
  //this is what deactivates a quest upon hero death
  Quest *quest = d_quests[a->getId()];
  if (quest && quest->isPendingDeletion() == false)
    questExpired(a->getId());

}

void QuestsManager::cityAction(City *c, Stack *s, 
			       CityDefeatedAction action, int gold)
{
  std::map<Uint32,Quest*>::iterator it;
  //did any of the heroes who have outstanding quests do this?
  for (it = d_quests.begin(); it != d_quests.end(); it++) 
    {
      if ((*it).second == NULL)
	continue;
      if ((*it).second->isPendingDeletion() == true)
	continue;
      if (!s)
	(*it).second->cityAction(c, action, false, gold);
      else
	{
	  for (Stack::iterator sit = s->begin(); sit != s->end(); sit++)
	    {
	      if ((*it).second == NULL) //fixme: how is this null sometimes?
		break;
	      if ((*sit)->getId() == (*it).second->getHeroId())
		(*it).second->cityAction(c, action, true, gold);
	      else
		(*it).second->cityAction(c, action, false, gold);
	    }
	}
    }
}

void QuestsManager::cityRazed(City *c, Stack *s)
{
  cityAction(c, s, CITY_DEFEATED_RAZE, 0);
  //did we raze a city we care about in another quest?
}
void QuestsManager::citySacked(City *c, Stack *s, int gold)
{
  cityAction(c, s, CITY_DEFEATED_SACK, gold);
}
void QuestsManager::cityPillaged(City *c, Stack *s, int gold)
{
  cityAction(c, s, CITY_DEFEATED_PILLAGE, gold);
}
void QuestsManager::cityOccupied(City *c, Stack *s)
{
  cityAction(c, s, CITY_DEFEATED_OCCUPY, 0);
}
void QuestsManager::nextTurn(Player *p)
{
  // go through our inactive list and remove quests belonging to us
  for (std::list<Quest*>::iterator it = d_inactive_quests.begin();
       it != d_inactive_quests.end(); it++)
    {
      if ((*it)->getPlayer() == p)
	{
	  quest_expired.emit(*it);
	  Quest *q = *it;
	  it = d_inactive_quests.erase(it);
	  if (q)
	    delete q;
	}
    }
}
