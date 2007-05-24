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
#include "army.h"
#include "xmlhelper.h"

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
    _sharedInit();
}
//======================================================================
QuestsManager::QuestsManager(XML_Helper* helper)
{
    _sharedInit();
    debug("QuestsManager: registerTag!");
    helper->registerTag("quest", sigc::mem_fun((*this), &QuestsManager::load));
}
//======================================================================
QuestsManager::~QuestsManager()
{
    std::map<Uint32,Quest*>::iterator it;

    for (it = d_quests.begin(); it != d_quests.end(); it++) 
        delete (*it).second;
    _cleanup();
}
//======================================================================
Quest* QuestsManager::createNewQuest(Uint32 heroId)
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
            quest = new QuestCityRaze(*this, heroId);
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
void QuestsManager::questCompleted(Uint32 heroId)
{
    Quest *quest = d_quests[heroId];
    
    // now the reward - at the moment a very simple one (just money):
    int gold = rand() % 1000;
    quest->getHero()->getPlayer()->giveReward(gold);
    
    quest_completed.emit(quest, gold);
    
    debug("deactivate quest");
    _deactivateQuest(heroId);
    debug("quest deactivated");
}
//========================================================================
void QuestsManager::questExpired(Uint32 heroId)
{
    Quest *quest = d_quests[heroId];

    if (quest == 0)
        return;
    
    quest_expired.emit(quest);
    
    debug("deactivate quest");
    _deactivateQuest(heroId);
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
    
    std::map<Uint32,Quest*>::const_iterator it;
    for (it = d_quests.begin(); it != d_quests.end(); it++) 
        retval &= ((*it).second)->save(helper);

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
            d_quests[hero] = quest;
        
        return true;
    }

    return false;
}
//======================================================================
void QuestsManager::_sharedInit()
{
    debug("QuestsManager constructor: connecting to the signals")

    const Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::const_iterator it = pl->begin(); it != pl->end(); it++)
        (*it)->sdyingArmy.connect( sigc::mem_fun(*this, &QuestsManager::_dyingArmy));

    sendingTurn.connect( sigc::mem_fun(*this, &QuestsManager::_cleanup));

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
void QuestsManager::_dyingArmy(Army* army, std::vector<Uint32> culprits)
{
    if (!army->isHero())
        return;

    if (d_quests.count(army->getId()))
    {
        debug("QuestsManager: the hero is dead, clearing his quests...");
        _deactivateQuest(army->getId());
    }
}
//========================================================================
void QuestsManager::_deactivateQuest(Uint32 heroId)
{
    Quest *q = d_quests[heroId];
    q->deactivate();
    d_inactive_quests.push(q);
    // delete it from hash of active quests
    d_quests.erase(heroId);
}
//======================================================================
void QuestsManager::_cleanup(Player::Type type)
{
    debug("QuestsManager: _cleanup!");

    while (d_inactive_quests.empty() == false)
    {
        delete d_inactive_quests.front();
        d_inactive_quests.pop();
    }
}
