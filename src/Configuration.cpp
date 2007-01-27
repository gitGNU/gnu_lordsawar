//  This program is free software; you can redistribute it and/or modiry
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

#include "Configuration.h"
#include "defs.h"
#include <fstream>
#include <iostream>
#include<fstream>
#include <stdlib.h>
#include "string_tokenizer.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

// define static variables

bool Configuration::s_smoothScrolling = false;
bool Configuration::s_showNextPlayer = true;
#ifndef __WIN32__
string Configuration::s_dataPath = FREELORDS_DATADIR;
string Configuration::s_savePath = string(getenv("HOME"))+string("/.freelords/");
#else
string Configuration::s_dataPath = "./data/";
string Configuration::s_savePath = "./saves/";
#endif
string Configuration::s_lang = setlocale(LC_ALL, "");
int Configuration::s_width = 800;
int Configuration::s_height = 600;
int Configuration::s_displaySpeedDelay = 0;
Uint32 Configuration::s_flags = 0;
Uint32 Configuration::s_surfaceFlags = SDL_SWSURFACE;
Uint32 Configuration::s_cacheSize = 1000000;
bool Configuration::s_zipfiles = false;
bool Configuration::s_hardware = false;
bool Configuration::s_ggz = false;
bool Configuration::s_fullScreen = false;
bool Configuration::s_musicenable = true;
Uint32 Configuration::s_musicvolume = 64;
Uint32 Configuration::s_musiccache = 10000000;
string Configuration::s_filename = "";

Configuration::Configuration()
{ 
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
        cout << _("Found configuration file: ") << fileName << endl;

        //parse the file
        XML_Helper helper(fileName.c_str(), ios::in, false);
        helper.registerTag("freelordsrc", SigC::slot(*this, 
                    &Configuration::parseConfiguration));
    
        return helper.parse();
    }
    else return false;
}

bool Configuration::saveConfigurationFile(string filename)
{
    bool retval = true;
    char stmp[10];
    sprintf(stmp,"%dx%d",s_width,s_height);

    XML_Helper helper(filename, ios::out, Configuration::s_zipfiles);

    //start writing
    retval &= helper.begin(FREELORDS_CONFIG_VERSION);
    retval &= helper.openTag("freelordsrc");
    
    //save the values 
    retval &= helper.saveData("datapath",s_dataPath);
    retval &= helper.saveData("savepath", s_savePath);
    retval &= helper.saveData("lang", s_lang);
    retval &= helper.saveData("resolution",string(stmp));
    retval &= helper.saveData("fullscreen", s_fullScreen);
    retval &= helper.saveData("cachesize", s_cacheSize);
    retval &= helper.saveData("hardware", s_hardware);
    retval &= helper.saveData("zipfiles", s_zipfiles);
    retval &= helper.saveData("smoothscrolling", s_smoothScrolling);
    retval &= helper.saveData("speeddelay", s_displaySpeedDelay);
    retval &= helper.saveData("shownextplayer", s_showNextPlayer);
    retval &= helper.saveData("musicenable", s_musicenable);
    retval &= helper.saveData("musicvolume", s_musicvolume);
    retval &= helper.saveData("musiccache", s_musiccache);
    
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
    
    if (helper->getVersion() != FREELORDS_CONFIG_VERSION)
    {
            cerr <<_("Configuration file has wrong version, we want ");
            std::cerr <<FREELORDS_SAVEGAME_VERSION <<",\n";
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

    //fullscreen?
    retval = helper->getData(s_fullScreen, "fullscreen");
    if (retval)
    {
        if (s_fullScreen)
        {
            s_flags |= SDL_FULLSCREEN;
        }
        else
        {
            s_flags &= ~SDL_FULLSCREEN;
        }
    }

    // parse resolution
    retval = helper->getData(temp, "resolution");
    if (retval)
    {
        string::size_type idx;
        idx = temp.find("x");
        if (idx != string::npos)
        {
            string width = temp.substr(0, idx);
            s_width = atoi(width.c_str());
            string height = temp.substr(idx+1, temp.length());
            s_height = atoi(height.c_str());
        }
    }

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

    //parse if smoothscrolling should be enabled and the speed delay
    helper->getData(s_smoothScrolling, "smoothscrolling");
    helper->getData(s_displaySpeedDelay, "speeddelay");

    //parse if nextplayer dialog should be enabled
    helper->getData(s_showNextPlayer, "shownextplayer");

    // parse musicsettings
    helper->getData(s_musicenable, "musicenable");
    helper->getData(s_musicvolume, "musicvolume");
    helper->getData(s_musiccache, "musiccache");
    
    return true;
}

// End of file
