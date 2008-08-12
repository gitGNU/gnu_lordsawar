//  Copyright (C) 2002, 2003 Michael Bartl
//  Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
//  Copyright (C) 2004, 2005, 2006 Andrea Paternesi
//  Copyright (C) 2005 Josef Spillner
//  Copyright (C) 2006, 2007, 2008 Ben Asselstine
//  Copyright (C) 2007 Ole Laursen
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
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <sigc++/functors/mem_fun.h>

#include "Configuration.h"

#include "defs.h"
#include "string_tokenizer.h"
#include "xmlhelper.h"

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
int Configuration::s_displaySpeedDelay = 0;
int Configuration::s_displayFightRoundDelayFast = 250;
int Configuration::s_displayFightRoundDelaySlow = 500;
Uint32 Configuration::s_surfaceFlags = SDL_SWSURFACE;
Uint32 Configuration::s_cacheSize = 1000000;
bool Configuration::s_zipfiles = false;
int Configuration::s_autosave_policy = 1;
bool Configuration::s_hardware = false;
bool Configuration::s_ggz = false;
bool Configuration::s_musicenable = true;
Uint32 Configuration::s_musicvolume = 64;
Uint32 Configuration::s_musiccache = 10000000;
string Configuration::s_filename = "";
bool Configuration::s_see_opponents_stacks = false;
bool Configuration::s_see_opponents_production = false;
bool Configuration::s_play_with_quests = true;
bool Configuration::s_hidden_map = false;
bool Configuration::s_diplomacy = false;
GameParameters::NeutralCities Configuration::s_neutral_cities = GameParameters::AVERAGE;
GameParameters::RazingCities Configuration::s_razing_cities = GameParameters::ALWAYS;
bool Configuration::s_intense_combat = false;
bool Configuration::s_military_advisor = false;
bool Configuration::s_random_turns = false;
bool Configuration::s_quick_start = false;
bool Configuration::s_cusp_of_war = false;
bool Configuration::s_decorated = true;

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
    retval &= helper.saveData("hardware", s_hardware);
    retval &= helper.saveData("zipfiles", s_zipfiles);
    retval &= helper.saveData("autosave_policy", s_autosave_policy);
    retval &= helper.saveData("speeddelay", s_displaySpeedDelay);
    retval &= helper.saveData("fightrounddelayfast", s_displayFightRoundDelayFast);
    retval &= helper.saveData("fightrounddelayslow", s_displayFightRoundDelaySlow);
    retval &= helper.saveData("shownextplayer", s_showNextPlayer);
    retval &= helper.saveData("musicenable", s_musicenable);
    retval &= helper.saveData("musicvolume", s_musicvolume);
    retval &= helper.saveData("musiccache", s_musiccache);
    retval &= helper.saveData("view_enemies", s_see_opponents_stacks);
    retval &= helper.saveData("view_production", s_see_opponents_production);
    retval &= helper.saveData("quests", s_play_with_quests);
    retval &= helper.saveData("hidden_map", s_hidden_map);
    retval &= helper.saveData("diplomacy", s_diplomacy);
    std::string neutral_cities_str = neutralCitiesToString(GameParameters::NeutralCities(s_neutral_cities));
    retval &= helper.saveData("neutral_cities", neutral_cities_str);
    std::string razing_cities_str = razingCitiesToString(GameParameters::RazingCities(s_razing_cities));
    retval &= helper.saveData("razing_cities", razing_cities_str);
    retval &= helper.saveData("intense_combat", s_intense_combat);
    retval &= helper.saveData("military_advisor", s_military_advisor);
    retval &= helper.saveData("random_turns", s_random_turns);
    retval &= helper.saveData("quick_start", s_quick_start);
    retval &= helper.saveData("cusp_of_war", s_cusp_of_war);
    retval &= helper.saveData("decorated", s_decorated);
    retval &= helper.closeTag();
    
    if (!retval)
    {
        std::cerr <<_("Configuration: Something went wrong while saving.\n");
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
    int autosave_policy;
    
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
            cerr <<_("I make a backup copy from ") << orig << " to " << dest << endl;

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

    //parse surface flags
    retval = helper->getData(s_hardware, "hardware");
    if (retval)
    {
        if (s_hardware)
        {
            s_surfaceFlags &= ~(SDL_SWSURFACE);
            s_surfaceFlags |= SDL_HWSURFACE;
        }
        else
        {
            s_surfaceFlags &= ~(SDL_HWSURFACE);
            s_surfaceFlags |= SDL_SWSURFACE;
        }
    }

    //parse if savefiles should be zipped
    retval = helper->getData(zipping, "zipfiles");
    if (retval)
        s_zipfiles = zipping;

    //parse when and how to save autosave files
    retval = helper->getData(autosave_policy, "autosave_policy");
    if (retval)
        s_autosave_policy = autosave_policy;

    //parse the speed delays
    helper->getData(s_displaySpeedDelay, "speeddelay");
    helper->getData(s_displayFightRoundDelayFast, "fightrounddelayfast");
    helper->getData(s_displayFightRoundDelaySlow, "fightrounddelayslow");

    //parse if nextplayer dialog should be enabled
    helper->getData(s_showNextPlayer, "shownextplayer");

    // parse musicsettings
    helper->getData(s_musicenable, "musicenable");
    helper->getData(s_musicvolume, "musicvolume");
    helper->getData(s_musiccache, "musiccache");
    
    helper->getData(s_see_opponents_stacks, "view_enemies");
    helper->getData(s_see_opponents_production, "view_production");
    helper->getData(s_play_with_quests, "quests");
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
    helper->getData(s_quick_start, "quick_start");
    helper->getData(s_cusp_of_war, "cusp_of_war");
    helper->getData(s_decorated, "decorated");
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
            std::cerr << _("Couldn't save the new configuration file...") << std::endl;
            std::cerr << _("Check permissions of your home directory....aborting!") << std::endl;
	    exit(-1);
	}
	else
	    std::cerr <<_("Created the standard configuration file ") << Configuration::configuration_file_path << std::endl;
    }
    
#ifndef __WIN32__
    //Check if the save game directory exists. If not, try to create it.
    struct stat testdir;

    if (stat(Configuration::s_savePath.c_str(), &testdir)
        || !S_ISDIR(testdir.st_mode))
    {
        Uint32 mask = 0755; //make directory only readable for user and group
        if (mkdir(Configuration::s_savePath.c_str(), mask))
        {
            std::cerr <<_("Couldn't create save game directory ");
            std::cerr << Configuration::s_savePath <<".\n";
            std::cerr <<_("Check permissions and the entries in your lordsawarrc file!") << std::endl;
            exit(-1);
        }
    }
#endif
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
