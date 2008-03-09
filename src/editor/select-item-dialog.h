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

#ifndef SELECT_ITEM_DIALOG_H
#define SELECT_ITEM_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm/dialog.h>
#include <gtkmm/container.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treeview.h>
#include <gtkmm/textview.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>


class Item;

//! Scenario editor.  Select an Item object from the Itemlist.
class SelectItemDialog: public sigc::trackable
{
 public:
    SelectItemDialog();

    void set_parent_window(Gtk::Window &parent);

    void run();

    const Item *get_selected_item() { return selected_item; }
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    Gtk::Button *select_button;

    const Item *selected_item;

    Gtk::TreeView *items_treeview;
    class ItemsColumns: public Gtk::TreeModelColumnRecord {
    public:
	ItemsColumns() 
        { add(name); add(item);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Item *> item;
    };
    const ItemsColumns items_columns;
    Glib::RefPtr<Gtk::ListStore> items_list;


    void addItem(Item *item);
    
    void set_select_button_state();
};

#endif
