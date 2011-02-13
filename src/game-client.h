// Copyright (C) 2008 Ole Laursen
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

#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include "config.h"

#include <list>
#include <memory>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
class XML_Helper;
class NetworkAction;
class NetworkHistory;

#include "network-common.h"
#include "game-station.h"

class NetworkConnection;
class GameScenario;
class Player;

class GameClient: public GameStation
{
public:
        
  //! Returns the singleton instance.  Creates a new one if neccessary.
  static GameClient* getInstance();

  //! Deletes the singleton instance.
  static void deleteInstance();

  void start(std::string host, guint32 port, std::string nick);
  void disconnect();
  void request_seat_manifest();

  sigc::signal<void> client_connected;
  sigc::signal<void> client_disconnected; 
  sigc::signal<void> client_forcibly_disconnected; //server went away
  sigc::signal<void> client_could_not_connect;
  
  void sit_down (Player *player);
  void stand_up (Player *player);
  void chat(std::string message);

  std::string getHost() const{return d_host;};
  guint32 getPort() const{return d_port;};

  void sendRoundOver();

protected:
  GameClient();
  ~GameClient();

private:
  std::auto_ptr<NetworkConnection> network_connection;
  int player_id;

  void sit_or_stand (Player *player, bool sit);
  void onConnected();
  void onConnectionLost();
  bool onGotMessage(MessageType type, std::string message);

  void onActionDone(NetworkAction *action);
  void sendActions();

  void onHistoryDone(NetworkHistory *history);
  void sendHistories();

  void gotTurnOrder (std::string payload);
  void gotKillPlayer(Player *player);

  void sat_down(Player *player, std::string nickname);
  void stood_up(Player *player, std::string nickname);

  std::list<NetworkAction*> actions;
  std::list<NetworkHistory*> histories;
  //! A static pointer for the singleton instance.
  static GameClient * s_instance;
  bool d_connected;
  std::string d_host;
  guint32 d_port;
};

#endif
