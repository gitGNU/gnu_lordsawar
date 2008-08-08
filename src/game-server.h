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
#include "game-client-decoder.h"

class NetworkServer;
class Participant;
class NetworkAction;
class NetworkHistory;
class Player;
class XML_Helper;
class GameScenario;

class GameServer: public GameClientDecoder 
{
public:
        
  //! Returns the singleton instance.  Creates a new one if neccessary.
  static GameServer * getInstance();

  //! Deletes the singleton instance.
  static void deleteInstance();

  bool isListening();
  void start(GameScenario *game_scenario, int port, std::string nick);

  void sit_down (Player *player);
  void stand_up (Player *player);
  void chat(std::string message);
  void sendTurnOrder();
  sigc::signal<void> remote_participant_connected;
  sigc::signal<void, std::string> remote_participant_joins;
  sigc::signal<void, Player*, std::string> player_sits;
  sigc::signal<void, Player*, std::string> player_stands;
  sigc::signal<void, std::string> remote_participant_departs;
  sigc::signal<void> remote_participant_disconnected;
  sigc::signal<void> playerlist_reorder_received;

  void setGameScenario(GameScenario *scenario) {d_game_scenario = scenario;};

protected:
  GameServer();
  ~GameServer();

private:
  GameScenario *d_game_scenario;
  void listenForActions();
  void listenForHistories();
  void onActionDone(NetworkAction *action);
  void onHistoryDone(NetworkHistory *history);

  void join(void *conn, std::string payload);
  void notifyJoin (std::string nickname);
  void depart(void *conn);
  void notifyDepart (void *conn, std::string nickname);
  void sit(void *conn, Player *player, std::string nickname);
  void notifySit(Player *player, std::string nickname);
  void stand(void *conn, Player *player, std::string nickname);
  void notifyStand(Player *player, std::string nickname);
  void gotRemoteActions(void *conn, const std::string &payload);
  void gotRemoteHistory(void *conn, const std::string &payload);
  void notifyChat(std::string message);

  void sendMap(Participant *part);
  void sendSeats(void *conn);
  void sendChatRoster(void *conn);

  void sendActions(Participant *part);
  void sendHistories(Participant *part);

  std::auto_ptr<NetworkServer> network_server;

  std::list<Participant *> participants;
  std::list<Uint32> players_seated_locally;

  Participant * play_by_mail_participant;

  Participant *findParticipantByConn(void *conn);
  
  void onGotMessage(void *conn, MessageType type, std::string message);
  void onConnectionLost(void *conn);
  void onConnectionMade(void *conn);
  void clearNetworkActionlist(std::list<NetworkAction*> &actions);
  void clearNetworkHistorylist(std::list<NetworkHistory*> &histories);
  bool dumpActionsAndHistories(XML_Helper *helper);
  bool dumpActionsAndHistories(XML_Helper *helper, Player *player);

  void gotChat(void *conn, std::string message);



  bool add_to_player_list(std::list<Uint32> &list, Uint32 id);
  void remove_from_player_list(std::list<Uint32> &list, Uint32 id);
  //! A static pointer for the singleton instance.
  static GameServer * s_instance;
};

#endif
