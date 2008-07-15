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

#ifndef ARMY_BONUS_DIALOG_H
#define ARMY_BONUS_DIALOG_H

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

// dialog for showing the bonuses that armies have
class ArmyBonusDialog: public sigc::trackable
{
 public:
    ArmyBonusDialog(Player *p);

    void set_parent_window(Gtk::Window &parent);

    void run();
    void hide();

 private:
    std::auto_ptr<Gtk::Dialog> dialog;

    Gtk::TreeView *armies_treeview;

    class ArmiesColumns: public Gtk::TreeModelColumnRecord {
    public:
	ArmiesColumns() 
        { add(image); add(name); add(str);
	  add(move); add(move_image); add(bonus);}
	
	Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > image;
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Uint32> str;
	Gtk::TreeModelColumn<Uint32> move;
	Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > move_image;
	Gtk::TreeModelColumn<Glib::ustring> bonus;
    };
    const ArmiesColumns armies_columns;
    Glib::RefPtr<Gtk::ListStore> armies_list;
 private:
    void addArmyType(Uint32 army_type);
    Player *d_player; //show armies in this player's colour
};

#endif
