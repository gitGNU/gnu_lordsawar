//  Copyright (C) 2008 Ben Asselstine
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

#include <libglademm/xml.h>
#include <gtkmm/eventbox.h>
#include <sigc++/functors/mem_fun.h>
#include <gtkmm/label.h>

#include "network-game-selector-dialog.h"

#include "glade-helpers.h"
#include "input-helpers.h"
#include "../defs.h"

NetworkGameSelectorDialog::NetworkGameSelectorDialog()
{
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create
	(get_glade_path() + "/pick-network-game-to-join-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    decorate(dialog.get());
    window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));

    xml->get_widget("hostname_entry", hostname_entry);
    hostname_entry->set_activates_default(true);
    hostname_entry->signal_changed().connect
	(sigc::mem_fun(this, &NetworkGameSelectorDialog::on_hostname_changed));
    xml->get_widget("connect_button", connect_button);
    connect_button->set_sensitive(false);
}


void NetworkGameSelectorDialog::on_hostname_changed()
{
  //validate the ip/hostname
  if (hostname_entry->get_text().length() > 0)
    connect_button->set_sensitive(true);
  else
    connect_button->set_sensitive(false);
}

void NetworkGameSelectorDialog::set_parent_window(Gtk::Window &parent)
{
  dialog->set_transient_for(parent);
  //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void NetworkGameSelectorDialog::hide()
{
  dialog->hide();
}

bool NetworkGameSelectorDialog::run()
{
  int response = dialog->run();
  if (response == 0)
    {
      hide();
      game_selected.emit(hostname_entry->get_text(), LORDSAWAR_PORT);
      return true;
    }
  else
    return false;
}
