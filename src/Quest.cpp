// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004 Andrea Paternesi
// Copyright (C) 2007, 2008, 2009 Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
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

#include "Quest.h"
#include "QuestsManager.h"
#include "hero.h"
#include "playerlist.h"
#include "stacklist.h"
#include "history.h"

std::string Quest::d_tag = "quest";
using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)


Quest::Quest(QuestsManager& q_mgr, guint32 hero, Type type)
    :d_q_mgr(q_mgr), d_hero(hero), d_type(type), d_pending(false)
{
  Hero *h = getHeroById(hero);
  d_player_id = h->getOwner()->getId();
  d_hero_name = h->getName();
}

Quest::Quest(QuestsManager& q_mgr, XML_Helper* helper)
    :d_q_mgr(q_mgr)
{
    std::string s;
    helper->getData(s, "type");
    d_type = questTypeFromString(s);
    helper->getData(d_hero, "hero");
    helper->getData(d_hero_name, "hero_name");
    helper->getData(d_pending, "pending_deletion");
    helper->getData(d_player_id, "player");
}

Hero* Quest::getHeroById(guint32 hero, Stack** stack)
{
    Playerlist* pl = Playerlist::getInstance();
	Playerlist::const_iterator pit;
    for (pit = pl->begin(); pit != pl->end(); pit++)
    {
        Stacklist* sl = (*pit)->getStacklist();
        for (Stacklist::const_iterator it = sl->begin(); it != sl->end(); it++)
        {
            for (Stack::const_iterator sit = (*it)->begin(); sit != (*it)->end(); sit++)
            {
                if ( ((*sit)->isHero()) && ((*sit)->getId() == hero) )
                {
                    if (stack)
                        *stack = (*it);
                    // isHero is TRUE, so dynamic_cast should succeed
                    return dynamic_cast<Hero*>(*sit);
                }
            }
        }
    }
    return NULL;
}

bool Quest::save(XML_Helper* helper) const
{
    bool retval = true;

    std::string s;
    s = questTypeToString(Quest::Type(d_type));
    retval &= helper->saveData("type", s);
    retval &= helper->saveData("hero", d_hero);
    retval &= helper->saveData("hero_name", d_hero_name);
    retval &= helper->saveData("pending_deletion", d_pending);
    retval &= helper->saveData("player", d_player_id);

    return retval;
}

std::string Quest::getHeroNameForDeadHero() const
{
  return getHeroNameForDeadHero(d_hero);
}

std::string Quest::getHeroNameForDeadHero(guint32 id)
{
  std::list<History *>events;
  events = Playerlist::getInstance()->getHistoryForHeroId(id);
  if (events.size() == 0)
    return "";
  History *history = events.front();
  History_HeroEmerges *event = dynamic_cast<History_HeroEmerges*>(history);
  return event->getHeroName();
}

std::string Quest::questTypeToString(const Quest::Type type)
{
  switch (type)
    {
    case Quest::KILLHERO:
      return "Quest::KILLHERO";
    case Quest::KILLARMIES:
      return "Quest::KILLARMIES";
    case Quest::CITYSACK:
      return "Quest::CITYSACK";
    case Quest::CITYRAZE:
      return "Quest::CITYRAZE";
    case Quest::CITYOCCUPY:
      return "Quest::CITYOCCUPY";
    case Quest::KILLARMYTYPE:
      return "Quest::KILLARMYTYPE";
    case Quest::PILLAGEGOLD:
      return "Quest::PILLAGEGOLD";
    }
  return "Quest::KILLHERO";
}

Quest::Type Quest::questTypeFromString(std::string str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Quest::Type(atoi(str.c_str()));
  if (str == "Quest::KILLHERO")
    return Quest::KILLHERO;
  else if (str == "Quest::KILLARMIES")
    return Quest::KILLARMIES;
  else if (str == "Quest::CITYSACK")
    return Quest::CITYSACK;
  else if (str == "Quest::CITYRAZE")
    return Quest::CITYRAZE;
  else if (str == "Quest::CITYOCCUPY")
    return Quest::CITYOCCUPY;
  else if (str == "Quest::KILLARMYTYPE")
    return Quest::KILLARMYTYPE;
  else if (str == "Quest::PILLAGEGOLD")
    return Quest::PILLAGEGOLD;
    
  return Quest::KILLHERO;
}
