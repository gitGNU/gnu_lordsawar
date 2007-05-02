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

#include <sstream>
#include <sigc++/functors/mem_fun.h>

#include "QEnemyArmies.h"
#include "QuestsManager.h"
#include "playerlist.h"
#include "defs.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)
//=======================================================================
QuestEnemyArmies::QuestEnemyArmies(QuestsManager& q_mgr, Uint32 hero)
    : Quest(q_mgr, hero, Quest::KILLARMIES), d_killed(0)
{
    // have us be informed when hostilities break out
    const Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::const_iterator it = pl->begin(); it != pl->end(); it++)
        (*it)->sdyingArmy.connect( sigc::mem_fun(*this, &QuestEnemyArmies::dyingArmy));
    
    /** we have to kill 14-20 units: 14 + rand(0..6) */
    d_to_kill = 14 + (rand() % 7);
    debug("to kill: " << _how_many_to_kill);
    
    initDescription();
}
//=======================================================================
QuestEnemyArmies::QuestEnemyArmies(QuestsManager& q_mgr, XML_Helper* helper) 
    : Quest(q_mgr, helper)
{
    // we want to be informed about fight causalties
    const Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::const_iterator it = pl->begin(); it != pl->end(); it++)
        (*it)->sdyingArmy.connect( sigc::mem_fun(*this, &QuestEnemyArmies::dyingArmy));
    
    helper->getData(d_to_kill, "to_kill");
    helper->getData(d_killed,  "killed");

    initDescription();
}
//=======================================================================
bool QuestEnemyArmies::save(XML_Helper *helper) const
{
    bool retval = true;

    retval &= helper->openTag("quest");
    retval &= Quest::save(helper);
    retval &= helper->saveData("to_kill", d_to_kill);
    retval &= helper->saveData("killed",  d_killed);
    retval &= helper->closeTag();

    return retval;
}
//=======================================================================
std::string QuestEnemyArmies::getProgress() const
{
    char buffer[101]; buffer[100]='\0';
    snprintf(buffer, 100, _("You have killed %i so far."), d_killed);
    return std::string(buffer);
}
//=======================================================================
void QuestEnemyArmies::getSuccessMsg(std::queue<std::string>& msgs) const
{
    char buffer[101]; buffer[100]='\0';
    snprintf(buffer, 100, ngettext("You have managed to slaughter %i army.",
            "You have managed to slaughter %i armies.", d_killed), d_killed);
    
    msgs.push(std::string(buffer));
    msgs.push(_("Well done!"));
}
//=======================================================================
void QuestEnemyArmies::getExpiredMsg(std::queue<std::string>& msgs) const
{
    // This quest should never expire, so this is just a dummy function
}
//=======================================================================
void QuestEnemyArmies::dyingArmy(Army *army, std::vector<Uint32> culprits)
{
    debug("QuestEnemyArmies: dyingArmy - pending = " << (int)d_pending);

    if (!isActive())
        return;
    
    // did our hero kill them?
    for (unsigned int i = 0; i < culprits.size(); i++)
    {
        if (culprits[i] == d_hero)
        {
            d_killed++;
            if (d_killed >= d_to_kill)
            {
                debug("CONGRATULATIONS: QUEST 'ENEMY ARMIES' IS COMPLETED!");
                d_q_mgr.questCompleted(d_hero);
            }
            break;
        }
    }
}
//=======================================================================
void QuestEnemyArmies::initDescription()
{
    char buffer[101]; buffer[100]='\0';
    snprintf(buffer, 100, ngettext("You must destroy %i unit of enemy armies",
            "You must destroy %i units of enemy armies", d_to_kill), d_to_kill);

    d_description = std::string(buffer);
}
