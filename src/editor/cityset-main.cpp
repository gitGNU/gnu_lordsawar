//  Copyright (C) 2007, 2008, 2009, 2010, 2011 Ben Asselstine
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
#include "file-compat.h"
#include "recently-edited-file-list.h"
#include "cityset-window.h"


int max_vector_width;
int main(int argc, char* argv[])
{
    srand(time(NULL));         // set the random seed

    initialize_configuration();
    RecentlyEditedFileList::support_backward_compatibility();
    FileCompat::getInstance()->initialize();
    Vector<int>::setMaximumWidth(1000);

    setlocale(LC_ALL, Configuration::s_lang.c_str());
    bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    // Check if citysets are in the path (otherwise exit)
    Cityset::scanSystemCollection();

    // init GUI stuff
    g_set_application_name(N_("LordsAWar! Cityset Editor"));

    RecentlyEditedFileList::getInstance()->loadFromFile();

    CitySetWindow* cityset_window = NULL;
    try
    {
	Gtk::Main kit(argc, argv);

	if (argc > 1)
	  cityset_window = new CitySetWindow (argv[1]);
	else
	  cityset_window = new CitySetWindow;

	cityset_window->show();
	
	kit.run(cityset_window->get_window());
	delete cityset_window;
    }
    catch (const Glib::Error &ex) {
	std::cerr << ex.what() << std::endl;
    }
    
    return EXIT_SUCCESS;
}
