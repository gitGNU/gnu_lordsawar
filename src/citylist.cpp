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

#include "citylist.h"
#include "city.h"
#include "playerlist.h"
#include "xmlhelper.h"

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

Citylist* Citylist::s_instance = 0;


Citylist* Citylist::getInstance()
{
    if (s_instance == 0)
        s_instance = new Citylist();

    return s_instance;
}

Citylist* Citylist::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new Citylist(helper);
    return s_instance;
}

void Citylist::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

Citylist::Citylist()
{
}

Citylist::Citylist(XML_Helper* helper)
{
    // simply ask the helper to inform us when a city tag is opened
    helper->registerTag("city", sigc::mem_fun(this, &Citylist::load));
    helper->registerTag("army", sigc::mem_fun(this, &Citylist::load));
}

Citylist::~Citylist()
{
}

int Citylist::countCities(Player* player) const
{
    int cities = 0;
    
    for (const_iterator it = begin(); it != end(); it++)
    {
        if ((*it).isBurnt())
          continue;
        if ((*it).getPlayer() == player) cities++;
    }
    
    return cities;
}

void Citylist::nextTurn(Player* p)
{
    debug("next_turn(" <<p->getName() <<")");

    // This iteration adds the city production to the player    
    for (iterator it = begin(); it != end(); it++)
    {
        if ((*it).getPlayer() == p)
            (*it).nextTurn();
    }

    // In a second round, add the gold. The idea is that a player cannot
    // produce units if his gold is <0. However, to be consistent, we may not
    // add the gold in the first run, because then the player may come above 0
    // gold, which means some cities produce units, others not.
    for (const_iterator it = begin(); it != end(); it++)
        if ((*it).getPlayer() == p)
            p->addGold((*it).getGold());
}

City* Citylist::getNearestEnemyCity(const Vector<int>& pos)
{
    int diff = -1;
    iterator diffit;
    Player* p = Playerlist::getInstance()->getActiveplayer();
    
    for (iterator it = begin(); it != end(); ++it)
    {
        if ((*it).isBurnt())
            continue;

        if ((*it).getPlayer() != p &&
	    p->getDiplomaticState((*it).getPlayer()) == Player::AT_WAR)
        {        
            Vector<int> p = (*it).getPos();
            int delta = abs(p.x - pos.x);
            if (delta < abs(p.y - pos.y))
                delta = abs(p.y - pos.y);
            if ((diff > delta) || (diff == -1))
            {
                diff = delta;
                diffit = it;
            }
        }
    }

    if (diff == -1) return 0;
    return &(*diffit);
}

City* Citylist::getNearestForeignCity(const Vector<int>& pos)
{
    int diff = -1;
    iterator diffit;
    Player* p = Playerlist::getInstance()->getActiveplayer();
    
    for (iterator it = begin(); it != end(); ++it)
    {
        if ((*it).isBurnt())
            continue;

        if ((*it).getPlayer() != p)
        {        
            Vector<int> p = (*it).getPos();
            int delta = abs(p.x - pos.x);
            if (delta < abs(p.y - pos.y))
                delta = abs(p.y - pos.y);
            if ((diff > delta) || (diff == -1))
            {
                diff = delta;
                diffit = it;
            }
        }
    }

    if (diff == -1) return 0;
    return &(*diffit);
}

City* Citylist::getNearestCity(const Vector<int>& pos, int dist)
{
  City *c = getNearestCity(pos);
  if (c->getPos().x <= pos.x + dist && c->getPos().x >= pos.x - dist &&
      c->getPos().y <= pos.y + dist && c->getPos().y >= pos.y - dist)
    return c;
  return NULL;
}

City* Citylist::getNearestFriendlyCity(const Vector<int>& pos, int dist)
{
  City *c = getNearestFriendlyCity(pos);
  if (c->getPos().x <= pos.x + dist && c->getPos().x >= pos.x - dist &&
      c->getPos().y <= pos.y + dist && c->getPos().y >= pos.y - dist)
    return c;
  return NULL;
}

City* Citylist::getNearestFriendlyCity(const Vector<int>& pos)
{
    int diff = -1;
    iterator diffit;
    Player* p = Playerlist::getInstance()->getActiveplayer();
    
    for (iterator it = begin(); it != end(); ++it)
    {
        if ((*it).isBurnt())
            continue;

        if ((*it).getPlayer() == p)
        {
            Vector<int> p = (*it).getPos();
            int delta = abs(p.x - pos.x);
            if (delta < abs(p.y - pos.y))
                delta = abs(p.y - pos.y);
            
            if ((diff > delta) || (diff == -1))
            {
                diff = delta;
                diffit = it;
            }
        }
    }
    
    if (diff == -1) return 0;
    return &(*diffit);
}

