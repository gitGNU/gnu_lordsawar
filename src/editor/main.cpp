//  Copyright (C) 2007, Ole Laursen
//  Copyright (C) 2007, 2008 Ben Asselstine
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
#include <gtkmm.h>

#include "Configuration.h"
#include "File.h"
#include "GraphicsCache.h"
#include "timing.h"

#include "main-window.h"


sigc::connection on_timer_registered(Timing::timer_slot s, int msecs_interval)
{
    return Glib::signal_timeout().connect(s, msecs_interval);
}

int main(int argc, char* argv[])
{
    srand(time(NULL));         // set the random seed

    initialize_configuration();

    setlocale(LC_ALL, Configuration::s_lang.c_str());
    textdomain ("lordsawar");

    // Check if armysets are in the path (otherwise exit)
    File::scanArmysets();

    // init GUI stuff
    g_set_application_name(_("LordsAWar! Scenario Editor"));
    Timing::instance().timer_registered.connect(
	sigc::ptr_fun(on_timer_registered));

    try
    {
	Gtk::Main kit(argc, argv);

	std::auto_ptr<MainWindow> main_window(new MainWindow);
	main_window->init(640, 480);
    
	main_window->sdl_initialized.connect(
	    sigc::mem_fun(main_window.get(), &MainWindow::show_initial_map));
	main_window->show();
	
	kit.run(main_window->get_window());
    }
    catch (const Glib::Error &ex) {
	std::cerr << ex.what() << std::endl;
    }
    
    return EXIT_SUCCESS;
}
