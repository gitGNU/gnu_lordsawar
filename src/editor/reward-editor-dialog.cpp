//  Copyright (C) 2008, 2009, 2011, 2014 Ben Asselstine
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

#include "reward-editor-dialog.h"

#include "glade-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "reward.h"
#include "ruin.h"
#include "Item.h"
#include "army.h"
#include "GameMap.h"
#include "select-item-dialog.h"
#include "select-army-dialog.h"
#include "select-hidden-ruin-dialog.h"
#include "armyproto.h"

RewardEditorDialog::RewardEditorDialog(Gtk::Window &parent, Player *player, bool hidden_ruins, Reward *r)
{
  d_player = player;
  d_hidden_ruins = hidden_ruins;
  hidden_ruin = NULL;
  reward = NULL;
  item = NULL;
  ally = NULL;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path()
				+ "/reward-editor-dialog.ui");

  xml->get_widget("dialog", dialog);
  dialog->set_transient_for(parent);
  xml->get_widget("gold_hbox", gold_hbox);
  xml->get_widget("gold_radiobutton", gold_radiobutton);
  gold_radiobutton->signal_toggled().connect
    (sigc::mem_fun(*this, &RewardEditorDialog::on_gold_toggled));
  xml->get_widget("item_hbox", item_hbox);
  xml->get_widget("item_radiobutton", item_radiobutton);
  item_radiobutton->signal_toggled().connect
    (sigc::mem_fun(*this, &RewardEditorDialog::on_item_toggled));
  xml->get_widget("allies_hbox", allies_hbox);
  xml->get_widget("allies_radiobutton", allies_radiobutton);
  allies_radiobutton->signal_toggled().connect
    (sigc::mem_fun(*this, &RewardEditorDialog::on_allies_toggled));
  xml->get_widget("map_hbox", map_hbox);
  xml->get_widget("map_radiobutton", map_radiobutton);
  map_radiobutton->signal_toggled().connect
    (sigc::mem_fun(*this, &RewardEditorDialog::on_map_toggled));
  xml->get_widget("hidden_ruin_hbox", hidden_ruin_hbox);
  xml->get_widget("hidden_ruin_radiobutton", hidden_ruin_radiobutton);
  hidden_ruin_radiobutton->signal_toggled().connect
    (sigc::mem_fun(*this, &RewardEditorDialog::on_hidden_ruin_toggled));
  xml->get_widget("gold_spinbutton", gold_spinbutton);
  xml->get_widget("randomize_gold_button", randomize_gold_button);
  randomize_gold_button->signal_clicked().connect
    (sigc::mem_fun(*this, &RewardEditorDialog::on_randomize_gold_clicked));
  on_gold_toggled();
  xml->get_widget("item_button", item_button);
  item_button->signal_clicked().connect
    (sigc::mem_fun(*this, &RewardEditorDialog::on_item_clicked));
  xml->get_widget("clear_item_button", clear_item_button);
  clear_item_button->signal_clicked().connect
    (sigc::mem_fun(*this, &RewardEditorDialog::on_clear_item_clicked));
  xml->get_widget("randomize_item_button", randomize_item_button);
  randomize_item_button->signal_clicked().connect
    (sigc::mem_fun(*this, &RewardEditorDialog::on_randomize_item_clicked));
  set_item_name();

  xml->get_widget("num_allies_spinbutton", num_allies_spinbutton);
  xml->get_widget("ally_button", ally_button);
  ally_button->signal_clicked().connect
    (sigc::mem_fun(*this, &RewardEditorDialog::on_ally_clicked));
  xml->get_widget("clear_ally_button", clear_ally_button);
  clear_ally_button->signal_clicked().connect
    (sigc::mem_fun(*this, &RewardEditorDialog::on_clear_ally_clicked));
  xml->get_widget("randomize_allies_button", randomize_allies_button);
  randomize_allies_button->signal_clicked().connect
    (sigc::mem_fun(*this, &RewardEditorDialog::on_randomize_allies_clicked));
  set_ally_name();

  xml->get_widget("map_x_spinbutton", map_x_spinbutton);
  xml->get_widget("map_y_spinbutton", map_y_spinbutton);
  xml->get_widget("map_width_spinbutton", map_width_spinbutton);
  xml->get_widget("map_height_spinbutton", map_height_spinbutton);
  xml->get_widget("randomize_map_button", randomize_map_button);
  randomize_map_button->signal_clicked().connect
    (sigc::mem_fun(*this, &RewardEditorDialog::on_randomize_map_clicked));
  map_x_spinbutton->set_range (0, GameMap::getInstance()->getWidth() - 1);
  map_y_spinbutton->set_range (0, GameMap::getInstance()->getHeight() - 1);
  map_width_spinbutton->set_range (1, GameMap::getInstance()->getWidth());
  map_height_spinbutton->set_range (1, GameMap::getInstance()->getHeight());

  xml->get_widget("hidden_ruin_button", hidden_ruin_button);
  hidden_ruin_button->signal_clicked().connect
    (sigc::mem_fun(*this, &RewardEditorDialog::on_hidden_ruin_clicked));
  xml->get_widget("clear_hidden_ruin_button", clear_hidden_ruin_button);
  clear_hidden_ruin_button->signal_clicked().connect
    (sigc::mem_fun(*this, &RewardEditorDialog::on_clear_hidden_ruin_clicked));
  xml->get_widget("randomize_hidden_ruin_button", randomize_hidden_ruin_button);
  randomize_hidden_ruin_button->signal_clicked().connect
    (sigc::mem_fun(*this, &RewardEditorDialog::on_randomize_hidden_ruin_clicked));
  set_hidden_ruin_name();
  hidden_ruin_radiobutton->set_sensitive(hidden_ruins);

  if (r)
    {
      if (r->getType() == Reward::ITEM)
	{
	  reward = new Reward_Item(*static_cast<Reward_Item*>(r));
	  item = static_cast<Reward_Item*>(reward)->getItem();
	}
      else if (r->getType() == Reward::ALLIES)
	{
	  reward = new Reward_Allies(*static_cast<Reward_Allies*>(r));
	  ally = new ArmyProto(*static_cast<Reward_Allies*>(reward)->getArmy());
	}
      else if (r->getType() == Reward::RUIN)
	{
	  reward = new Reward_Ruin(*static_cast<Reward_Ruin*>(r));
	  hidden_ruin = new Ruin(*static_cast<Reward_Ruin*>(reward)->getRuin());
	}
      else if (r->getType() == Reward::MAP)
	reward = new Reward_Map(*static_cast<Reward_Map*>(r));
      else if (r->getType() == Reward::GOLD)
	reward = new Reward_Gold(*static_cast<Reward_Gold*>(r));
    }
    
  if (reward)
    fill_in_reward_info();
}

