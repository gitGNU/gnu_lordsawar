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

    xml->get_widget("reward_gold_hbox", reward_gold_hbox);
    xml->get_widget("reward_gold_radiobutton", reward_gold_radiobutton);
    reward_gold_radiobutton->signal_toggled().connect(
	sigc::mem_fun(*this, &RewardDialog::on_reward_gold_toggled));
    xml->get_widget("reward_item_hbox", reward_item_hbox);
    xml->get_widget("reward_item_radiobutton", reward_item_radiobutton);
    reward_item_radiobutton->signal_toggled().connect(
	sigc::mem_fun(*this, &RewardDialog::on_reward_item_toggled));
    xml->get_widget("reward_allies_hbox", reward_allies_hbox);
    xml->get_widget("reward_allies_radiobutton", reward_allies_radiobutton);
    reward_allies_radiobutton->signal_toggled().connect(
	sigc::mem_fun(*this, &RewardDialog::on_reward_allies_toggled));
    xml->get_widget("reward_map_hbox", reward_map_hbox);
    xml->get_widget("reward_map_radiobutton", reward_map_radiobutton);
    reward_map_radiobutton->signal_toggled().connect(
	sigc::mem_fun(*this, &RewardDialog::on_reward_map_toggled));
    xml->get_widget("reward_random_radiobutton", reward_random_radiobutton);
    reward_random_radiobutton->signal_toggled().connect(
	sigc::mem_fun(*this, &RewardDialog::on_reward_random_toggled));

    reward_random_radiobutton->set_active(true);
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


void RewardDialog::on_reward_gold_toggled()
{
  reward_gold_hbox->set_sensitive(true);
  reward_item_hbox->set_sensitive(false);
  reward_allies_hbox->set_sensitive(false);
  reward_map_hbox->set_sensitive(false);
}

void RewardDialog::on_reward_item_toggled()
{
  reward_gold_hbox->set_sensitive(false);
  reward_item_hbox->set_sensitive(true);
  reward_allies_hbox->set_sensitive(false);
  reward_map_hbox->set_sensitive(false);
}

void RewardDialog::on_reward_allies_toggled()
{
  reward_gold_hbox->set_sensitive(false);
  reward_item_hbox->set_sensitive(false);
  reward_allies_hbox->set_sensitive(true);
  reward_map_hbox->set_sensitive(false);
}

void RewardDialog::on_reward_map_toggled()
{
  reward_gold_hbox->set_sensitive(false);
  reward_item_hbox->set_sensitive(false);
  reward_allies_hbox->set_sensitive(false);
  reward_map_hbox->set_sensitive(true);
}

void RewardDialog::on_reward_random_toggled()
{
  reward_gold_hbox->set_sensitive(false);
  reward_item_hbox->set_sensitive(false);
  reward_allies_hbox->set_sensitive(false);
  reward_map_hbox->set_sensitive(false);
}
