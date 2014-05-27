// Copyright (C) 2011, 2014 Ben Asselstine
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
#include <stdlib.h>
#include "Configuration.h"
#include "defs.h"
#include "vector.h"
#include "ucompose.hpp"
#include "ghs-client-tool.h"
#include "profile.h"
#include "profilelist.h"

int max_vector_width;
    
int main(int argc, char* argv[])
{
  Profile *profile = NULL;
  std::string host;
  std::string file;
  std::string unhost;
  bool show_list = false;
  bool reload = false;
  bool terminate = false;
  srand(time(NULL));         // set the random seed

  initialize_configuration();
  Vector<int>::setMaximumWidth(1000);

  setlocale(LC_ALL, Configuration::s_lang.c_str());

  int port = 0;
  Gtk::Main *gtk_main = new Gtk::Main(argc, argv);
  if (argc > 1)
    {
      for (int i = 2; i <= argc; i++)
	{
          std::string parameter(argv[i-1]); 
	  if (parameter == "--port" || parameter == "-p")
	    {
	      i++;
	      //convert the next argument
	      char* error = 0;
	      long userport = strtol(argv[i-1], &error, 10);
	      if (error && (*error != '\0'))
		{
                  std::cerr <<_("non-numerical value for --port") <<std::endl;
		  exit(-1);
		}
              if (userport > 65535 || userport < 1000)
                {
                  std::cerr <<_("invalid value for --port") <<std::endl;
		  exit(-1);
                }
              port = userport;
	    }
          else if (parameter == "--profile" || parameter == "-P")
            {
              profile = Profilelist::getInstance()->findProfileById(parameter);
              if (!profile)
                {
                  std::cerr << _("invalid profile id") << std::endl;
                  exit(-1);
                }
            }
          else if (parameter == "--list" || parameter == "-l")
            {
              show_list = true;
            }
          else if (parameter == "--reload" || parameter == "-R")
            {
              reload = true;
            }
          else if (parameter == "--host" || parameter == "-h")
            {
              file = parameter;
            }
          else if (parameter == "--unhost" || parameter == "-u")
            {
              unhost = parameter;
            }
          else if (parameter == "--terminate" || parameter == "-t")
            {
              terminate = true;
            }
	  else if (parameter == "--help" || parameter == "-?")
	    {
              std::cout << Glib::get_prgname() << " " << _("[OPTION]... [HOST]") << std::endl << std::endl;
              std::cout << "LordsAWar! Game-list Client " << _("version") << " " << VERSION << std::endl << std::endl;
              std::cout << _("Options:") << std::endl << std::endl; 
              std::cout << "  -?, --help                 " << _("Display this help and exit") <<std::endl;
              std::cout << "  -P, --profile <id>         " << _("Use this identity, specified by profile id") << std::endl;
              std::cout << "  -p, --port <number>        " << _("Connect to the server on the given port") << std::endl;
              std::cout << "  -l, --list                 " << _("See a list of hosted games") << std::endl;
              std::cout << "  -R, --reload               " << _("Reload the game list from disk") << std::endl;
              std::cout << "  -u, --unhost <id>          " << _("Stop hosting a game (specified by scenario id)") << std::endl;
              std::cout << "  -h, --host <file>          " << _("Host a game") << std::endl;
              std::cout << "  -t, --terminate            " << _("Stop the server") << std::endl;
              std::cout << std::endl;
              std::cout << String::ucompose ("%1", _("If HOST is not specified on the command-line, this tool will try to connect to \nthe game-host server at 127.0.0.1.")) << std::endl;
              std::cout << std::endl;
              std::cout << _("Report bugs to") << " <" << PACKAGE_BUGREPORT ">." << std::endl;
	      exit(0);
	    }
          else
            host = parameter;
	}
    }

  if (port == 0)
    port = LORDSAWAR_GAMEHOST_PORT;

  if (!show_list && !reload && unhost.empty() && file.empty())
    {
      Glib::ustring s = 
        String::ucompose("Try `%1 --help' for more information.",
                         Glib::get_prgname());
      std::cout << s << std::endl;
      return EXIT_SUCCESS;
    }
  if (host == "")
    host = "127.0.0.1";
  GhsClientTool tool(host, port, profile, show_list, reload, unhost, file, 
                     terminate);

  gtk_main->run();
  //delete gtk_main;

  return EXIT_SUCCESS;
}
