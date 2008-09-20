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

#include <iostream>
#include <sstream>
#include <assert.h>
#include <sigc++/functors/mem_fun.h>

#include "QCityOccupy.h"
#include "QuestsManager.h"
#include "citylist.h"
#include "playerlist.h"
#include "stack.h"
#include "xmlhelper.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

//=======================================================================
QuestCityOccupy::QuestCityOccupy (QuestsManager& mgr, Uint32 hero) 
    : Quest(mgr, hero, Quest::CITYOCCUPY)
{
    // find us a victim
    City* c = chooseToOccupy(getHero()->getOwner());
    assert(c);      // should never fail because isFeasible is checked first

    d_city = c->getId();
    d_targets.push_back(c->getPos());
    debug("city_id = " << d_city);
    initDescription();
}
//=======================================================================
QuestCityOccupy::QuestCityOccupy (QuestsManager& q_mgr, XML_Helper* helper) 
     : Quest(q_mgr, helper)
{
    helper->getData(d_city, "city");
    d_targets.push_back(getCity()->getPos());
    initDescription();
}
//=======================================================================
QuestCityOccupy::QuestCityOccupy (QuestsManager& mgr, Uint32 hero, Uint32 target) 
    : Quest(mgr, hero, Quest::CITYOCCUPY)
{
    d_city = target;
    d_targets.push_back(getCity()->getPos());
    initDescription();
}
//=======================================================================
bool QuestCityOccupy::isFeasible(Uint32 heroId)
{
  if (QuestCityOccupy::chooseToOccupy(getHeroById(heroId)->getOwner()))
    return true;

  return false;
}
//=======================================================================
bool QuestCityOccupy::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(Quest::d_tag);
    retval &= Quest::save(helper);
    retval &= helper->saveData("city", d_city);
    retval &= helper->closeTag();

    return retval;
}
//=======================================================================
std::string QuestCityOccupy::getProgress() const
{
    return _("You aren't afraid of doing it, are you?");
}
//=======================================================================
void QuestCityOccupy::getSuccessMsg(std::queue<std::string>& msgs) const
{
    msgs.push(_("The priests thank you for occupying this evil place."));
}
//=======================================================================
void QuestCityOccupy::getExpiredMsg(std::queue<std::string>& msgs) const
{
    char buf[101]; buf[100] = '\0';
    const City* c = getCity();

    snprintf(buf, 100, _("The occupation of city \"%s\" could not be "
			 "accomplished."), 
	     c->getName().c_str());
    msgs.push(buf);
}
//=======================================================================
City* QuestCityOccupy::getCity() const
{
    Citylist* cl = Citylist::getInstance();
    for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
        if ((*it).getId() == d_city)
            return &(*it);

    return 0;
}
//=======================================================================
void QuestCityOccupy::initDescription()
{
  const City* c = getCity();
  char buffer[121]; buffer[120]='\0';

  snprintf(buffer, 100, _("You must take over the city \"%s\" and occupy it."),
	   c->getName().c_str());

  d_description = std::string(buffer);
}
//=======================================================================
City * QuestCityOccupy::chooseToOccupy(Player *p)
{
  std::vector<City*> cities;

  // Collect all cities
  Citylist* cl = Citylist::getInstance();
  for (Citylist::iterator it = cl->begin(); it != cl->end(); ++it)
    if (!(*it).isBurnt() && (*it).getOwner() != p &&
	(*it).getOwner() != Playerlist::getInstance()->getNeutral())
      cities.push_back(&(*it));

  // Find a suitable city for us to occupy
  if (cities.empty())
    return 0;

  return cities[rand() % cities.size()];
}
	 
void QuestCityOccupy::armyDied(Army *a, bool heroIsCulprit)
{
  ;
  //this quest does nothing when an army dies
}

void QuestCityOccupy::cityAction(City *c, CityDefeatedAction action, 
				 bool heroIsCulprit, int gold)
{
  if (!isActive())
    return;
  if (!c)
    return;
  if (c->getId() != d_city)
    return;
  //did our hero occupy the city? success.
  //did our hero do something else with the city?  expire.
  //did another of our stacks take the city?  expire.
  //did another player take the city? do nothing
  switch (action)
    {
    case CITY_DEFEATED_OCCUPY: //somebody occupied
      if (heroIsCulprit) //quest hero did
	d_q_mgr.questCompleted(d_hero);
      else if (c->getOwner() == getHero()->getOwner()) //our stack did
	d_q_mgr.questExpired(d_hero);
      break;
    case CITY_DEFEATED_RAZE: //somebody razed
      if (heroIsCulprit) // quest hero
	d_q_mgr.questExpired(d_hero);
      else if (c->getOwner() == getHero()->getOwner()) // our stack razed
	d_q_mgr.questExpired(d_hero);
      else // their stack did
	d_q_mgr.questExpired(d_hero);
      break;
    case CITY_DEFEATED_SACK: //somebody sacked
      if (heroIsCulprit) // quest hero did
	d_q_mgr.questExpired(d_hero);
      else if (c->getOwner() == getHero()->getOwner()) // our stack did
	d_q_mgr.questExpired(d_hero);
      else // their stack did
	d_q_mgr.questExpired(d_hero);
      break;
    case CITY_DEFEATED_PILLAGE: //somebody pillaged
      if (heroIsCulprit) // quest hero did
	d_q_mgr.questExpired(d_hero);
      else if (c->getOwner() == getHero()->getOwner()) // our stack did
	d_q_mgr.questExpired(d_hero);
      break;
    }
}
