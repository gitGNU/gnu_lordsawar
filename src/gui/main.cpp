//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2010, 2014, 2017 Ben Asselstine
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

#include "recently-played-game-list.h"
#include "citysetlist.h"
#include "tilesetlist.h"
#include "shieldsetlist.h"
#include "armysetlist.h"
#include "profilelist.h"
#include "gamelist.h"
#include "file-compat.h"
#include "gui/builder-cache.h"

#include "main.h"

#include "driver.h"
#include "defs.h"
#include "File.h"
#include "Configuration.h"
#include "timing.h"
#include "fight-window.h"


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
    start_net_test_scenario = false;
    speedy = false;
    own_all_on_round_two = false;
    start_stress_test = false;
    start_editor = false;
    start_robots = 0;
    start_headless_server = false;
    load_filename = "";
    turn_filename = "";
    random_number_seed = 0;
    port = 0;
    cacheSize = 0;
    
    Glib::thread_init();
    try
    {
	impl->gtk_main = new Gtk::Main(argc, argv);

	g_set_application_name("LordsAWar!");

        Gtk::Settings::get_default()->property_gtk_application_prefer_dark_theme() = true;

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

void Main::start_main_loop()
{
  if (random_number_seed)
    Rnd::set_seed(random_number_seed);
  else
    Rnd::set_seed(time(NULL));

  try
    {
      if (impl->driver != NULL)
        {
          delete impl->driver;
          impl->driver = NULL;
        }
      if (speedy)
        {
          FightWindow::s_quick_all = true;
          Configuration::s_displaySpeedDelay = 0;
        }
      impl->driver = new Driver(start_editor, load_filename);
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

void Main::initialize ()
{
  if (configuration_file_path != "")
    Configuration::s_configuration_file_path = configuration_file_path;
  if (save_path != "")
    Configuration::s_savePath = save_path;
  initialize_configuration();
  if (cacheSize)
    Configuration::s_cacheSize = cacheSize;
  Profilelist::support_backward_compatibility();
  RecentlyPlayedGameList::support_backward_compatibility();
  Gamelist::support_backward_compatibility();
  FileCompat::support_backward_compatibility_for_common_files();
  FileCompat::getInstance()->initialize();
  Vector<int>::setMaximumWidth(1000);
  RecentlyPlayedGameList::getInstance()->load();


  // Check if armysets are in the path (otherwise exit)
  Armysetlist::scan(Armyset::file_extension);
  Tilesetlist::scan(Tileset::file_extension);
  Shieldsetlist::scan(Shieldset::file_extension);
  Citysetlist::scan(Cityset::file_extension);
  BuilderCache::getInstance();

}
