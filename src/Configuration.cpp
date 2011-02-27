//  Copyright (C) 2002, 2003 Michael Bartl
//  Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
//  Copyright (C) 2004, 2005, 2006 Andrea Paternesi
//  Copyright (C) 2005 Josef Spillner
//  Copyright (C) 2006, 2007, 2008 Ben Asselstine
//  Copyright (C) 2007 Ole Laursen
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

#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <sigc++/functors/mem_fun.h>

#include "Configuration.h"

#include "xmlhelper.h"
#include "defs.h"
#include "File.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

// define static variables

bool Configuration::s_showNextPlayer = true;
#ifndef __WIN32__
string Configuration::configuration_file_path = string(getenv("HOME")) + "/.lordsawarrc";
string Configuration::s_dataPath = LORDSAWAR_DATADIR;
string Configuration::s_savePath = string(getenv("HOME"))+string("/.lordsawar/");
#else
string Configuration::configuration_file_path = "/.lordsawarrc";
string Configuration::s_dataPath = "./data/";
string Configuration::s_savePath = "./saves/";
#endif
string Configuration::s_lang = "";
int Configuration::s_displaySpeedDelay = 300000;
int Configuration::s_displayFightRoundDelayFast = 250;
int Configuration::s_displayFightRoundDelaySlow = 500;
bool Configuration::s_displayCommentator = true;
guint32 Configuration::s_cacheSize = 1000000;
bool Configuration::s_zipfiles = false;
int Configuration::s_autosave_policy = 1;
bool Configuration::s_musicenable = true;
guint32 Configuration::s_musicvolume = 64;
guint32 Configuration::s_musiccache = 10000000;
string Configuration::s_filename = "";
bool Configuration::s_see_opponents_stacks = false;
bool Configuration::s_see_opponents_production = false;
GameParameters::QuestPolicy Configuration::s_play_with_quests = GameParameters::ONE_QUEST_PER_PLAYER;
bool Configuration::s_hidden_map = false;
bool Configuration::s_diplomacy = false;
GameParameters::NeutralCities Configuration::s_neutral_cities = GameParameters::AVERAGE;
GameParameters::RazingCities Configuration::s_razing_cities = GameParameters::ALWAYS;
bool Configuration::s_intense_combat = false;
bool Configuration::s_military_advisor = false;
bool Configuration::s_random_turns = false;
GameParameters::QuickStartPolicy Configuration::s_quick_start = GameParameters::NO_QUICK_START;
bool Configuration::s_cusp_of_war = false;
bool Configuration::s_decorated = true;
bool Configuration::s_remember_recent_games = true;
bool Configuration::s_remember_recently_edited_files = true;
guint32 Configuration::s_double_click_threshold = 400; //milliseconds
std::string Configuration::s_gamelist_server_hostname = "lordsawar.com";
guint32 Configuration::s_gamelist_server_port = LORDSAWAR_GAMELIST_PORT;

Configuration::Configuration()
{
    char *s = setlocale(LC_ALL, "");
    if (s)
	Configuration::s_lang = s;
}

Configuration::~Configuration()
{
}

// check if file exists and parse it

bool Configuration::loadConfigurationFile(string fileName)
{
    debug("loadConfiguration()");
    s_filename=fileName;
     
    ifstream in(fileName.c_str());
    if (in)
    {
        //cout << _("Found configuration file: ") << fileName << endl;

        //parse the file
        XML_Helper helper(fileName.c_str(), ios::in, false);
        helper.registerTag(
	    "lordsawarrc",
	    sigc::mem_fun(*this, &Configuration::parseConfiguration));
    
        return helper.parse();
    }
    else return false;
}

