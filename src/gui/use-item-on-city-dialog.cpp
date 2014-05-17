//  Copyright (C) 2011, 2012 Ben Asselstine
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

#include <config.h>

#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>

#include "use-item-on-city-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "GameMap.h"
#include "File.h"
#include "GraphicsCache.h"
#include "city.h"

UseItemOnCityDialog::UseItemOnCityDialog(SelectCityMap::Type type)
{
  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path()
                                     + "/use-item-on-city-dialog.ui");

  xml->get_widget("dialog", dialog);
  decorate(dialog);
  dialog->set_icon_from_file(File::getMiscFile("various/castle_icon.png"));
  window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));

  xml->get_widget("map_image", map_image);
  xml->get_widget("continue_button", continue_button);

  citymap = new SelectCityMap(type);
  citymap->map_changed.connect(
                               sigc::mem_fun(this, &UseItemOnCityDialog::on_map_changed));
  citymap->city_selected.connect(
                                 sigc::mem_fun(this, &UseItemOnCityDialog::on_city_selected));

  Gtk::EventBox *map_eventbox;
  xml->get_widget("map_eventbox", map_eventbox);
  map_eventbox->add_events(Gdk::BUTTON_PRESS_MASK);
  map_eventbox->signal_button_press_event().connect
    ( sigc::mem_fun(*this, &UseItemOnCityDialog::on_map_mouse_button_event));

  continue_button->set_sensitive(false);

  xml->get_widget("label", label);
  switch (type)
    {
    case SelectCityMap::ANY_CITY:
      label->set_text(_("Select a city to target."));
      break;
    case SelectCityMap::FRIENDLY_CITY:
      label->set_text(_("Select one of your cities to target."));
      break;
    case SelectCityMap::ENEMY_CITY:
      label->set_text(_("Select an enemy city to target."));
      break;
    case SelectCityMap::NEUTRAL_CITY:
      label->set_text(_("Select a neutral city to target."));
      break;
    }

}

UseItemOnCityDialog::~UseItemOnCityDialog()
{
  delete dialog;
  delete citymap;
}

void UseItemOnCityDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void UseItemOnCityDialog::hide()
{
  dialog->hide();
}

City*UseItemOnCityDialog::run()
{
    citymap->resize();
    citymap->draw(Playerlist::getActiveplayer());
    dialog->show_all();
    dialog->run();
    return citymap->get_selected_city();
}

void UseItemOnCityDialog::on_map_changed(Cairo::RefPtr<Cairo::Surface> map)
{
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
    Gdk::Pixbuf::create(map, 0, 0, citymap->get_width(), citymap->get_height());
  map_image->property_pixbuf() = pixbuf;
}

void UseItemOnCityDialog::on_city_selected(City *city)
{
  continue_button->set_sensitive(true);
}

bool UseItemOnCityDialog::on_map_mouse_button_event(GdkEventButton *e)
{
  if (e->type != GDK_BUTTON_PRESS)
    return true;	// useless event
  citymap->mouse_button_event(to_input_event(e));
  return true;
}

