// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009, 2011, 2014, 2015 Ben Asselstine
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

#include <config.h>

#include <iostream>

#ifdef LW_SOUND
#include <gstreamermm/init.h>
#endif

#include "Configuration.h"
#include "File.h"
#include "recently-played-game-list.h"
#include "citysetlist.h"
#include "tilesetlist.h"
#include "shieldsetlist.h"
#include "armysetlist.h"
#include "profilelist.h"
#include "gamelist.h"
#include "file-compat.h"
#include "gui/builder-cache.h"

#include "gui/main.h"


int max_vector_width;
int main(int argc, char* argv[])
{

  initialize_configuration();
  Profilelist::support_backward_compatibility();
  RecentlyPlayedGameList::support_backward_compatibility();
  Gamelist::support_backward_compatibility();
  FileCompat::support_backward_compatibility_for_common_files();
  FileCompat::getInstance()->initialize();
  Vector<int>::setMaximumWidth(1000);
  RecentlyPlayedGameList::getInstance()->load();

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
          Glib::ustring parameter(argv[i-1]); 
	  if (parameter == "-c" || parameter == "--cache-size")
	    {
	      i++;
	      //convert the next argument
	      char* error = 0;
	      long size = strtol(argv[i-1], &error, 10);
	      if (error && (*error != '\0'))
		{
                  std::cerr <<_("non-numerical value for cache size") <<std::endl;
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
                  std::cerr <<_("non-numerical value for --seed") <<std::endl;
		  exit(-1);
		}
              kit.random_number_seed = seed;
	    }
	  else if (parameter == "--port" || parameter == "-p")
	    {
	      i++;
	      //convert the next argument
	      char* error = 0;
	      long port = strtol(argv[i-1], &error, 10);
	      if (error && (*error != '\0'))
		{
                  std::cerr <<_("non-numerical value for --port") <<std::endl;
		  exit(-1);
		}
              if (port > 65535 || port < 1000)
                {
                  std::cerr <<_("invalid value for --port") <<std::endl;
		  exit(-1);
                }
              kit.port = port;
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
	  else if (parameter == "--host" || parameter == "-H")
	    {
	      kit.start_headless_server = true;
	    }
	  else if (parameter == "--help" || parameter == "-h")
	    {
              std::cout << Glib::get_prgname() << " [OPTION]... [FILE]" << std::endl << std::endl;
              std::cout << "LordsAWar! " << _("version") << " " << VERSION << std::endl << std::endl;
              std::cout << _("Options:") << std::endl << std::endl; 
              std::cout << "  -h, --help                 " << _("Shows this help screen") <<std::endl;
              std::cout << "  -c, --cache-size <size>    " << _("Set the cache size for imagery to SIZE bytes") <<std::endl;
              std::cout << "  -t, --test                 " << _("Start with a test-scenario") << std::endl;
              std::cout << "  -S, --seed <number>        " << _("Seed the random number generator with NUMBER") << std::endl;
              std::cout << "  -s, --stress-test          " << _("Non-interactive stress test") << std::endl;
              std::cout << "  -r, --robots               " << _("Non-interactive network stress test") << std::endl;
              std::cout << "  -H, --host                 " << _("Start a headless server") << std::endl;
              std::cout << "  -p, --port <number>        " << _("Start the server on the given port") << std::endl;
              std::cout << std::endl;
              std::cout << _("FILE can be a saved game file (.sav), or a map (.map) file.") << std::endl;
              std::cout << std::endl;
              std::cout << _("Report bugs to") << " <" << PACKAGE_BUGREPORT ">." << std::endl;
	      exit(0);
	    }
	  else
	    kit.load_filename = parameter;
	}
    }

  if (kit.load_filename != "" && kit.start_test_scenario)
    {
      std::cerr <<_("Error: Cannot specify -t and have a file specified.") << std::endl;
      exit (1);
    }

  if (kit.load_filename != "" && kit.start_stress_test)
    {
      std::cerr <<_("Error: Cannot specify -s and have a file specified.") << std::endl;
      exit (1);
    }

  if (kit.start_stress_test && kit.start_test_scenario)
    {
      std::cerr <<_("Error: Cannot specify -s and -t simultaneously.") << std::endl;
      exit (1);
    }

  if (kit.turn_filename != "" && kit.load_filename == "")
    {
      std::cerr <<_("Error: Must specify a file to load when specifying --turn.") << std::endl;
      exit (1);
    }


  // Check if armysets are in the path (otherwise exit)
  Armysetlist::scan(Armyset::file_extension);
  Tilesetlist::scan(Tileset::file_extension);
  Shieldsetlist::scan(Shieldset::file_extension);
  Citysetlist::scan(Cityset::file_extension);
  BuilderCache::getInstance();


#ifdef LW_SOUND
  Gst::init(argc, argv);
#endif
  kit.start_main_loop();

  return EXIT_SUCCESS;
}
