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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef HERO_LEVELS_DIALOG_H
#define HERO_LEVELS_DIALOG_H

#include <memory>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <gtkmm/dialog.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treeview.h>
#include <list>
#include <SDL.h>

class Player;
class Hero;

// dialog for showing and changing the order in which army types fight
// in battle
class HeroLevelsDialog: public sigc::trackable
{
 public:
    HeroLevelsDialog(Player *player);

    void set_parent_window(Gtk::Window &parent);

    void run();

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
	Gtk::TreeModelColumn<Uint32> exp;
	Gtk::TreeModelColumn<Uint32> needs;
	Gtk::TreeModelColumn<Uint32> str;
	Gtk::TreeModelColumn<Uint32> move;
    };
    const HeroesColumns heroes_columns;
    Glib::RefPtr<Gtk::ListStore> heroes_list;
 private:
    void addHero(Hero *h);
};

#endif
