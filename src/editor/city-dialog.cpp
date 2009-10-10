//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>
#include <gtkmm.h>

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
#include "GraphicsCache.h"

#include "select-army-dialog.h"


CityDialog::CityDialog(City *cit, CreateScenarioRandomize *randomizer)
	: strength_column(_("Strength"), strength_renderer),
	moves_column(_("Max Moves"), moves_renderer),
	duration_column(_("Turns"), duration_renderer),
	upkeep_column(_("Upkeep"), upkeep_renderer)
{
    city = cit;
    d_randomizer = randomizer;
    
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/city-dialog.ui");

    xml->get_widget("dialog", dialog);

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

    player_combobox->set_active(player_no);
    player_combobox->signal_changed().connect
      (sigc::mem_fun(this, &CityDialog::on_player_changed));
    Gtk::Alignment *alignment;
    xml->get_widget("player_alignment", alignment);
    alignment->add(*player_combobox);
    
    
    // setup the army list
    army_list = Gtk::ListStore::create(army_columns);

    xml->get_widget("army_treeview", army_treeview);
    army_treeview->set_model(army_list);
    
    army_treeview->append_column("", army_columns.image);
    strength_renderer.property_editable() = true;
    strength_renderer.signal_edited()
      .connect(sigc::mem_fun(*this, &CityDialog::on_strength_edited));
    strength_column.set_cell_data_func
	      ( strength_renderer, 
		sigc::mem_fun(*this, &CityDialog::cell_data_strength));
    army_treeview->append_column(strength_column);

    moves_renderer.property_editable() = true;
    moves_renderer.signal_edited()
      .connect(sigc::mem_fun(*this, &CityDialog::on_moves_edited));
    moves_column.set_cell_data_func
	      ( moves_renderer, 
		sigc::mem_fun(*this, &CityDialog::cell_data_moves));
    army_treeview->append_column(moves_column);

    upkeep_renderer.property_editable() = true;
    upkeep_renderer.signal_edited()
      .connect(sigc::mem_fun(*this, &CityDialog::on_upkeep_edited));
    upkeep_column.set_cell_data_func
	      ( upkeep_renderer, 
		sigc::mem_fun(*this, &CityDialog::cell_data_upkeep));
    army_treeview->append_column(upkeep_column);

    duration_renderer.property_editable() = true;
    duration_renderer.signal_edited()
      .connect(sigc::mem_fun(*this, &CityDialog::on_turns_edited));
    duration_column.set_cell_data_func
	      ( duration_renderer, 
		sigc::mem_fun(*this, &CityDialog::cell_data_turns));
    army_treeview->append_column(strength_column);

    army_treeview->append_column(_("Name"), army_columns.name);

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
CityDialog::~CityDialog()
{
  delete dialog;
}

void CityDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void CityDialog::on_player_changed()
{
  GraphicsCache *gc = GraphicsCache::getInstance();
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
		  player->getStacklist()->add(s);
		  //change the ownership of the stack
		  s->setPlayer(player);
		  //and all of it's armies
		  for (Stack::iterator it = s->begin(); it != s->end(); it++)
		    (*it)->setOwner(player);
		}
	    }
	}
    }
  for (Gtk::TreeIter j = army_list->children().begin(),
       jend = army_list->children().end(); j != jend; ++j)
    {
      const ArmyProdBase *a = (*j)[army_columns.army];
      (*j)[army_columns.image] = gc->getArmyPic(player->getArmyset(),
						a->getTypeId(), 
						player, NULL)->to_pixbuf();
    }
}

void CityDialog::run()
{
  dialog->show_all();
  int response = dialog->run();

  if (response == Gtk::RESPONSE_ACCEPT)	// accepted
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
      if (name_entry->get_text() != City::getDefaultName())
	d_randomizer->pushRandomCityName(name_entry->get_text());
    }
}

