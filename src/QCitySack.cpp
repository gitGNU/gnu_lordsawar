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

#include "QCitySack.h"
#include "QuestsManager.h"
#include "citylist.h"
#include "playerlist.h"
#include "stack.h"
#include "defs.h"
#include "xmlhelper.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

//=======================================================================
QuestCitySack::QuestCitySack (QuestsManager& mgr, Uint32 hero) 
    : Quest(mgr, hero, Quest::CITYSACK)
{
    // find us a victim
    City* c = chooseToSack(getHero()->getPlayer());
    assert(c);      // should never fail because isFeasible is checked first

    d_city = c->getId();
    d_targets.push_back(c->getPos());
    debug("city_id = " << d_ruin);
    initDescription();
}
//=======================================================================
QuestCitySack::QuestCitySack (QuestsManager& q_mgr, XML_Helper* helper) 
     : Quest(q_mgr, helper)
{
    // let us stay in touch with the world...
    helper->getData(d_city, "city");
    d_targets.push_back(getCity()->getPos());
    initDescription();
}
//=======================================================================
bool QuestCitySack::isFeasible(Uint32 heroId)
{
  if (QuestCitySack::chooseToSack(getHeroById(heroId)->getPlayer()))
    return true;
  return false;
}
//=======================================================================
bool QuestCitySack::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("quest");
    retval &= Quest::save(helper);
    retval &= helper->saveData("city", d_city);
    retval &= helper->closeTag();

    return retval;
}
//=======================================================================
std::string QuestCitySack::getProgress() const
{
    return _("You aren't afraid of doing it, are you?");
}
//=======================================================================
void QuestCitySack::getSuccessMsg(std::queue<std::string>& msgs) const
{
    msgs.push(_("The priests thank you for sacking this evil place."));
}
//=======================================================================
void QuestCitySack::getExpiredMsg(std::queue<std::string>& msgs) const
{
    char buf[101]; buf[100] = '\0';
    const City* c = getCity();

    snprintf(buf, 100, _("The sacking of \"%s\" could not be accomplished."), 
	     c->getName().c_str());
    msgs.push(buf);
}
//=======================================================================
City* QuestCitySack::getCity() const
{
    Citylist* cl = Citylist::getInstance();
    for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
        if ((*it).getId() == d_city)
            return &(*it);

    return 0;
}
//=======================================================================
void QuestCitySack::initDescription()
{
    const City* c = getCity();
    char buffer[121]; buffer[120]='\0';
    
    snprintf(buffer, 100, _("You must sack the city \"%s\" located at (%i,%i)."),
            c->getName().c_str(), c->getPos().x, c->getPos().y);

    d_description = std::string(buffer);
}
//=======================================================================
City * QuestCitySack::chooseToSack(Player *p)
{
    std::vector<City*> cities;

    // Collect all cities
    Citylist* cl = Citylist::getInstance();
    for (Citylist::iterator it = cl->begin(); it != cl->end(); ++it)
        if (!(*it).isBurnt() && (*it).getPlayer() != p && 
            (*it).getNoOfBasicProd() > 1)
            cities.push_back(&(*it));

    // Find a suitable city for us to sack
    if (cities.empty())
        return 0;

    return cities[rand() % cities.size()];
}
	 
void QuestCitySack::armyDied(Army *a, bool heroIsCulprit)
{
  ;
  //this quest does nothing when an army dies
}

void QuestCitySack::cityAction(City *c, CityDefeatedAction action, 
			       bool heroIsCulprit, int gold)
{
  if (!isActive())
    return;
  if (action == CITY_DEFEATED_SACK && heroIsCulprit && c &&
      c->getId() == d_city)
    {
      debug("CONGRATULATIONS: QUEST 'CITY SACK' IS COMPLETED!");
      d_q_mgr.questCompleted(d_hero);
    }
}
