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

#ifndef PLAYERS_DIALOG_H
#define PLAYERS_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <gtkmm/window.h>
#include <gtkmm/dialog.h>
#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/cellrenderercombo.h>
#include <gtkmm/treeview.h>


class Player;

class PlayersDialog
{
 public:
    PlayersDialog();
    ~PlayersDialog();

    void set_parent_window(Gtk::Window &parent);

    void run();
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;

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
    
    void increment_current_player_name();
    void add_player(const Glib::ustring &type, const Glib::ustring &name,
		    int gold, Player *player);
};

#endif
