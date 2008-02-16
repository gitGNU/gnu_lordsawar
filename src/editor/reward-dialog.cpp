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
#include "../Item.h"
#include "../army.h"
#include "../GameMap.h"
#include "select-item-dialog.h"
#include "select-army-dialog.h"

RewardDialog::RewardDialog(Player *player)
{
  d_player = player;
  reward = NULL;
  item = NULL;
  ally = NULL;

  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path()
				+ "/reward-dialog.glade");

  Gtk::Dialog *d = 0;
  xml->get_widget("dialog", d);
  dialog.reset(d);

  xml->get_widget("gold_hbox", gold_hbox);
  xml->get_widget("gold_radiobutton", gold_radiobutton);
  gold_radiobutton->signal_toggled().connect
    (sigc::mem_fun(*this, &RewardDialog::on_gold_toggled));
  xml->get_widget("item_hbox", item_hbox);
  xml->get_widget("item_radiobutton", item_radiobutton);
  item_radiobutton->signal_toggled().connect
    (sigc::mem_fun(*this, &RewardDialog::on_item_toggled));
  xml->get_widget("allies_hbox", allies_hbox);
  xml->get_widget("allies_radiobutton", allies_radiobutton);
  allies_radiobutton->signal_toggled().connect
    (sigc::mem_fun(*this, &RewardDialog::on_allies_toggled));
  xml->get_widget("map_hbox", map_hbox);
  xml->get_widget("map_radiobutton", map_radiobutton);
  map_radiobutton->signal_toggled().connect
    (sigc::mem_fun(*this, &RewardDialog::on_map_toggled));
  xml->get_widget("gold_spinbutton", gold_spinbutton);
  xml->get_widget("randomize_gold_button", randomize_gold_button);
  randomize_gold_button->signal_clicked().connect
    (sigc::mem_fun(*this, &RewardDialog::on_randomize_gold_clicked));
  on_gold_toggled();
  xml->get_widget("item_button", item_button);
  item_button->signal_clicked().connect
    (sigc::mem_fun(*this, &RewardDialog::on_item_clicked));
  xml->get_widget("clear_item_button", clear_item_button);
  clear_item_button->signal_clicked().connect
    (sigc::mem_fun(*this, &RewardDialog::on_clear_item_clicked));
  xml->get_widget("randomize_item_button", randomize_item_button);
  randomize_item_button->signal_clicked().connect
    (sigc::mem_fun(*this, &RewardDialog::on_randomize_item_clicked));
  set_item_name();

  xml->get_widget("num_allies_spinbutton", num_allies_spinbutton);
  xml->get_widget("ally_button", ally_button);
  ally_button->signal_clicked().connect
    (sigc::mem_fun(*this, &RewardDialog::on_ally_clicked));
  xml->get_widget("clear_ally_button", clear_ally_button);
  clear_ally_button->signal_clicked().connect
    (sigc::mem_fun(*this, &RewardDialog::on_clear_ally_clicked));
  xml->get_widget("randomize_allies_button", randomize_allies_button);
  randomize_allies_button->signal_clicked().connect
    (sigc::mem_fun(*this, &RewardDialog::on_randomize_allies_clicked));
  set_ally_name();

  xml->get_widget("map_x_spinbutton", map_x_spinbutton);
  xml->get_widget("map_y_spinbutton", map_y_spinbutton);
  xml->get_widget("map_width_spinbutton", map_width_spinbutton);
  xml->get_widget("map_height_spinbutton", map_height_spinbutton);
  xml->get_widget("randomize_map_button", randomize_map_button);
  randomize_map_button->signal_clicked().connect
    (sigc::mem_fun(*this, &RewardDialog::on_randomize_map_clicked));
  map_x_spinbutton->set_range (0, GameMap::getInstance()->getWidth() - 1);
  map_y_spinbutton->set_range (0, GameMap::getInstance()->getHeight() - 1);
  map_width_spinbutton->set_range (1, GameMap::getInstance()->getWidth());
  map_height_spinbutton->set_range (1, GameMap::getInstance()->getHeight());
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
      if (gold_radiobutton->get_active() == true)
	reward = new Reward_Gold(gold_spinbutton->get_value_as_int());
      else if (item_radiobutton->get_active() == true)
	reward = new Reward_Item(item);
      else if (allies_radiobutton->get_active() == true)
	reward = new Reward_Allies(ally, 
				   num_allies_spinbutton->get_value_as_int());
      else if (map_radiobutton->get_active() == true)
	reward = new Reward_Map 
	  (new Location("", Vector<int>(map_x_spinbutton->get_value_as_int(), 
					map_y_spinbutton->get_value_as_int()), 
			1), 
	   map_height_spinbutton->get_value_as_int(),
	   map_width_spinbutton->get_value_as_int());

	  
      reward->setName(reward->getDescription());
    }
  else
    {
      if (ally)
	delete ally;
      if (item)
	delete item;
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

void RewardDialog::on_randomize_gold_clicked()
{
  gold_spinbutton->set_value(Reward_Gold::getRandomGoldPieces());
}

void RewardDialog::on_item_clicked()
{
  SelectItemDialog d;
  d.run();
  if (d.get_selected_item())
    {
      on_clear_item_clicked();
      item = new Item(*(d.get_selected_item()));
      set_item_name();
    }
}

void RewardDialog::on_clear_item_clicked()
{
  if (item)
    {
      delete item;
      item = NULL;
    }
  set_item_name();
}

void RewardDialog::on_randomize_item_clicked()
{
  on_clear_item_clicked();
  item = Reward_Item::getRandomItem();
  set_item_name();
}

void RewardDialog::set_item_name()
{
  Glib::ustring name;
  if (item)
    name = item->getName();
  else
    name = _("No item");

  item_button->set_label(name);
}
    
void RewardDialog::on_ally_clicked()
{
  SelectArmyDialog d(d_player, false, true);
  d.run();
  if (d.get_selected_army())
    {
      on_clear_ally_clicked();
      ally = new Army(*(d.get_selected_army()));
      set_ally_name();
    }
}

void RewardDialog::on_clear_ally_clicked()
{
  if (ally)
    {
      delete ally;
      ally = NULL;
    }
  set_ally_name();
}

void RewardDialog::on_randomize_allies_clicked()
{
  const Army *a = Reward_Allies::randomArmyAlly();
  if (!a)
    return;
    
  on_clear_ally_clicked();
  ally = new Army(*a);
  num_allies_spinbutton->set_value(Reward_Allies::getRandomAmountOfAllies());

  set_ally_name();
}

void RewardDialog::set_ally_name()
{
  Glib::ustring name;
  if (ally)
    name = ally->getName();
  else
    name = _("No ally");

  ally_button->set_label(name);
}

void RewardDialog::on_randomize_map_clicked()
{
  int x;
  int width = GameMap::getInstance()->getWidth();
  x = rand() % (width - (width / 10));
  int y;
  int height = GameMap::getInstance()->getHeight();
  y = rand() % (height - (height / 10));
  map_x_spinbutton->set_value(x);
  map_y_spinbutton->set_value(y);
  map_width_spinbutton->set_value((rand() % (width - x)) + 1);
  map_height_spinbutton->set_value((rand() % (height - y)) + 1);
}

