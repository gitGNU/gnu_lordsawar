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

#ifndef HERO_DIALOG_H
#define HERO_DIALOG_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>

#include "vector.h"
#include "heroesmap.h"

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
    std::auto_ptr<HeroesMap> heroesmap;

    Hero *hero;
    Vector<int> pos;
    Gtk::Image *map_image;
    Gtk::TreeView *heroes_treeview;
    Gtk::TreeView *item_treeview;
    Gtk::TreeView *events_treeview;
    Gtk::Button *drop_button;
    Gtk::Button *pickup_button;
    Gtk::Label *info_label1;
    Gtk::Label *info_label2;
    Gtk::Button *next_button;
    Gtk::Button *prev_button;

    class HeroesColumns: public Gtk::TreeModelColumnRecord {
    public:
	HeroesColumns() 
        { add(hero); add(name); }
	
	Gtk::TreeModelColumn<Hero *> hero;
	Gtk::TreeModelColumn<Glib::ustring> name;
    };
    const HeroesColumns heroes_columns;
    Glib::RefPtr<Gtk::ListStore> heroes_list;

    class ItemColumns: public Gtk::TreeModelColumnRecord {
    public:
	ItemColumns() 
        { add(image); add(name); add(attributes); add(status); add(item); }
	
	Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixmap> > image;
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

    void on_hero_changed();
    void on_item_selection_changed();
    void on_drop_clicked();
    void on_pickup_clicked();
    void on_next_clicked();
    void on_prev_clicked();

    void add_item(Item *item, bool in_backpack);
    void add_hero(Hero *hero);
    void addHistoryEvent(History *event);
    void fill_in_info_labels();

    void on_map_changed(Glib::RefPtr<Gdk::Pixmap> map);
    bool on_map_mouse_button_event(GdkEventButton *e);
    void show_hero();
    void update_buttons();
    bool inhibit_hero_changed;
    void update_hero_list();
};

#endif