bool Configuration::saveConfigurationFile(string filename)
{
    bool retval = true;

    XML_Helper helper(filename, ios::out, Configuration::s_zipfiles);

    //start writing
    retval &= helper.begin(LORDSAWAR_CONFIG_VERSION);
    retval &= helper.openTag("lordsawarrc");
    
    //save the values 
    retval &= helper.saveData("datapath",s_dataPath);
    retval &= helper.saveData("savepath", s_savePath);
    retval &= helper.saveData("lang", s_lang);
    retval &= helper.saveData("cachesize", s_cacheSize);
    retval &= helper.saveData("zipfiles", s_zipfiles);
    std::string autosave_policy_str = savingPolicyToString(SavingPolicy(s_autosave_policy));
    retval &= helper.saveData("autosave_policy", autosave_policy_str);
    retval &= helper.saveData("speeddelay", s_displaySpeedDelay);
    retval &= helper.saveData("fightrounddelayfast", s_displayFightRoundDelayFast);
    retval &= helper.saveData("fightrounddelayslow", s_displayFightRoundDelaySlow);
    retval &= helper.saveData("commentator", s_displayCommentator);
    retval &= helper.saveData("shownextplayer", s_showNextPlayer);
    retval &= helper.saveData("musicenable", s_musicenable);
    retval &= helper.saveData("musicvolume", s_musicvolume);
    retval &= helper.saveData("musiccache", s_musiccache);
    retval &= helper.saveData("view_enemies", s_see_opponents_stacks);
    retval &= helper.saveData("view_production", s_see_opponents_production);
    std::string quest_policy_str = questPolicyToString(GameParameters::QuestPolicy(s_play_with_quests));
    retval &= helper.saveData("quests", quest_policy_str);
    retval &= helper.saveData("hidden_map", s_hidden_map);
    retval &= helper.saveData("diplomacy", s_diplomacy);
    std::string neutral_cities_str = neutralCitiesToString(GameParameters::NeutralCities(s_neutral_cities));
    retval &= helper.saveData("neutral_cities", neutral_cities_str);
    std::string razing_cities_str = razingCitiesToString(GameParameters::RazingCities(s_razing_cities));
    retval &= helper.saveData("razing_cities", razing_cities_str);
    retval &= helper.saveData("intense_combat", s_intense_combat);
    retval &= helper.saveData("military_advisor", s_military_advisor);
    retval &= helper.saveData("random_turns", s_random_turns);
    std::string quick_start_str = quickStartPolicyToString(GameParameters::QuickStartPolicy(s_quick_start));
    retval &= helper.saveData("quick_start", quick_start_str);
    retval &= helper.saveData("cusp_of_war", s_cusp_of_war);
    retval &= helper.saveData("decorated", s_decorated);
    retval &= helper.saveData("remember_recent_games", s_remember_recent_games);
    retval &= helper.saveData("remember_recently_edited_files", s_remember_recently_edited_files);
    retval &= helper.saveData("double_click_threshold", 
			      s_double_click_threshold);
    retval &= helper.saveData("gamelist_server_hostname", 
			      s_gamelist_server_hostname);
    retval &= helper.saveData("gamelist_server_port", 
			      s_gamelist_server_port);
    retval &= helper.closeTag();
    
    if (!retval)
    {
        std::cerr << "Configuration: Something went wrong while saving.\n";
        return false;
    }
    
    //    retval &= helper.closeTag();
    helper.close();

    return true;
}

// parse the configuration file and set the variables

