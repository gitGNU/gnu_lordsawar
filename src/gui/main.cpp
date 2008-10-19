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

#include <assert.h>
#include <memory>
#include <iostream>
#include <assert.h>
#include <glib.h>
#include <gtkmm/main.h>
#include <sigc++/trackable.h>
#include <sigc++/functors/mem_fun.h>

#include "main.h"

#include "driver.h"
#include "defs.h"
#include "File.h"
#include "Configuration.h"
#include "timing.h"


struct Main::Impl: public sigc::trackable 
{
    std::auto_ptr<Gtk::Main> gtk_main;
    std::auto_ptr<Driver> driver;

    sigc::connection on_timer_registered(Timing::timer_slot s,
					 int msecs_interval);
};


static Main *singleton;

Main::Main(int &argc, char **&argv)
    : impl(new Impl)
{
    singleton = this;

    start_test_scenario = false;
    start_stress_test = false;
    start_robots = 0;
    load_filename = "";
    turn_filename = "";
    record = "";
    
    Glib::thread_init();
    try
    {
	impl->gtk_main.reset(new Gtk::Main(argc, argv));

	g_set_application_name("LordsAWar!");

	Timing::instance().timer_registered.connect(
	    sigc::mem_fun(*impl, &Main::Impl::on_timer_registered));
    }
    catch (const Glib::Error &ex) {
	std::cerr << ex.what() << std::endl;
    }
}

Main::~Main()
{
    delete impl;
    singleton = 0;
}

Main &Main::instance()
{
    assert(singleton != 0);
    return *singleton;
}

bool Main::iterate_main_loop()
{
    try
    {
	impl->gtk_main->iteration(false);
    }
    catch (const Glib::Error &ex) {
	std::cerr << ex.what() << std::endl;
    }

    return true;
}

void Main::start_main_loop()
{
    if (Configuration::s_decorated)
      Gtk::RC::add_default_file(File::getMiscFile("gtkrc"));

    try
    {
	impl->driver.reset(new Driver(load_filename));
	impl->gtk_main->run();
    }
    catch (const Glib::Error &ex) {
	std::cerr << ex.what() << std::endl;
    }
}

void Main::stop_main_loop()
{
    try
    {
	impl->gtk_main->quit();
    }
    catch (const Glib::Error &ex) {
	std::cerr << ex.what() << std::endl;
    }
}

sigc::connection Main::Impl::on_timer_registered(Timing::timer_slot s,
						 int msecs_interval)
{
    return Glib::signal_timeout().connect(s, msecs_interval);
}

