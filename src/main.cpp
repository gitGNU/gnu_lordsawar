// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
// Copyright (C) 2005, 2006 Josef Spillner
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
#include "cityset.h"
#include "tileset.h"
#include "shieldset.h"
#include "armyset.h"

#include "gui/main.h"


using namespace std;


int max_vector_width;
int main(int argc, char* argv[])
{
  srand(time(NULL));         // set the random seed

  initialize_configuration();
  Vector<int>::setMaximumWidth(1000);
  RecentlyPlayedGameList::getInstance()->loadFromFile(File::getSavePath() + "/" + RECENTLY_PLAYED_LIST);

  #if ENABLE_NLS
  //cout << "Configuration::s_lang.c_str(): " << Configuration::s_lang.c_str() << endl;
  setlocale(LC_ALL, Configuration::s_lang.c_str());
  //setlocale(LC_ALL, "");
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
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
		  cerr <<_("non-numerical value for cache size") <<endl;
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
		  cerr <<_("non-numerical value for seed value") <<endl;
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
	  else if (parameter == "--help" || parameter == "-h")
	    {
	      cout << argv[0] << " [OPTION]... [FILE]" << endl << endl;
	      cout << "LordsAWar! " << _("version") << " " << VERSION << endl << endl;
	      cout << _("Options:") << endl << endl; 
	      cout << "  -h, --help                 " << _("Shows this help screen") <<endl;
	      cout << "  -c <size>                  " << _("Set the cache size for imagery to SIZE bytes") <<endl;
	      cout << "  -t, --test                 " << _("Start with a test-scenario") << endl;
	      cout << "  -S, --seed <number>        " << _("Seed the random number generator with NUMBER") << endl;
	      cout << "  -s, --stress-test          " << _("Non-interactive stress test") << endl;
	      cout << "  -r, --robots               " << _("Non-interactive network stress test") << endl;
	      cout << endl;
	      cout << _("FILE can be a saved game file (.sav), or a map (.map) file.") << endl;
	      cout << endl;
	      cout << _("Report bugs to") << " <" << PACKAGE_BUGREPORT ">." << endl;
	      exit(0);
	    }
	  else
	    kit.load_filename = parameter;
	}
    }

  if (kit.load_filename != "" && kit.start_test_scenario)
    {
      cerr <<_("Error: Cannot specify -t and have a file specified.") << endl;
      exit (1);
    }

  if (kit.load_filename != "" && kit.start_stress_test)
    {
      cerr <<_("Error: Cannot specify -s and have a file specified.") << endl;
      exit (1);
    }

  if (kit.start_stress_test && kit.start_test_scenario)
    {
      cerr <<_("Error: Cannot specify -s and -t simultaneously.") << endl;
      exit (1);
    }

  if (kit.turn_filename != "" && kit.load_filename == "")
    {
      cerr <<_("Error: Must specify a file to load when specifying --turn.") << endl;
      exit (1);
    }


  // Check if armysets are in the path (otherwise exit)
  Armyset::scanSystemCollection();
  Tileset::scanSystemCollection();
  Shieldset::scanSystemCollection();
  Cityset::scanSystemCollection();


  kit.start_main_loop();

  return EXIT_SUCCESS;
}
