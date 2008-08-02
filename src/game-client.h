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
  GameClient();
  ~GameClient();

  void setPlayerId(int id) {player_id = id;};

  void start(std::string host, int port);

  sigc::signal<void> client_connected;
  sigc::signal<void> client_disconnected;
  
private:
  std::auto_ptr<NetworkConnection> network_connection;
  int player_id;

  void onConnected();
  void onConnectionLost();
  void onGotMessage(MessageType type, std::string message);
};

#endif
