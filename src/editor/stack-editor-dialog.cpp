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

#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>

#include "stack-editor-dialog.h"

#include "glade-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "stack.h"
#include "army.h"
#include "armyproto.h"
#include "hero.h"
#include "heroproto.h"
#include "GraphicsCache.h"
#include "playerlist.h"
#include "stacklist.h"
#include "hero-editor-dialog.h"
#include "GameMap.h"

#include "select-army-dialog.h"


namespace 
{
    int const max_stack_size = 8;
}

StackEditorDialog::StackEditorDialog(Stack *s, int m)
	: strength_column(_("Strength"), strength_renderer),
	moves_column(_("Max Moves"), moves_renderer),
	upkeep_column(_("Upkeep"), upkeep_renderer)
{
    stack = s;
    min_size = m;
    player_combobox = 0;
    
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/stack-editor-dialog.ui");

    xml->get_widget("dialog", dialog);

    if (stack->getOwner())
    {
	// setup the player combo
	player_combobox = manage(new Gtk::ComboBoxText);

	int c = 0, player_no = 0;
	for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
		 end = Playerlist::getInstance()->end(); i != end; ++i, ++c)
	{
	    Player *player = *i;
	    player_combobox->append_text(player->getName());
	    if (player == stack->getOwner())
		player_no = c;
	}

	player_combobox->set_active(player_no);
	player_combobox->signal_changed().connect
	  (sigc::mem_fun(*this, &StackEditorDialog::on_player_changed));

	Gtk::Box *box;
	xml->get_widget("player_hbox", box);
	box->pack_start(*player_combobox, Gtk::PACK_SHRINK);
    }
    
    // setup the army list
    army_list = Gtk::ListStore::create(army_columns);

    xml->get_widget("army_treeview", army_treeview);
    army_treeview->set_model(army_list);
    
    army_treeview->append_column("", army_columns.image);

    strength_renderer.property_editable() = true;
    strength_renderer.signal_edited()
      .connect(sigc::mem_fun(*this, &StackEditorDialog::on_strength_edited));
    strength_column.set_cell_data_func
	      ( strength_renderer, 
		sigc::mem_fun(*this, &StackEditorDialog::cell_data_strength));
    army_treeview->append_column(strength_column);

    moves_renderer.property_editable() = true;
    moves_renderer.signal_edited()
      .connect(sigc::mem_fun(*this, &StackEditorDialog::on_moves_edited));
    moves_column.set_cell_data_func
	      ( moves_renderer, 
		sigc::mem_fun(*this, &StackEditorDialog::cell_data_moves));
    army_treeview->append_column(moves_column);

    upkeep_renderer.property_editable() = true;
    upkeep_renderer.signal_edited()
      .connect(sigc::mem_fun(*this, &StackEditorDialog::on_upkeep_edited));
    upkeep_column.set_cell_data_func
	      ( upkeep_renderer, 
		sigc::mem_fun(*this, &StackEditorDialog::cell_data_upkeep));
    army_treeview->append_column(upkeep_column);

    army_treeview->append_column(_("Name"), army_columns.name);

    xml->get_widget("fortified_checkbutton", fortified_checkbutton);
    fortified_checkbutton->set_active(stack->getFortified());
    fortified_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &StackEditorDialog::on_fortified_toggled));

    xml->get_widget("add_button", add_button);
    xml->get_widget("remove_button", remove_button);
    xml->get_widget("copy_button", copy_button);
    xml->get_widget("edit_hero_button", edit_hero_button);

    add_button->signal_clicked().connect(
	sigc::mem_fun(this, &StackEditorDialog::on_add_clicked));
    remove_button->signal_clicked().connect(
	sigc::mem_fun(this, &StackEditorDialog::on_remove_clicked));
    copy_button->signal_clicked().connect(
	sigc::mem_fun(this, &StackEditorDialog::on_copy_clicked));
    edit_hero_button->signal_clicked().connect(
	sigc::mem_fun(this, &StackEditorDialog::on_edit_hero_clicked));

    army_treeview->get_selection()->signal_changed()
	.connect(sigc::mem_fun(this, &StackEditorDialog::on_selection_changed));
    
    for (Stack::iterator i = stack->begin(), end = stack->end(); i != end; ++i)
	add_army(*i);
  set_button_sensitivity();
}

StackEditorDialog::~StackEditorDialog()
{
  delete dialog;
}
void StackEditorDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

