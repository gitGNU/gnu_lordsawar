//  Copyright (C) 2008 Ben Asselstine
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

#ifndef GAME_LOBBY_DIALOG_H
#define GAME_LOBBY_DIALOG_H

#include <string>
#include <memory>
#include <vector>
#include <gtkmm/dialog.h>
#include <gtkmm/image.h>
#include <gtkmm/box.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <sigc++/signal.h>
#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/cellrenderercombo.h>
#include <gtkmm/treeview.h>
#include "../citymap.h"
#include "../GameScenario.h"

struct SDL_Surface;

class Player;

//! dialog for showing the scenario and who's joined
class GameLobbyDialog//: public sigc::trackable
{
 public:
    GameLobbyDialog(std::string filename, bool has_ops);
    GameLobbyDialog(GameScenario *game_scenario, bool has_ops);

    ~GameLobbyDialog();

    void set_parent_window(Gtk::Window &parent);

    bool run();
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    //! The mini map that shows the scenario map
    std::auto_ptr<CityMap> citymap;
    Gtk::Image *map_image;

    void initDialog(GameScenario *gamescenario);
    void on_map_changed(SDL_Surface *map);
    bool d_destroy_gamescenario;
    GameScenario *d_game_scenario;
    Gtk::Label *turn_label;
    Gtk::Label *scenario_name_label;
    Gtk::Label *cities_label;
    void update_scenario_details();
    void update_player_details();
    void on_show_options_clicked();
    void update_city_map();

    Gtk::TreeView *player_treeview;
    
    class PlayerColumns: public Gtk::TreeModelColumnRecord {
    public:
	PlayerColumns()
	    {add(shield); add(type); add(name); add(status); add(turn); add(player);}
	
	Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > shield;
	Gtk::TreeModelColumn<Glib::ustring> type, name, status;
	Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > turn;
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

    void cell_data_type(Gtk::CellRenderer *renderer, const Gtk::TreeIter &i);
    void on_type_edited(const Glib::ustring &path,
			const Glib::ustring &new_text);

    Gtk::CellRendererCombo status_renderer;
    Gtk::TreeViewColumn status_column;
    
    class PlayerStatusColumns: public Gtk::TreeModelColumnRecord {
    public:
	PlayerStatusColumns()
	    { add(status); }
	
	Gtk::TreeModelColumn<Glib::ustring> status;
    };
    const PlayerTypeColumns player_status_columns;
    Glib::RefPtr<Gtk::ListStore> player_status_list;
    void cell_data_status(Gtk::CellRenderer *renderer, const Gtk::TreeIter& i);
    
    void add_player(const Glib::ustring &type, const Glib::ustring &name,
		    Player *player);
    bool d_has_ops;
};

#endif
