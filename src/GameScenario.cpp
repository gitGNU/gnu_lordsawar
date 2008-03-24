// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2006 Andrea Patton
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
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

#include "GameScenario.h"
#include "MapGenerator.h"
#include "playerlist.h"
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
#include "xmlhelper.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

bool GameScenario::s_see_opponents_stacks = false;
bool GameScenario::s_see_opponents_production = false;
bool GameScenario::s_play_with_quests = true;
bool GameScenario::s_hidden_map = false;
bool GameScenario::s_diplomacy = false;
bool GameScenario::s_cusp_of_war = false;
GameParameters::NeutralCities GameScenario::s_neutral_cities = GameParameters::AVERAGE;
GameParameters::RazingCities GameScenario::s_razing_cities = GameParameters::ALWAYS;
bool GameScenario::s_intense_combat = false;
bool GameScenario::s_military_advisor = false;
bool GameScenario::s_random_turns = false;
bool GameScenario::s_surrender_already_offered = false;

GameScenario::GameScenario(std::string name,std::string comment, bool turnmode)
    :d_round(0), d_name(name),d_comment(comment), d_turnmode(turnmode)
{
    Armysetlist::getInstance();
    Armysetlist::getInstance()->instantiatePixmaps();
    Tilesetlist::getInstance();
    Tilesetlist::getInstance()->instantiatePixmaps();
    Shieldsetlist::getInstance();
    Shieldsetlist::getInstance()->instantiatePixmaps();

    if (fl_counter == 0)
        fl_counter = new FL_Counter();
}

// savegame is a filename with absolute path!

GameScenario::GameScenario(string savegame, bool& broken)
    :d_turnmode(true)
{
    Armysetlist::getInstance();
    Armysetlist::getInstance()->instantiatePixmaps();
    Tilesetlist::getInstance();
    Tilesetlist::getInstance()->instantiatePixmaps();
    Shieldsetlist::getInstance();
    Shieldsetlist::getInstance()->instantiatePixmaps();

    broken = false;
    XML_Helper helper(savegame, ios::in, Configuration::s_zipfiles);

    helper.registerTag("scenario", sigc::mem_fun(this, &GameScenario::load));
    helper.registerTag("itemlist", sigc::mem_fun(this, &GameScenario::load));
    helper.registerTag("playerlist", sigc::mem_fun(this, &GameScenario::load));
    helper.registerTag("map", sigc::mem_fun(this, &GameScenario::load));
    helper.registerTag("citylist", sigc::mem_fun(this, &GameScenario::load));
    helper.registerTag("templelist", sigc::mem_fun(this, &GameScenario::load));
    helper.registerTag("ruinlist", sigc::mem_fun(this, &GameScenario::load));
    helper.registerTag("rewardlist", sigc::mem_fun(this, &GameScenario::load));
    helper.registerTag("signpostlist", sigc::mem_fun(this, &GameScenario::load));
    helper.registerTag("roadlist", sigc::mem_fun(this, &GameScenario::load));
    helper.registerTag("counter", sigc::mem_fun(this, &GameScenario::load));
    helper.registerTag("questlist", sigc::mem_fun(this, &GameScenario::load));
    helper.registerTag("bridgelist", sigc::mem_fun(this, &GameScenario::load));
    helper.registerTag("portlist", sigc::mem_fun(this, &GameScenario::load));
    helper.registerTag("vectoredunitlist", sigc::mem_fun(this, &GameScenario::load));

    //now parse the document and close the file afterwards
    if (!helper.parse())
    {
        broken = true;
    }

    helper.close();
    GameMap::getInstance()->calculateBlockedAvenues();
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

    if (fl_counter)
    {
        delete fl_counter;
        fl_counter = 0;
    }
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

void GameScenario::nextRound()
{
    d_round++;

    char filename[1024];
    if (Configuration::s_autosave_policy == 2)
      snprintf(filename,sizeof(filename), "autosave-%03d.sav", d_round - 1);
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
        debug(_("The Filename lacks the extension --> ") << strtoken->getLastToken())
        goodfilename += "." + extension;
    }

    delete strtoken;

    XML_Helper helper(goodfilename, ios::out, Configuration::s_zipfiles);

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

    //save the private GameScenario data last due to dependencies
    retval &= helper.openTag("scenario");
    retval &= helper.saveData("name", d_name);
    retval &= helper.saveData("comment", d_comment);
    retval &= helper.saveData("turn", d_round);
    retval &= helper.saveData("turnmode", d_turnmode);
    retval &= helper.saveData("view_enemies", s_see_opponents_stacks);
    retval &= helper.saveData("view_production", s_see_opponents_production);
    retval &= helper.saveData("quests", s_play_with_quests);
    retval &= helper.saveData("hidden_map", s_hidden_map);
    retval &= helper.saveData("diplomacy", s_diplomacy);
    retval &= helper.saveData("cusp_of_war", s_cusp_of_war);
    retval &= helper.saveData("neutral_cities", (int) s_neutral_cities);
    retval &= helper.saveData("razing_cities", (int) s_razing_cities);
    retval &= helper.saveData("intense_combat", s_intense_combat);
    retval &= helper.saveData("military_advisor", s_military_advisor);
    retval &= helper.saveData("random_turns", s_random_turns);
    retval &= helper.saveData("surrender_already_offered", 
			      s_surrender_already_offered);
    
    retval &= helper.closeTag();
    
    if (!retval)
    {
        std::cerr <<_("GameScenario: Something went wrong with saving.\n");
        return false;
    }
    
    retval &= helper.closeTag();

    helper.close();

    if (retval)
        return true;
        
    std::cerr <<_("GameScenario: Something went wrong with saving.\n");
    return false;
}