int StackEditorDialog::run()
{
    dialog->show_all();
    int response = dialog->run();

    if (response == Gtk::RESPONSE_ACCEPT)	// accepted
    {
	// remove removed armies from stack
	for (Stack::iterator i = stack->begin(), end = stack->end(); i != end;)
	{
	    Army *a = *i;
	    ++i;
	    
	    bool found = false;
	    for (Gtk::TreeIter j = army_list->children().begin(),
		     jend = army_list->children().end(); j != jend; ++j)
		if ((*j)[army_columns.army] == a)
		{
		    found = true;
		    break;
		}

	    if (!found)
	    {
		stack->remove(a);
		delete a;
	    }
	}

	//set the stats for all of the armies in our list
	for (Gtk::TreeIter j = army_list->children().begin(),
		 jend = army_list->children().end(); j != jend; ++j)
	{
	    Army *a = (*j)[army_columns.army];
	    a->setStat(Army::STRENGTH, (*j)[army_columns.strength]);
	    a->setStat(Army::MOVES, (*j)[army_columns.moves]);
	    a->setUpkeep((*j)[army_columns.upkeep]);
	}
	bool ship = stack->hasShip();
	// add added armies to stack
	for (Gtk::TreeIter j = army_list->children().begin(),
		 jend = army_list->children().end(); j != jend; ++j)
	{
	    Army *a = (*j)[army_columns.army];
	    
	    a->setInShip(ship);
	    if (std::find(stack->begin(), stack->end(), a) == stack->end())
		stack->push_back(a);
	}

	// now set allegiance, it's important to do it after possibly new stack
	// armies have been added
	if (player_combobox)
	{
	    Player *player = get_selected_player();
            if (stack->getOwner() != player)
              {
                Player *old_active = Playerlist::getActiveplayer();
                Playerlist::getInstance()->setActiveplayer(player);
                Stack *new_stack = new Stack(*stack);
                GameMap::getInstance()->removeStack(stack);
                new_stack->setPlayer(player);
                bool putted = GameMap::getInstance()->putStack(new_stack);
                stack->getOwner()->deleteStack(stack);
                player->addStack(new_stack);
                Playerlist::getInstance()->setActiveplayer(old_active);
              }
	}
    }
    return response;
}

Player *StackEditorDialog::get_selected_player()
{
  int c = 0, row = player_combobox->get_active_row_number();
  Player *player = Playerlist::getInstance()->getNeutral();
  for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
       end = Playerlist::getInstance()->end(); i != end; ++i, ++c)
    if (c == row)
      {
	player = *i;
	break;
      }
  return player;
}

void StackEditorDialog::on_copy_clicked()
{
    Gtk::TreeIter i = army_treeview->get_selection()->get_selected();
    if (i)
      {
	Player *player = get_selected_player();
	Army *army = (*i)[army_columns.army];
        Army *new_army = new Army(*army, player);
        new_army->assignNewId();
	add_army(new_army);
      }

    set_button_sensitivity();
}
void StackEditorDialog::on_add_clicked()
{
    SelectArmyDialog d(stack->getOwner(), true);
    d.set_parent_window(*dialog);
    d.run();

    Player *player = get_selected_player();
    const ArmyProto *army = d.get_selected_army();
    if (army)
      {
	if (army->isHero() == true)
	  {
	    HeroProto *hp = new HeroProto(*army);
	    hp->setOwnerId(player->getId());
	    add_army(new Hero(*hp));
	    delete hp;
	  }
	else
	  add_army(new Army(*army, player));
      }
}
    

void StackEditorDialog::on_edit_hero_clicked()
{
    Gtk::TreeIter i = army_treeview->get_selection()->get_selected();
    if (i)
    {
	Army *army = (*i)[army_columns.army];
	Hero *hero = dynamic_cast<Hero*>(army);
	HeroEditorDialog d(hero);
	d.run();
	(*i)[army_columns.name] = hero->getName();
    }

}
void StackEditorDialog::on_remove_clicked()
{
    Gtk::TreeIter i = army_treeview->get_selection()->get_selected();
    if (i)
    {
	Army *army = (*i)[army_columns.army];
	army_list->erase(i);
	if (std::find(stack->begin(), stack->end(), army) == stack->end())
	    delete army;
    }

    set_button_sensitivity();
}

void StackEditorDialog::add_army(Army *a)
{
    GraphicsCache *gc = GraphicsCache::getInstance();
    Gtk::TreeIter i = army_list->append();
    (*i)[army_columns.army] = a;
    (*i)[army_columns.image] = gc->getArmyPic(a->getOwner()->getArmyset(),
					      a->getTypeId(), a->getOwner(),
					      NULL)->to_pixbuf();
    (*i)[army_columns.strength] = a->getStat(Army::STRENGTH, false);
    (*i)[army_columns.moves] = a->getStat(Army::MOVES, false);
    (*i)[army_columns.upkeep] = a->getUpkeep();
    (*i)[army_columns.name] = a->getName();

    army_treeview->get_selection()->select(i);
    
    set_button_sensitivity();
}

