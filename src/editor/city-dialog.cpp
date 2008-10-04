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

#include "city-dialog.h"

#include "glade-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "city.h"
#include "armyprodbase.h"
#include "army.h"
#include "armyproto.h"
#include "armyprodbase.h"
#include "playerlist.h"
#include "stacklist.h"
#include "citylist.h"
#include "CreateScenarioRandomize.h"

#include "select-army-dialog.h"


CityDialog::CityDialog(City *cit, CreateScenarioRandomize *randomizer)
{
    city = cit;
    d_randomizer = randomizer;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/city-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    xml->get_widget("capital_checkbutton", capital_checkbutton);
    capital_checkbutton->set_active(city->isCapital());

    xml->get_widget("name_entry", name_entry);
    name_entry->set_text(city->getName());
    
    xml->get_widget("income_spinbutton", income_spinbutton);
    income_spinbutton->set_value(city->getGold());

    xml->get_widget("burned_checkbutton", burned_checkbutton);
    burned_checkbutton->set_active(city->isBurnt());
    
    // setup the player combo
    player_combobox = manage(new Gtk::ComboBoxText);

    int c = 0, player_no = 0;
    for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
	     end = Playerlist::getInstance()->end(); i != end; ++i, ++c)
    {
	Player *player = *i;
	player_combobox->append_text(player->getName());
	if (player == city->getOwner())
	    player_no = c;
    }

    player_combobox->signal_changed().connect
      (sigc::mem_fun(this, &CityDialog::on_player_changed));
    player_combobox->set_active(player_no);
    Gtk::Alignment *alignment;
    xml->get_widget("player_alignment", alignment);
    alignment->add(*player_combobox);
    
    
    // setup the army list
    army_list = Gtk::ListStore::create(army_columns);

    xml->get_widget("army_treeview", army_treeview);
    army_treeview->set_model(army_list);
    
    army_treeview->append_column(_("Name"), army_columns.name);
    army_treeview->append_column_editable(_("Strength"), army_columns.strength);
    army_treeview->append_column(_("Max Moves"), army_columns.moves);
    army_treeview->append_column(_("Upkeep"), army_columns.upkeep);
    army_treeview->append_column_editable(_("Turns"), army_columns.duration);

    xml->get_widget("add_button", add_button);
    xml->get_widget("remove_button", remove_button);
    xml->get_widget("randomize_armies_button", randomize_armies_button);
    xml->get_widget("randomize_name_button", randomize_name_button);
    xml->get_widget("randomize_income_button", randomize_income_button);

    add_button->signal_clicked().connect(
	sigc::mem_fun(this, &CityDialog::on_add_clicked));
    remove_button->signal_clicked().connect(
	sigc::mem_fun(this, &CityDialog::on_remove_clicked));
    randomize_armies_button->signal_clicked().connect(
	sigc::mem_fun(this, &CityDialog::on_randomize_armies_clicked));
    randomize_name_button->signal_clicked().connect(
	sigc::mem_fun(this, &CityDialog::on_randomize_name_clicked));
    randomize_income_button->signal_clicked().connect(
	sigc::mem_fun(this, &CityDialog::on_randomize_income_clicked));

    army_treeview->get_selection()->signal_changed()
	.connect(sigc::mem_fun(this, &CityDialog::on_selection_changed));

    for (unsigned int i = 0; i < city->getMaxNoOfProductionBases(); i++)
    {
	const ArmyProdBase* a = city->getProductionBase(i);
	if (a)
	    add_army(a);
    }
}

void CityDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void CityDialog::on_player_changed()
{
  // set allegiance
  int c = 0, row = player_combobox->get_active_row_number();
  Player *player = Playerlist::getInstance()->getNeutral();
  for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
       end = Playerlist::getInstance()->end(); i != end; ++i, ++c)
    if (c == row)
      {
	player = *i;
	break;
      }
  city->setOwner(player);
  //look for stacks in the city, and set them to this player
  for (unsigned int x = 0; x < city->getSize(); x++)
    {
      for (unsigned int y = 0; y < city->getSize(); y++)
	{
	  Stack *s = Stacklist::getObjectAt(city->getPos().x + x, 
					    city->getPos().y + y);
	  if (s)
	    {
	      if (s->getOwner() != player)
		{
		  //remove it from the old player's list of stacks
		  s->getOwner()->getStacklist()->remove(s);
		  //and give it to the new player list of stacks
		  player->getStacklist()->push_back(s);
		  //change the ownership of the stack
		  s->setPlayer(player);
		  //and all of it's armies
		  for (Stack::iterator it = s->begin(); it != s->end(); it++)
		    (*it)->setOwner(player);
		}
	    }
	}
    }
}

