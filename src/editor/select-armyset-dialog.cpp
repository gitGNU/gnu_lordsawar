//  Copyright (C) 2009 Ben Asselstine
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
#include <assert.h>

#include "select-armyset-dialog.h"

#include "glade-helpers.h"
#include "gui/input-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "armyset.h"
#include "armysetlist.h"

SelectArmysetDialog::SelectArmysetDialog()
{
    selected_armyset = 0;
    
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/select-armyset-dialog.ui");

    xml->get_widget("dialog", dialog);
    
    xml->get_widget("select_button", select_button);

    xml->get_widget("armysets_treeview", armysets_treeview);
    armysets_list = Gtk::ListStore::create(armysets_columns);
    armysets_treeview->set_model(armysets_list);
    armysets_treeview->append_column("", armysets_columns.name);
    armysets_treeview->set_headers_visible(false);

    Armysetlist *armysetlist = Armysetlist::getInstance();
    Armysetlist::iterator iter = armysetlist->begin();
    for (;iter != armysetlist->end(); iter++)
      addArmyset(*iter);
      
    guint32 max = armysetlist->size();
    if (max)
      {
	Gtk::TreeModel::Row row;
	row = armysets_treeview->get_model()->children()[0];
	if(row)
	  armysets_treeview->get_selection()->select(row);
      }
}

SelectArmysetDialog::~SelectArmysetDialog()
{
  delete dialog;
}
void SelectArmysetDialog::addArmyset(Armyset*armyset)
{
  Gtk::TreeIter i = armysets_list->append();
  (*i)[armysets_columns.name] = armyset->getName();
  (*i)[armysets_columns.armyset] = armyset;
}

void SelectArmysetDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void SelectArmysetDialog::run()
{
    dialog->show_all();
    int response = dialog->run();

    if (response != Gtk::RESPONSE_ACCEPT)
	selected_armyset = 0;
    else
      {
	Glib::RefPtr<Gtk::TreeSelection> selection = 
	  armysets_treeview->get_selection();
	Gtk::TreeModel::iterator iterrow = selection->get_selected();

	if (iterrow) 
	  {
	    Gtk::TreeModel::Row row = *iterrow;
	    selected_armyset = row[armysets_columns.armyset];
	  }
      }
}

