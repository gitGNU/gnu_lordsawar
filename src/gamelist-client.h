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

#ifndef GAMELIST_CLIENT_H
#define GAMELIST_CLIENT_H

#include "config.h"

#include <list>
#include <memory>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <glibmm.h>
class XML_Helper;

#include "network-gls-common.h"

class NetworkConnection;
class RecentlyPlayedGame;
class RecentlyPlayedGameList;
class Profile;

class GamelistClient
{
public:
        
  //! Returns the singleton instance.  Creates a new one if neccessary.
  static GamelistClient* getInstance();

  //! Deletes the singleton instance.
  static void deleteInstance();

  void start(std::string host, guint32 port, Profile *p);
  void disconnect();

  void request_game_list();
  sigc::signal<void, RecentlyPlayedGameList*, std::string> received_game_list;

  void request_advertising(RecentlyPlayedGame *game);
  sigc::signal<void, std::string, std::string> received_advertising_response;

  void request_advertising_removal(std::string scenario_id);
  sigc::signal<void, std::string, std::string> received_advertising_removal_response;

  void request_reload();
  sigc::signal<void, std::string> received_reload_response;

  sigc::signal<void> client_connected;
  sigc::signal<void> client_disconnected; 
  sigc::signal<void> client_forcibly_disconnected; //server went away
  sigc::signal<void> client_could_not_connect;
  
  std::string getHost() const{return d_host;};
  guint32 getPort() const{return d_port;};

protected:
  GamelistClient();
  ~GamelistClient();

private:
  std::auto_ptr<NetworkConnection> network_connection;

  void onConnected();
  void onConnectionLost();
  bool onGotMessage(int type, std::string message);
  bool loadRecentlyPlayedGameList(std::string tag, XML_Helper *helper);

  static GamelistClient * s_instance;
  std::string d_host;
  guint32 d_port;
  bool d_connected;
  std::string d_profile_id;
  RecentlyPlayedGameList *d_recently_played_game_list;
 
};

#endif