RewardEditorDialog::~RewardEditorDialog()
{
  delete dialog;
}

void RewardEditorDialog::fill_in_reward_info()
{
  if (reward->getType() == Reward::GOLD)
    {
      Reward_Gold *r = static_cast<Reward_Gold*>(reward);
      gold_spinbutton->set_value(r->getGold());
      gold_radiobutton->set_active(true);
    }
  else if (reward->getType() == Reward::ITEM)
    {
      set_item_name();
      item_radiobutton->set_active(true);
    }
  else if (reward->getType() == Reward::ALLIES)
    {
      Reward_Allies *r = static_cast<Reward_Allies*>(reward);
      num_allies_spinbutton->set_value(r->getNoOfAllies());
      set_ally_name();
      allies_radiobutton->set_active(true);
    }
  else if (reward->getType() == Reward::MAP)
    {
      Reward_Map *r = static_cast<Reward_Map*>(reward);
      map_x_spinbutton->set_value(r->getLocation().x);
      map_y_spinbutton->set_value(r->getLocation().y);
      map_width_spinbutton->set_value(r->getWidth());
      map_height_spinbutton->set_value(r->getHeight());
	  map_radiobutton->set_active(true);
    }
  else if (reward->getType() == Reward::RUIN)
    {
      set_hidden_ruin_name();
      hidden_ruin_radiobutton->set_active(true);
    }

  //reward holds a reward
}

int RewardEditorDialog::run()
{
  dialog->show_all();
  int response = dialog->run();

  if (response == Gtk::RESPONSE_ACCEPT)	// accepted
    {
      if (gold_radiobutton->get_active() == true)
	reward = new Reward_Gold(gold_spinbutton->get_value_as_int());
      else if (item_radiobutton->get_active() == true && item)
	reward = new Reward_Item(item);
      else if (allies_radiobutton->get_active() == true && ally)
	reward = new Reward_Allies(ally, 
				   num_allies_spinbutton->get_value_as_int());
      else if (map_radiobutton->get_active() == true)
	reward = new Reward_Map 
	  (Vector<int>(map_x_spinbutton->get_value_as_int(), 
		       map_y_spinbutton->get_value_as_int()), "",
	   map_height_spinbutton->get_value_as_int(),
	   map_width_spinbutton->get_value_as_int());
      else if (hidden_ruin_radiobutton->get_active() == true && hidden_ruin)
	reward = new Reward_Ruin(hidden_ruin);
      else
	{
	  if (reward)
	    {
	      delete reward;
	      reward = NULL;
	    }
	}

	  
      if (reward)
	{
	reward->setName(reward->getDescription());
	}
    }
  else
    {
      if (ally)
	delete ally;
      if (item)
	delete item;
    }
  return response;
}


void RewardEditorDialog::on_gold_toggled()
{
  gold_hbox->set_sensitive(true);
  item_hbox->set_sensitive(false);
  allies_hbox->set_sensitive(false);
  map_hbox->set_sensitive(false);
  hidden_ruin_hbox->set_sensitive(false);
}

