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

#ifndef GUI_DRIVER_H
#define GUI_DRIVER_H

#include <memory>
#include <string>
#include <sigc++/trackable.h>

#include "splash-window.h"
#include "game-window.h"
#include "game-lobby-dialog.h"
#include "new-network-game-download-window.h"
#include "game-parameters.h"

class Profile;
// takes care of setting up the splash window and the game window, the
// interaction between them and the model classes
// it also takes care of the game lobby window.
class Driver: public sigc::trackable
{
 public:
    Driver(std::string load_filename);
    ~Driver();

    void run();

 private:
    GameWindow* game_window;
    GameLobbyDialog* game_lobby_dialog;
    SplashWindow* splash_window;
    NewNetworkGameDownloadWindow* download_window;
    std::string d_load_filename;
    sigc::connection heartbeat_conn;
    Player::Type robot_player_type;
    std::string d_advertised_scenario_id;
    unsigned int number_of_robots;
    std::string game_scenario_downloaded;
    sigc::signal<void, std::string> game_scenario_received;
    sigc::signal<void, Player*> player_replaced;

    void on_new_game_requested(GameParameters g);
    void on_new_remote_network_game_requested(std::string host, unsigned short port, Profile *p);
    void on_new_hosted_network_game_requested(GameParameters g, int port, Profile *p, bool advertised, bool remotely_hosted);
    void on_new_pbm_game_requested(GameParameters g);
    void on_game_scenario_downloaded(std::string filename);
    void on_game_scenario_received(std::string path, Profile *p);
    void on_load_requested(std::string filename);
    void on_quit_requested();

    void on_game_ended();
    void on_game_ended_and_start_new();

    void init_game_window();


    void on_hosted_player_sat_down(Player *player);
    void on_hosted_player_stood_up(Player *player);
    void on_hosted_player_changed_name(Player *player, Glib::ustring name);
    void on_hosted_player_changed_type(Player *player, int type);
    void on_client_player_sat_down(Player *player);
    void on_client_player_stood_up(Player *player);
    void on_client_player_changed_name(Player *player, Glib::ustring name);
    void on_client_player_changed_type(Player *player, int type);
    void on_server_went_away();
    void on_client_could_not_connect();

    GameScenario *new_game(GameParameters g);
    GameScenario *load_game(std::string file_path);
    void stress_test();
    void stressTestNextRound();

    void lordsawaromatic(std::string host, unsigned short port, Player::Type type, int num_players);
    void on_game_scenario_received_for_robots(std::string path);
  

    void heartbeat();

    void on_client_player_chat(std::string message);
    void on_hosted_player_chat(std::string message);
    void on_hosted_player_says_game_may_begin();

    void on_show_lobby_requested();

    void start_network_game_requested(GameScenario *game_scenario,
				      NextTurnNetworked *next_turn);

    void on_keep_network_play_going(); //starts a new round

    void on_game_may_begin_for_robots(GameScenario *game_scenario, NextTurnNetworked *next_turn);

    GameScenario *create_new_scenario(GameParameters &g, GameScenario::PlayMode m);

    void get_default(int num_players, GameParameters &g);
    void serve (GameScenario *game_scenario);
    void remotely_serve (GameScenario *game_scenario, Profile *p);
    void on_client_sits_down_in_headless_server_game(Player *p, std::string nick);
  
    void on_could_not_bind_to_port_for_headless_server(int port);
    void on_could_not_bind_to_port (int port);

    void advertise_game(GameScenario *game_scenario, Profile *p);
    void on_connected_to_gamelist_server_for_advertising(GameScenario *game_scenario, Profile *p);

    void on_advertising_response_received(std::string scenario_id, std::string err);
    void unadvertise_game(std::string scenario_id, Profile *p);
    void on_connected_to_gamelist_server_for_advertising_removal(std::string scenario_id);
    void on_advertising_removal_response_received(std::string scenario_id, std::string err);

    void on_advertised_game_round_ends(GameScenario *game_scenario, Profile *p);

    void on_connected_to_gamehost_server_for_hosting_request (GameScenario *game_scenario);
    void on_got_game_host_response(std::string scenario_id, std::string err, GameScenario *game_scenario);
    void on_remote_game_hosted(std::string scenario_id, guint32 port, std::string err);
};


#endif
