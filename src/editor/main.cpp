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

#include <pgapplication.h>
#include <pglog.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <physfs.h>

#include "../defs.h"
#include "../Configuration.h"
#include "../File.h"
#include "MainScreen.h"
#include "config.h"

using namespace std;

SDL_Rect* getBestResolution(Uint32 mymode)
{
    SDL_Rect **modes;
    int i;

    // Get available fullscreen/hardware modes
    modes = SDL_ListModes(NULL, mymode);

    // Check if there are any modes available
    if(modes == (SDL_Rect **)0)
    {
        cerr << _("No modes available!\n");
        exit(-1);
    }

    // Check if any resolution is ok
    if(modes == (SDL_Rect **)-1)
    {
        cout << _("All resolutions available.\n");
        return 0;
    }
    else
    {
        // Print valid modes
        cout << _("\nAvailable Modes\n");

        for(i = 0; modes[i]; ++i)
        {
            cout << _("Resolution = ") << modes[i]->w << "x" << modes[i]->h << endl;
        }
        if ((i == 0) && (modes[i]->w == 640))
        {
            cout << _("You need at least 800 x 600 resolution to run the editor\n");
            exit(-1);
        }
    }

    return modes[0];    // return the highest resolution
}


int main(int argc, char* argv[])
{
    char version[10];
    strcpy(version,VERSION);
    char theme[20];
    strcpy(theme, "freelords");
    srand(time(NULL));         // set the random seed

    Configuration conf;

    // we check for ~/.freelordsrc file
    bool foundconf = false;
    bool saveconf = false;

#ifndef __WIN32__
    // read the environment variable HOME
    char* home = getenv("HOME");
    foundconf |= conf.loadConfigurationFile(std::string(home) + "/.freelordsrc");

    if (!foundconf)
    {
    	std::cerr <<_("Couldn't find any good configuration file...trying auto-configuration.") << std::endl;

        // we did not find the configuration file so we create the default one
        saveconf |= conf.saveConfigurationFile(std::string(home) + "/.freelordsrc");
        if (!saveconf) 
	{
            std::cerr <<_("Couldn't save the new configuration file...") << std::endl;
            std::cerr <<_("Check permissions of your home directory....aborting!\n");
	    exit(-1);
	}

    	std::cerr <<_("Created the standard configuration file ") << std::string(home) + "/.freelordsrc" << std::endl;
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
            std::cerr <<_("Check permissions and the entries in your freelordsrc file!\n");
            exit(-1);
        }
    }
#else
    foundconf |= conf.loadConfigurationFile("./freelordsrc");

    if (!foundconf)
    {
    	std::cerr <<_("Couldn't find any good configuration file...trying auto-configuration.") << std::endl;

        // we did not find the configuration file so we create the default one
        saveconf |= conf.saveConfigurationFile("./freelordsrc");
        if (!saveconf) 
	{
            std::cerr <<_("Couldn't save the new configuration file...") << std::endl;
            std::cerr <<_("Check permissions of your home directory....aborting!\n");
	    exit(-1);
	}
    	std::cerr <<_("Created the standard configuration file ") << "./freelordsrc" << std::endl;
    }    
#endif

    setlocale (LC_ALL, Configuration::s_lang.c_str());

#ifndef __WIN32__
    bindtextdomain ("freelords",PO_PATH);
#else
    bindtextdomain ("freelords","./locale/");
#endif
    textdomain ("freelords");

    // the editor should always save the files unzipped
    Configuration::s_zipfiles = false;

    if (argc > 1)
    {
        for (int i = 2; i <= argc; i++)
        {
            string parameter(argv[i-1]); 
            if (parameter == "--fullscreen" || parameter == "-f")
            {
                Configuration::s_flags |= SDL_FULLSCREEN;
            }
            if (parameter == "-r1280")
            {
                Configuration::s_width = 1280;
                Configuration::s_height = 1024;
            }
            if (parameter == "-r1024")
            {
                Configuration::s_width = 1024;
                Configuration::s_height = 786;
            }
            if (parameter == "-r800")
            {
                Configuration::s_width = 800;
                Configuration::s_height = 600;
            }
            if (parameter == "--help" || parameter == "-h")
            {
                cout << endl;
                cout << "FreeLords " << version << endl << endl;
                cout << _("Available parameters:") << endl << endl; 
                cout << _("-h,      --help             Shows this help screen\n");
                cout << _("-r1280                      Resolution set to 1280x1024") << endl;
                cout << _("-r1024                      Resolution set to 1024x786") << endl;
                cout << _("-r800                       Resolution set to 800x600") << endl;
                cout << _("-f,      --fullscreen         Fullscreen mode") << endl;
                cout << _("Please send bug reports to ulf82@users.sf.net") << endl << endl;
                exit(0);
            }
        }
    }

    //quick hack: set the log level to a low niveau. Background: I often run
    //paragui with --enable-debug, but then the console is flooded with
    //debug messages. To prevent this, I decrease the log level.
    PG_LogConsole::SetLogLevel(PG_LOG_MSG);
    
    PG_Application app;
    app.SetEmergencyQuit(false);
    
    char buf[101]; buf[100] = '\0';
    snprintf(buf, 100, "%s/theme", Configuration::s_dataPath.c_str());
    //    if (!app.LoadTheme(theme, true, Configuration::s_dataPath.c_str()))
    if (!app.LoadTheme(theme, true, buf))
    {
        cerr << _("Unable to load theme: ") << theme << endl;
        exit(-1);
    }
    
    // Check if armysets are in the path (otherwise exit)
    File::scanArmysets();

    app.SetFontSize(14);

    // if resolution hasn't been set explicitly in the configuration file
    // or on the commandline guess the best resolution
    if (Configuration::s_width == -1)
    {
        cout << _("no resolution provided: guessing") << endl;
        SDL_Rect* mode = getBestResolution(Configuration::s_flags &
                                           Configuration::s_surfaceFlags);

        // take the best mode
        if (mode != 0)
        {
            Configuration::s_width = mode->w;
            Configuration::s_height = mode->h;
           }
        // couldn't guess a mode - most probably windowed mode
        // set minimum of 800x600
        else
        {
            Configuration::s_width = 800;
            Configuration::s_height = 600;
        }
       }       

    cout << _("Using resolution: ") << Configuration::s_width << "x" << Configuration::s_height << endl;    
    // crude hack: add the path of the icon and load it. This should be
    // generalized a bit (one could use more physfs)
    PHYSFS_addToSearchPath(Configuration::s_dataPath.c_str(), true);
    app.SetIcon("various/editor.png"); 

    if (!app.InitScreen(Configuration::s_width, Configuration::s_height, 16,
                        Configuration::s_flags))
    {
        cerr << _("Couldn't initialize X Windowing System\n");
        exit(1);
    }

    char title[51]; title[50] = '\0';
    snprintf(title, 50, _("FreeLords Editor %s"), version);
        
    SDL_WM_SetCaption(title, "");

    E_MainScreen screen(&app, PG_Rect(0, 0, Configuration::s_width, Configuration::s_height));
    screen.Show();
    
    app.Run();

    return EXIT_SUCCESS;
}