bool Configuration::parseConfiguration(string tag, XML_Helper* helper)
{
    debug("parseConfiguration()");
    
    string temp;
    bool retval,zipping;
    
    if (helper->getVersion() != LORDSAWAR_CONFIG_VERSION)
    {
            cerr <<_("Configuration file has wrong version, we want ");
            std::cerr <<LORDSAWAR_SAVEGAME_VERSION <<",\n";
            cerr <<_("Configuration file offers ") << helper->getVersion() <<".\n";

            string orig = s_filename;
            string dest = s_filename+".OLD";
	    //#ifndef __WIN32__
	    //            string orig = "./"+s_filename;
	    //            string dest = "./"+s_filename+".OLD";
	    //#else
	    //            string orig = string(getenv("HOME"))+s_filename;
	    //            string dest = string(getenv("HOME"))+s_filename+".OLD";
	    //#endif
            cerr << "I make a backup copy from " << orig << " to " << dest << endl;

            ofstream ofs(dest.c_str());
	    ifstream ifs(orig.c_str());
	    ofs << ifs.rdbuf();
	    ofs.close();

            //int ret= system(command.c_str());

            //if (ret == -1)
	    //{
            //     cerr << _("An error occurred while executing command : ") << command << endl;
            //     exit(-1);
	    //}
            return false;
    }
   
    //get the paths
    retval = helper->getData(temp, "datapath");
    if (retval)
    {
        s_dataPath = temp;
    }
        
    retval = helper->getData(temp, "savepath");
    if (retval)
    {
        s_savePath = temp;
    }

    if (helper->getData(temp, "lang"))
        s_lang = temp;
    
    //parse cache size
    retval = helper->getData(temp, "cachesize");
    if (retval)
        s_cacheSize = atoi(temp.c_str());

    //parse if savefiles should be zipped
    retval = helper->getData(zipping, "zipfiles");
    if (retval)
        s_zipfiles = zipping;

    //parse when and how to save autosave files
    std::string autosave_policy_str;
    helper->getData(autosave_policy_str, "autosave_policy");
    s_autosave_policy = savingPolicyFromString(autosave_policy_str);

    //parse the speed delays
    helper->getData(s_displaySpeedDelay, "speeddelay");
    helper->getData(s_displayFightRoundDelayFast, "fightrounddelayfast");
    helper->getData(s_displayFightRoundDelaySlow, "fightrounddelayslow");

    //parse whether or not the commentator should be shown
    helper->getData(s_displayCommentator, "commentator");

    //parse if nextplayer dialog should be enabled
    helper->getData(s_showNextPlayer, "shownextplayer");

    // parse musicsettings
    helper->getData(s_musicenable, "musicenable");
    helper->getData(s_musicvolume, "musicvolume");
    helper->getData(s_musiccache, "musiccache");
    
    helper->getData(s_see_opponents_stacks, "view_enemies");
    helper->getData(s_see_opponents_production, "view_production");
    std::string quest_policy_str;
    helper->getData(quest_policy_str, "quests");
    s_play_with_quests = questPolicyFromString(quest_policy_str);
    helper->getData(s_hidden_map, "hidden_map");
    helper->getData(s_diplomacy, "diplomacy");
    std::string neutral_cities_str;
    helper->getData(neutral_cities_str, "neutral_cities");
    s_neutral_cities = neutralCitiesFromString(neutral_cities_str);
    std::string razing_cities_str;
    helper->getData(razing_cities_str, "razing_cities");
    s_razing_cities = razingCitiesFromString(razing_cities_str);
    helper->getData(s_intense_combat, "intense_combat");
    helper->getData(s_military_advisor, "military_advisor");
    helper->getData(s_random_turns, "random_turns");
    std::string quick_start_str;
    helper->getData(quick_start_str, "quick_start");
    s_quick_start = quickStartPolicyFromString(quick_start_str);
    helper->getData(s_cusp_of_war, "cusp_of_war");
    helper->getData(s_decorated, "decorated");
    helper->getData(s_remember_recent_games, "remember_recent_games");
    helper->getData(s_remember_recently_edited_files, "remember_recently_edited_files");
    helper->getData(s_double_click_threshold, "double_click_threshold");
    helper->getData(s_gamelist_server_hostname, "gamelist_server_hostname");
    helper->getData(s_gamelist_server_port, "gamelist_server_port");
    return true;
}

