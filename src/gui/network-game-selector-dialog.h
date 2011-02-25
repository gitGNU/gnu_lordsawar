//  Copyright (C) 2008, 2009 Ben Asselstine
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

#ifndef NETWORK_GAME_SELECTOR_DIALOG_H
#define NETWORK_GAME_SELECTOR_DIALOG_H

#include <memory>
#include <string>
#include <sigc++/trackable.h>
#include <gtkmm.h>

class Profile;
class RecentlyPlayedNetworkedGame;
#include "decorated.h"
// dialog for joining remote games
class NetworkGameSelectorDialog: public Decorated
{
 public:
    NetworkGameSelectorDialog(Profile *p);
    ~NetworkGameSelectorDialog();

    sigc::signal<void, std::string /*ip*/, unsigned short /*port*/> game_selected;

    void set_parent_window(Gtk::Window &parent);

    void hide();
    bool run();
    
 private:
    Gtk::Dialog* dialog;
    Gtk::Entry *hostname_entry;
    Gtk::SpinButton *port_spinbutton;
    Gtk::Button *connect_button;
    Gtk::Button *clear_button;

    void on_hostname_changed();

    Gtk::TreeView *recent_treeview;

    class RecentlyJoinedGamesColumns: public Gtk::TreeModelColumnRecord {
    public:
	RecentlyJoinedGamesColumns() 
        { add(name); add(turn);
	  add(number_of_players); add(number_of_cities); add(host); add(port);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<unsigned int> turn;
	Gtk::TreeModelColumn<unsigned int> number_of_players;
	Gtk::TreeModelColumn<unsigned int> number_of_cities;
	Gtk::TreeModelColumn<Glib::ustring> host;
	Gtk::TreeModelColumn<unsigned int> port;
    };
    const RecentlyJoinedGamesColumns recently_joined_games_columns;
    Glib::RefPtr<Gtk::ListStore> recently_joined_games_list;
    void addRecentlyJoinedGame(RecentlyPlayedNetworkedGame*);

    void on_recent_game_selected();
    void on_clear_clicked();
};

#endif