void CityDialog::on_add_clicked()
{
  SelectArmyDialog d(city->getOwner());
  d.set_parent_window(*dialog);
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
  if (existing_name == City::getDefaultName())
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
  GraphicsCache *gc = GraphicsCache::getInstance();
  Gtk::TreeIter i = army_list->append();
  (*i)[army_columns.army] = a;
  (*i)[army_columns.image] = gc->getArmyPic(city->getOwner()->getArmyset(),
					    a->getTypeId(), city->getOwner(),
					    NULL)->to_pixbuf();
  (*i)[army_columns.strength] = a->getStrength();
  (*i)[army_columns.moves] = a->getMaxMoves();
  (*i)[army_columns.upkeep] = a->getUpkeep();
  (*i)[army_columns.duration] = a->getProduction();
  (*i)[army_columns.name] = a->getName();
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

void CityDialog::cell_data_strength(Gtk::CellRenderer *renderer,
				     const Gtk::TreeIter& i)
{
    dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_adjustment()
          = new Gtk::Adjustment((*i)[army_columns.strength], 
				MIN_STRENGTH_FOR_ARMY_UNITS, 
				MAX_STRENGTH_FOR_ARMY_UNITS, 1);
    dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_text() = 
      String::ucompose("%1", (*i)[army_columns.strength]);
}

void CityDialog::on_strength_edited(const Glib::ustring &path,
				   const Glib::ustring &new_text)
{
  int str = atoi(new_text.c_str());
  if (str < MIN_STRENGTH_FOR_ARMY_UNITS || str > MAX_STRENGTH_FOR_ARMY_UNITS)
    return;
  (*army_list->get_iter(Gtk::TreePath(path)))[army_columns.strength] = str;
}

void CityDialog::cell_data_moves(Gtk::CellRenderer *renderer,
				  const Gtk::TreeIter& i)
{
    dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_adjustment()
          = new Gtk::Adjustment((*i)[army_columns.moves], 
				MIN_MOVES_FOR_ARMY_UNITS, 
				MAX_MOVES_FOR_ARMY_UNITS, 1);
    dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_text() = 
      String::ucompose("%1", (*i)[army_columns.moves]);
}

void CityDialog::on_moves_edited(const Glib::ustring &path,
				   const Glib::ustring &new_text)
{
  int moves = atoi(new_text.c_str());
  if (moves < MIN_MOVES_FOR_ARMY_UNITS || moves > MAX_MOVES_FOR_ARMY_UNITS)
    return;
  (*army_list->get_iter(Gtk::TreePath(path)))[army_columns.moves] = moves;
}

void CityDialog::cell_data_turns(Gtk::CellRenderer *renderer,
				   const Gtk::TreeIter& i)
{
    dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_adjustment()
          = new Gtk::Adjustment((*i)[army_columns.duration], 
				MIN_PRODUCTION_TURNS_FOR_ARMY_UNITS, 
				MAX_PRODUCTION_TURNS_FOR_ARMY_UNITS, 1);
    dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_text() = 
      String::ucompose("%1", (*i)[army_columns.duration]);
}

void CityDialog::on_turns_edited(const Glib::ustring &path,
				   const Glib::ustring &new_text)
{
  int turns = atoi(new_text.c_str());
  if (turns < MIN_PRODUCTION_TURNS_FOR_ARMY_UNITS || 
      turns > MAX_PRODUCTION_TURNS_FOR_ARMY_UNITS)
    return;
  (*army_list->get_iter(Gtk::TreePath(path)))[army_columns.duration] = turns;
}

void CityDialog::cell_data_upkeep(Gtk::CellRenderer *renderer,
				   const Gtk::TreeIter& i)
{
    dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_adjustment()
          = new Gtk::Adjustment((*i)[army_columns.upkeep], 0, 20, 1);
    dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_text() = 
      String::ucompose("%1", (*i)[army_columns.upkeep]);
}

void CityDialog::on_upkeep_edited(const Glib::ustring &path,
				   const Glib::ustring &new_text)
{
  int upkeep = atoi(new_text.c_str());
  if (upkeep < 0 || upkeep > 20)
    return;
  (*army_list->get_iter(Gtk::TreePath(path)))[army_columns.upkeep] = upkeep;
}

