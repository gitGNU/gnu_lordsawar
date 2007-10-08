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
#include <sigc++/functors/mem_fun.h>
#include <gtkmm/alignment.h>

#include "city-dialog.h"

#include "glade-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../city.h"
#include "../army.h"
#include "../playerlist.h"
#include "../stacklist.h"
#include "../citylist.h"

#include "select-army-dialog.h"


CityDialog::CityDialog(City *cit)
{
    city = cit;
    
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
	if (player == city->getPlayer())
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

    add_button->signal_clicked().connect(
	sigc::mem_fun(this, &CityDialog::on_add_clicked));
    remove_button->signal_clicked().connect(
	sigc::mem_fun(this, &CityDialog::on_remove_clicked));

    army_treeview->get_selection()->signal_changed()
	.connect(sigc::mem_fun(this, &CityDialog::on_selection_changed));

    for (int i = 0; i < city->getMaxNoOfBasicProd(); i++)
    {
	const Army* a = city->getArmy(i);
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
  city->setPlayer(player);
  //look for stacks in the city, and set them to this player
  for (unsigned int x = 0; x < city->getSize(); x++)
    {
      for (unsigned int y = 0; y < city->getSize(); y++)
	{
	  Stack *s = Stacklist::getObjectAt(city->getPos().x + x, 
					    city->getPos().y + y);
	  if (s)
	    {
	      if (s->getPlayer() != player)
		{
		  //remove it from the old player's list of stacks
		  s->getPlayer()->getStacklist()->remove(s);
		  //and give it to the new player list of stacks
		  player->getStacklist()->push_back(s);
		  //change the ownership of the stack
		  s->setPlayer(player);
		  //and all of it's armies
		  for (Stack::iterator it = s->begin(); it != s->end(); it++)
		    (*it)->setPlayer(player);
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
      int c = 0;
      // set attributes
      bool capital = capital_checkbutton->get_active();
      if (capital)
	{
	  // make sure player doesn't have other capitals
	  Citylist* cl = Citylist::getInstance();
	  for (Citylist::iterator i = cl->begin(); i != cl->end(); ++i)
	    if ((*i).isCapital() && (*i).getPlayer() == city->getPlayer())
	      {
		(*i).setCapital(false);
		(*i).setCapitalOwner(NULL);
	      }
	  city->setCapital(true);
	  city->setCapitalOwner(city->getPlayer());
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
	  const Army *a = (*i)[army_columns.army];
	  Army *army = new Army(*a);
	  army->setStat(Army::STRENGTH, (*i)[army_columns.strength]);
	  army->setProduction((*i)[army_columns.duration]);
	  city->addBasicProd(c, army);

	  // FIXME: use (*i)[army_columns.duration] to set special city
	  // production ability
	}
      for (; c < city->getMaxNoOfBasicProd(); ++c)
	city->removeBasicProd(c);
    }
}

void CityDialog::on_add_clicked()
{
  SelectArmyDialog d(city->getPlayer());
  d.set_parent_window(*dialog.get());
  d.run();

  const Army *army = d.get_selected_army();
  if (army)
    add_army(army);
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

void CityDialog::add_army(const Army *a)
{
  Gtk::TreeIter i = army_list->append();
  (*i)[army_columns.army] = a;
  (*i)[army_columns.name] = a->getName();
  (*i)[army_columns.strength] = a->getStat(Army::STRENGTH, false);
  (*i)[army_columns.moves] = a->getStat(Army::MOVES, false);
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
  int armies = army_list->children().size();
  add_button->set_sensitive(armies < city->getMaxNoOfBasicProd());
  remove_button->set_sensitive(i);
}

