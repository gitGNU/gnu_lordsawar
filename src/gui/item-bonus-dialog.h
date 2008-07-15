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

#ifndef ITEM_BONUS_DIALOG_H
#define ITEM_BONUS_DIALOG_H

#include <memory>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <gtkmm/dialog.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treeview.h>
#include <list>
#include <SDL.h>
class Item;

// dialog for showing the bonuses that items have
class ItemBonusDialog: public sigc::trackable
{
 public:
    ItemBonusDialog();

    void set_parent_window(Gtk::Window &parent);

    void hide();
    void run();

 private:
    std::auto_ptr<Gtk::Dialog> dialog;

    Gtk::TreeView *items_treeview;

    class ItemsColumns: public Gtk::TreeModelColumnRecord {
    public:
	ItemsColumns() 
        { add(name); add(bonus);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Glib::ustring> bonus;
    };
    const ItemsColumns items_columns;
    Glib::RefPtr<Gtk::ListStore> items_list;
 private:
    void addItem(Item *item);
};

#endif
