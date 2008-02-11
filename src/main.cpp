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

#include "Configuration.h"
#include "File.h"
#include "GraphicsCache.h"

#include "gui/main.h"


using namespace std;


int main(int argc, char* argv[])
{
    srand(time(NULL));         // set the random seed

    initialize_configuration();

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
                cout << "LordsAWar " << FL_VERSION << endl << endl;
                cout << _("Available parameters:") << endl << endl; 
                cout << _("-h,      --help             Shows this help screen\n");
                cout << _("-c <size>                   Set the maximum cache size") <<endl;
                cout << _("-t,      --test             Starting with a test-scenario") << endl;
#ifdef WITH_GGZ
                // deprecated, but should stay reserved for future use
                //cout << _("-g,      --ggz              Run game in GGZ mode") << endl;
#endif
		cout << endl << endl;
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
    File::scanTilesets();
    File::scanShieldsets();
    File::scanCitysets();


    kit.start_main_loop();
    
    return EXIT_SUCCESS;
}
