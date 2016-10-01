//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2010, 2014 Ben Asselstine
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

#include <memory>
#include <iostream>
#include <assert.h>
#include <glib.h>
#include <gtkmm.h>
#include <sigc++/trackable.h>
#include <sigc++/functors/mem_fun.h>
#include <time.h>
#include "rnd.h"

#include "main.h"

#include "driver.h"
#include "defs.h"
#include "File.h"
#include "Configuration.h"
#include "timing.h"


struct Main::Impl: public sigc::trackable 
{
    Gtk::Main* gtk_main;
    Driver* driver;

    sigc::connection on_timer_registered(Timing::timer_slot s,
					 int msecs_interval);
};


static Main *singleton;

Main::Main(int &argc, char **&argv)
    : impl(new Impl)
{
  impl->driver = NULL;
    singleton = this;

    start_test_scenario = false;
    start_stress_test = false;
    start_robots = 0;
    start_headless_server = false;
    load_filename = "";
    turn_filename = "";
    random_number_seed = 0;
    port = 0;
    
    Glib::thread_init();
    try
    {
	impl->gtk_main = new Gtk::Main(argc, argv);

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
    delete impl->driver;
    delete impl->gtk_main;
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
  if (random_number_seed)
    Rnd::set_seed(random_number_seed);
  else
    Rnd::set_seed(time(NULL));

  if (Configuration::s_decorated)
    {
      ;
    }

  try
    {
      if (impl->driver != NULL)
        {
          delete impl->driver;
          impl->driver = NULL;
        }
      impl->driver = new Driver(load_filename);
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

