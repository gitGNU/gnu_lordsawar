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
    case HERO_CITY_WON:
      return (new History_HeroCityWon(helper));
    case CITY_RAZED:
      return (new History_CityRazed(helper));
    case HERO_QUEST_STARTED:
      return (new History_HeroQuestStarted(helper));
    case HERO_QUEST_COMPLETED:
      return (new History_HeroQuestCompleted(helper));
    case HERO_KILLED_IN_CITY:
      return (new History_HeroKilledInCity(helper));
    case HERO_KILLED_IN_BATTLE:
      return (new History_HeroKilledInBattle(helper));
    case HERO_KILLED_SEARCHING:
      return (new History_HeroKilledSearching(helper));
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
    case HERO_CITY_WON:
      return 
	(new History_HeroCityWon(*dynamic_cast<const History_HeroCityWon*>(a)));
    case CITY_RAZED:
      return 
	(new History_CityRazed(*dynamic_cast<const History_CityRazed*>(a)));
    case HERO_QUEST_STARTED:
      return 
	(new History_HeroQuestStarted
          (*dynamic_cast<const History_HeroQuestStarted*>(a)));
    case HERO_QUEST_COMPLETED:
      return 
	(new History_HeroQuestCompleted
          (*dynamic_cast<const History_HeroQuestCompleted*>(a)));
    case HERO_KILLED_IN_CITY:
      return 
	(new History_HeroKilledInCity
          (*dynamic_cast<const History_HeroKilledInCity*>(a)));
    case HERO_KILLED_IN_BATTLE:
      return 
	(new History_HeroKilledInBattle
          (*dynamic_cast<const History_HeroKilledInBattle*>(a)));
    case HERO_KILLED_SEARCHING:
      return 
	(new History_HeroKilledSearching
          (*dynamic_cast<const History_HeroKilledSearching*>(a)));
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
  d_hero = hero->getName();
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
//History_HeroEmerges

History_HeroEmerges::History_HeroEmerges()
:History(History::HERO_EMERGES), d_hero(""), d_city("")
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
  d_hero = hero->getName();
  d_city = city->getName();
  return true;
}

//-----------------------------------------------------------------------------
//History_CityWon

History_CityWon::History_CityWon()
:History(History::CITY_WON), d_city(0)
{
}

History_CityWon::History_CityWon(XML_Helper* helper)
:History(History::CITY_WON)
{
  helper->getData(d_city, "city");
}

History_CityWon::~History_CityWon()
{
}

std::string History_CityWon::dump() const
{
  std::stringstream s;

  s <<"city " << d_city << " has been won";
  s <<"\n";

  return s.str();
}

bool History_CityWon::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag("history");
  retval &= helper->saveData("type", d_type);
  retval &= helper->saveData("city", d_city);
  retval &= helper->closeTag();

  return retval;
}

bool History_CityWon::fillData(City *city)
{
  d_city = city->getId();
  return true;
}

//-----------------------------------------------------------------------------
//History_HeroCityWon

History_HeroCityWon::History_HeroCityWon()
:History(History::HERO_CITY_WON), d_city(""), d_hero("")
{
}

History_HeroCityWon::History_HeroCityWon(XML_Helper* helper)
:History(History::HERO_CITY_WON)
{
  helper->getData(d_city, "city");
  helper->getData(d_hero, "hero");
}

History_HeroCityWon::~History_HeroCityWon()
{
}

std::string History_HeroCityWon::dump() const
{
  std::stringstream s;

  s <<"city " << d_city << " has been won";
  s <<"  by " << d_hero;
  s <<"\n";

  return s.str();
}

bool History_HeroCityWon::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag("history");
  retval &= helper->saveData("type", d_type);
  retval &= helper->saveData("city", d_city);
  retval &= helper->saveData("hero", d_hero);
  retval &= helper->closeTag();

  return retval;
}

bool History_HeroCityWon::fillData(Hero *hero, City *city)
{
  d_city = city->getName();
  d_hero = hero->getName();
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
//History_HeroQuestStarted

History_HeroQuestStarted::History_HeroQuestStarted()
:History(History::HERO_QUEST_STARTED), d_hero("")
{
}

History_HeroQuestStarted::History_HeroQuestStarted(XML_Helper* helper)
:History(History::HERO_QUEST_STARTED)
{
  helper->getData(d_hero, "hero");
}

History_HeroQuestStarted::~History_HeroQuestStarted()
{
}

std::string History_HeroQuestStarted::dump() const
{
  std::stringstream s;

  s <<"hero " << d_hero<< " gets a quest\n";

  return s.str();
}

bool History_HeroQuestStarted::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag("history");
  retval &= helper->saveData("type", d_type);
  retval &= helper->saveData("hero", d_hero);
  retval &= helper->closeTag();

  return retval;
}

