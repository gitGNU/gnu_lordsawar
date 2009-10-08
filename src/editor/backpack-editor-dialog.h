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

#ifndef BACKPACK_EDITOR_DIALOG_H
#define BACKPACK_EDITOR_DIALOG_H

#include <sigc++/trackable.h>
#include <gtkmm.h>

class Item;
class Backpack;

// dialog for showing info about a hero, esp. about the hero's items
class BackpackEditorDialog
{
 public:
    BackpackEditorDialog(Backpack *backpack);
    ~BackpackEditorDialog();

    void set_parent_window(Gtk::Window &parent);

    int run();
    void hide();
    
 private:
    Backpack *backpack; //destination backpack
    Backpack *working; //the backpack we're going to work with before that
    Gtk::Dialog* dialog;

    Gtk::TreeView *item_treeview;
    Gtk::Button *remove_button;
    Gtk::Button *add_button;

    class ItemColumns: public Gtk::TreeModelColumnRecord {
    public:
	ItemColumns() 
        { add(name); add(attributes); add(item); }
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Glib::ustring> attributes;
	Gtk::TreeModelColumn<Item *> item;
    };
    const ItemColumns item_columns;
    Glib::RefPtr<Gtk::ListStore> item_list;


    void on_item_selection_changed();
    void on_remove_item_clicked();
    void on_add_item_clicked();

    void add_item(Item *item);

    void fill_bag();
};

#endif
