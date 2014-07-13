//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2010, 2011, 2014 Ben Asselstine
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
#include <time.h>
#include <gtkmm.h>

#include "Configuration.h"
#include "File.h"
#include "timing.h"
#include "armyset.h"
#include "file-compat.h"
#include "setlist.h"

#include "main-window.h"
#include "editor-splash-window.h"


sigc::connection on_timer_registered(Timing::timer_slot s, int msecs_interval)
{
    return Glib::signal_timeout().connect(s, msecs_interval);
}

int max_vector_width;
int main(int argc, char* argv[])
{
  std::string load_filename;
  srand(time(NULL));         // set the random seed

  initialize_configuration();
  FileCompat::getInstance()->initialize();
  Vector<int>::setMaximumWidth(1000);

  setlocale(LC_ALL, Configuration::s_lang.c_str());
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  // Check if armysets are in the path (otherwise exit)
  SetList::scan(Armyset::file_extension);

  // init GUI stuff
  g_set_application_name(N_("LordsAWar! Scenario Editor"));
  Timing::instance().timer_registered.connect(
                                              sigc::ptr_fun(on_timer_registered));

  Gtk::Main kit(argc, argv);
  if (argc > 1)
    {
      for (int i = 2; i <= argc; i++)
        {
          Glib::ustring parameter(argv[i-1]); 
          if (parameter == "--help" || parameter == "-h")
            {
              std::cout << Glib::get_prgname() << " [OPTION]... [FILE]" << std::endl << std::endl;
              std::cout << "LordsAWar! " << _("version") << " " << VERSION << std::endl << std::endl;
              std::cout << _("Options:") << std::endl << std::endl; 
              std::cout << "  -h, --help                 " << _("Shows this help screen") <<std::endl;
              std::cout << std::endl;
              std::cout << _("FILE can be a saved game file (.sav), or a map (.map) file.") << std::endl;
              std::cout << std::endl;
              std::cout << _("Report bugs to") << " <" << PACKAGE_BUGREPORT ">." << std::endl;
              exit(0);
            }
          else
            load_filename = parameter;
        }
    }

  EditorSplashWindow d;
  d.run();
  d.hide();
  MainWindow* main_window = NULL;
  try
    {
      MainWindow* main_window;
      if (argc > 1)
        main_window = new MainWindow (load_filename);
      else
        main_window = new MainWindow;
      main_window->show();

      main_window->init();
      kit.run(main_window->get_window());
    }
  catch (const Glib::Error &ex) {
    std::cerr << ex.what() << std::endl;
  }
  delete main_window;

  return EXIT_SUCCESS;
}
