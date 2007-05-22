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

#include "QRuinSearch.h"
#include "QuestsManager.h"
#include "ruinlist.h"
#include "citylist.h"
#include "playerlist.h"
#include "stack.h"
#include "defs.h"
#include "xmlhelper.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

//=======================================================================
QuestRuinSearch::QuestRuinSearch(QuestsManager& mgr, Uint32 hero) 
    : Quest(mgr, hero, Quest::RUINSEARCH)
{
    // we want to stay informed about ruin searches
    const Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::const_iterator it = pl->begin(); it != pl->end(); it++)
        (*it)->ssearchingRuin.connect(
	    sigc::mem_fun(this, &QuestRuinSearch::ruinSearched));

    // find us a ruin
    Ruin* r = chooseToSearch();
    assert(r);      // should never fail because isFeasible is checked first

    d_ruin = r->getId();
    debug("ruin_id = " << d_ruin);
    initDescription();
}
//=======================================================================
QuestRuinSearch::QuestRuinSearch(QuestsManager& q_mgr, XML_Helper* helper) 
     : Quest(q_mgr, helper)
{
    // let us stay in touch with the world...
    const Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::const_iterator it = pl->begin(); it != pl->end(); it++)
        (*it)->ssearchingRuin.connect(
	    sigc::mem_fun(this, &QuestRuinSearch::ruinSearched));
    
    helper->getData(d_ruin, "ruin");
    initDescription();
}
//=======================================================================
bool QuestRuinSearch::isFeasible(Uint32 heroId)
{
    return (chooseToSearch() != 0);
}
//=======================================================================
bool QuestRuinSearch::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("quest");
    retval &= Quest::save(helper);
    retval &= helper->saveData("ruin", d_ruin);
    retval &= helper->closeTag();

    return retval;
}
//=======================================================================
std::string QuestRuinSearch::getProgress() const
{
    return _("You aren't afraid of doing it, do you?");
}
//=======================================================================
void QuestRuinSearch::getSuccessMsg(std::queue<std::string>& msgs) const
{
    msgs.push(_("The priests thank you for exploring this gloomy place."));
    msgs.push(_("Now they can set a temple here."));
}
//=======================================================================
void QuestRuinSearch::getExpiredMsg(std::queue<std::string>& msgs) const
{
    char buf[101]; buf[100] = '\0';
    const Ruin* r = getRuin();

    snprintf(buf, 100, _("You failed searching the ruin \"%s\"."), r->getName().c_str());
    msgs.push(buf);
    msgs.push(_("Others had more courage!"));
}
//=======================================================================
Ruin* QuestRuinSearch::getRuin() const
{
    Ruinlist* rl = Ruinlist::getInstance();
    for (Ruinlist::iterator it = rl->begin(); it != rl->end(); it++)
        if ((*it).getId() == d_ruin)
            return &(*it);

    return 0;
}
//=======================================================================
void QuestRuinSearch::ruinSearched(Ruin* ruin, Stack* s)
{
    // some basic considerations have to be done
    if ((ruin->getId() != d_ruin) || !isActive())
        return;
    
    // look if our hero is in the list of (surviving) explorers
    for (Stack::const_iterator it = s->begin(); it != s->end(); it++)
        if ((*it)->isHero() && ((*it)->getId() == d_hero))
        {
            debug("CONGRATULATIONS: QUEST 'RUIN SEARCH' IS COMPLETED!");
            d_q_mgr.questCompleted(d_hero);
            return;
        }

    // another hero succeeded in searching the ruin
    debug("WHAT A PITY: QUEST 'RUIN SEARCH' IS COMPLETED BUT BY ANOTHER HERO OF YOU!");
    d_q_mgr.questExpired(d_hero);
}
//=======================================================================
void QuestRuinSearch::initDescription()
{
    const Ruin* r = getRuin();
    char buffer[121]; buffer[120]='\0';
    
    std::string s = Citylist::getInstance()->getNearestCity(r->getPos())->getName();
    snprintf(buffer, 100, _("You must search the ruin \"%s\" located at (%i,%i) near the city of %s"),
            r->getName().c_str(), r->getPos().x, r->getPos().y, s.c_str());

    d_description = std::string(buffer);
}
//=======================================================================
Ruin* QuestRuinSearch::chooseToSearch()
{
    std::vector<Ruin*> ruins;

    // Collect all ruins
    Ruinlist* rl = Ruinlist::getInstance();
    for (Ruinlist::iterator it = rl->begin(); it != rl->end(); ++it)
        if (!(*it).isSearched())
            ruins.push_back(&(*it));

    // Find a suitable ruin for us
    if (ruins.empty())
        return 0;

    return ruins[rand() % ruins.size()];
}
