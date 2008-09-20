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

#include <stdlib.h>
#include <sstream>
#include <sigc++/functors/mem_fun.h>

#include "history.h"
#include "hero.h"
#include "heroproto.h"
#include "city.h"

std::string History::d_tag = "history";
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
  std::string type_str;
  helper->getData(type_str, "type");
  History::Type t = historyTypeFromString(type_str);

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
    case SCORE:
      return (new History_Score(helper));
    case PLAYER_VANQUISHED:
      return (new History_PlayerVanquished(helper));
    case DIPLOMATIC_PEACE:
      return (new History_DiplomacyPeace(helper));
    case DIPLOMATIC_WAR:
      return (new History_DiplomacyWar(helper));
    case DIPLOMATIC_TREACHERY:
      return (new History_DiplomacyTreachery(helper));
    case HERO_FINDS_ALLIES:
      return (new History_HeroFindsAllies(helper));
    case END_TURN:
      return (new History_EndTurn(helper));
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
    case SCORE:
      return 
	(new History_Score(*dynamic_cast<const History_Score*>(a)));
    case PLAYER_VANQUISHED:
      return 
	(new History_PlayerVanquished
	 (*dynamic_cast<const History_PlayerVanquished*>(a)));
    case DIPLOMATIC_PEACE:
      return 
	(new History_DiplomacyPeace
	 (*dynamic_cast<const History_DiplomacyPeace*>(a)));
    case DIPLOMATIC_WAR:
      return 
	(new History_DiplomacyWar
	 (*dynamic_cast<const History_DiplomacyWar*>(a)));
    case DIPLOMATIC_TREACHERY:
      return 
	(new History_DiplomacyTreachery
	 (*dynamic_cast<const History_DiplomacyTreachery*>(a)));
    case HERO_FINDS_ALLIES:
      return 
	(new History_HeroFindsAllies
          (*dynamic_cast<const History_HeroFindsAllies*>(a)));
    case END_TURN:
      return 
	(new History_EndTurn(*dynamic_cast<const History_EndTurn*>(a)));
    }

  return 0;
}

History::History (XML_Helper *helper)
{
  std::string type_str;
  helper->getData(type_str, "type");
  d_type = historyTypeFromString(type_str);
}

bool History::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(History::d_tag);
    retval &= saveContents(helper);
    retval &= helper->closeTag();

    return retval;
}

bool History::saveContents(XML_Helper* helper) const
{
    bool retval = true;

    std::string type_str = historyTypeToString(History::Type(d_type));
    retval &= helper->saveData("type", type_str);
    retval &= doSave(helper);

    return retval;
}

//-----------------------------------------------------------------------------
//History_StartTurn

History_StartTurn::History_StartTurn()
:History(History::START_TURN)
{
}

History_StartTurn::History_StartTurn(const History_StartTurn &history)
:History(history)
{
}

History_StartTurn::History_StartTurn(XML_Helper* helper)
:History(helper)
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

bool History_StartTurn::doSave(XML_Helper* helper) const
{
  bool retval = true;

  return retval;
}

bool History_StartTurn::fillData()
{
  return true;
}

//-----------------------------------------------------------------------------
//History_FoundSage

History_FoundSage::History_FoundSage()
:History(History::FOUND_SAGE), d_hero("")
{
}

History_FoundSage::History_FoundSage(const History_FoundSage &history)
:History(history), d_hero(history.d_hero)
{
}

History_FoundSage::History_FoundSage(XML_Helper* helper)
:History(helper)
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

bool History_FoundSage::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("hero", d_hero);

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

History_GoldTotal::History_GoldTotal(const History_GoldTotal &history)
:History(history), d_gold(history.d_gold)
{
}

History_GoldTotal::History_GoldTotal(XML_Helper* helper)
:History(helper)
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

bool History_GoldTotal::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("gold", d_gold);

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

History_HeroEmerges::History_HeroEmerges(const History_HeroEmerges &history)
:History(history), d_hero(history.d_hero), d_city(history.d_city)
{
}

History_HeroEmerges::History_HeroEmerges(XML_Helper* helper)
:History(helper)
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

bool History_HeroEmerges::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("hero", d_hero);
  retval &= helper->saveData("city", d_city);

  return retval;
}

bool History_HeroEmerges::fillData(HeroProto *hero, City *city)
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