void CityDialog::run()
{
  dialog->show_all();
  int response = dialog->run();

  if (response == 0)		// accepted
    {
      unsigned int c = 0;
      // set attributes
      bool capital = capital_checkbutton->get_active();
      if (capital)
	{
	  // make sure player doesn't have other capitals
	  Citylist* cl = Citylist::getInstance();
	  for (Citylist::iterator i = cl->begin(); i != cl->end(); ++i)
	    if ((*i)->isCapital() && (*i)->getOwner() == city->getOwner())
	      {
		(*i)->setCapital(false);
		(*i)->setCapitalOwner(NULL);
	      }
	  city->setCapital(true);
	  city->setCapitalOwner(city->getOwner());
	}
      else
	{
	  city->setCapital(false);
	  city->setCapitalOwner(NULL);
	}

      city->setName(name_entry->get_text());
      city->setGold(income_spinbutton->get_value_as_int());
      city->setBurnt(burned_checkbutton->get_active());

      // set production slots
      c = 0;
      for (Gtk::TreeIter i = army_list->children().begin(),
	   end = army_list->children().end(); i != end; ++i, ++c)
	{
	  const ArmyProdBase *a = (*i)[army_columns.army];
	  ArmyProdBase *army = new ArmyProdBase(*a);
	  army->setStrength((*i)[army_columns.strength]);
	  army->setProduction((*i)[army_columns.duration]);
	  city->addProductionBase(c, army);

	  // FIXME: use (*i)[army_columns.duration] to set special city
	  // production ability
	}
      for (; c < city->getMaxNoOfProductionBases(); ++c)
	city->removeProductionBase(c);
    }
  else
    {
      if (name_entry->get_text() != DEFAULT_CITY_NAME)
	d_randomizer->pushRandomCityName(name_entry->get_text());
    }
}

void CityDialog::on_add_clicked()
{
  SelectArmyDialog d(city->getOwner());
  d.set_parent_window(*dialog.get());
  d.run();

  const ArmyProto *army = d.get_selected_army();
  if (army)
    add_army(new ArmyProdBase(*army));
}


void CityDialog::on_remove_clicked()
{
  Gtk::TreeIter i = army_treeview->get_selection()->get_selected();
  if (i)
    {
      army_list->erase(i);
    }

  set_button_sensitivity();
}

void CityDialog::on_randomize_armies_clicked()
{
  const ArmyProdBase *army;
  army_list->clear();
  city->setRandomArmytypes(true, 1);
  for (unsigned int i = 0; i < city->getMaxNoOfProductionBases(); i++)
    {
      army = city->getProductionBase(i);
      if (army)
	add_army(army);
    }
  set_button_sensitivity();
}

void CityDialog::on_randomize_name_clicked()
{
  std::string existing_name = name_entry->get_text();
  if (existing_name == DEFAULT_CITY_NAME)
    name_entry->set_text(d_randomizer->popRandomCityName());
  else
    {
      name_entry->set_text(d_randomizer->popRandomCityName());
      d_randomizer->pushRandomCityName(existing_name);
    }
}

void CityDialog::on_randomize_income_clicked()
{
  int gold;
  gold = d_randomizer->getRandomCityIncome(capital_checkbutton->get_active());
  income_spinbutton->set_value(gold);
}

void CityDialog::add_army(const ArmyProdBase *a)
{
  Gtk::TreeIter i = army_list->append();
  (*i)[army_columns.army] = a;
  (*i)[army_columns.name] = a->getName();
  (*i)[army_columns.strength] = a->getStrength();
  (*i)[army_columns.moves] = a->getMaxMoves();
  (*i)[army_columns.upkeep] = a->getUpkeep();
  (*i)[army_columns.duration] = a->getProduction();

  army_treeview->get_selection()->select(i);

  set_button_sensitivity();
}

void CityDialog::on_selection_changed()
{
  set_button_sensitivity();
}

void CityDialog::set_button_sensitivity()
{
  Gtk::TreeIter i = army_treeview->get_selection()->get_selected();
  unsigned int armies = army_list->children().size();
  add_button->set_sensitive(armies < city->getMaxNoOfProductionBases());
  remove_button->set_sensitive(i);
}