bool GameScenario::load(std::string tag, XML_Helper* helper)
{
    if (tag == "scenario")
    {
        if (helper->getVersion() != LORDSAWAR_SAVEGAME_VERSION)
        {
            cerr <<_("savefile has wrong version, we want ");
            std::cerr <<LORDSAWAR_SAVEGAME_VERSION <<",\n";
            cerr <<_("savefile offers ") <<helper->getVersion() <<".\n";
            return false;
        }
    
        debug("loading scenario")
        helper->getData(d_round, "turn");
        helper->getData(d_turnmode, "turnmode");
        helper->getData(d_name, "name");
        helper->getData(d_comment, "comment");
        helper->getData(s_see_opponents_stacks, "view_enemies");
        helper->getData(s_see_opponents_production, "view_production");
        helper->getData(s_play_with_quests, "quests");
        helper->getData(s_hidden_map, "hidden_map");
        helper->getData(s_diplomacy, "diplomacy");
        helper->getData(s_cusp_of_war, "cusp_of_war");
        int val = -1;
        helper->getData(val, "neutral_cities");
        s_neutral_cities = GameParameters::NeutralCities (val);
        val = -1;
        helper->getData(val, "razing_cities");
        s_razing_cities = GameParameters::RazingCities (val);
        helper->getData(s_intense_combat, "intense_combat");
        helper->getData(s_military_advisor, "military_advisor");
        helper->getData(s_random_turns, "random_turns");
        helper->getData(s_surrender_already_offered, 
			"surrender_already_offered");

        return true;
    }

    if (tag == "counter")
    {
        debug("loading counter")
        fl_counter = new FL_Counter(helper);
        return true;
    }

    if (tag == "itemlist")
    {
        debug("loading items");
        Itemlist::getInstance(helper);
        return true;
    }

    if (tag == "playerlist")
    {
        debug("loading players");
        Playerlist::getInstance(helper);
        return true;
    }

    if (tag == "map")
    {
        debug("loading map")
        GameMap::getInstance(helper);
        return true;
    }

    if (tag == "citylist")
    {
        debug("loading cities")

        Citylist::getInstance(helper);
        return true;
    }

    if (tag == "templelist")
    {
        debug("loading temples")
        Templelist::getInstance(helper);
        return true;
    }

    if (tag == "ruinlist")
    {
        debug("loading ruins")
        Ruinlist::getInstance(helper);
        return true;
    }

    if (tag == "rewardlist")
    {
        debug("loading rewards")
        Rewardlist::getInstance(helper);
        return true;
    }

    if (tag == "signpostlist")
    {
        debug("loading signposts")
            Signpostlist::getInstance(helper);
        return true;
    }

    if (tag == "roadlist")
    {
        debug("loading roads")
        Roadlist::getInstance(helper);
        return true;
    }

    if (tag == "questlist")
    {
        debug("loading quests")
        QuestsManager::getInstance(helper);
        return true;
    }

    if (tag == "vectoredunitlist")
    {
        debug("loading vectored units")
        VectoredUnitlist::getInstance(helper);
        return true;
    }

    if (tag == "portlist")
    {
        debug("loading ports")
        Portlist::getInstance(helper);
        return true;
    }

    if (tag == "bridgelist")
    {
        debug("loading bridges")
        Bridgelist::getInstance(helper);
        return true;
    }

    return false;
}

int GameScenario::calculate_difficulty_rating(GameParameters g)
{
  float total_difficulty = 0;
  int max_player_difficulty = 73;
  int players_on = 0;
  for (std::vector<GameParameters::Player>::iterator it = g.players.begin(); 
       it != g.players.end(); it++)
    {
      if ((*it).type != GameParameters::Player::OFF)
	players_on++;

    }
  int players_off = g.players.size() - players_on;

  //find out how much each player is worth
  float player_difficulty;
  player_difficulty = (float) max_player_difficulty / (float) players_off;
  if (players_on != 0)
    player_difficulty = (float)max_player_difficulty / (float)players_on;

  //go through all players, adding up difficulty points for each
  for (std::vector<GameParameters::Player>::iterator i = g.players.begin(); 
       i != g.players.end(); i++)
    {
      if ((*i).type == GameParameters::Player::HUMAN || 
	  ((*i).type == GameParameters::Player::HARD))
	total_difficulty += player_difficulty;
      else if ((*i).type == GameParameters::Player::EASY)
	total_difficulty += player_difficulty;
      //FIXME: when the hard player gets better, switch this.
	//total_difficulty += (player_difficulty * 0.325);
      //else if ((*i).type == GameParameters::Player::EASIER)
	//total_difficulty += (player_difficulty * 0.655);
    }
  if (g.diplomacy)
    total_difficulty += (float) 3.0;
  if (g.hidden_map)
    total_difficulty += (float) 3.0;
  if (g.play_with_quests)
    total_difficulty += (float) 3.0;
  if (g.see_opponents_production == false)
    total_difficulty += (float) 2.0;
  if (g.see_opponents_stacks == false)
    total_difficulty += (float) 2.0;
  if (g.neutral_cities == GameParameters::STRONG)
    total_difficulty += (float) 3.0;
  else if (g.neutral_cities == GameParameters::ACTIVE)
    total_difficulty += (float) 6.0;
  if (g.razing_cities == GameParameters::ON_CAPTURE)
    total_difficulty += (float) 3.0;
  else if (g.razing_cities == GameParameters::NEVER)
    total_difficulty += (float) 6.0;
  if (g.cusp_of_war == true)
    total_difficulty += (float) 2.0;
  return (int) total_difficulty;
}
