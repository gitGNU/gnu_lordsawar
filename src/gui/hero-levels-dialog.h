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

#ifndef HERO_LEVELS_DIALOG_H
#define HERO_LEVELS_DIALOG_H

#include <memory>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <gtkmm.h>
#include <list>

class Player;
class Hero;

#include "decorated.h"

// dialog for showing hero information
class HeroLevelsDialog: public Decorated
{
 public:
    HeroLevelsDialog(Player *player);
    HeroLevelsDialog(std::list<Hero*> heroes);

    void set_parent_window(Gtk::Window &parent);

    void run();
    void hide();

 private:
    std::auto_ptr<Gtk::Dialog> dialog;

    Player *player;
    Gtk::TreeView *heroes_treeview;

    class HeroesColumns: public Gtk::TreeModelColumnRecord {
    public:
	HeroesColumns() 
        { add(image); add(name); add(level);
	  add(exp); add(needs); add(str); add(move);}
	
	Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > image;
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Glib::ustring> level;
	Gtk::TreeModelColumn<guint32> exp;
	Gtk::TreeModelColumn<guint32> needs;
	Gtk::TreeModelColumn<guint32> str;
	Gtk::TreeModelColumn<guint32> move;
    };
    const HeroesColumns heroes_columns;
    Glib::RefPtr<Gtk::ListStore> heroes_list;
 private:

    void init(Player *theplayer);
    void addHero(Hero *h);
};

#endif