void RewardEditorDialog::on_item_toggled()
{
  gold_hbox->set_sensitive(false);
  item_hbox->set_sensitive(true);
  allies_hbox->set_sensitive(false);
  map_hbox->set_sensitive(false);
  hidden_ruin_hbox->set_sensitive(false);
}

void RewardEditorDialog::on_allies_toggled()
{
  gold_hbox->set_sensitive(false);
  item_hbox->set_sensitive(false);
  allies_hbox->set_sensitive(true);
  map_hbox->set_sensitive(false);
  hidden_ruin_hbox->set_sensitive(false);
}

void RewardEditorDialog::on_map_toggled()
{
  gold_hbox->set_sensitive(false);
  item_hbox->set_sensitive(false);
  allies_hbox->set_sensitive(false);
  map_hbox->set_sensitive(true);
  hidden_ruin_hbox->set_sensitive(false);
}

void RewardEditorDialog::on_hidden_ruin_toggled()
{
  gold_hbox->set_sensitive(false);
  item_hbox->set_sensitive(false);
  allies_hbox->set_sensitive(false);
  map_hbox->set_sensitive(false);
  hidden_ruin_hbox->set_sensitive(true);
}

void RewardEditorDialog::on_randomize_gold_clicked()
{
  gold_spinbutton->set_value(Reward_Gold::getRandomGoldPieces());
}

void RewardEditorDialog::on_item_clicked()
{
  SelectItemDialog d(*dialog);
  d.run();
  guint32 id = 0;
  const ItemProto *itemproto = d.get_selected_item(id);
  if (itemproto)
    {
      on_clear_item_clicked();
      item = new Item(*itemproto, id);
      set_item_name();
    }
}

void RewardEditorDialog::on_clear_item_clicked()
{
  if (item)
    {
      delete item;
      item = NULL;
    }
  set_item_name();
}

void RewardEditorDialog::on_randomize_item_clicked()
{
  on_clear_item_clicked();
  item = Reward_Item::getRandomItem();
  set_item_name();
}

void RewardEditorDialog::set_item_name()
{
  Glib::ustring name;
  if (item)
    name = item->getName();
  else
    name = _("No item");

  item_button->set_label(name);
}
    
void RewardEditorDialog::on_ally_clicked()
{
  SelectArmyDialog d(*dialog, d_player, false, false, true);
  d.run();
  if (d.get_selected_army())
    {
      on_clear_ally_clicked();
      ally = new ArmyProto(*(d.get_selected_army()));
      set_ally_name();
    }
}

void RewardEditorDialog::on_clear_ally_clicked()
{
  if (ally)
    {
      delete ally;
      ally = NULL;
    }
  set_ally_name();
}

void RewardEditorDialog::on_randomize_allies_clicked()
{
  const ArmyProto *a = Reward_Allies::randomArmyAlly();
  if (!a)
    return;
    
  on_clear_ally_clicked();
  ally = new ArmyProto(*a);
  num_allies_spinbutton->set_value(Reward_Allies::getRandomAmountOfAllies());

  set_ally_name();
}

void RewardEditorDialog::set_ally_name()
{
  Glib::ustring name;
  if (ally)
    name = ally->getName();
  else
    name = _("No ally");

  ally_button->set_label(name);
}

void RewardEditorDialog::on_randomize_map_clicked()
{
  int x, y, width, height;
  Reward_Map::getRandomMap(&x, &y, &width, &height);
  map_x_spinbutton->set_value(x);
  map_y_spinbutton->set_value(y);
  map_width_spinbutton->set_value(width);
  map_height_spinbutton->set_value(height);
}

void RewardEditorDialog::on_hidden_ruin_clicked()
{
  SelectHiddenRuinDialog d(*dialog);
  d.run();
  if (d.get_selected_hidden_ruin())
    {
      on_clear_hidden_ruin_clicked();
      hidden_ruin = new Ruin(*(d.get_selected_hidden_ruin()));
      set_hidden_ruin_name();
    }
}

void RewardEditorDialog::on_clear_hidden_ruin_clicked()
{
  if (hidden_ruin)
    {
      delete hidden_ruin;
      hidden_ruin = NULL;
    }
  set_hidden_ruin_name();
}

void RewardEditorDialog::on_randomize_hidden_ruin_clicked()
{
  Ruin *ruin = Reward_Ruin::getRandomHiddenRuin();
  if (ruin)
    {
      on_clear_hidden_ruin_clicked();
      hidden_ruin = new Ruin(*ruin);
      set_hidden_ruin_name();
    }
}

void RewardEditorDialog::set_hidden_ruin_name()
{
  Glib::ustring name;
  if (hidden_ruin)
    name = hidden_ruin->getName();
  else
    name = _("No Ruin");

  hidden_ruin_button->set_label(name);
}
