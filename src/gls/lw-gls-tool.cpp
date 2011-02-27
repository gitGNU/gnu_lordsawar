// Copyright (C) 2011 Ben Asselstine
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
#include "Configuration.h"
#include "defs.h"
#include "vector.h"
#include "ucompose.hpp"
#include "gls-client-tool.h"

using namespace std;

int max_vector_width;
    
int main(int argc, char* argv[])
{
  bool advertise = false;
  bool show_list = false;
  bool reload = false;
  srand(time(NULL));         // set the random seed

  initialize_configuration();
  Vector<int>::setMaximumWidth(1000);

  setlocale(LC_ALL, Configuration::s_lang.c_str());

  int port = 0;
  std::list<std::string> unadvertise;
  Gtk::Main *gtk_main = new Gtk::Main(argc, argv);
  if (argc > 1)
    {
      for (int i = 2; i <= argc; i++)
	{
	  string parameter(argv[i-1]); 
	  if (parameter == "--port" || parameter == "-p")
	    {
	      i++;
	      //convert the next argument
	      char* error = 0;
	      long userport = strtol(argv[i-1], &error, 10);
	      if (error && (*error != '\0'))
		{
		  cerr <<_("non-numerical value for --port") <<endl;
		  exit(-1);
		}
              if (userport > 65535 || userport < 1000)
                {
		  cerr <<_("invalid value for --port") <<endl;
		  exit(-1);
                }
              port = userport;
	    }
          else if (parameter == "--unadvertise" || parameter == "-u")
            {
              unadvertise.push_back(argv[i-1]);
            }
          else if (parameter == "--advertise" || parameter == "-a")
            {
              advertise = true;
            }
          else if (parameter == "--list" || parameter == "-l")
            {
              show_list = true;
            }
          else if (parameter == "--reload" || parameter == "-r")
            {
              reload = true;
            }
	  else if (parameter == "--help" || parameter == "-?")
	    {
	      cout << Glib::get_prgname() << " [OPTION]..." << endl << endl;
	      cout << "LordsAWar! Game-list Client " << _("version") << " " << VERSION << endl << endl;
	      cout << _("Options:") << endl << endl; 
	      cout << "  -?, --help                 " << _("Shows this help screen") <<endl;
	      cout << "  -p, --port <number>        " << _("Connect to the server on the given port") << endl;
	      cout << "  -u, --unadvertise <id>     " << _("Remove a game, specified by scenario id") << endl;
	      cout << "  -a, --advertise            " << _("Add a game") << endl;
	      cout << "  -l, --list                 " << _("See a list of games") << endl;
	      cout << "  -l, --reload               " << _("Reload the game list from disk") << endl;
	      cout << endl;
	      cout << _("Report bugs to") << " <" << PACKAGE_BUGREPORT ">." << endl;
	      exit(0);
	    }
	}
    }

  if (port == 0)
    port = LORDSAWAR_GAMELIST_PORT;

  if (!show_list && unadvertise.empty() == true && !advertise && !reload)
    {
      Glib::ustring s = 
        String::ucompose("Try `%1 --help' for more information.",
                         Glib::get_prgname());
      std::cout << s << std::endl;
      return EXIT_SUCCESS;
    }
  GlsClientTool tool(port, show_list, unadvertise, advertise, reload);

  gtk_main->run();
  //delete gtk_main;

  return EXIT_SUCCESS;
}
