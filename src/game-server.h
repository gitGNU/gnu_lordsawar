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

#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include "config.h"

#include <memory>
#include <list>
#include <sigc++/trackable.h>

#include "network-common.h"

class NetworkServer;
class Participant;
class Action;

class GameServer: public sigc::trackable
{
public:
  GameServer();
  ~GameServer();

  void start();
  
private:
  void listenForActions();
  void onActionDone(Action *action);

  void join(void *conn);
  void gotActions(void *conn, const std::string &payload);

  void sendMap(Participant *part);
  void sendActions(Participant *part);

  std::auto_ptr<NetworkServer> network_server;

  std::list<Participant *> participants;

  Participant *findParticipantByConn(void *conn);
  
  void onGotMessage(void *conn, MessageType type, std::string message);
  void onConnectionLost(void *conn);
};

#endif
