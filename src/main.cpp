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
#include "recently-played-game-list.h"

#include "gui/main.h"


using namespace std;


int main(int argc, char* argv[])
{
  srand(time(NULL));         // set the random seed

  initialize_configuration();
  RecentlyPlayedGameList::getInstance()->loadFromFile(File::getSavePath() + "/recently-played.xml");

  #if ENABLE_NLS
  cout << "Configuration::s_lang.c_str(): " << Configuration::s_lang.c_str() << endl;
  //cout << "setlocale: " << setlocale(LC_ALL, Configuration::s_lang.c_str()) << endl;  //Not working?
  cout << "setlocale: " << setlocale(LC_ALL, "") << endl;
  cout << "bindtextdomain: " << bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR) << endl;
  cout << "bind_textdomain_codeset: " << bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8") << endl;
  cout << "textdomain: " << textdomain (GETTEXT_PACKAGE) << endl;
  #endif

  Main kit(argc, argv);

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
	  else if (parameter == "--seed" || parameter == "-S")
	    {
	      i++;
	      //convert the next argument
	      char* error = 0;
	      long seed = strtol(argv[i-1], &error, 10);
	      if (error && (*error != '\0'))
		{
		  cerr <<_("non-numerical value for seed value\n");
		  exit(-1);
		}
	      srand(seed);
	    }
	  else if (parameter == "--turn")
	    {
	      i++;
	      kit.turn_filename = argv[i-1];
	    }
	  else if (parameter == "--test" || parameter == "-t")
	    {
	      kit.start_test_scenario = true;
	    }
	  else if (parameter == "--stress-test" || parameter == "-s")
	    {
	      kit.start_stress_test = true;
	    }
	  else if (parameter == "--robots" || parameter == "-r")
	    {
	      kit.start_robots = -1;
	    }
	  else if (parameter == "--record" || parameter == "-R")
	    {
	      i++;
	      kit.record = argv[i-1];
	    }
	  else if (parameter == "--help" || parameter == "-h")
	    {
	      cout << argv[0] << " [OPTION]... [FILE]" << endl << endl;
	      cout << "LordsAWar! version " << VERSION << endl << endl;
	      cout << _("Options:") << endl << endl; 
	      cout << _("  -h, --help                 Shows this help screen") <<endl;
	      cout << _("  -c <size>                  Set the cache size for imagery to SIZE bytes") <<endl;
	      cout << _("  -t, --test                 Start with a test-scenario") << endl;
	      cout << _("  -S, --seed <number>        Seed the random number generator with NUMBER") << endl;
	      cout << _("  -s, --stress-test          Non-interactive stress test") << endl;
	      cout << _("  -r, --robots               Non-interactive network stress test") << endl;
	      cout << _("  -R, --record FILE          Record gameplay to FILE") << endl;
	      cout << endl;
	      cout << "FILE can be a saved game file (.sav), or a map (.map) file." << endl;
	      cout << endl;
	      cout << "Report bugs to <" << PACKAGE_BUGREPORT ">." << endl;
	      exit(0);
	    }
	  else
	    kit.load_filename = parameter;
	}
    }

  if (kit.load_filename != "" && kit.start_test_scenario)
    {
      cerr <<"Error: Cannot specify -t and have a file specified." << endl;
      exit (1);
    }

  if (kit.load_filename != "" && kit.start_stress_test)
    {
      cerr <<"Error: Cannot specify -s and have a file specified." << endl;
      exit (1);
    }

  if (kit.start_stress_test && kit.start_test_scenario)
    {
      cerr <<"Error: Cannot specify -s and -t simultaneously." << endl;
      exit (1);
    }

  if (kit.turn_filename != "" && kit.load_filename == "")
    {
      cerr <<"Error: Must specify a file to load when specifying --turn." << endl;
      exit (1);
    }


  // Check if armysets are in the path (otherwise exit)
  File::scanArmysets();
  File::scanTilesets();
  File::scanShieldsets();
  File::scanCitysets();


  kit.start_main_loop();

  return EXIT_SUCCESS;
}
