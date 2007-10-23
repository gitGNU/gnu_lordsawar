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
GameParameters::NeutralCities GameScenario::s_neutral_cities = GameParameters::AVERAGE;
bool GameScenario::s_intense_combat = false;
bool GameScenario::s_military_advisor = false;
bool GameScenario::s_random_turns = false;

GameScenario::GameScenario(std::string name,std::string comment, bool turnmode)
    :d_round(0), d_name(name),d_comment(comment), d_turnmode(turnmode)
{
    Armysetlist::getInstance();
    Armysetlist::getInstance()->instantiatePixmaps();
    Tilesetlist::getInstance();
    Tilesetlist::getInstance()->instantiatePixmaps();
    Itemlist::createInstance();

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
    Itemlist::createInstance();

    broken = false;
    XML_Helper helper(savegame, ios::in, Configuration::s_zipfiles);

    helper.registerTag("scenario", sigc::mem_fun(this, &GameScenario::load));
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
    Itemlist::deleteInstance();
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
               std::string(File::getSavePath() + "autosave.sav").c_str()))
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
    retval &= helper.saveData("neutral_cities", (int) s_neutral_cities);
    retval &= helper.saveData("intense_combat", s_intense_combat);
    retval &= helper.saveData("military_advisor", s_military_advisor);
    retval &= helper.saveData("random_turns", s_random_turns);
    
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
        int val = -1;
        helper->getData(val, "neutral_cities");
        s_neutral_cities = GameParameters::NeutralCities (val);
        helper->getData(s_intense_combat, "intense_combat");
        helper->getData(s_military_advisor, "military_advisor");
        helper->getData(s_random_turns, "random_turns");

        //TODO: for later when it becomes crucial: deal with loading of
        //something else than simple games
        return true;
    }

    if (tag == "counter")
    {
        debug("loading counter")
        fl_counter = new FL_Counter(helper);
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

