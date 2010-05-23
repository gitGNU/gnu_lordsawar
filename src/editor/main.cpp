//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2010 Ben Asselstine
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
#include <gtkmm.h>

#include "Configuration.h"
#include "File.h"
#include "GraphicsCache.h"
#include "timing.h"
#include "armyset.h"
#include "recently-edited-file-list.h"

#include "main-window.h"


sigc::connection on_timer_registered(Timing::timer_slot s, int msecs_interval)
{
    return Glib::signal_timeout().connect(s, msecs_interval);
}

int max_vector_width;
int main(int argc, char* argv[])
{
    srand(time(NULL));         // set the random seed

    initialize_configuration();
    Vector<int>::setMaximumWidth(1000);

    setlocale(LC_ALL, Configuration::s_lang.c_str());
    textdomain ("lordsawar");

    // Check if armysets are in the path (otherwise exit)
    Armyset::scanSystemCollection();

    // init GUI stuff
    g_set_application_name(_("LordsAWar! Scenario Editor"));
    Timing::instance().timer_registered.connect(
	sigc::ptr_fun(on_timer_registered));

    RecentlyEditedFileList::getInstance()->loadFromFile();
	
    MainWindow* main_window = NULL;
    try
    {
	Gtk::Main kit(argc, argv);

	MainWindow* main_window;
	if (argc > 1)
	  main_window = new MainWindow (argv[1]);
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
