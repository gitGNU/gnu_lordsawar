// Copyright (C) 2008 Ole Laursen
// Copyright (C) 2008 Ben Asselstine
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
#include <sigc++/signal.h>

#include "network-common.h"

class NetworkServer;
class Participant;
class NetworkAction;
class NetworkHistory;
class Player;
class XML_Helper;

class GameServer: public sigc::trackable
{
public:
        
  //! Returns the singleton instance.  Creates a new one if neccessary.
  static GameServer * getInstance();

  //! Deletes the singleton instance.
  static void deleteInstance();

  void start();

  sigc::signal<void, Player*> client_disconnected;
  sigc::signal<void, Player*> client_connected;

protected:
  GameServer();
  ~GameServer();

private:
  void listenForActions();
  void listenForHistories();
  void onActionDone(NetworkAction *action);
  void onHistoryDone(NetworkHistory *history);

  void join(void *conn, Player *player);
  void gotActions(void *conn, const std::string &payload);
  void gotHistory(void *conn, const std::string &payload);

  void sendMap(Participant *part);
  void sendActions(Participant *part);
  void sendHistories(Participant *part);

  std::auto_ptr<NetworkServer> network_server;

  std::list<Participant *> participants;

  Participant * play_by_mail_participant;

  Participant *findParticipantByConn(void *conn);
  
  void onGotMessage(void *conn, MessageType type, std::string message);
  void onConnectionLost(void *conn);
  void clearNetworkActionlist(std::list<NetworkAction*> actions);
  void clearNetworkHistorylist(std::list<NetworkHistory*> histories);
  bool dumpActionsAndHistories(XML_Helper *helper);
  bool dumpActionsAndHistories(XML_Helper *helper, Player *player);

  //! A static pointer for the singleton instance.
  static GameServer * s_instance;
};

#endif