City* Citylist::getNearestCity(const Vector<int>& pos)
{
    int diff = -1;
    iterator diffit;

    for (iterator it = begin(); it != end(); ++it)
    {
          if ((*it).isBurnt())
              continue;
          
          Vector<int> p = (*it).getPos();
          int delta = abs(p.x - pos.x);
          if (delta < abs(p.y - pos.y))
              delta = abs(p.y - pos.y);
          
          if ((diff > delta) || (diff == -1))
          {
              diff = delta;
              diffit = it;
          }
    }
    
    if (diff == -1) return 0;
    return &(*diffit);
}

City* Citylist::getNearestVisibleCity(const Vector<int>& pos)
{
    int diff = -1;
    iterator diffit;

    for (iterator it = begin(); it != end(); ++it)
    {
          if ((*it).isBurnt())
              continue;

          if ((*it).isFogged() == true)
              continue;
          
          Vector<int> p = (*it).getPos();
          int delta = abs(p.x - pos.x);
          if (delta < abs(p.y - pos.y))
              delta = abs(p.y - pos.y);
          
          if ((diff > delta) || (diff == -1))
          {
              diff = delta;
              diffit = it;
          }
    }
    
    if (diff == -1) return 0;
    return &(*diffit);
}

City* Citylist::getNearestVisibleCity(const Vector<int>& pos, int dist)
{
  City *c = getNearestVisibleCity(pos);
  if (c->getPos().x <= pos.x + dist && c->getPos().x >= pos.x - dist &&
      c->getPos().y <= pos.y + dist && c->getPos().y >= pos.y - dist)
    return c;
  return NULL;
}

City* Citylist::getNearestVisibleFriendlyCity(const Vector<int>& pos, int dist)
{
  City *c = getNearestFriendlyCity(pos);
  if (c->getPos().x <= pos.x + dist && c->getPos().x >= pos.x - dist &&
      c->getPos().y <= pos.y + dist && c->getPos().y >= pos.y - dist)
    return c;
  return NULL;
}

City* Citylist::getNearestVisibleFriendlyCity(const Vector<int>& pos)
{
    int diff = -1;
    iterator diffit;
    Player* p = Playerlist::getInstance()->getActiveplayer();
    
    for (iterator it = begin(); it != end(); ++it)
    {
        if ((*it).isBurnt())
            continue;

	if ((*it).isFogged())
	    continue;

        if ((*it).getPlayer() == p)
        {
            Vector<int> p = (*it).getPos();
            int delta = abs(p.x - pos.x);
            if (delta < abs(p.y - pos.y))
                delta = abs(p.y - pos.y);
            
            if ((diff > delta) || (diff == -1))
            {
                diff = delta;
                diffit = it;
            }
        }
    }
    
    if (diff == -1) return 0;
    return &(*diffit);
}

City* Citylist::getNearestNeutralCity(const Vector<int>& pos)
{
    int diff = -1;
    iterator diffit;
    
    for (iterator it = begin(); it != end(); ++it)
    {
        if ((*it).isBurnt())
            continue;

        if ((*it).getPlayer() == Playerlist::getInstance()->getNeutral())
        {
            Vector<int> p = (*it).getPos();
            int delta = abs(p.x - pos.x);
            if (delta < abs(p.y - pos.y))
                delta = abs(p.y - pos.y);
            
            if ((diff > delta) || (diff == -1))
            {
                diff = delta;
                diffit = it;
            }
        }
    }
    
    if (diff == -1) return 0;
    return &(*diffit);
}


City* Citylist::getFirstCity(Player* p)
{
    for (iterator it = begin(); it != end(); it++)
        if ((*it).getPlayer() == p)
            return &(*it);

    return 0;
}

bool Citylist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("citylist");

    for (const_iterator it = begin(); it != end(); it++)
        (*it).save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

bool Citylist::load(std::string tag, XML_Helper* helper)
{
    if (tag == "army")
      {
	//add it to the right city
	Citylist::iterator it = end();
	it--;
	City *city = &*it;
	Army *a = new Army (helper, Army::PRODUCTION_BASE);
	city->addBasicProd(-1, a);
	return true;
      }
    if (tag == "city")
      {
	City c(helper);
	push_back(c);
	return true;
      }
    return false;
}

void Citylist::changeOwnership(Player *old_owner, Player *new_owner)
{
  for (iterator it = begin(); it != end(); it++)
    if ((*it).getPlayer() == old_owner)
      {
	(*it).setPlayer(new_owner);
	if ((*it).isCapital())
	  if ((*it).getCapitalOwner() == old_owner)
	    (*it).setCapitalOwner(new_owner);
      }
}

// End of file
