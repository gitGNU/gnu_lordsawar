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
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <sigc++/signal.h>
#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textview.h>
#include <gtkmm/cellrenderercombo.h>
#include <gtkmm/cellrenderertoggle.h>
#include <gtkmm/treeview.h>
#include "../citymap.h"
#include "../GameScenario.h"

struct SDL_Surface;

class Player;

//! dialog for showing the scenario and who's joined
class GameLobbyDialog//: public sigc::trackable
{
 public:
    GameLobbyDialog(GameScenario *game_scenario, bool has_ops);

    ~GameLobbyDialog();

    void set_parent_window(Gtk::Window &parent);

    void hide();
    bool run();
    
  sigc::signal<void, Player*> player_sat_down;
  sigc::signal<void, Player*> player_stood_up;
  sigc::signal<void, std::string> message_sent;

 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    //! The mini map that shows the scenario map
    std::auto_ptr<CityMap> citymap;
    Gtk::Image *map_image;

    void initDialog(GameScenario *gamescenario);
    void on_map_changed(SDL_Surface *map);
    GameScenario *d_game_scenario;
    Gtk::Label *turn_label;
    Gtk::Label *scenario_name_label;
    Gtk::Label *cities_label;
    Gtk::Button *play_button;
    Gtk::Button *cancel_button;
    Gtk::ScrolledWindow *chat_scrolledwindow;
    Gtk::TextView *chat_textview;
    Gtk::Entry *chat_entry;

    void update_scenario_details();
    void update_player_details();
    void on_show_options_clicked();
    void update_city_map();

    Gtk::TreeView *player_treeview;
    
    class PlayerColumns: public Gtk::TreeModelColumnRecord {
    public:
	PlayerColumns()
	    {add(shield); add(sitting); add(person); add(name); add(type); add(status); add(turn); add(player_id);}
	
	Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > shield;
	Gtk::TreeModelColumn<bool> sitting;
	Gtk::TreeModelColumn<Glib::ustring> person, name, type, status;
	Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > turn;
	Gtk::TreeModelColumn<Uint32> player_id;
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
    const PlayerStatusColumns player_status_columns;
    Glib::RefPtr<Gtk::ListStore> player_status_list;
    void cell_data_status(Gtk::CellRenderer *renderer, const Gtk::TreeIter& i);

    Gtk::CellRendererToggle sitting_renderer;
    Gtk::TreeViewColumn sitting_column;
    class PlayerSittingColumns: public Gtk::TreeModelColumnRecord {
    public:
	PlayerSittingColumns()
	    { add(sitting); }
	
	Gtk::TreeModelColumn<bool> sitting;
    };
    const PlayerSittingColumns player_sitting_columns;
    Glib::RefPtr<Gtk::ListStore> player_sitting_list;
    void cell_data_sitting(Gtk::CellRenderer *renderer, const Gtk::TreeIter& i);
    
    Gtk::TreeView *people_treeview;
    
    class PeopleColumns: public Gtk::TreeModelColumnRecord {
    public:
	PeopleColumns()
	    {add(nickname);}
	
	Gtk::TreeModelColumn<Glib::ustring> nickname;
    };
    const PeopleColumns people_columns;
    Glib::RefPtr<Gtk::ListStore> people_list;

    void add_player(const Glib::ustring &type, const Glib::ustring &name,
		    Player *player);
    void on_player_selected();
    bool d_has_ops;
    void update_buttons();
    void on_remote_player_ends_turn(Player *p);

    void on_remote_participant_joins(std::string nickname);
    void on_remote_participant_departs(std::string nickname);
    void on_player_stands(Player *p, std::string nickname);
    void on_player_sits(Player *p, std::string nickname);
    void on_remote_player_changes_name(Player *p);
    void on_remote_player_died(Player *p);
    void on_play_clicked();
    void on_cancel_clicked();
    void on_sitting_changed(Gtk::CellEditable *editable, const Glib::ustring &path);

    void on_chat_key_pressed(GdkEventKey *event);
    void on_chatted(std::string nickname, std::string message);


};

#endif
