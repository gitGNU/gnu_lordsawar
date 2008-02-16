//  Copyright (C) 2007, Ole Laursen
//  Copyright (C) 2007, 2008 Ben Asselstine
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
#include <gtkmm/alignment.h>

#include "ruin-dialog.h"

#include "glade-helpers.h"
#include "../ucompose.hpp"
#include "../playerlist.h"
#include "../CreateScenarioRandomize.h"
#include "../defs.h"
#include "../ruin.h"
#include "../stack.h"
#include "../army.h"

#include "select-army-dialog.h"
#include "select-reward-dialog.h"
#include "reward-dialog.h"

RuinDialog::RuinDialog(Ruin *r, CreateScenarioRandomize *randomizer)
{
    d_randomizer = randomizer;
    ruin = r;
    reward = NULL;
    
    // copy occupant to be able to undo if the dialog is cancelled
    Stack *occupant = ruin->getOccupant();
    if (occupant)
	keeper = new Stack(*occupant);
    else
	keeper = new Stack(0, ruin->getPos());
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/ruin-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    xml->get_widget("name_entry", name_entry);
    name_entry->set_text(ruin->getName());

    xml->get_widget("type_entry", type_entry);
    type_entry->set_value(ruin->getType());

    xml->get_widget("keeper_button", keeper_button);
    keeper_button->signal_clicked().connect(
	sigc::mem_fun(this, &RuinDialog::on_keeper_clicked));

    xml->get_widget("clear_keeper_button", clear_keeper_button);
    clear_keeper_button->signal_clicked().connect(
	sigc::mem_fun(this, &RuinDialog::on_clear_keeper_clicked));

    xml->get_widget("randomize_name_button", randomize_name_button);
    randomize_name_button->signal_clicked().connect(
	sigc::mem_fun(this, &RuinDialog::on_randomize_name_clicked));

    xml->get_widget("randomize_keeper_button", randomize_keeper_button);
    randomize_keeper_button->signal_clicked().connect(
	sigc::mem_fun(this, &RuinDialog::on_randomize_keeper_clicked));

    set_keeper_name();
    xml->get_widget("sage_checkbutton", sage_button);
    sage_button->set_active(ruin->hasSage());
   
    xml->get_widget("hidden_checkbutton", hidden_button);
    hidden_button->set_active(ruin->isHidden());
    hidden_button->signal_toggled().connect(
	    sigc::mem_fun(this, &RuinDialog::on_hidden_toggled));
    // setup the player combo
    player_combobox = manage(new Gtk::ComboBoxText);

    int c = 0, player_no = 0;
    for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
	     end = Playerlist::getInstance()->end(); i != end; ++i, ++c)
    {
	Player *player = *i;
	player_combobox->append_text(player->getName());
	if (player == ruin->getOwner())
	    player_no = c;
    }

    player_combobox->set_active(player_no);


    Gtk::Alignment *alignment;
    xml->get_widget("player_alignment", alignment);
    alignment->add(*player_combobox);
    on_hidden_toggled();

    xml->get_widget("new_reward_hbox", new_reward_hbox);
    xml->get_widget("new_reward_radiobutton", new_reward_radiobutton);
    new_reward_radiobutton->signal_toggled().connect(
	sigc::mem_fun(*this, &RuinDialog::on_new_reward_toggled));
    xml->get_widget("random_reward_radiobutton", random_reward_radiobutton);
    random_reward_radiobutton->signal_toggled().connect(
	sigc::mem_fun(*this, &RuinDialog::on_random_reward_toggled));

    xml->get_widget("reward_button", reward_button);
    reward_button->signal_clicked().connect(
	sigc::mem_fun(this, &RuinDialog::on_reward_clicked));

    xml->get_widget("clear_reward_button", clear_reward_button);
    clear_reward_button->signal_clicked().connect(
	sigc::mem_fun(this, &RuinDialog::on_clear_reward_clicked));

    xml->get_widget("randomize_reward_button", randomize_reward_button);
    randomize_reward_button->signal_clicked().connect(
	sigc::mem_fun(this, &RuinDialog::on_randomize_reward_clicked));

    xml->get_widget("reward_list_button", reward_list_button);
    reward_list_button->signal_clicked().connect(
	sigc::mem_fun(this, &RuinDialog::on_reward_list_clicked));

    if (ruin->getReward() == NULL)
      random_reward_radiobutton->set_active(true);
    else
      {
	reward = ruin->getReward();
	new_reward_radiobutton->set_active(true);
      }

    set_reward_name();
}

void RuinDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void RuinDialog::run()
{
    dialog->show_all();
    int response = dialog->run();

    if (response == 0)		// accepted
    {
        ruin->setName(name_entry->get_text());
	ruin->setType(type_entry->get_value_as_int());

	// get rid of old occupant and insert new
	Stack *occupant = ruin->getOccupant();
	if (occupant)
	    delete occupant;

	if (keeper->empty())
	{
	    delete keeper;
	    keeper = 0;
	}

        ruin->setSage(sage_button->get_active());
	if (sage_button->get_active() == false)
	  ruin->setOccupant(keeper);
        ruin->setHidden(hidden_button->get_active());
        if (hidden_button->get_active())
          {
	    // set owner
	    int c = 0, row = player_combobox->get_active_row_number();
	    Player *player = Playerlist::getInstance()->getNeutral();
	    for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
		     end = Playerlist::getInstance()->end(); i != end; ++i, ++c)
	        if (c == row)
	        {
		    player = *i;
		    break;
	        }
	    ruin->setOwner(player);
          }
        else
          ruin->setOwner(NULL);
	keeper = 0;
      ruin->setReward(reward);
    }
    else
    {
        //put the ruin name back.
	if (name_entry->get_text() != DEFAULT_RUIN_NAME)
	  d_randomizer->pushRandomRuinName(name_entry->get_text());
	delete keeper;
	keeper = 0;
    }
}

void RuinDialog::set_keeper_name()
{
    Glib::ustring name;
    if (keeper && !keeper->empty())
	name = keeper->getStrongestArmy()->getName();
    else
	name = _("No keeper");
    
    keeper_button->set_label(name);
}

void RuinDialog::on_hidden_toggled()
{
  if (hidden_button->get_active())
    player_combobox->set_sensitive (true);
  else
    player_combobox->set_sensitive (false);
}

void RuinDialog::on_clear_keeper_clicked()
{
  keeper->flClear();
  set_keeper_name();
}

void RuinDialog::on_keeper_clicked()
{
    SelectArmyDialog d(keeper->getPlayer(), true);
    d.set_parent_window(*dialog.get());
    d.run();

    const Army *army = d.get_selected_army();
    if (army)
      {
	keeper->flClear();
	Army *a = new Army(*army);
	keeper->push_back(a);
      }

    set_keeper_name();
}

void RuinDialog::on_randomize_name_clicked()
{
  std::string existing_name = name_entry->get_text();
  if (existing_name == DEFAULT_RUIN_NAME)
    name_entry->set_text(d_randomizer->popRandomRuinName());
  else
    {
      name_entry->set_text(d_randomizer->popRandomRuinName());
      d_randomizer->pushRandomRuinName(existing_name);
    }
}

void RuinDialog::on_randomize_keeper_clicked()
{
  keeper->flClear();
  Army *a = d_randomizer->getRandomRuinKeeper
    (Playerlist::getInstance()->getNeutral());
  keeper->push_back(a);
  set_keeper_name();
}

void RuinDialog::on_new_reward_toggled()
{
  new_reward_hbox->set_sensitive(true);
}

void RuinDialog::on_random_reward_toggled()
{
  new_reward_hbox->set_sensitive(false);
}

void RuinDialog::on_reward_list_clicked()
{
  SelectRewardDialog d;
  d.set_parent_window(*dialog.get());
  d.run();
  const Reward *picked_reward = d.get_selected_reward();
  if (picked_reward)
    {
      on_clear_reward_clicked();
      reward = new Reward(*picked_reward);
    }

    set_reward_name();
}

void RuinDialog::on_reward_clicked()
{
  RewardDialog d(keeper->getPlayer(), false);
  d.run();
  if (d.get_reward())
    {
      on_clear_reward_clicked();
      reward = d.get_reward();
      set_reward_name();
    }
}

void RuinDialog::on_clear_reward_clicked()
{
  if (reward)
    {
      delete reward;
      reward = NULL;
    }
  set_reward_name();
}

void RuinDialog::on_randomize_reward_clicked()
{
  on_clear_reward_clicked();
  reward = d_randomizer->getNewRandomReward(false);
  set_reward_name();
}

void RuinDialog::set_reward_name()
{
  Glib::ustring name;
  if (reward)
    name = reward->getName();
  else
    name = _("No reward");

  reward_button->set_label(name);
}

