// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
// Copyright (C) 2007, 2008 Ole Laursen
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
#include <fstream>
#include <stdlib.h>
#include <errno.h>
#include <sigc++/functors/mem_fun.h>
#include <uuid/uuid.h>

#include "GameScenario.h"
#include "Campaign.h"
#include "MapGenerator.h"
#include "playerlist.h"
#include "FogMap.h"
#include "citylist.h"
#include "ruinlist.h"
#include "rewardlist.h"
#include "templelist.h"
#include "bridgelist.h"
#include "portlist.h"
#include "roadlist.h"
#include "signpostlist.h"
#include "city.h"
#include "ruin.h"
#include "File.h"
#include "armysetlist.h"
#include "tilesetlist.h"
#include "shieldsetlist.h"
#include "stacklist.h"
#include "stack.h"
#include "GameMap.h"
#include "player.h"
#include "Configuration.h"
#include "real_player.h"
#include "ai_dummy.h"
#include "ai_fast.h"
#include "counter.h"
#include "army.h"
#include "QuestsManager.h"
#include "Itemlist.h"
#include "string_tokenizer.h"
#include "player.h"
#include "vectoredunitlist.h"
#include "history.h"
#include "xmlhelper.h"

std::string GameScenario::d_tag = "scenario";
using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

GameScenario::GameScenario(std::string name,std::string comment, bool turnmode,
			   GameScenario::PlayMode playmode)
    :d_name(name),d_comment(comment), d_turnmode(turnmode), d_playmode(playmode)
{
    Armysetlist::getInstance();
    Tilesetlist::getInstance();
    Shieldsetlist::getInstance();

    if (fl_counter == 0)
      fl_counter = new FL_Counter();

    setNewRandomId();
}

// savegame is a filename with absolute path!

GameScenario::GameScenario(string savegame, bool& broken)
  :d_turnmode(true), d_playmode(GameScenario::HOTSEAT)
{
  XML_Helper helper(savegame, ios::in, Configuration::s_zipfiles);
  broken = loadWithHelper(helper);
  helper.close();
}

GameScenario::GameScenario(XML_Helper &helper, bool& broken)
  : d_turnmode(true), d_playmode(GameScenario::HOTSEAT)
{
  broken = loadWithHelper(helper);
}

void GameScenario::quickStart()
{
  Playerlist *plist = Playerlist::getInstance();
  Citylist *clist = Citylist::getInstance();
  Vector <int> pos;
  // no neutral cities
  // divvy up the neutral cities among other non-neutral players
  int cities_left = clist->size() - plist->size() + 1;
  unsigned int citycount[MAX_PLAYERS];
  memset (citycount, 0, sizeof (citycount));
  Playerlist::iterator pit = plist->begin();

  while (cities_left > 0)
    {
      if (*pit != plist->getNeutral())
	{
	  citycount[(*pit)->getId()]++;
	  cities_left--;
	}

      pit++;
      if (pit == plist->end())
	pit = plist->begin();
    }

  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      for (unsigned int j = 0; j < citycount[i]; j++)
	{
	  Player *p = plist->getPlayer(i);
	  if (!p)
	    continue;
	  if (p == plist->getNeutral())
	    continue;
	  pos = clist->getFirstCity(p)->getPos();
	  City *c = clist->getNearestNeutralCity(pos);
	  c->conquer(p);
	  History_CityWon *item = new History_CityWon();
	  item->fillData(c);
	  p->addHistory(item);
	}
    }
}

bool GameScenario::setupFog(bool hidden_map)
{
  Playerlist *pl = Playerlist::getInstance();
  Playerlist::iterator it = pl->begin();
  for (; it != pl->end(); it++)
    {
      if (hidden_map)
	(*it)->getFogMap()->fill(FogMap::CLOSED);
      else
	(*it)->getFogMap()->fill(FogMap::OPEN);
    }
  return true;
}

bool GameScenario::setupStacks(bool hidden_map)
{
  if (!hidden_map)
    return true;
  for (Playerlist::iterator it = Playerlist::getInstance()->begin();
       it != Playerlist::getInstance()->end(); it++)
    {
      if ((*it) == Playerlist::getInstance()->getNeutral())
	continue;
      for (Stacklist::iterator sit = (*it)->getStacklist()->begin(); 
	   sit != (*it)->getStacklist()->end(); sit++)
	(*sit)->deFog();
    }
  return true;
}

