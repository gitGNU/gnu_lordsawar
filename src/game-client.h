// Copyright (C) 2008 Ole Laursen
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
#include "game-client-decoder.h"

class NetworkConnection;
class GameScenario;
class Player;

class GameClient: public GameClientDecoder
{
public:
        
  //! Returns the singleton instance.  Creates a new one if neccessary.
  static GameClient* getInstance();

  //! Deletes the singleton instance.
  static void deleteInstance();

  void start(std::string host, int port, std::string nick);
  void request_seat_manifest();

  sigc::signal<void> remote_participant_joins;
  sigc::signal<void, Player*> player_sits;
  sigc::signal<void, Player*> player_stands;
  sigc::signal<void> remote_participant_departs;
  sigc::signal<void> client_connected;
  sigc::signal<void> client_disconnected;
  sigc::signal<void> client_could_not_connect;
  
  void sit_down (Player *player);
  void stand_up (Player *player);
  void chat(std::string message);
protected:
  GameClient();
  ~GameClient();

private:
  std::auto_ptr<NetworkConnection> network_connection;
  int player_id;

  void onConnected();
  void onConnectionLost();
  void onGotMessage(MessageType type, std::string message);

  void listenForActions(Player *player);
  void onActionDone(NetworkAction *action);
  void sendActions();
  void clearNetworkActionlist(std::list<NetworkAction*> actions);

  void listenForHistories(Player *player);
  void onHistoryDone(NetworkHistory *history);
  void sendHistories();
  void clearNetworkHistorylist(std::list<NetworkHistory*> histories);

  void gotTurnOrder (std::string payload);

  void sat_down(Player *player);
  void stood_up(Player *player);

  std::list<NetworkAction*> actions;
  std::list<NetworkHistory*> histories;
  //! A static pointer for the singleton instance.
  static GameClient * s_instance;
  bool d_connected;
};

#endif
