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

#include <stdlib.h>
#include <sstream>
#include <sigc++/functors/mem_fun.h>

#include "history.h"
#include "hero.h"
#include "city.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<< x << endl<<flush;}
//#define debug(x)

History::History(Type type)
:d_type(type)
{
}

History::~History()
{
}

History* History::handle_load(XML_Helper* helper)
{
  Uint32 t;
  helper->getData(t, "type");

  switch (t)
    {
    case START_TURN:
      return (new History_StartTurn(helper));
    case FOUND_SAGE:
      return (new History_FoundSage(helper));
    case GOLD_TOTAL:
      return (new History_GoldTotal(helper));
    case HERO_EMERGES:
      return (new History_HeroEmerges(helper));
    case CITY_WON:
      return (new History_CityWon(helper));
    case CITY_RAZED:
      return (new History_CityRazed(helper));
    case HERO_QUEST:
      return (new History_HeroQuest(helper));
    }

  return 0;
}

History* History::copy(const History* a)
{
  switch(a->getType())
    {
    case START_TURN:
      return 
	(new History_StartTurn(*dynamic_cast<const History_StartTurn*>(a)));
    case FOUND_SAGE:
      return 
	(new History_FoundSage(*dynamic_cast<const History_FoundSage*>(a)));
    case GOLD_TOTAL:
      return 
	(new History_GoldTotal(*dynamic_cast<const History_GoldTotal*>(a)));
    case HERO_EMERGES:
      return 
	(new History_HeroEmerges(*dynamic_cast<const History_HeroEmerges*>(a)));
    case CITY_WON:
      return 
	(new History_CityWon(*dynamic_cast<const History_CityWon*>(a)));
    case CITY_RAZED:
      return 
	(new History_CityRazed(*dynamic_cast<const History_CityRazed*>(a)));
    case HERO_QUEST:
      return 
	(new History_HeroQuest(*dynamic_cast<const History_HeroQuest*>(a)));
    }

  return 0;
}

//-----------------------------------------------------------------------------
//History_StartTurn

History_StartTurn::History_StartTurn()
:History(History::START_TURN)
{
}

History_StartTurn::History_StartTurn(XML_Helper* helper)
:History(History::START_TURN)
{
}

History_StartTurn::~History_StartTurn()
{
}

std::string History_StartTurn::dump() const
{
  std::stringstream s;

  s <<"player starts a turn" << "\n";

  return s.str();
}

bool History_StartTurn::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag("history");
  retval &= helper->saveData("type", d_type);
  retval &= helper->closeTag();

  return retval;
}

bool History_StartTurn::fillData()
{
  return true;
}

//-----------------------------------------------------------------------------
//History_FoundSage

History_FoundSage::History_FoundSage()
:History(History::FOUND_SAGE), d_hero(0)
{
}

History_FoundSage::History_FoundSage(XML_Helper* helper)
:History(History::FOUND_SAGE)
{
  helper->getData(d_hero, "hero");
}

History_FoundSage::~History_FoundSage()
{
}

std::string History_FoundSage::dump() const
{
  std::stringstream s;

  s <<"player found a sage with hero " << d_hero<< "\n";

  return s.str();
}

bool History_FoundSage::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag("history");
  retval &= helper->saveData("type", d_type);
  retval &= helper->saveData("hero", d_hero);
  retval &= helper->closeTag();

  return retval;
}

bool History_FoundSage::fillData(Hero *hero)
{
  d_hero = hero->getId();
  return true;
}

//-----------------------------------------------------------------------------
//History_GoldTotal

History_GoldTotal::History_GoldTotal()
:History(History::GOLD_TOTAL), d_gold(0)
{
}

History_GoldTotal::History_GoldTotal(XML_Helper* helper)
:History(History::GOLD_TOTAL)
{
  helper->getData(d_gold, "gold");
}

History_GoldTotal::~History_GoldTotal()
{
}

std::string History_GoldTotal::dump() const
{
  std::stringstream s;

  s <<"player has " << d_gold<< " gold pieces in total\n";

  return s.str();
}

