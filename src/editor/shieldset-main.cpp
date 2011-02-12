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
#include "recently-edited-file-list.h"

#include "shieldset-window.h"


int max_vector_width;
int main(int argc, char* argv[])
{
    srand(time(NULL));         // set the random seed

    initialize_configuration();
    Vector<int>::setMaximumWidth(1000);

    setlocale(LC_ALL, Configuration::s_lang.c_str());
    textdomain ("lordsawar");

    // Check if shieldsets are in the path (otherwise exit)
    Shieldset::scanSystemCollection();

    // init GUI stuff
    g_set_application_name(N_("LordsAWar! Shieldset Editor"));

  
    RecentlyEditedFileList::getInstance()->loadFromFile();
    ShieldSetWindow* shieldset_window = NULL;
    try
    {
	Gtk::Main kit(argc, argv);

	if (argc > 1)
	  shieldset_window = new ShieldSetWindow (argv[1]);
	else
	  shieldset_window = new ShieldSetWindow;

	shieldset_window->show();
	
	kit.run(shieldset_window->get_window());
	delete shieldset_window;
    }
    catch (const Glib::Error &ex) {
	std::cerr << ex.what() << std::endl;
    }
    
    return EXIT_SUCCESS;
}
