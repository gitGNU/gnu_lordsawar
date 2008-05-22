// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
// Copyright (C) 2005, 2006 Josef Spillner
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
            if (parameter == "--help" || parameter == "-h")
            {
                cout << endl;
                cout << "LordsAWar " << FL_VERSION << endl << endl;
                cout << _("Available parameters:") << endl << endl; 
                cout << _("-h,      --help             Shows this help screen\n");
                cout << _("-c <size>                   Set the maximum cache size") <<endl;
                cout << _("-t,      --test             Starting with a test-scenario") << endl;
		cout << endl << endl;
                exit(0);
            }
        }
    }

    // Check if armysets are in the path (otherwise exit)
    File::scanArmysets();
    File::scanTilesets();
    File::scanShieldsets();
    File::scanCitysets();


    kit.start_main_loop();
    
    return EXIT_SUCCESS;
}