bool History_GoldTotal::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag("history");
  retval &= helper->saveData("type", d_type);
  retval &= helper->saveData("gold", d_gold);
  retval &= helper->closeTag();

  return retval;
}

bool History_GoldTotal::fillData(int gold)
{
  d_gold = gold;
  return true;
}

//-----------------------------------------------------------------------------
//History_HeroEmerg

History_HeroEmerges::History_HeroEmerges()
:History(History::HERO_EMERGES), d_hero(0), d_city(0)
{
}

History_HeroEmerges::History_HeroEmerges(XML_Helper* helper)
:History(History::HERO_EMERGES)
{
  helper->getData(d_hero, "hero");
  helper->getData(d_city, "city");
}

History_HeroEmerges::~History_HeroEmerges()
{
}

std::string History_HeroEmerges::dump() const
{
  std::stringstream s;

  s <<"hero " << d_hero << " emerges in " << d_city << "\n";

  return s.str();
}

bool History_HeroEmerges::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag("history");
  retval &= helper->saveData("type", d_type);
  retval &= helper->saveData("hero", d_hero);
  retval &= helper->saveData("city", d_city);
  retval &= helper->closeTag();

  return retval;
}

bool History_HeroEmerges::fillData(Hero *hero, City *city)
{
  d_hero = hero->getId();
  d_city = city->getId();
  return true;
}

//-----------------------------------------------------------------------------
//History_CityWon

History_CityWon::History_CityWon()
:History(History::CITY_WON), d_city(0), d_hero(0)
{
}

History_CityWon::History_CityWon(XML_Helper* helper)
:History(History::CITY_WON)
{
  helper->getData(d_city, "city");
  helper->getData(d_hero, "hero");
}

History_CityWon::~History_CityWon()
{
}

std::string History_CityWon::dump() const
{
  std::stringstream s;

  s <<"city " << d_city << " has been won";
  if (d_hero)
    s <<"  by " << d_hero;
  s <<"\n";

  return s.str();
}

bool History_CityWon::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag("history");
  retval &= helper->saveData("type", d_type);
  retval &= helper->saveData("city", d_city);
  retval &= helper->saveData("hero", d_hero);
  retval &= helper->closeTag();

  return retval;
}

bool History_CityWon::fillData(City *city, Hero *hero)
{
  d_city = city->getId();
  if (hero)
    d_hero = hero->getId();
  return true;
}

//-----------------------------------------------------------------------------
//History_CityRazed

History_CityRazed::History_CityRazed()
:History(History::CITY_RAZED), d_city(0)
{
}

History_CityRazed::History_CityRazed(XML_Helper* helper)
:History(History::CITY_RAZED)
{
  helper->getData(d_city, "city");
}

History_CityRazed::~History_CityRazed()
{
}

std::string History_CityRazed::dump() const
{
  std::stringstream s;

  s <<"city " << d_city << " has been razed\n";

  return s.str();
}

bool History_CityRazed::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag("history");
  retval &= helper->saveData("type", d_type);
  retval &= helper->saveData("city", d_city);
  retval &= helper->closeTag();

  return retval;
}

bool History_CityRazed::fillData(City *city)
{
  d_city = city->getId();
  return true;
}

//-----------------------------------------------------------------------------
//History_HeroQuest

History_HeroQuest::History_HeroQuest()
:History(History::HERO_QUEST), d_hero(0)
{
}

History_HeroQuest::History_HeroQuest(XML_Helper* helper)
:History(History::HERO_QUEST)
{
  helper->getData(d_hero, "hero");
}

History_HeroQuest::~History_HeroQuest()
{
}

std::string History_HeroQuest::dump() const
{
  std::stringstream s;

  s <<"hero " << d_hero<< " gets a quest\n";

  return s.str();
}

bool History_HeroQuest::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag("history");
  retval &= helper->saveData("type", d_type);
  retval &= helper->saveData("hero", d_hero);
  retval &= helper->closeTag();

  return retval;
}

bool History_HeroQuest::fillData(Hero *hero)
{
  d_hero = hero->getId();
  return true;
}

