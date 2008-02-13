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

#include "stack-dialog.h"

#include "glade-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../stack.h"
#include "../army.h"
#include "../playerlist.h"
#include "../stacklist.h"

#include "select-army-dialog.h"


namespace 
{
    int const max_stack_size = 8;
}

StackDialog::StackDialog(Stack *s, int m)
{
    stack = s;
    min_size = m;
    player_combobox = 0;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/stack-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    if (stack->getPlayer())
    {
	// setup the player combo
	player_combobox = manage(new Gtk::ComboBoxText);

	int c = 0, player_no = 0;
	for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
		 end = Playerlist::getInstance()->end(); i != end; ++i, ++c)
	{
	    Player *player = *i;
	    player_combobox->append_text(player->getName());
	    if (player == stack->getPlayer())
		player_no = c;
	}

	player_combobox->set_active(player_no);

	Gtk::Box *box;
	xml->get_widget("player_hbox", box);
	box->pack_start(*player_combobox, Gtk::PACK_SHRINK);
    }
    
    // setup the army list
    army_list = Gtk::ListStore::create(army_columns);

    xml->get_widget("army_treeview", army_treeview);
    army_treeview->set_model(army_list);
    
    army_treeview->append_column(_("Name"), army_columns.name);
    // note to translators: abbreviation of Strength
    army_treeview->append_column(_("Str"), army_columns.strength);
    // note to translators: abbreviation of Moves
    army_treeview->append_column(_("Mov"), army_columns.moves);
    // note to translators: abbreviation of Hitpoints
    army_treeview->append_column(_("HP"), army_columns.hitpoints);
    // note to translators: abbreviation of Upkeep
    army_treeview->append_column(_("Upk"), army_columns.upkeep);

    xml->get_widget("add_button", add_button);
    xml->get_widget("remove_button", remove_button);

    add_button->signal_clicked().connect(
	sigc::mem_fun(this, &StackDialog::on_add_clicked));
    remove_button->signal_clicked().connect(
	sigc::mem_fun(this, &StackDialog::on_remove_clicked));

    army_treeview->get_selection()->signal_changed()
	.connect(sigc::mem_fun(this, &StackDialog::on_selection_changed));
    
    for (Stack::iterator i = stack->begin(), end = stack->end(); i != end; ++i)
	add_army(*i);
}

void StackDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void StackDialog::run()
{
    dialog->show_all();
    int response = dialog->run();

    if (response == 0)		// accepted
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

	// add added armies to stack
	for (Gtk::TreeIter j = army_list->children().begin(),
		 jend = army_list->children().end(); j != jend; ++j)
	{
	    Army *a = (*j)[army_columns.army];
	    
	    if (std::find(stack->begin(), stack->end(), a) == stack->end())
		stack->push_back(a);
	}

	// now set allegiance, it's important to do it after possibly new stack
	// armies have been added
	if (player_combobox)
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

	    Player *stack_player = stack->getPlayer();
	    if (stack_player)
		stack_player->getStacklist()->remove(stack);
	
	    player->addStack(stack);
	    stack->setPlayer(player);
	    for (Stack::iterator it = stack->begin(); it != stack->end(); it++)
	      (*it)->setPlayer(player);
	}
    }
}

void StackDialog::on_add_clicked()
{
    SelectArmyDialog d(stack->getPlayer());
    d.set_parent_window(*dialog.get());
    d.run();

    const Army *army = d.get_selected_army();
    if (army)
	add_army(new Army(*army));
}
    

void StackDialog::on_remove_clicked()
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

void StackDialog::add_army(Army *a)
{
    Gtk::TreeIter i = army_list->append();
    (*i)[army_columns.army] = a;
    (*i)[army_columns.name] = a->getName();
    (*i)[army_columns.strength] = a->getStat(Army::STRENGTH, false);
    (*i)[army_columns.moves] = a->getStat(Army::MOVES, false);
    (*i)[army_columns.hitpoints] = a->getStat(Army::HP, false);
    (*i)[army_columns.upkeep] = a->getUpkeep();

    army_treeview->get_selection()->select(i);
    
    set_button_sensitivity();
}

void StackDialog::on_selection_changed()
{
    set_button_sensitivity();
}

void StackDialog::set_button_sensitivity()
{
    Gtk::TreeIter i = army_treeview->get_selection()->get_selected();
    int armies = army_list->children().size();
    add_button->set_sensitive(armies < max_stack_size);
    remove_button->set_sensitive(armies > min_size && i);
}

