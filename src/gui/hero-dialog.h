//  Copyright (C) 2007 Ole Laursen
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

#ifndef HERO_DIALOG_H
#define HERO_DIALOG_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm/dialog.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treeview.h>
#include <gtkmm/button.h>

#include "vector.h"

class Hero;
class Item;
class History;

#include "decorated.h"

// dialog for showing info about a hero, esp. about the hero's items
class HeroDialog: public Decorated
{
 public:
    HeroDialog(Hero *hero, Vector<int> pos);

    void set_parent_window(Gtk::Window &parent);

    void run();
    void hide();
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;

    Hero *hero;
    Vector<int> pos;
    Gtk::TreeView *item_treeview;
    Gtk::TreeView *events_treeview;
    Gtk::Button *drop_button;
    Gtk::Button *pickup_button;
    Gtk::Label *info_label1;
    Gtk::Label *info_label2;

    class ItemColumns: public Gtk::TreeModelColumnRecord {
    public:
	ItemColumns() 
        { add(image); add(name); add(attributes); add(status); add(item); }
	
	Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > image;
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Glib::ustring> attributes;
	Gtk::TreeModelColumn<Glib::ustring> status;
	Gtk::TreeModelColumn<Item *> item;
    };
    const ItemColumns item_columns;
    Glib::RefPtr<Gtk::ListStore> item_list;

    class EventsColumns: public Gtk::TreeModelColumnRecord {
    public:
	EventsColumns() 
        { add(desc); add(history); }
	
	Gtk::TreeModelColumn<Glib::ustring> desc;
	Gtk::TreeModelColumn<History *> history;
    };
    const EventsColumns events_columns;
    Glib::RefPtr<Gtk::ListStore> events_list;

    void on_selection_changed();
    void on_drop_clicked();
    void on_pickup_clicked();

    void add_item(Item *item, bool in_backpack);
    void addHistoryEvent(History *event);
    void fill_in_info_labels();
};

#endif