bool GameScenario::setupCities(bool quick_start)
{
  debug("GameScenario::setupCities")

  for (Playerlist::iterator it = Playerlist::getInstance()->begin();
       it != Playerlist::getInstance()->end(); it++)
    {
      if ((*it) == Playerlist::getInstance()->getNeutral())
	continue;
      City *city = Citylist::getInstance()->getFirstCity(*it);
      if (city)
	{
	  city->deFog(city->getOwner());
	  History_CityWon *item = new History_CityWon();
	  item->fillData(city);
	  city->getOwner()->addHistory(item);
	}
    }

  if (quick_start)
    quickStart();

  for (Citylist::iterator it = Citylist::getInstance()->begin();
       it != Citylist::getInstance()->end(); it++)
    {
      if ((*it)->getOwner() == Playerlist::getInstance()->getNeutral())
	{
	  switch (GameScenario::s_neutral_cities)
	    {
	    case GameParameters::AVERAGE:
	      (*it)->produceScout();
	      break;
	    case GameParameters::STRONG:
	      (*it)->produceStrongestProductionBase();
	      break;
	    case GameParameters::ACTIVE:
	      if (rand () % 100 >  20)
		(*it)->produceStrongestProductionBase();
	      else
		(*it)->produceWeakestProductionBase();
	      break;
	    }
	  (*it)->setActiveProductionSlot(-1);
	}
      else
	{
	  if ((*it)->isCapital())
	    (*it)->produceStrongestProductionBase();
	  else
	    (*it)->produceWeakestProductionBase();

	  (*it)->setActiveProductionSlot(0);
	}

    }

  return true;
}
void GameScenario::setupDiplomacy(bool diplomacy)
{
  Playerlist *pl = Playerlist::getInstance();
    // Set up diplomacy
    for (Playerlist::iterator pit = pl->begin(); pit != pl->end(); pit++)
      {
	if (pl->getNeutral() == (*pit))
	  continue;
	for (Playerlist::iterator it = pl->begin(); it != pl->end(); it++)
	  {
	    if (pl->getNeutral() == (*it))
	      continue;
	    if (*pit == *it)
	      continue;
	    if (diplomacy == false)
	      {
		(*pit)->proposeDiplomacy(Player::PROPOSE_WAR, *it);
		(*pit)->declareDiplomacy(Player::AT_WAR, *it);
	      }
	    else 
	      {
		(*pit)->proposeDiplomacy(Player::NO_PROPOSAL, *it);
		(*pit)->declareDiplomacy(Player::AT_PEACE, *it);
	      }
	  }
      }
    if (diplomacy)
      pl->calculateDiplomaticRankings();
}

