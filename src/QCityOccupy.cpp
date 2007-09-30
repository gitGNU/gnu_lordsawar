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

#include "QCityOccupy.h"
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
QuestCityOccupy::QuestCityOccupy (QuestsManager& mgr, Uint32 hero) 
    : Quest(mgr, hero, Quest::CITYOCCUPY)
{
    // we want to stay informed about city occupations
    const Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::const_iterator it = pl->begin(); it != pl->end(); it++)
      {
        (*it)->soccupyingCity.connect(
	    sigc::mem_fun(this, &QuestCityOccupy::cityOccupied));
        (*it)->srazingCity.connect(
	    sigc::mem_fun(this, &QuestCityOccupy::cityOccupied));
      }

    // find us a victim
    City* c = chooseToOccupy(getHero()->getPlayer());
    assert(c);      // should never fail because isFeasible is checked first

    d_city = c->getId();
    d_targets.push_back(c->getPos());
    debug("city_id = " << d_ruin);
    initDescription();
}
//=======================================================================
QuestCityOccupy::QuestCityOccupy (QuestsManager& q_mgr, XML_Helper* helper) 
     : Quest(q_mgr, helper)
{
    // let us stay in touch with the world...
    const Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::const_iterator it = pl->begin(); it != pl->end(); it++)
      {
        (*it)->soccupyingCity.connect(
	    sigc::mem_fun(this, &QuestCityOccupy::cityOccupied));
        (*it)->srazingCity.connect(
	    sigc::mem_fun(this, &QuestCityOccupy::cityOccupied));
      }
    
    helper->getData(d_city, "city");
    d_targets.push_back(getCity()->getPos());
    initDescription();
}
//=======================================================================
bool QuestCityOccupy::isFeasible(Uint32 heroId)
{
    return true;
}
//=======================================================================
bool QuestCityOccupy::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("quest");
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

    snprintf(buf, 100, _("You failed to occupy the city \"%s\"."), c->getName().c_str());
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
void QuestCityOccupy::cityOccupied(City* city, Stack* s)
{
    // some basic considerations have to be done
    if ((city->getId() != d_city) || !isActive())
        return;
    
    //somebody's stack has occupied this city or has razed it
    if (city->isBurnt() == false)
      {
	// look if our hero is in the list of occupiers
	for (Stack::const_iterator it = s->begin(); it != s->end(); it++)
	  if ((*it)->isHero() && ((*it)->getId() == d_hero))
	    {
	      debug("CONGRATULATIONS: QUEST 'CITY OCCUPY' IS COMPLETED!");
	      d_q_mgr.questCompleted(d_hero);
	      return;
	    }
      }
    else
      {
	// looks like we razed this city, or someone else did
	debug("WHAT A PITY: QUEST 'CITY OCCUPY' CANNOT BE COMPLETED!");
	d_q_mgr.questExpired(d_hero);
      }
}
//=======================================================================
void QuestCityOccupy::initDescription()
{
  const City* c = getCity();
  char buffer[121]; buffer[120]='\0';

  snprintf(buffer, 100, _("You must occupy the city \"%s\" located at (%i,%i)."),
	   c->getName().c_str(), c->getPos().x, c->getPos().y);

  d_description = std::string(buffer);
}
//=======================================================================
City * QuestCityOccupy::chooseToOccupy(Player *p)
{
  std::vector<City*> cities;

  // Collect all cities
  Citylist* cl = Citylist::getInstance();
  for (Citylist::iterator it = cl->begin(); it != cl->end(); ++it)
    if (!(*it).isBurnt() && (*it).getPlayer() != p)
      cities.push_back(&(*it));

  // Find a suitable city for us to occupy
  if (cities.empty())
    return 0;

  return cities[rand() % cities.size()];
}
