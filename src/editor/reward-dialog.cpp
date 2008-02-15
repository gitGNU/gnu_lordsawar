//  Copyright (C) 2008, Ben Asselstine
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
#include <sigc++/functors/mem_fun.h>

#include "reward-dialog.h"

#include "glade-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../reward.h"

RewardDialog::RewardDialog(Reward *t)
{
  reward = t;

  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path()
				+ "/reward-dialog.glade");

  Gtk::Dialog *d = 0;
  xml->get_widget("dialog", d);
  dialog.reset(d);

    xml->get_widget("gold_hbox", gold_hbox);
    xml->get_widget("gold_radiobutton", gold_radiobutton);
    gold_radiobutton->signal_toggled().connect(
	sigc::mem_fun(*this, &RewardDialog::on_gold_toggled));
    xml->get_widget("item_hbox", item_hbox);
    xml->get_widget("item_radiobutton", item_radiobutton);
    item_radiobutton->signal_toggled().connect(
	sigc::mem_fun(*this, &RewardDialog::on_item_toggled));
    xml->get_widget("allies_hbox", allies_hbox);
    xml->get_widget("allies_radiobutton", allies_radiobutton);
    allies_radiobutton->signal_toggled().connect(
	sigc::mem_fun(*this, &RewardDialog::on_allies_toggled));
    xml->get_widget("map_hbox", map_hbox);
    xml->get_widget("map_radiobutton", map_radiobutton);
    map_radiobutton->signal_toggled().connect(
	sigc::mem_fun(*this, &RewardDialog::on_map_toggled));
}

void RewardDialog::set_parent_window(Gtk::Window &parent)
{
  dialog->set_transient_for(parent);
  //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void RewardDialog::run()
{
  dialog->show_all();
  int response = dialog->run();

  if (response == 0)		// accepted
    {
      ;
    }
  else
    {
      ;
    }
}


void RewardDialog::on_gold_toggled()
{
  gold_hbox->set_sensitive(true);
  item_hbox->set_sensitive(false);
  allies_hbox->set_sensitive(false);
  map_hbox->set_sensitive(false);
}

void RewardDialog::on_item_toggled()
{
  gold_hbox->set_sensitive(false);
  item_hbox->set_sensitive(true);
  allies_hbox->set_sensitive(false);
  map_hbox->set_sensitive(false);
}

void RewardDialog::on_allies_toggled()
{
  gold_hbox->set_sensitive(false);
  item_hbox->set_sensitive(false);
  allies_hbox->set_sensitive(true);
  map_hbox->set_sensitive(false);
}

void RewardDialog::on_map_toggled()
{
  gold_hbox->set_sensitive(false);
  item_hbox->set_sensitive(false);
  allies_hbox->set_sensitive(false);
  map_hbox->set_sensitive(true);
}