void StackEditorDialog::on_selection_changed()
{
    set_button_sensitivity();
}

void StackEditorDialog::set_button_sensitivity()
{
    Gtk::TreeIter i = army_treeview->get_selection()->get_selected();
    int armies = army_list->children().size();
    add_button->set_sensitive(armies < max_stack_size);
    copy_button->set_sensitive(armies < max_stack_size);
    remove_button->set_sensitive(armies > min_size && i);
    if (i)
      {
	Army *army = (*i)[army_columns.army];
	if (army->isHero())
	  {
	    edit_hero_button->set_sensitive(true);
	    copy_button->set_sensitive(false);
	  }
	else
	  edit_hero_button->set_sensitive(false);
      }
  Player *player = get_selected_player();
  if (player == Playerlist::getInstance()->getNeutral())
    fortified_checkbutton->set_sensitive(false);
  else
    fortified_checkbutton->set_sensitive(true);
}

void StackEditorDialog::on_fortified_toggled()
{
  stack->setFortified(fortified_checkbutton->get_active());
}
	  
void StackEditorDialog::on_player_changed()
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  Player *player = get_selected_player();
  if (player == Playerlist::getInstance()->getNeutral())
    fortified_checkbutton->set_active(false);
  set_button_sensitivity();
	
  for (Gtk::TreeIter j = army_list->children().begin(),
       jend = army_list->children().end(); j != jend; ++j)
    {
      Army *a = (*j)[army_columns.army];
      (*j)[army_columns.image] = gc->getArmyPic(player->getArmyset(),
						a->getTypeId(), 
						player, NULL)->to_pixbuf();
    }
}
void StackEditorDialog::cell_data_strength(Gtk::CellRenderer *renderer,
				     const Gtk::TreeIter& i)
{
    dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_adjustment()
          = new Gtk::Adjustment((*i)[army_columns.strength], 
				MIN_STRENGTH_FOR_ARMY_UNITS, 
				MAX_STRENGTH_FOR_ARMY_UNITS, 1);
    dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_text() = 
      String::ucompose("%1", (*i)[army_columns.strength]);
}

void StackEditorDialog::on_strength_edited(const Glib::ustring &path,
				   const Glib::ustring &new_text)
{
  int str = atoi(new_text.c_str());
  if (str < (int)MIN_STRENGTH_FOR_ARMY_UNITS || str > 
      (int)MAX_STRENGTH_FOR_ARMY_UNITS)
    return;
  (*army_list->get_iter(Gtk::TreePath(path)))[army_columns.strength] = str;
}

void StackEditorDialog::cell_data_moves(Gtk::CellRenderer *renderer,
				  const Gtk::TreeIter& i)
{
    dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_adjustment()
          = new Gtk::Adjustment((*i)[army_columns.moves], 
				MIN_MOVES_FOR_ARMY_UNITS, 
				MAX_MOVES_FOR_ARMY_UNITS, 1);
    dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_text() = 
      String::ucompose("%1", (*i)[army_columns.moves]);
}

void StackEditorDialog::on_moves_edited(const Glib::ustring &path,
				   const Glib::ustring &new_text)
{
  int moves = atoi(new_text.c_str());
  if (moves < (int)MIN_MOVES_FOR_ARMY_UNITS || moves > 
      (int)MAX_MOVES_FOR_ARMY_UNITS)
    return;
  (*army_list->get_iter(Gtk::TreePath(path)))[army_columns.moves] = moves;
}

void StackEditorDialog::cell_data_upkeep(Gtk::CellRenderer *renderer,
				   const Gtk::TreeIter& i)
{
    dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_adjustment()
          = new Gtk::Adjustment((*i)[army_columns.upkeep], 
				MIN_UPKEEP_FOR_ARMY_UNITS, 
				MAX_UPKEEP_FOR_ARMY_UNITS, 1);
    dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_text() = 
      String::ucompose("%1", (*i)[army_columns.upkeep]);
}

void StackEditorDialog::on_upkeep_edited(const Glib::ustring &path,
				   const Glib::ustring &new_text)
{
  int upkeep = atoi(new_text.c_str());
  if (upkeep < (int)MIN_UPKEEP_FOR_ARMY_UNITS || 
      upkeep > (int)MAX_UPKEEP_FOR_ARMY_UNITS)
    return;
  (*army_list->get_iter(Gtk::TreePath(path)))[army_columns.upkeep] = upkeep;
}

