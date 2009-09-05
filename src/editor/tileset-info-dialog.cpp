//  Copyright (C) 2008, 2009 Ben Asselstine
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

#include <config.h>

#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>

#include "tileset-info-dialog.h"

#include "glade-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "signpost.h"


TileSetInfoDialog::TileSetInfoDialog(Tileset *tileset)
{
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/tileset-info-dialog.ui");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    xml->get_widget("name_entry", name_entry);
    name_entry->set_text(tileset->getName());
    
    xml->get_widget("description_textview", description_textview);
    description_textview->get_buffer()->set_text(tileset->getInfo());

    xml->get_widget("road_colorbutton", road_colorbutton);
    Gdk::Color c;
    SDL_Color sdl;
    sdl = tileset->getRoadColor();
    c.set_red(sdl.r * 255); c.set_green(sdl.g * 255); c.set_blue(sdl.b * 255);
    road_colorbutton->set_color(c);
    d_tileset = tileset;
}

void TileSetInfoDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

bool TileSetInfoDialog::run()
{
    dialog->show_all();
    int response = dialog->run();

    if (response == Gtk::RESPONSE_ACCEPT)	// accepted
    {
        d_tileset->setName(name_entry->get_text());
        d_tileset->setInfo(description_textview->get_buffer()->get_text());
	Gdk::Color c = road_colorbutton->get_color();
	SDL_Color sdl;
	memset (&sdl, 0, sizeof (sdl));
	sdl.r = c.get_red() / 255;
	sdl.g = c.get_green() / 255;
	sdl.b = c.get_blue() / 255;
	d_tileset->setRoadColor(sdl);
	return true;
    }
    return false;
}

