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

#include "playerlist.h"

#include "citylist.h"
#include "defs.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

Playerlist* Playerlist::s_instance = 0;
Player* Playerlist::d_activeplayer = 0;
bool Playerlist::s_finish = false;

Playerlist* Playerlist::getInstance()
{
    if (s_instance == 0)
        s_instance = new Playerlist();

    return s_instance;
}

Playerlist* Playerlist::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new Playerlist(helper);

    return s_instance;
}

void Playerlist::deleteInstance()
{
    if (s_instance)
    {
        delete s_instance;
        s_instance = 0;
    }
}

Playerlist::Playerlist()
    :d_neutral(0)
{
    s_finish = false;
    d_activeplayer = 0;
}

Playerlist::Playerlist(XML_Helper* helper)
    :d_neutral(0)
{
    d_activeplayer=0;
    //load data. This consists currently of two values: size (for checking
    //the size; we don't use it yet) and active(which player is active)
    //we do it by calling load with playerlist as string
    load("playerlist", helper);
    s_finish = false;

    helper->registerTag("player", sigc::mem_fun(this, &Playerlist::load));
}

Playerlist::~Playerlist()
{
    iterator it = begin();
    while (!empty())
        it = flErase(it);
}

void Playerlist::checkPlayers()
{
    debug("checkPlayers()");
    iterator it = begin ();
    while (it != end ())
    {
        debug("checkPlayers() iter");
        //ignore the neutral player as well as dead and immortal ones
        if (((*it) == d_neutral) || ((*it)->isDead()) || ((*it)->isImmortal()))
        {
            debug("checkPlayers() dead?");
            it++;
            continue;
        }

        if (!Citylist::getInstance()->countCities((*it)))
        {
            debug("checkPlayers() city?");
            iterator nextit = it;
            nextit++;

            (*it)->kill();
            splayerDead.emit(*it);

            it = nextit;    // do this at the end to catch abuse of invalid it
        } else {
            debug("checkPlayers() inc");
            ++it;
        }
    }
}

void Playerlist::nextPlayer()
{
    debug("nextPlayer()");

    iterator it;

    if (!d_activeplayer)
    {
        it = begin();
    }
    else
    {
        for (it = begin(); it != end(); ++it)
        {
            if ((*it) == d_activeplayer)
            {
                it++;
                break;
            }
        }
    }

    // in case we have got a dead player, continue iterating. This breaks
    // if we have ONLY dead players which we assume never happens.
    while ((it == end()) || ((*it)->isDead()))
    {
        if (it == end())
        {
            it = begin();
            continue;
        }
        it++;
    }

    d_activeplayer = (*it);
    debug("got player: " <<d_activeplayer->getName())
}

Player* Playerlist::getPlayer(string name) const
{
    debug("getPlayer()");
    for (const_iterator it = begin(); it != end(); ++it)
    {
        if ((*it)->getName() == name) return (*it);
    }
    return 0;
}

Player* Playerlist::getPlayer(Uint32 id) const
{
    debug("getPlayer()");
    for (const_iterator it = begin(); it != end(); it++)
    {
        if (id == (*it)->getId())
            return (*it);
    }

    return 0;
}

int Playerlist::getNoOfPlayers() const
{
    int number = 0;

    for (const_iterator it = begin(); it != end(); it++)
    {
        if (((*it) != d_neutral) && !(*it)->isDead())
            number++;
    }

    return number;
}

Player* Playerlist::getFirstLiving() const
{
    for (const_iterator it = begin(); ; it++)
        if (!(*it)->isDead())
            return (*it);
}

bool Playerlist::save(XML_Helper* helper) const
{
    //to prevent segfaults
    if (!d_activeplayer)
        d_activeplayer = (*begin());

    bool retval = true;

    retval &= helper->openTag("playerlist");
    retval &= helper->saveData("active", d_activeplayer->getId());
    retval &= helper->saveData("neutral", d_neutral->getId());

    for (const_iterator it = begin(); it != end(); it++)
        retval &= (*it)->save(helper);

    retval &= helper->closeTag();

    return retval;
}

bool Playerlist::load(string tag, XML_Helper* helper)
{
    static Uint32 active = 0;
    static Uint32 neutral = 0;

    if (tag == "playerlist") //only called in the constructor
    {
        helper->getData(active, "active");
        helper->getData(neutral, "neutral");
        return true;
    }

    if (tag != "player")
        return false;

    //else tag == "player"
    Player* p = Player::loadPlayer(helper);
    if(p == 0)
        return false;

    //insert player...
    push_back(p);

    //set neutral and active
    if (p->getId() == neutral)
        d_neutral = p;
    if (p->getId() == active)
        d_activeplayer = p;

    return true;
}

Playerlist::iterator Playerlist::flErase(Playerlist::iterator it)
{
    if ((*it) == d_neutral)
        d_neutral = 0;

    delete (*it);
    return erase (it);
}