History_CityWon::History_CityWon(const History_CityWon &history)
:History(history), d_city(history.d_city)
{
}

History_CityWon::History_CityWon(XML_Helper* helper)
:History(helper)
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

bool History_CityWon::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("city", d_city);

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
:History(History::HERO_CITY_WON), d_hero(""), d_city("")
{
}

History_HeroCityWon::History_HeroCityWon(const History_HeroCityWon &history)
:History(history), d_hero(history.d_hero), d_city(history.d_city)
{
}

History_HeroCityWon::History_HeroCityWon(XML_Helper* helper)
:History(helper)
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

bool History_HeroCityWon::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("city", d_city);
  retval &= helper->saveData("hero", d_hero);

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

History_CityRazed::History_CityRazed(const History_CityRazed &history)
:History(history), d_city(history.d_city)
{
}

History_CityRazed::History_CityRazed(XML_Helper* helper)
:History(helper)
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

bool History_CityRazed::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("city", d_city);

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

History_HeroQuestStarted::History_HeroQuestStarted(const History_HeroQuestStarted &history)
:History(history), d_hero(history.d_hero)
{
}

History_HeroQuestStarted::History_HeroQuestStarted(XML_Helper* helper)
:History(helper)
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

bool History_HeroQuestStarted::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("hero", d_hero);

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

History_HeroQuestCompleted::History_HeroQuestCompleted(const History_HeroQuestCompleted &history)
:History(history), d_hero(history.d_hero)
{
}

History_HeroQuestCompleted::History_HeroQuestCompleted(XML_Helper* helper)
:History(helper)
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

bool History_HeroQuestCompleted::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("hero", d_hero);

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

History_HeroKilledInCity::History_HeroKilledInCity(const History_HeroKilledInCity &history)
:History(history), d_hero(history.d_hero), d_city(history.d_city)
{
}

History_HeroKilledInCity::History_HeroKilledInCity(XML_Helper* helper)
:History(helper)
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

bool History_HeroKilledInCity::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("hero", d_hero);
  retval &= helper->saveData("city", d_city);

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

History_HeroKilledInBattle::History_HeroKilledInBattle(const History_HeroKilledInBattle &history)
:History(history), d_hero(history.d_hero)
{
}

History_HeroKilledInBattle::History_HeroKilledInBattle(XML_Helper* helper)
:History(helper)
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

bool History_HeroKilledInBattle::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("hero", d_hero);

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

History_HeroKilledSearching::History_HeroKilledSearching(const History_HeroKilledSearching &history)
:History(history), d_hero(history.d_hero)
{
}

History_HeroKilledSearching::History_HeroKilledSearching(XML_Helper* helper)
:History(helper)
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

bool History_HeroKilledSearching::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("hero", d_hero);

  return retval;
}

bool History_HeroKilledSearching::fillData(Hero *hero)
{
  d_hero = hero->getName();
  return true;
}

//-----------------------------------------------------------------------------
//History_Score

History_Score::History_Score()
:History(History::SCORE), d_score(0)
{
}

History_Score::History_Score(const History_Score &history)
:History(history), d_score(history.d_score)
{
}

History_Score::History_Score(XML_Helper* helper)
:History(helper)
{
  helper->getData(d_score, "score");
}

History_Score::~History_Score()
{
}

std::string History_Score::dump() const
{
  std::stringstream s;

  s <<"player has a score of " << d_score<< "\n";

  return s.str();
}

bool History_Score::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("score", d_score);

  return retval;
}

bool History_Score::fillData(Uint32 score)
{
  d_score = score;
  return true;
}

//-----------------------------------------------------------------------------
//History_PlayerVanquished

History_PlayerVanquished::History_PlayerVanquished()
:History(History::PLAYER_VANQUISHED)
{
}

History_PlayerVanquished::History_PlayerVanquished(const History_PlayerVanquished &history)
:History(history)
{
}

History_PlayerVanquished::History_PlayerVanquished(XML_Helper* helper)
:History(helper)
{
}

History_PlayerVanquished::~History_PlayerVanquished()
{
}

std::string History_PlayerVanquished::dump() const
{
  std::stringstream s;

  s <<"player has been vanquished!\n";

  return s.str();
}

bool History_PlayerVanquished::doSave(XML_Helper* helper) const
{
  bool retval = true;

  return retval;
}

