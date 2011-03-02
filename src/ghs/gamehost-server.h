// Copyright (C) 2011 Ben Asselstine
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

#ifndef GAMEHOST_SERVER_H
#define GAMEHOST_SERVER_H

#include "config.h"

#include <memory>
#include <list>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <glibmm.h>

#include "timing.h"
#include "network-ghs-common.h"

class NetworkServer;
class XML_Helper;
class Profile;
class GameScenario;
class HostedGame;
class HostGameRequest;

class GamehostServer
{
public:
        
  //! Returns the singleton instance.  Creates a new one if neccessary.
  static GamehostServer * getInstance();

  static const int TOO_MANY_PROFILES_AWAITING_MAPS = 100;
  static const int ONE_HOUR_OLD = 60 * 60;

  //! Deletes the singleton instance.
  static void deleteInstance();

  bool isListening();
  void start(int port);

  void reload();

  //get functions
  std::string getHostname() const {return hostname;};

  //set functions
  void setHostname(std::string host) {hostname = host;};
  void setMembers(std::list<std::string> profile_ids) {members = profile_ids;};

  // signals
  sigc::signal<void, int> port_in_use;

  // statics
  static std::list<std::string> load_members_from_file(std::string file);

protected:
  GamehostServer();
  ~GamehostServer();

private:
  std::auto_ptr<NetworkServer> network_server;
  std::string hostname;
  std::list<HostGameRequest*> host_game_requests;
  std::list<std::string> members;

  bool onGotMessage(void *conn, int type, std::string message);
  void onConnectionLost(void *conn);
  void onConnectionMade(void *conn);
  sigc::connection on_timer_registered(Timing::timer_slot s, int msecs_interval);
  void on_connected_to_gamelist_server_for_advertising_removal(std::string scenario_id);
  void on_advertising_removal_response_received(std::string scenario_id, std::string err);
  void on_connected_to_gamelist_server_for_advertising(HostedGame *game);
  void on_advertising_response_received(std::string scenario_id, std::string err);
  void on_child_setup();
  bool loadProfile(std::string tag, XML_Helper *helper, Profile **profile);

  // helpers
  void sendList(void *conn);
  void unhost(void *conn, std::string profile_id, std::string scenario_id, std::string &err);
  HostedGame* host(GameScenario *game_scenario, Profile *profile, std::string &err);
  void run_game(GameScenario *game_scenario, Glib::Pid *child_pid, guint32 port, std::string &err);
  void get_profile_and_scenario_id(std::string payload, Profile **profile, std::string &scenario_id, std::string &err);
  guint32 get_free_port();
  bool is_member(std::string profile_id);

  void cleanup_old_profiles_awaiting_maps(int stale = ONE_HOUR_OLD);
  bool add_to_profiles_awaiting_maps(Profile *profile, std::string scenario_id);
  Profile *remove_from_profiles_awaiting_maps(std::string scenario_id);

  //! A static pointer for the singleton instance.
  static GamehostServer * s_instance;

};

#endif
