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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <sys/stat.h>

#include "Configuration.h"
#include "File.h"
#include "GraphicsCache.h"

#include "gui/main.h"


using namespace std;


int main(int argc, char* argv[])
{
    char version[10];
    strcpy(version,FL_VERSION);
    char theme[20];
    strcpy(theme, "lordsawar");
    srand(time(NULL));         // set the random seed

    Configuration conf;

#ifdef FL_NO_TIMERS
    std::cerr << "timers??????????? =" << FL_NO_TIMERS << std::endl;
#endif

    // we check for ~/.lordsawarrc file
    bool foundconf = false;
    bool saveconf = false;

    // FIXME: clean up this mess, move it somewhere else
#ifndef __WIN32__
    // read the environment variable HOME
    char* home = getenv("HOME");
    foundconf |= conf.loadConfigurationFile(std::string(home) + "/.lordsawarrc");

    if (!foundconf)
    {
    	std::cerr <<_("Couldn't find any good configuration file...trying auto-configuration.") << std::endl;

        // we did not find the configuration file so we create the default one
        saveconf |= conf.saveConfigurationFile(std::string(home) + "/.lordsawarrc");
        if (!saveconf) 
	{
            std::cerr <<_("Couldn't save the new configuration file...") << std::endl;
            std::cerr <<_("Check permissions of your home directory....aborting!\n");
	    exit(-1);
	}

    	std::cerr <<_("Created the standard configuration file ") << std::string(home) + "/.lordsawarrc" << std::endl;
    }

    //Check if the save game directory exists. If not, try to create it.
    struct stat testdir;

    if (stat(Configuration::s_savePath.c_str(), &testdir)
        || !S_ISDIR(testdir.st_mode))
    {
        Uint32 mask = 0755; //make directory only readable for user and group
        if (mkdir(Configuration::s_savePath.c_str(), mask))
        {
            std::cerr <<_("Couldn't create save game directory ");
            std::cerr <<Configuration::s_savePath <<".\n";
            std::cerr <<_("Check permissions and the entries in your lordsawarrc file!\n");
            exit(-1);
        }
    }
#else
    foundconf |= conf.loadConfigurationFile("./lordsawarrc");

    if (!foundconf)
    {
    	std::cerr <<_("Couldn't find any good configuration file...trying auto-configuration.") << std::endl;

        // we did not find the configuration file so we create the default one
        saveconf |= conf.saveConfigurationFile("./lordsawarrc");
        if (!saveconf) 
	{
            std::cerr <<_("Couldn't save the new configuration file...") << std::endl;
            std::cerr <<_("Check permissions of your home directory....aborting!\n");
	    exit(-1);
	}
    	std::cerr <<_("Created the standard configuration file ") << "./lordsawarrc" << std::endl;
    }    
#endif

    setlocale(LC_ALL, Configuration::s_lang.c_str());
#ifndef __WIN32__
    bindtextdomain ("lordsawar",PO_PATH);
#else
    bindtextdomain ("lordsawar","./locale/");
#endif

    Main kit(argc, argv);

    textdomain ("lordsawar");
    if (argc > 1)
    {
        for (int i = 2; i <= argc; i++)
        {
            string parameter(argv[i-1]); 
            if (parameter == "-c")
            {
                i++;
                //convert the next argument
                char* error = 0;
                long size = strtol(argv[i-1], &error, 10);
                if (error && (*error != '\0'))
                {
                    cerr <<_("non-numerical value for cache size\n");
                    exit(-1);
                }
                Configuration::s_cacheSize = size;
            }
            if (parameter == "--fullscreen" || parameter == "-f")
            {
                Configuration::s_flags |= SDL_FULLSCREEN;
            }
            if (parameter == "--test" || parameter == "-t")
            {
                kit.start_test_scenario = true;
            }
#ifdef WITH_GGZ
            if (parameter == "-g" || parameter == "--ggz")
            {
                Configuration::s_ggz = true;
            }
#endif 
            if (parameter == "--help" || parameter == "-h")
            {
                cout << endl;
                cout << "LordsAWar " << version << endl << endl;
                cout << _("Available parameters:") << endl << endl; 
                cout << _("-h,      --help             Shows this help screen\n");
                cout << _("-f,      --fullscreen       Fullscreen mode") << endl;
                cout << _("-c <size>                   set the maximum cache size") <<endl;
                cout << _("-t,      --test             Starting with a test-scenario") << endl;
#ifdef WITH_GGZ
                // deprecated, but should stay reserved for future use
                //cout << _("-g,      --ggz              Run game in GGZ mode") << endl;
#endif
		cout << endl << endl;
                cout << _("Please send bug reports to ulf82@users.sf.net") << endl << endl;
                exit(0);
            }
        }
    }

    // New GGZ versions (>= 0.0.12) support the GGZMODE environment variable
    // It is recommended over the -g/--ggz usage, which will be kept for compatibility
#ifdef WITH_GGZ
    char* ggzmode = getenv("GGZMODE");
    if(ggzmode)
    {
        cout << _("Detected GGZ Gaming Zone environment") << endl;
        Configuration::s_ggz = true;
    }
#endif

    // Check if armysets are in the path (otherwise exit)
    File::scanArmysets();

    char title[50];
    sprintf(title, "LordsAWar %s", version);


    kit.start_main_loop();
    
    return EXIT_SUCCESS;
}
