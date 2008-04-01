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

#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include "config.h"

#include <memory>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>

#include "network-common.h"

class NetworkConnection;
class GameScenario;

class GameClient: public sigc::trackable
{
public:
  GameClient();
  ~GameClient();

  void start(std::string host, int port);

  sigc::signal<void, std::string> game_scenario_received;
  
private:
  std::auto_ptr<NetworkConnection> network_connection;

  void gotScenario(const std::string &payload);
  void gotActions(const std::string &payload);

  void onConnected();
  void onConnectionLost();
  void onGotMessage(MessageType type, std::string message);
};

#endif