bool GameScenario::loadWithHelper(XML_Helper& helper)
{
  Armysetlist::getInstance();
  Tilesetlist::getInstance();
  Shieldsetlist::getInstance();

  bool broken = false;

  helper.registerTag(d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(Campaign::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(Itemlist::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(Playerlist::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(GameMap::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(Citylist::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(Templelist::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(Ruinlist::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(Rewardlist::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(Signpostlist::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(Roadlist::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(FL_Counter::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(QuestsManager::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(Bridgelist::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(Portlist::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(VectoredUnitlist::d_tag, sigc::mem_fun(this, &GameScenario::load));

  if (!helper.parse())
    broken = true;

  GameMap::getInstance()->calculateBlockedAvenues();

  return broken;
}


GameScenario::~GameScenario()
{
  // GameMap is a Singleton so we need a function to delete it
  GameMap::deleteInstance();
  Itemlist::deleteInstance();
  Playerlist::deleteInstance();
  Citylist::deleteInstance();
  Templelist::deleteInstance();
  Ruinlist::deleteInstance();
  Rewardlist::deleteInstance();
  Signpostlist::deleteInstance();
  Portlist::deleteInstance();
  Bridgelist::deleteInstance();
  Roadlist::deleteInstance();
  QuestsManager::deleteInstance();
  VectoredUnitlist::deleteInstance();
  Campaign::deleteInstance();

  if (fl_counter)
    {
      delete fl_counter;
      fl_counter = 0;
    }
  if (Configuration::s_autosave_policy == 1)
    {
      std::string filename = File::getSavePath() + "autosave.sav";
      remove(filename.c_str());
    }
  GameScenarioOptions::s_round = 0;
} 

std::string GameScenario::getName(bool translate) const
{
  if (translate)
    return __(d_name);

  return d_name;
}

std::string GameScenario::getComment(bool translate) const
{
  if (translate)
    return __(d_comment);

  return d_comment;
}

bool GameScenario::saveGame(string filename, string extension) const
{
  bool retval = true;
  string goodfilename=filename;

  stringTokenizer * strtoken= new stringTokenizer(filename,"/.\\ ");

  if (strtoken->getLastToken() == extension)
    {
      debug(_("The Filename is well formed"))
	std::cerr <<"";  //dummy call if debug statement is commented out
    }
  else 
    {
      debug(_("The Filename lacks the extension --> ") << extension)
	goodfilename += "." + extension;
    }

  delete strtoken;

  XML_Helper helper(goodfilename, ios::out, Configuration::s_zipfiles);
  retval &= saveWithHelper(helper);
  helper.close();

  if (retval)
    return true;

  std::cerr <<_("GameScenario: Something went wrong with saving.\n");
  return false;
}

bool GameScenario::saveWithHelper(XML_Helper &helper) const
{
  bool retval = true;

  //start writing
  retval &= helper.begin(LORDSAWAR_SAVEGAME_VERSION);
  retval &= helper.openTag("lordsawar");

  //if retval is still true it propably doesn't change throughout the rest
  //now save the single object's data
  retval &= fl_counter->save(&helper);
  retval &= Itemlist::getInstance()->save(&helper);
  retval &= Playerlist::getInstance()->save(&helper);
  retval &= GameMap::getInstance()->save(&helper);
  retval &= Citylist::getInstance()->save(&helper);
  retval &= Templelist::getInstance()->save(&helper);
  retval &= Ruinlist::getInstance()->save(&helper);
  retval &= Rewardlist::getInstance()->save(&helper);
  retval &= Signpostlist::getInstance()->save(&helper);
  retval &= Roadlist::getInstance()->save(&helper);
  retval &= Portlist::getInstance()->save(&helper);
  retval &= Bridgelist::getInstance()->save(&helper);
  retval &= QuestsManager::getInstance()->save(&helper);
  retval &= VectoredUnitlist::getInstance()->save(&helper);
  retval &= Campaign::getInstance()->save(&helper);

  //save the private GameScenario data last due to dependencies
  retval &= helper.openTag(GameScenario::d_tag);
  retval &= helper.saveData("id", d_id);
  retval &= helper.saveData("name", d_name);
  retval &= helper.saveData("comment", d_comment);
  retval &= helper.saveData("turn", s_round);
  retval &= helper.saveData("turnmode", d_turnmode);
  retval &= helper.saveData("view_enemies", s_see_opponents_stacks);
  retval &= helper.saveData("view_production", s_see_opponents_production);
  retval &= helper.saveData("quests", s_play_with_quests);
  retval &= helper.saveData("hidden_map", s_hidden_map);
  retval &= helper.saveData("diplomacy", s_diplomacy);
  retval &= helper.saveData("cusp_of_war", s_cusp_of_war);
  std::string neutral_cities_str = Configuration::neutralCitiesToString(GameParameters::NeutralCities(s_neutral_cities));
  retval &= helper.saveData("neutral_cities", neutral_cities_str);
  std::string razing_cities_str = Configuration::razingCitiesToString(GameParameters::RazingCities(s_razing_cities));
  retval &= helper.saveData("razing_cities", razing_cities_str);
  retval &= helper.saveData("intense_combat", s_intense_combat);
  retval &= helper.saveData("military_advisor", s_military_advisor);
  retval &= helper.saveData("random_turns", s_random_turns);
  retval &= helper.saveData("surrender_already_offered", 
			    s_surrender_already_offered);
  std::string playmode_str = playModeToString(GameScenario::PlayMode(d_playmode));
  retval &= helper.saveData("playmode", playmode_str);

  retval &= helper.closeTag();

  retval &= helper.closeTag();

  return retval;
}

bool GameScenario::load(std::string tag, XML_Helper* helper)
{
  if (tag == GameScenario::d_tag)
    {
      if (helper->getVersion() != LORDSAWAR_SAVEGAME_VERSION)
	{
	  cerr <<_("savefile has wrong version, we want ");
	  std::cerr <<LORDSAWAR_SAVEGAME_VERSION <<",\n";
	  cerr <<_("savefile offers ") <<helper->getVersion() <<".\n";
	  return false;
	}

      debug("loading scenario")

      helper->getData(d_id, "id");
      helper->getData(s_round, "turn");
      helper->getData(d_turnmode, "turnmode");
      helper->getData(d_name, "name");
      helper->getData(d_comment, "comment");
      helper->getData(s_see_opponents_stacks, "view_enemies");
      helper->getData(s_see_opponents_production, "view_production");
      helper->getData(s_play_with_quests, "quests");
      helper->getData(s_hidden_map, "hidden_map");
      helper->getData(s_diplomacy, "diplomacy");
      helper->getData(s_cusp_of_war, "cusp_of_war");
      std::string neutral_cities_str;
      helper->getData(neutral_cities_str, "neutral_cities");
      s_neutral_cities = Configuration::neutralCitiesFromString(neutral_cities_str);
      std::string razing_cities_str;
      helper->getData(razing_cities_str, "razing_cities");
      s_razing_cities = Configuration::razingCitiesFromString(razing_cities_str);
      helper->getData(s_intense_combat, "intense_combat");
      helper->getData(s_military_advisor, "military_advisor");
      helper->getData(s_random_turns, "random_turns");
      helper->getData(s_surrender_already_offered, 
		      "surrender_already_offered");
      std::string playmode_str;
      helper->getData(playmode_str, "playmode");
      d_playmode = GameScenario::playModeFromString(playmode_str);

      return true;
    }
  
  if (tag == Campaign::d_tag)
    {
      debug("loading campaign")
      Campaign::getInstance(helper);
      return true;
    }

  if (tag == FL_Counter::d_tag)
    {
      debug("loading counter")
	fl_counter = new FL_Counter(helper);
      return true;
    }

  if (tag == Itemlist::d_tag)
    {
      debug("loading items");
      Itemlist::getInstance(helper);
      return true;
    }

  if (tag == Playerlist::d_tag)
    {
      debug("loading players");
      Playerlist::getInstance(helper);
      return true;
    }

  if (tag == GameMap::d_tag)
    {
      debug("loading map")
	GameMap::getInstance(helper);
      return true;
    }

  if (tag == Citylist::d_tag)
    {
      debug("loading cities")

	Citylist::getInstance(helper);
      return true;
    }

  if (tag == Templelist::d_tag)
    {
      debug("loading temples")
	Templelist::getInstance(helper);
      return true;
    }

  if (tag == Ruinlist::d_tag)
    {
      debug("loading ruins")
	Ruinlist::getInstance(helper);
      return true;
    }

  if (tag == Rewardlist::d_tag)
    {
      debug("loading rewards")
	Rewardlist::getInstance(helper);
      return true;
    }

  if (tag == Signpostlist::d_tag)
    {
      debug("loading signposts")
	Signpostlist::getInstance(helper);
      return true;
    }

  if (tag == Roadlist::d_tag)
    {
      debug("loading roads")
	Roadlist::getInstance(helper);
      return true;
    }

  if (tag == QuestsManager::d_tag)
    {
      debug("loading quests")
	QuestsManager::getInstance(helper);
      return true;
    }

  if (tag == VectoredUnitlist::d_tag)
    {
      debug("loading vectored units")
	VectoredUnitlist::getInstance(helper);
      return true;
    }

  if (tag == Portlist::d_tag)
    {
      debug("loading ports")
	Portlist::getInstance(helper);
      return true;
    }

  if (tag == Bridgelist::d_tag)
    {
      debug("loading bridges")
	Bridgelist::getInstance(helper);
      return true;
    }

  return false;
}

void GameScenario::nextRound()
{
  s_round++;

  char filename[1024];
  if (Configuration::s_autosave_policy == 2)
    snprintf(filename,sizeof(filename), "autosave-%03d.sav", s_round - 1);
  else if (Configuration::s_autosave_policy == 1)
    snprintf(filename,sizeof(filename), "autosave.sav");
  else
    return;
  // autosave to the file "autosave.sav". This is crude, but should work
  //
  // As a more enhanced version: autosave to a temporary file, then rename
  // the file. Avoids screwing up the autosave if something goes wrong
  // (and we have a savefile for debugging)
  if (!saveGame(File::getSavePath() + "tmp.sav"))
    {
      std::cerr<<_("Autosave failed.\n");
      return;
    }
  if (rename(std::string(File::getSavePath() + "tmp.sav").c_str(),
	     std::string(File::getSavePath() + filename).c_str()))
    {
      char* err = strerror(errno);
      std::cerr <<_("Error while trying to rename the temporary file to autosave.sav\n");
      std::cerr <<_("Error: ") <<err <<std::endl;
    }
}

std::string GameScenario::playModeToString(const GameScenario::PlayMode mode)
{
  switch (mode)
    {
      case GameScenario::HOTSEAT:
	return "GameScenario::HOTSEAT";
	break;
      case GameScenario::NETWORKED:
	return "GameScenario::NETWORKED";
	break;
      case GameScenario::PLAY_BY_MAIL:
	return "GameScenario::PLAY_BY_MAIL";
	break;
    }
  return "GameScenario::HOTSEAT";
}

GameScenario::PlayMode GameScenario::playModeFromString(const std::string str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return GameScenario::PlayMode(atoi(str.c_str()));
  if (str == "GameScenario::HOTSEAT")
    return GameScenario::HOTSEAT;
  else if (str == "GameScenario::NETWORKED")
    return GameScenario::NETWORKED;
  else if (str == "GameScenario::PLAY_BY_MAIL")
    return GameScenario::PLAY_BY_MAIL;
  return GameScenario::HOTSEAT;
}
	
void GameScenario::setNewRandomId()
{
  char buf[40];
  uuid_t uu;
  uuid_generate_time(uu);
  uuid_unparse(uu, buf);
  d_id = buf;
}