void initialize_configuration()
{
    Configuration conf;

    bool foundconf = conf.loadConfigurationFile(Configuration::configuration_file_path);
    if (!foundconf)
    {
	bool saveconf = conf.saveConfigurationFile(Configuration::configuration_file_path);
	if (!saveconf)
	{
            std::cerr << "Couldn't save the new configuration file..." << std::endl;
            std::cerr << "Check permissions of your home directory....aborting!" << std::endl;
	    exit(-1);
	}
	else
	    std::cerr << "Created the standard configuration file " << Configuration::configuration_file_path << std::endl;
    }
    
    //Check if the save game directory exists. If not, try to create it.

    if (File::create_dir(Configuration::s_savePath) == false)
    {
        std::cerr << "Couldn't create save game directory ";
        std::cerr << Configuration::s_savePath <<".\n";
        std::cerr << "Check permissions and the entries in your lordsawarrc file!" << std::endl;
        exit(-1);
    }
    //Check if the personal armyset directory exists. If not, try to create it.
    if (File::create_dir(File::getUserArmysetDir()) == false)
    {
        std::cerr << "Couldn't create personal armyset directory ";
        std::cerr << File::getUserArmysetDir() <<".\n";
        std::cerr << "Check permissions and the entries in your lordsawarrc file!" << std::endl;
        exit(-1);
    }
    //Check if the personal tileset directory exists. If not, try to create it.
    if (File::create_dir(File::getUserTilesetDir()) == false)
    {
        std::cerr << "Couldn't create personal tileset directory ";
        std::cerr << File::getUserTilesetDir() <<".\n";
        std::cerr << "Check permissions and the entries in your lordsawarrc file!" << std::endl;
        exit(-1);
    }

    //Check if the personal maps directory exists. If not, try to create it.
    if (File::create_dir(File::getUserMapDir()) == false)
    {
        std::cerr << "Couldn't create personal map directory ";
        std::cerr << File::getUserMapDir() <<".\n";
        std::cerr << "Check permissions and the entries in your lordsawarrc file!" << std::endl;
        exit(-1);
    }

    //Check if the personal shieldset directory exists. If not, try to make it.
    if (File::create_dir(File::getUserShieldsetDir()) == false)
    {
        std::cerr << "Couldn't create personal shieldset directory ";
        std::cerr << File::getUserShieldsetDir() <<".\n";
        std::cerr << "Check permissions and the entries in your lordsawarrc file!" << std::endl;
        exit(-1);
    }

    //Check if the personal cityset directory exists. If not, try to make it.
    if (File::create_dir(File::getUserCitysetDir()) == false)
    {
        std::cerr << "Couldn't create personal cityset directory ";
        std::cerr << File::getUserCitysetDir() <<".\n";
        std::cerr << "Check permissions and the entries in your lordsawarrc file!" << std::endl;
        exit(-1);
    }
}

std::string Configuration::neutralCitiesToString(const GameParameters::NeutralCities neutrals)
{
  switch (neutrals)
    {
      case GameParameters::AVERAGE:
	return "GameParameters::AVERAGE";
	break;
      case GameParameters::STRONG:
	return "GameParameters::STRONG";
	break;
      case GameParameters::ACTIVE:
	return "GameParameters::ACTIVE";
	break;
      case GameParameters::DEFENSIVE:
	return "GameParameters::DEFENSIVE";
	break;
    }
  return "GameParameters::AVERAGE";
}

GameParameters::NeutralCities Configuration::neutralCitiesFromString(std::string str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return GameParameters::NeutralCities(atoi(str.c_str()));
  if (str == "GameParameters::AVERAGE")
    return GameParameters::AVERAGE;
  else if (str == "GameParameters::STRONG")
    return GameParameters::STRONG;
  else if (str == "GameParameters::ACTIVE")
    return GameParameters::ACTIVE;
  else if (str == "GameParameters::DEFENSIVE")
    return GameParameters::DEFENSIVE;
    
  return GameParameters::AVERAGE;
}

std::string Configuration::razingCitiesToString(const GameParameters::RazingCities razing)
{
  switch (razing)
    {
      case GameParameters::NEVER:
	return "GameParameters::NEVER";
	break;
      case GameParameters::ON_CAPTURE:
	return "GameParameters::ON_CAPTURE";
	break;
      case GameParameters::ALWAYS:
	return "GameParameters::ALWAYS";
	break;
    }
  return "GameParameters::ALWAYS";
}