bool History_HeroQuestStarted::fillData(Hero *hero)
{
  d_hero = hero->getName();
  return true;
}

//-----------------------------------------------------------------------------
//History_HeroQuestCompleted

History_HeroQuestCompleted::History_HeroQuestCompleted()
:History(History::HERO_QUEST_COMPLETED), d_hero("")
{
}

History_HeroQuestCompleted::History_HeroQuestCompleted(XML_Helper* helper)
:History(History::HERO_QUEST_COMPLETED)
{
  helper->getData(d_hero, "hero");
}

History_HeroQuestCompleted::~History_HeroQuestCompleted()
{
}

std::string History_HeroQuestCompleted::dump() const
{
  std::stringstream s;

  s <<"hero " << d_hero<< " gets a quest\n";

  return s.str();
}

bool History_HeroQuestCompleted::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag("history");
  retval &= helper->saveData("type", d_type);
  retval &= helper->saveData("hero", d_hero);
  retval &= helper->closeTag();

  return retval;
}

bool History_HeroQuestCompleted::fillData(Hero *hero)
{
  d_hero = hero->getName();
  return true;
}

//-----------------------------------------------------------------------------
//History_HeroKilledInCity

History_HeroKilledInCity::History_HeroKilledInCity()
:History(History::HERO_KILLED_IN_CITY), d_hero(""), d_city("")
{
}

History_HeroKilledInCity::History_HeroKilledInCity(XML_Helper* helper)
:History(History::HERO_KILLED_IN_CITY)
{
  helper->getData(d_hero, "hero");
  helper->getData(d_city, "city");
}

History_HeroKilledInCity::~History_HeroKilledInCity()
{
}

std::string History_HeroKilledInCity::dump() const
{
  std::stringstream s;

  s <<"hero " << d_hero << " died in city " << d_city << "\n";

  return s.str();
}

bool History_HeroKilledInCity::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag("history");
  retval &= helper->saveData("type", d_type);
  retval &= helper->saveData("hero", d_hero);
  retval &= helper->saveData("city", d_city);
  retval &= helper->closeTag();

  return retval;
}

bool History_HeroKilledInCity::fillData(Hero *hero, City *city)
{
  d_hero = hero->getName();
  d_city = city->getName();
  return true;
}

//-----------------------------------------------------------------------------
//History_HeroKilledInBattle

History_HeroKilledInBattle::History_HeroKilledInBattle()
:History(History::HERO_KILLED_IN_BATTLE), d_hero("")
{
}

History_HeroKilledInBattle::History_HeroKilledInBattle(XML_Helper* helper)
:History(History::HERO_KILLED_IN_BATTLE)
{
  helper->getData(d_hero, "hero");
}

History_HeroKilledInBattle::~History_HeroKilledInBattle()
{
}

std::string History_HeroKilledInBattle::dump() const
{
  std::stringstream s;

  s <<"hero " << d_hero<< " was killed in battle\n";

  return s.str();
}

bool History_HeroKilledInBattle::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag("history");
  retval &= helper->saveData("type", d_type);
  retval &= helper->saveData("hero", d_hero);
  retval &= helper->closeTag();

  return retval;
}

bool History_HeroKilledInBattle::fillData(Hero *hero)
{
  d_hero = hero->getName();
  return true;
}

//-----------------------------------------------------------------------------
//History_Hero KilledSearching

History_HeroKilledSearching::History_HeroKilledSearching()
:History(History::HERO_KILLED_SEARCHING), d_hero("")
{
}

History_HeroKilledSearching::History_HeroKilledSearching(XML_Helper* helper)
:History(History::HERO_KILLED_SEARCHING)
{
  helper->getData(d_hero, "hero");
}

History_HeroKilledSearching::~History_HeroKilledSearching()
{
}

std::string History_HeroKilledSearching::dump() const
{
  std::stringstream s;

  s <<"hero " << d_hero<< " killed searching a ruin\n";

  return s.str();
}

bool History_HeroKilledSearching::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag("history");
  retval &= helper->saveData("type", d_type);
  retval &= helper->saveData("hero", d_hero);
  retval &= helper->closeTag();

  return retval;
}

bool History_HeroKilledSearching::fillData(Hero *hero)
{
  d_hero = hero->getName();
  return true;
}