//-----------------------------------------------------------------------------
//History_DiplomacyPeace

History_DiplomacyPeace::History_DiplomacyPeace()
:History(History::DIPLOMATIC_PEACE), d_opponent_id(0)
{
}

History_DiplomacyPeace::History_DiplomacyPeace(const History_DiplomacyPeace &history)
:History(history), d_opponent_id(history.d_opponent_id)
{
}

History_DiplomacyPeace::History_DiplomacyPeace(XML_Helper* helper)
:History(helper)
{
  helper->getData(d_opponent_id, "opponent_id");
}

History_DiplomacyPeace::~History_DiplomacyPeace()
{
}

std::string History_DiplomacyPeace::dump() const
{
  std::stringstream s;

  s <<"peace has been won with player " << d_opponent_id;
  s <<"\n";

  return s.str();
}

bool History_DiplomacyPeace::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("opponent_id", d_opponent_id);

  return retval;
}

bool History_DiplomacyPeace::fillData(Player *opponent)
{
  d_opponent_id = opponent->getId();
  return true;
}

//-----------------------------------------------------------------------------
//History_DiplomacyWar

History_DiplomacyWar::History_DiplomacyWar()
:History(History::DIPLOMATIC_WAR), d_opponent_id(0)
{
}

History_DiplomacyWar::History_DiplomacyWar(const History_DiplomacyWar &history)
:History(history), d_opponent_id(history.d_opponent_id)
{
}

History_DiplomacyWar::History_DiplomacyWar(XML_Helper* helper)
:History(helper)
{
  helper->getData(d_opponent_id, "opponent_id");
}

History_DiplomacyWar::~History_DiplomacyWar()
{
}

std::string History_DiplomacyWar::dump() const
{
  std::stringstream s;

  s <<"war has been declared with player " << d_opponent_id;
  s <<"\n";

  return s.str();
}

bool History_DiplomacyWar::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("opponent_id", d_opponent_id);

  return retval;
}

bool History_DiplomacyWar::fillData(Player *opponent)
{
  d_opponent_id = opponent->getId();
  return true;
}

//-----------------------------------------------------------------------------
//History_DiplomacyTreachery

History_DiplomacyTreachery::History_DiplomacyTreachery()
:History(History::DIPLOMATIC_TREACHERY), d_opponent_id(0)
{
}

History_DiplomacyTreachery::History_DiplomacyTreachery(const History_DiplomacyTreachery &history)
:History(history), d_opponent_id(history.d_opponent_id)
{
}

History_DiplomacyTreachery::History_DiplomacyTreachery(XML_Helper* helper)
:History(helper)
{
  helper->getData(d_opponent_id, "opponent_id");
}

History_DiplomacyTreachery::~History_DiplomacyTreachery()
{
}

std::string History_DiplomacyTreachery::dump() const
{
  std::stringstream s;

  s <<"treachery on player " << d_opponent_id;
  s <<"\n";

  return s.str();
}

bool History_DiplomacyTreachery::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("opponent_id", d_opponent_id);

  return retval;
}

bool History_DiplomacyTreachery::fillData(Player *opponent)
{
  d_opponent_id = opponent->getId();
  return true;
}

//-----------------------------------------------------------------------------
//History_HeroFindsAllies

History_HeroFindsAllies::History_HeroFindsAllies()
:History(History::HERO_FINDS_ALLIES), d_hero("")
{
}

History_HeroFindsAllies::History_HeroFindsAllies(const History_HeroFindsAllies &history)
:History(history), d_hero(history.d_hero)
{
}

History_HeroFindsAllies::History_HeroFindsAllies(XML_Helper* helper)
:History(helper)
{
  helper->getData(d_hero, "hero");
}

History_HeroFindsAllies::~History_HeroFindsAllies()
{
}

std::string History_HeroFindsAllies::dump() const
{
  std::stringstream s;

  s <<"hero " << d_hero<< " finds some allies\n";

  return s.str();
}

bool History_HeroFindsAllies::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("hero", d_hero);

  return retval;
}

bool History_HeroFindsAllies::fillData(Hero *hero)
{
  d_hero = hero->getName();
  return true;
}

//-----------------------------------------------------------------------------
//History_EndTurn

History_EndTurn::History_EndTurn()
:History(History::END_TURN)
{
}

