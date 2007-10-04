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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <gtkmm/main.h>

#include "../Configuration.h"
#include "../File.h"
#include "../GraphicsCache.h"
#include "../timing.h"

#include "armyset-window.h"


int main(int argc, char* argv[])
{
    srand(time(NULL));         // set the random seed

    initialize_configuration();

    setlocale(LC_ALL, Configuration::s_lang.c_str());
#ifndef __WIN32__
    bindtextdomain ("lordsawar",PO_PATH);
#else
    bindtextdomain ("lordsawar","./locale/");
#endif
    textdomain ("lordsawar");

    // Check if armysets are in the path (otherwise exit)
    File::scanArmysets();

    // init GUI stuff
    g_set_application_name(_("LordsAWar! Armyset Editor"));

    try
    {
	Gtk::Main kit(argc, argv);

	std::auto_ptr<ArmySetWindow> armyset_window(new ArmySetWindow);
	armyset_window->init(64, 64);
	armyset_window->show();
	
	kit.run(armyset_window->get_window());
    }
    catch (const Glib::Error &ex) {
	std::cerr << ex.what() << std::endl;
    }
    
    return EXIT_SUCCESS;
}
