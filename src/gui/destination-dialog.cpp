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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.*

#include <config.h>

#include <libglademm/xml.h>
#include <gtkmm/eventbox.h>
#include <sigc++/functors/mem_fun.h>

#include "destination-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../GameMap.h"

DestinationDialog::DestinationDialog(City *c)
{
    city = c;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/destination-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    xml->get_widget("map_image", map_image);

    vectormap.reset(new VectorMap(city));
    vectormap->map_changed.connect(
	sigc::mem_fun(this, &DestinationDialog::on_map_changed));

    Gtk::EventBox *map_eventbox;
    xml->get_widget("map_eventbox", map_eventbox);
    map_eventbox->add_events(Gdk::BUTTON_PRESS_MASK);
    map_eventbox->signal_button_press_event().connect(
	sigc::mem_fun(*this, &DestinationDialog::on_map_mouse_button_event));
}

void DestinationDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void DestinationDialog::run()
{
    // FIXME: the overviewmap looks hideous at this scale
    vectormap->resize(GameMap::get_dim() * 4);
    vectormap->draw();
    dialog->show();
    dialog->run();
}

void DestinationDialog::on_map_changed(SDL_Surface *map)
{
    map_image->property_pixbuf() = to_pixbuf(map);
}

bool DestinationDialog::on_map_mouse_button_event(GdkEventButton *e)
{
    if (e->type != GDK_BUTTON_PRESS)
	return true;	// useless event
    
    vectormap->mouse_button_event(to_input_event(e));
    
    return true;
}