History_EndTurn::History_EndTurn(const History_EndTurn &history)
:History(history)
{
}

History_EndTurn::History_EndTurn(XML_Helper* helper)
:History(helper)
{
}

History_EndTurn::~History_EndTurn()
{
}

std::string History_EndTurn::dump() const
{
  std::stringstream s;

  s <<"player ends a turn" << "\n";

  return s.str();
}

bool History_EndTurn::doSave(XML_Helper* helper) const
{
  bool retval = true;

  return retval;
}

bool History_EndTurn::fillData()
{
  return true;
}

std::string History::historyTypeToString(const History::Type type)
{
  switch (type)
    {
    case History::START_TURN:
      return "History::START_TURN";
    case History::FOUND_SAGE:
      return "History::FOUND_SAGE";
    case History::GOLD_TOTAL:
      return "History::GOLD_TOTAL";
    case History::HERO_EMERGES:
      return "History::HERO_EMERGES";
    case History::CITY_WON:
      return "History::CITY_WON";
    case History::HERO_CITY_WON:
      return "History::HERO_CITY_WON";
    case History::CITY_RAZED:
      return "History::CITY_RAZED";
    case History::HERO_QUEST_STARTED:
      return "History::HERO_QUEST_STARTED";
    case History::HERO_QUEST_COMPLETED:
      return "History::HERO_QUEST_COMPLETED";
    case History::HERO_KILLED_IN_CITY:
      return "History::HERO_KILLED_IN_CITY";
    case History::HERO_KILLED_IN_BATTLE:
      return "History::HERO_KILLED_IN_BATTLE";
    case History::HERO_KILLED_SEARCHING:
      return "History::HERO_KILLED_SEARCHING";
    case History::SCORE:
      return "History::SCORE";
    case History::PLAYER_VANQUISHED:
      return "History::PLAYER_VANQUISHED";
    case History::DIPLOMATIC_PEACE:
      return "History::DIPLOMATIC_PEACE";
    case History::DIPLOMATIC_WAR:
      return "History::DIPLOMATIC_WAR";
    case History::DIPLOMATIC_TREACHERY:
      return "History::DIPLOMATIC_TREACHERY";
    case History::HERO_FINDS_ALLIES:
      return "History::HERO_FINDS_ALLIES";
    case History::END_TURN:
      return "History::END_TURN";
    }
  return "History::START_TURN";
}

History::Type History::historyTypeFromString(const std::string str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return History::Type(atoi(str.c_str()));
  if (str == "History::START_TURN")
    return History::START_TURN;
  else if (str == "History::FOUND_SAGE")
    return History::FOUND_SAGE;
  else if (str == "History::GOLD_TOTAL")
    return History::GOLD_TOTAL;
  else if (str == "History::HERO_EMERGES")
    return History::HERO_EMERGES;
  else if (str == "History::CITY_WON")
    return History::CITY_WON;
  else if (str== "History::HERO_CITY_WON")
    return History::HERO_CITY_WON;
  else if (str == "History::CITY_RAZED")
    return History::CITY_RAZED;
  else if (str == "History::HERO_QUEST_STARTED")
    return History::HERO_QUEST_STARTED;
  else if (str == "History::HERO_QUEST_COMPLETED")
    return History::HERO_QUEST_COMPLETED;
  else if (str == "History::HERO_KILLED_IN_CITY")
    return History::HERO_KILLED_IN_CITY;
  else if (str == "History::HERO_KILLED_IN_BATTLE")
    return History::HERO_KILLED_IN_BATTLE;
  else if (str == "History::HERO_KILLED_SEARCHING")
    return History::HERO_KILLED_SEARCHING;
  else if (str == "History::SCORE")
    return History::SCORE;
  else if (str == "History::PLAYER_VANQUISHED")
    return History::PLAYER_VANQUISHED;
  else if (str == "History::DIPLOMATIC_PEACE")
    return History::DIPLOMATIC_PEACE;
  else if (str == "History::DIPLOMATIC_WAR")
    return History::DIPLOMATIC_WAR;
  else if (str == "History::DIPLOMATIC_TREACHERY")
    return History::DIPLOMATIC_TREACHERY;
  else if (str == "History::HERO_FINDS_ALLIES")
    return History::HERO_FINDS_ALLIES;
  else if (str == "History::END_TURN")
    return History::END_TURN;
  return History::START_TURN;
}