GameParameters::RazingCities Configuration::razingCitiesFromString(std::string str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return GameParameters::RazingCities(atoi(str.c_str()));
  if (str == "GameParameters::NEVER")
    return GameParameters::NEVER;
  else if (str == "GameParameters::ON_CAPTURE")
    return GameParameters::ON_CAPTURE;
  else if (str == "GameParameters::ALWAYS")
    return GameParameters::ALWAYS;
    
  return GameParameters::ALWAYS;
}

std::string Configuration::savingPolicyToString(const Configuration::SavingPolicy policy)
{
  switch (policy)
    {
    case Configuration::NO_SAVING:
      return "Configuration::NO_SAVING";
      break;
    case Configuration::WRITE_UNNUMBERED_AUTOSAVE_FILE:
      return "Configuration::WRITE_UNNUMBERED_AUTOSAVE_FILE";
      break;
    case Configuration::WRITE_NUMBERED_AUTOSAVE_FILE:
      return "Configuration::WRITE_NUMBERED_AUTOSAVE_FILE";
      break;
    }
  return "Configuration::NO_SAVING";
}

Configuration::SavingPolicy Configuration::savingPolicyFromString(std::string str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Configuration::SavingPolicy(atoi(str.c_str()));
  if (str == "Configuration::NO_SAVING")
    return Configuration::NO_SAVING;
  else if (str == "Configuration::WRITE_UNNUMBERED_AUTOSAVE_FILE")
    return Configuration::WRITE_UNNUMBERED_AUTOSAVE_FILE;
  else if (str == "Configuration::WRITE_NUMBERED_AUTOSAVE_FILE")
    return Configuration::WRITE_NUMBERED_AUTOSAVE_FILE;
    
  return Configuration::WRITE_NUMBERED_AUTOSAVE_FILE;
}

std::string Configuration::quickStartPolicyToString(const GameParameters::QuickStartPolicy policy)
{
  switch (policy)
    {
    case GameParameters::NO_QUICK_START:
      return "GameParameters::NO_QUICK_START";
      break;
    case GameParameters::EVENLY_DIVIDED:
      return "GameParameters::EVENLY_DIVIDED";
      break;
    case GameParameters::AI_HEAD_START:
      return "GameParameters::AI_HEAD_START";
      break;
    }
  return "GameParameters::NO_QUICK_START";
}

GameParameters::QuickStartPolicy Configuration::quickStartPolicyFromString(std::string str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return GameParameters::QuickStartPolicy(atoi(str.c_str()));
  if (str == "GameParameters::NO_QUICK_START")
    return GameParameters::NO_QUICK_START;
  else if (str == "GameParameters::EVENLY_DIVIDED")
    return GameParameters::EVENLY_DIVIDED;
  else if (str == "GameParameters::AI_HEAD_START")
    return GameParameters::AI_HEAD_START;
    
  return GameParameters::NO_QUICK_START;
}

std::string Configuration::questPolicyToString(const GameParameters::QuestPolicy quest)
{
  switch (quest)
    {
      case GameParameters::NO_QUESTING:
	return "GameParameters::NO_QUESTING";
	break;
      case GameParameters::ONE_QUEST_PER_PLAYER:
	return "GameParameters::ONE_QUEST_PER_PLAYER";
	break;
      case GameParameters::ONE_QUEST_PER_HERO:
	return "GameParameters::ONE_QUEST_PER_HERO";
	break;
    }
  return "GameParameters::NO_QUESTING";
}

GameParameters::QuestPolicy Configuration::questPolicyFromString(std::string str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return GameParameters::QuestPolicy(atoi(str.c_str()));
  if (str == "GameParameters::NO_QUESTING")
    return GameParameters::NO_QUESTING;
  else if (str == "GameParameters::ONE_QUEST_PER_PLAYER")
    return GameParameters::ONE_QUEST_PER_PLAYER;
  else if (str == "GameParameters::ONE_QUEST_PER_HERO")
    return GameParameters::ONE_QUEST_PER_HERO;
    
  return GameParameters::NO_QUESTING;
}

