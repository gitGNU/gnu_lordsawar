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

#include "QPillageGold.h"
#include "QuestsManager.h"
#include "playerlist.h"
#include "defs.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)
//=======================================================================
QuestPillageGold::QuestPillageGold(QuestsManager& q_mgr, Uint32 hero)
    : Quest(q_mgr, hero, Quest::PILLAGEGOLD), d_pillaged(0)
{
    // have us be informed when we sack or pillage a city
    Player *p= getHero()->getPlayer();
    p->ssackingCity.connect(
                 sigc::mem_fun(this, &QuestPillageGold::citySackedOrPillaged));
    p->spillagingCity.connect(
                 sigc::mem_fun(this, &QuestPillageGold::citySackedOrPillaged));
   
    //pick an amount of gold to sack and pillage
    d_to_pillage = 850 + (rand() % 630);

    initDescription();
}
//=======================================================================
QuestPillageGold::QuestPillageGold(QuestsManager& q_mgr, XML_Helper* helper) 
    : Quest(q_mgr, helper)
{
    helper->getData(d_to_pillage, "to_pillage");
    helper->getData(d_pillaged,  "pillaged");

    Player *p= getHero()->getPlayer();
    p->ssackingCity.connect(
                 sigc::mem_fun(this, &QuestPillageGold::citySackedOrPillaged));
    p->spillagingCity.connect(
                 sigc::mem_fun(this, &QuestPillageGold::citySackedOrPillaged));

    initDescription();
}
//=======================================================================
bool QuestPillageGold::save(XML_Helper *helper) const
{
    bool retval = true;

    retval &= helper->openTag("quest");
    retval &= Quest::save(helper);
    retval &= helper->saveData("to_pillage", d_to_pillage);
    retval &= helper->saveData("pillaged",  d_pillaged);
    retval &= helper->closeTag();

    return retval;
}
//=======================================================================
std::string QuestPillageGold::getProgress() const
{
    char buffer[101]; buffer[100]='\0';
    snprintf(buffer, 100, _("You have already stolen %i gold."), d_pillaged);
    return std::string(buffer);
}
//=======================================================================
void QuestPillageGold::getSuccessMsg(std::queue<std::string>& msgs) const
{
    char buffer[101]; buffer[100]='\0';
    snprintf(buffer, 100, "You have managed to sack and pillage %i gold.", d_pillaged);
    
    msgs.push(std::string(buffer));
    msgs.push(_("Well done!"));
}
//=======================================================================
void QuestPillageGold::getExpiredMsg(std::queue<std::string>& msgs) const
{
    // This quest should never expire, so this is just a dummy function
}
//=======================================================================
void QuestPillageGold::citySackedOrPillaged (City* city, Stack* s, int gold,
                                             std::list<Uint32> sacked_types)
{
  if (!isActive())
    return;
  d_pillaged += gold;
  if (d_pillaged > d_to_pillage)
    {
      d_pillaged = d_to_pillage;
      d_q_mgr.questCompleted(d_hero);
    }
}
//=======================================================================
void QuestPillageGold::initDescription()
{
    char buffer[101]; buffer[100]='\0';
    snprintf(buffer, 100, "You shall sack and pillage %i gold from thy mighty foes", d_to_pillage);

    d_description = std::string(buffer);
}
