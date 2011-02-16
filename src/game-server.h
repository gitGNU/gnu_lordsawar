// Copyright (C) 2008 Ole Laursen
// Copyright (C) 2008, 2011 Ben Asselstine
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

#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include "config.h"

#include <memory>
#include <list>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>

#include "network-common.h"
#include "game-station.h"

class NetworkServer;
class Participant;
class NetworkAction;
class NetworkHistory;
class Player;
class XML_Helper;
class GameScenario;

class GameServer: public GameStation
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
  void name_change (Player *player, Glib::ustring name);
  void type_change (Player *player, int type);
  void chat(std::string message);
  void sendTurnOrder();
  void sendKillPlayer(Player *player);
  void sendOffPlayer(Player *player);
  void notifyClientsGameMayBeginNow();
  void notifyRoundOver();
  sigc::signal<void> remote_participant_connected;
  sigc::signal<void> remote_participant_disconnected;

  void setGameScenario(GameScenario *scenario) {d_game_scenario = scenario;};

  bool gameHasBegun();

protected:
  GameServer();
  ~GameServer();

private:
  GameScenario *d_game_scenario;
  bool d_game_has_begun;
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
  void change_name(void *conn, Player *player, Glib::ustring name);
  void notifyNameChange(Player *player, Glib::ustring name);
  void change_type(void *conn, Player *player, int type);
  void notifyTypeChange(Player *player, int type);
  void gotRemoteActions(void *conn, const std::string &payload);
  void gotRemoteHistory(void *conn, const std::string &payload);
  void notifyChat(std::string message);

  void sendMap(Participant *part);
  void sendSeats(void *conn);
  void sendSeat(void *conn, GameParameters::Player player, Glib::ustring nickname);
  void sendChatRoster(void *conn);

  void sendActions(Participant *part);
  void sendHistories(Participant *part);

  std::auto_ptr<NetworkServer> network_server;

  std::list<Participant *> participants;
  std::list<GameParameters::Player> players_seated_locally;
  std::map<guint32, bool> id_end_turn; //whether local players ended their turn

  Participant * play_by_mail_participant;

  Participant *findParticipantByConn(void *conn);
  Participant *findParticipantByNick(Glib::ustring nickname);
  Participant *findParticipantByPlayerId(guint32 id);
  
  bool onGotMessage(void *conn, MessageType type, std::string message);
  void onConnectionLost(void *conn);
  void onConnectionMade(void *conn);
  bool dumpActionsAndHistories(XML_Helper *helper);
  bool dumpActionsAndHistories(XML_Helper *helper, Player *player);

  void gotChat(void *conn, std::string message);

  bool player_already_sitting(Player *p);

  bool add_to_player_list(std::list<GameParameters::Player> &list, guint32 id,
                          Glib::ustring name, guint32 type);
  bool remove_from_player_list(std::list<GameParameters::Player> &list, guint32 id);
  bool update_player_type (std::list<GameParameters::Player> &list, guint32 id, guint32 type);
  bool update_player_name (std::list<GameParameters::Player> &list, guint32 id, Glib::ustring name);

  void syncLocalPlayers();

  void sendRoundStart();
  void player_finished_turn(Player *player);
  Glib::ustring make_nickname_unique(Glib::ustring nickname);

  bool check_for_all_players_having_ended_their_turn();
  void clear_end_turn_flag_for_all_players();
  void onLocalNonNetworkedActionDone(NetworkAction *action);
  void onLocalNonNetworkedHistoryDone(NetworkHistory *history);
  void onLocalNetworkedHistoryDone(NetworkHistory *history);

  //! A static pointer for the singleton instance.
  static GameServer * s_instance;
};

#endif
