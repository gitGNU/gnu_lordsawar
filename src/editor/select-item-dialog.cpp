//  Copyright (C) 2008, 2009 Ben Asselstine
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

#include "select-item-dialog.h"

#include "glade-helpers.h"
#include "gui/input-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "Item.h"
#include "Itemlist.h"

SelectItemDialog::SelectItemDialog()
{
    selected_item = 0;
    
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/select-item-dialog.ui");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    
    xml->get_widget("select_button", select_button);

    xml->get_widget("items_treeview", items_treeview);
    items_list = Gtk::ListStore::create(items_columns);
    items_treeview->set_model(items_list);
    items_treeview->append_column("", items_columns.name);
    items_treeview->set_headers_visible(false);

    Itemlist *itemlist = Itemlist::getInstance();
    Itemlist::iterator iter = itemlist->begin();
    for (;iter != itemlist->end(); iter++)
      addItemProto((*iter).second);
      
    guint32 max = itemlist->size();
    if (max)
      {
	Gtk::TreeModel::Row row;
	row = items_treeview->get_model()->children()[0];
	if(row)
	  items_treeview->get_selection()->select(row);
      }
}

void SelectItemDialog::addItemProto(ItemProto *item)
{
  Gtk::TreeIter i = items_list->append();
  (*i)[items_columns.name] = item->getName();
  (*i)[items_columns.item] = item;
}

void SelectItemDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void SelectItemDialog::run()
{
    dialog->show_all();
    int response = dialog->run();

    if (response != Gtk::RESPONSE_ACCEPT)
	selected_item = 0;
    else
      {
	Glib::RefPtr<Gtk::TreeSelection> selection = 
	  items_treeview->get_selection();
	Gtk::TreeModel::iterator iterrow = selection->get_selected();

	if (iterrow) 
	  {
	    Gtk::TreeModel::Row row = *iterrow;
	    selected_item = row[items_columns.item];
	  }
      }
}

