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

#ifndef PLAYERS_DIALOG_H
#define PLAYERS_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <gtkmm.h>

class Player;

//! Scenario editor.  Edit Player objects in the scenario.
class PlayersDialog
{
 public:
    PlayersDialog(int width, int height);
    ~PlayersDialog();

    void set_parent_window(Gtk::Window &parent);

    void run();
    
 private:
    Gtk::Dialog* dialog;

    Gtk::TreeView *player_treeview;
    
    class PlayerColumns: public Gtk::TreeModelColumnRecord {
    public:
	PlayerColumns()
	    { add(type); add(name); add(gold); add(player); }
	
	Gtk::TreeModelColumn<Glib::ustring> type, name;
	Gtk::TreeModelColumn<int> gold;
	Gtk::TreeModelColumn<Player *> player;
    };
    const PlayerColumns player_columns;
    Glib::RefPtr<Gtk::ListStore> player_list;
    
    Gtk::CellRendererCombo type_renderer;
    Gtk::TreeViewColumn type_column;
    
    class PlayerTypeColumns: public Gtk::TreeModelColumnRecord {
    public:
	PlayerTypeColumns()
	    { add(type); }
	
	Gtk::TreeModelColumn<Glib::ustring> type;
    };
    const PlayerTypeColumns player_type_columns;
    Glib::RefPtr<Gtk::ListStore> player_type_list;

    typedef std::vector<Glib::ustring> player_name_seq;
    player_name_seq default_player_names;
    
    void cell_data_type(Gtk::CellRenderer *renderer, const Gtk::TreeIter &i);
    void on_type_edited(const Glib::ustring &path,
			const Glib::ustring &new_text);
    
    void add_player(const Glib::ustring &type, const Glib::ustring &name,
		    int gold, Player *player);
    int d_width;
    int d_height;
};

#endif
