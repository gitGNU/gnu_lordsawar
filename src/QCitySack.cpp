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
    // we want to stay informed about city sacks
    const Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::const_iterator it = pl->begin(); it != pl->end(); it++)
        (*it)->ssackingCity.connect(
	    sigc::mem_fun(this, &QuestCitySack::citySacked));

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
    const Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::const_iterator it = pl->begin(); it != pl->end(); it++)
        (*it)->ssackingCity.connect(
	    sigc::mem_fun(this, &QuestCitySack::citySacked));
    
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
    return _("You aren't afraid of doing it, do you?");
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

    snprintf(buf, 100, _("You failed to sack the city \"%s\"."), c->getName().c_str());
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
void QuestCitySack::citySacked(City* city, Stack* s, int gold, std::list<Uint32> sacked_types)
{
    // some basic considerations have to be done
    if ((city->getId() != d_city) || !isActive())
        return;
    
    // look if our hero is in the list of (surviving) explorers
    for (Stack::const_iterator it = s->begin(); it != s->end(); it++)
        if ((*it)->isHero() && ((*it)->getId() == d_hero))
        {
            debug("CONGRATULATIONS: QUEST 'CITY SACK' IS COMPLETED!");
            d_q_mgr.questCompleted(d_hero);
            return;
        }

    // looks like we died trying to accomplish this quest
    debug("WHAT A PITY: QUEST 'CITY SACK' CANNOT BE COMPLETED!");
    d_q_mgr.questExpired(d_hero);
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
