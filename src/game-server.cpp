// Copyright (C) 2008 Ole Laursen
// Copyright (C) 2008 Ben Asselstine
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

#include <iostream>
#include <sstream>
#include <list>

#include "game-server.h"

#include "network-server.h"
#include "game.h"
#include "xmlhelper.h"
#include "GameScenario.h"
#include "playerlist.h"
#include "player.h"
#include "network-action.h"
#include "network-history.h"
#include "Configuration.h"
#include "network_player.h"
#include "real_player.h"
#include "GameScenarioOptions.h"


class NetworkAction;

struct Participant
{
  void *conn;
  std::list<guint32> players;
  std::list<NetworkAction *> actions;
  std::list<NetworkHistory *> histories;
  std::string nickname;
  bool round_finished;
};

GameServer * GameServer::s_instance = 0;


GameServer* GameServer::getInstance()
{
    if (s_instance == 0)
        s_instance = new GameServer();

    return s_instance;
}

void GameServer::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}


GameServer::GameServer()
{
}

GameServer::~GameServer()
{
  //say goodbye to all participants
  for (std::list<Participant *>::iterator i = participants.begin(),
         end = participants.end(); i != end; ++i)
    network_server->send((*i)->conn, MESSAGE_TYPE_SERVER_DISCONNECT, "bye");

  for (std::list<Participant *>::iterator i = participants.begin(),
         end = participants.end(); i != end; ++i)
    delete *i;
  players_seated_locally.clear();
}

bool GameServer::isListening()
{
  if (network_server.get() != NULL)
    return network_server->isListening();
  else
    return false;
}

void GameServer::start(GameScenario *game_scenario, int port, std::string nick)
{
  setGameScenario(game_scenario);
  setNickname(nick);

  if (network_server.get() != NULL && network_server->isListening())
    return;
  network_server.reset(new NetworkServer());
  network_server->got_message.connect
    (sigc::mem_fun(this, &GameServer::onGotMessage));
  network_server->connection_lost.connect
    (sigc::mem_fun(this, &GameServer::onConnectionLost));
  network_server->connection_made.connect
    (sigc::mem_fun(this, &GameServer::onConnectionMade));

  network_server->startListening(port);

  listenForLocalEvents(Playerlist::getInstance()->getNeutral());
}

void GameServer::checkRoundOver()
{
  bool all_finished = true;
  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i)
    {
      if ((*i)->round_finished == false)
	{
	  all_finished = false;
	  break;
	}
    }

  if (all_finished)
    {
      printf ("hooray!  we're all done the round.\n");
      for (std::list<Participant *>::iterator i = participants.begin(),
	   end = participants.end(); i != end; ++i)
	(*i)->round_finished = false;

      //now we can send the start round message, and begin the round ourselves.
      for (std::list<Participant *>::iterator i = participants.begin(),
	   end = participants.end(); i != end; ++i)
	network_server->send((*i)->conn, MESSAGE_TYPE_ROUND_START, "");

      round_begins.emit();
    }
}

void GameServer::gotRoundOver(void *conn)
{
  Participant *part = findParticipantByConn(conn);
  if (part)
    {
      //mark the participant as finished for this round
      part->round_finished = true;
      //are all participants finished?
      checkRoundOver();
    }
  return;
}

void GameServer::gotChat(void *conn, std::string message)
{
  Participant *part = findParticipantByConn(conn);
  if (part)
    {
      gotChatMessage(part->nickname, message);
      for (std::list<Participant *>::iterator i = participants.begin(),
	   end = participants.end(); i != end; ++i)
	{
	  //if ((*i)->conn != part->conn)
	  //network_server->send((*i)->conn, MESSAGE_TYPE_CHATTED, 
	  //part->nickname + ":" + message);
	  //else
	  network_server->send((*i)->conn, MESSAGE_TYPE_CHATTED, 
			       message);

	}
    }
  return;
}
void GameServer::onGotMessage(void *conn, MessageType type, std::string payload)
{
  std::cerr << "got message of type " << type << std::endl;
  switch (type) {
  case MESSAGE_TYPE_PING:
    std::cerr << "sending pong" << std::endl;
    network_server->send(conn, MESSAGE_TYPE_PONG, "");
    break;

  case MESSAGE_TYPE_PONG:
    break;

  case MESSAGE_TYPE_SENDING_ACTIONS:
    gotRemoteActions(conn, payload);
    break;

  case MESSAGE_TYPE_SENDING_MAP:
    // should never occur
    break;

  case MESSAGE_TYPE_SENDING_HISTORY:
    gotRemoteHistory(conn, payload);
    break;

  case MESSAGE_TYPE_PARTICIPANT_CONNECT:
    join(conn, payload);
    break;

  case MESSAGE_TYPE_REQUEST_SEAT_MANIFEST:
    sendSeats(conn);
    sendChatRoster(conn);
    break;

  case MESSAGE_TYPE_P1_SIT:
    sit(conn, Playerlist::getInstance()->getPlayer(0), payload);
    break;

  case MESSAGE_TYPE_P2_SIT:
    sit(conn, Playerlist::getInstance()->getPlayer(1), payload);
    break;

  case MESSAGE_TYPE_P3_SIT:
    sit(conn, Playerlist::getInstance()->getPlayer(2), payload);
    break;

  case MESSAGE_TYPE_P4_SIT:
    sit(conn, Playerlist::getInstance()->getPlayer(3), payload);
    break;

  case MESSAGE_TYPE_P5_SIT:
    sit(conn, Playerlist::getInstance()->getPlayer(4), payload);
    break;

  case MESSAGE_TYPE_P6_SIT:
    sit(conn, Playerlist::getInstance()->getPlayer(5), payload);
    break;

  case MESSAGE_TYPE_P7_SIT:
    sit(conn, Playerlist::getInstance()->getPlayer(6), payload);
    break;

  case MESSAGE_TYPE_P8_SIT:
    sit(conn, Playerlist::getInstance()->getPlayer(7), payload);
    break;

  case MESSAGE_TYPE_P1_STAND:
    stand(conn, Playerlist::getInstance()->getPlayer(0), payload);
    break;

  case MESSAGE_TYPE_P2_STAND:
    stand(conn, Playerlist::getInstance()->getPlayer(1), payload);
    break;

  case MESSAGE_TYPE_P3_STAND:
    stand(conn, Playerlist::getInstance()->getPlayer(2), payload);
    break;

  case MESSAGE_TYPE_P4_STAND:
    stand(conn, Playerlist::getInstance()->getPlayer(3), payload);
    break;

  case MESSAGE_TYPE_P5_STAND:
    stand(conn, Playerlist::getInstance()->getPlayer(4), payload);
    break;

  case MESSAGE_TYPE_P6_STAND:
    stand(conn, Playerlist::getInstance()->getPlayer(5), payload);
    break;

  case MESSAGE_TYPE_P7_STAND:
    stand(conn, Playerlist::getInstance()->getPlayer(6), payload);
    break;

  case MESSAGE_TYPE_P8_STAND:
    stand(conn, Playerlist::getInstance()->getPlayer(7), payload);
    break;

  case MESSAGE_TYPE_PARTICIPANT_DISCONNECT:
    depart(conn);
    break;

  case MESSAGE_TYPE_PARTICIPANT_CONNECTED:
    break;

  case MESSAGE_TYPE_CHAT:
    gotChat(conn, payload);
    break;

  case MESSAGE_TYPE_ROUND_OVER:
    //what do we do now?
    gotRoundOver(conn);
    break;

  case MESSAGE_TYPE_PARTICIPANT_DISCONNECTED:
    break;

  case MESSAGE_TYPE_P1_SAT_DOWN:
  case MESSAGE_TYPE_P2_SAT_DOWN:
  case MESSAGE_TYPE_P3_SAT_DOWN:
  case MESSAGE_TYPE_P4_SAT_DOWN:
  case MESSAGE_TYPE_P5_SAT_DOWN:
  case MESSAGE_TYPE_P6_SAT_DOWN:
  case MESSAGE_TYPE_P7_SAT_DOWN:
  case MESSAGE_TYPE_P8_SAT_DOWN:
  case MESSAGE_TYPE_P1_STOOD_UP:
  case MESSAGE_TYPE_P2_STOOD_UP:
  case MESSAGE_TYPE_P3_STOOD_UP:
  case MESSAGE_TYPE_P4_STOOD_UP:
  case MESSAGE_TYPE_P5_STOOD_UP:
  case MESSAGE_TYPE_P6_STOOD_UP:
  case MESSAGE_TYPE_P7_STOOD_UP:
  case MESSAGE_TYPE_P8_STOOD_UP:
  case MESSAGE_TYPE_SERVER_DISCONNECT:
  case MESSAGE_TYPE_CHATTED:
  case MESSAGE_TYPE_TURN_ORDER:
  case MESSAGE_TYPE_KILL_PLAYER:
  case MESSAGE_TYPE_ROUND_START:
    //faulty client
    break;
  }
}

void GameServer::onConnectionMade(void *conn)
{
  remote_participant_connected.emit();
}

void GameServer::onConnectionLost(void *conn)
{
  std::cerr << "connection lost" << std::endl;

  Participant *part = findParticipantByConn(conn);
  if (part)
    {
      std::list<guint32> players_to_stand = part->players;

      depart(conn);
      participants.remove(part);
      for (std::list<guint32>::iterator it = players_to_stand.begin();
	   it != players_to_stand.end(); it++)
	notifyStand(Playerlist::getInstance()->getPlayer(*it), d_nickname);
      remote_participant_disconnected.emit();
    }
  delete part;
}

Participant *GameServer::findParticipantByConn(void *conn)
{
  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i)
    if ((*i)->conn == conn)
      return *i;

  return 0;
}

void GameServer::onActionDone(NetworkAction *action)
{
  std::string desc = action->toString();
  std::cerr << "Game Server got " << desc <<"\n";

  if (action->getAction()->getType() == Action::END_TURN)
    local_player_moved(action->getOwner());
  if (action->getAction()->getType() == Action::INIT_TURN)
    local_player_starts_move(action->getOwner());

  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    {
      (*i)->actions.push_back(action);
      sendActions(*i);
      clearNetworkActionlist((*i)->actions);
    }

}

void GameServer::onHistoryDone(NetworkHistory *history)
{
  std::string desc = history->toString();
  std::cerr << "Game Server got " << desc <<"\n";

  if (history->getHistory()->getType() == History::PLAYER_VANQUISHED)
    local_player_died(history->getOwner());

  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    {
      (*i)->histories.push_back(history);
      sendHistories(*i);
      clearNetworkHistorylist((*i)->histories);
    }
}

void GameServer::notifyJoin(std::string nickname)
{
  remote_participant_joins.emit(nickname);
  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    {
      network_server->send((*i)->conn, MESSAGE_TYPE_PARTICIPANT_CONNECTED, 
			   nickname);
      //network_server->send((*i)->conn, MESSAGE_TYPE_CHATTED, 
      //nickname + " connected.");
    }
  gotChatMessage("[server]", nickname + " connected.");
}

void GameServer::notifyDepart(void *conn, std::string nickname)
{
  remote_participant_departs.emit(nickname);
  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    {
      if ((*i)->conn == conn)
	continue;
      network_server->send((*i)->conn, MESSAGE_TYPE_PARTICIPANT_DISCONNECTED, 
			   nickname);
      network_server->send((*i)->conn, MESSAGE_TYPE_CHATTED, 
			   nickname + " disconnected.");
    }
  gotChatMessage("", nickname + " disconnected.");
}

void GameServer::notifySit(Player *player, std::string nickname)
{
  if (!player)
    return;
  player_sits.emit(player, nickname);
  MessageType type;
  switch (player->getId())
    {
    case 0: type = MESSAGE_TYPE_P1_SAT_DOWN; break;
    case 1: type = MESSAGE_TYPE_P2_SAT_DOWN; break;
    case 2: type = MESSAGE_TYPE_P3_SAT_DOWN; break;
    case 3: type = MESSAGE_TYPE_P4_SAT_DOWN; break;
    case 4: type = MESSAGE_TYPE_P5_SAT_DOWN; break;
    case 5: type = MESSAGE_TYPE_P6_SAT_DOWN; break;
    case 6: type = MESSAGE_TYPE_P7_SAT_DOWN; break;
    case 7: type = MESSAGE_TYPE_P8_SAT_DOWN; break;
    default:
	    return;
    }

  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    {
      network_server->send((*i)->conn, type, nickname);
      network_server->send((*i)->conn, MESSAGE_TYPE_CHATTED, 
			   nickname + " assumes control of " + 
			   player->getName() +".");
    }
  gotChatMessage("", nickname + " assumes control of " + 
		 player->getName() +".");
}

void GameServer::join(void *conn, std::string nickname)
{
  bool new_participant = false;
  std::cout << "JOIN: " << conn << std::endl;

  Participant *part = findParticipantByConn(conn);
  if (!part) {
    part = new Participant;
    part->conn = conn;
    part->nickname = nickname;
    participants.push_back(part);
    new_participant = true;
  }
  if (new_participant)
    {
      sendMap(part);
    }

  notifyJoin(nickname);
}

void GameServer::depart(void *conn)
{
  std::cout << "DEPART: " << conn << std::endl;

  Participant *part = findParticipantByConn(conn);

  notifyDepart(conn, part->nickname);
  //we don't delete the participant, it gets deleted when it disconnects.
  //see onConnectionLost
}

bool GameServer::player_already_sitting(Player *p)
{
  //check if the player p is already sitting down as a participant.
  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    {
      for (std::list<guint32>::iterator j = (*i)->players.begin(); 
	   j != (*i)->players.end(); j++)
	{
	  if (p->getId() == *j)
	    return true;
	}
    }
  return false;
}

void GameServer::sit(void *conn, Player *player, std::string nickname)
{
  std::cout << "SIT: " << conn << " " << player << std::endl;

  if (!player || !conn)
    return;
  Participant *part = findParticipantByConn(conn);
  if (!part) 
    return;

  //fixme: is another player already sitting here?
  if (player_already_sitting(player) == true)
    return;

  add_to_player_list(part->players, player->getId());

  if (player)
    dynamic_cast<NetworkPlayer*>(player)->setConnected(true);

  notifySit(player, nickname);
}

void GameServer::notifyStand(Player *player, std::string nickname)
{
  if (!player)
    return;
  player_stands.emit(player, nickname);
  MessageType type;
  switch (player->getId())
    {
    case 0: type = MESSAGE_TYPE_P1_STOOD_UP; break;
    case 1: type = MESSAGE_TYPE_P2_STOOD_UP; break;
    case 2: type = MESSAGE_TYPE_P3_STOOD_UP; break;
    case 3: type = MESSAGE_TYPE_P4_STOOD_UP; break;
    case 4: type = MESSAGE_TYPE_P5_STOOD_UP; break;
    case 5: type = MESSAGE_TYPE_P6_STOOD_UP; break;
    case 6: type = MESSAGE_TYPE_P7_STOOD_UP; break;
    case 7: type = MESSAGE_TYPE_P8_STOOD_UP; break;
    default:
	    return;
    }

  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    {
      network_server->send((*i)->conn, type, nickname);
      network_server->send((*i)->conn, MESSAGE_TYPE_CHATTED, 
			   nickname + " relinquishes control of " + 
			   player->getName() +".");
    }
  gotChatMessage("", nickname + " relinquishes control of " + 
		 player->getName() +".");
}

bool
GameServer::add_to_player_list(std::list<guint32> &list, guint32 id)
{
  bool found = false;
  for (std::list<guint32>::iterator i = list.begin(); i != list.end(); i++)
    {
      if (*i == id)
	{
	  found = true;
	  break;
	}
    }
  if (found == false)
    list.push_back(id);
  return found;
}

bool
GameServer::remove_from_player_list(std::list<guint32> &list, guint32 id)
{
  //remove player id from part.
  for (std::list<guint32>::iterator i = list.begin(); 
       i != list.end(); i++)
    {
      if (*i == id)
	{
	  list.erase (i);
	  return true;
	}
    }
  return false;
}

void GameServer::stand(void *conn, Player *player, std::string nickname)
{
  std::cout << "STAND: " << conn << " " << player << std::endl;
  if (!player || !conn)
    return;

  Participant *part = findParticipantByConn(conn);
  if (!part) 
    return;

  //remove player id from part.
  bool found = remove_from_player_list (part->players, player->getId());

  if (!found)
    //okay somebody's trying to boot another player.
    return;

  if (player && player->getType() == Player::NETWORKED)
    dynamic_cast<NetworkPlayer*>(player)->setConnected(false);
  notifyStand(player, nickname);
}

void GameServer::gotRemoteActions(void *conn, const std::string &payload)
{
  gotActions(payload);
  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    if ((*i)->conn != conn)
      network_server->send((*i)->conn, MESSAGE_TYPE_SENDING_ACTIONS, payload);
}

void GameServer::gotRemoteHistory(void *conn, const std::string &payload)
{
  gotHistories(payload);
  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    if ((*i)->conn != conn)
      network_server->send((*i)->conn, MESSAGE_TYPE_SENDING_HISTORY, payload);
}

void GameServer::sendMap(Participant *part)
{
  Playerlist *pl = Playerlist::getInstance();

  // first hack the players so the player type we serialize is right
  std::vector<Player*> players;
  for (Playerlist::iterator i = pl->begin(); i != pl->end(); ++i) 
    {
      bool connected = false;
      players.push_back(*i);
      if ((*i)->isComputer() == true)
	connected = true;
      NetworkPlayer *new_p = new NetworkPlayer(**i);
      new_p->setConnected(connected);
      pl->swap(*i, new_p);
    }


  // send the map, and save it to a file somewhere temporarily
  std::string tmpfile = "lw.XXXX";
  int fd = Glib::file_open_tmp(tmpfile, "lw.XXXX");
  close(fd);
  File::erase(tmpfile);
  tmpfile += ".sav";

  d_game_scenario->saveGame(tmpfile, "sav");

  std::cerr << "sending map" << std::endl;
  network_server->send(part->conn, MESSAGE_TYPE_SENDING_MAP, tmpfile);
  File::erase (tmpfile);

  // unhack the players
  std::vector<Player*>::iterator j = players.begin();
  for (Playerlist::iterator i = pl->begin(); i != pl->end(); ++i, ++j) 
    {
      pl->swap(*i, *j);
      //delete *i;
    }
}

void GameServer::sendActions(Participant *part)
{
  std::ostringstream os;
  XML_Helper helper(&os);

  helper.begin("1");
  helper.openTag("actions");

  for (std::list<NetworkAction *>::iterator i = part->actions.begin(),
       end = part->actions.end(); i != end; ++i)
    (**i).save(&helper);

  helper.closeTag();

  std::cerr << "sending actions" << std::endl;
  network_server->send(part->conn, MESSAGE_TYPE_SENDING_ACTIONS, os.str());
}

void GameServer::sendHistories(Participant *part)
{
  std::ostringstream os;
  XML_Helper helper(&os);

  helper.begin("1");
  helper.openTag("histories");

  for (std::list<NetworkHistory *>::iterator i = part->histories.begin(),
       end = part->histories.end(); i != end; ++i)
    (**i).save(&helper);

  helper.closeTag();

  std::cerr << "sending histories" << std::endl;
  network_server->send(part->conn, MESSAGE_TYPE_SENDING_HISTORY, os.str());
}

bool GameServer::dumpActionsAndHistories(XML_Helper *helper, Player *player)
{
  Participant *part = NULL;
  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i)
    {
      bool found = false;
      for (std::list<guint32>::iterator it = (*i)->players.begin();
	   it != (*i)->players.end(); it++)
	{
	  if (*it == player->getId())
	    {
	      found = true;
	      break;
	    }
	}

      if (found)
	{
	  part = *i;
	  break;
	}
    }
  if (part == NULL)
    return false;
  for (std::list<NetworkHistory *>::iterator i = part->histories.begin(),
       end = part->histories.end(); i != end; ++i)
    (**i).save(helper);
  for (std::list<NetworkAction *>::iterator i = part->actions.begin(),
       end = part->actions.end(); i != end; ++i)
    (**i).save(helper);
  return true;
}

bool GameServer::dumpActionsAndHistories(XML_Helper *helper)
{
  Player *player = Playerlist::getActiveplayer();
  return dumpActionsAndHistories(helper, player);
}

void GameServer::sit_down (Player *player)
{
  if (!player)
    return;
  if (player->getType() == Player::NETWORKED)
    {
      //alright, we want to sit down as this player
      //convert the network player to a human player
      dynamic_cast<NetworkPlayer*>(player)->setConnected(true);
      RealPlayer *new_p = new RealPlayer (*player);
      Playerlist::getInstance()->swap(player, new_p);
      stopListeningForLocalEvents(player);
      listenForLocalEvents(new_p);
      delete player;
      add_to_player_list (players_seated_locally, new_p->getId());
      notifySit(new_p, d_nickname);
    }
  else if (player->getType() == Player::HUMAN)
    {
      // do nothing
    }
  else // an ai player
    {
      stopListeningForLocalEvents(player);
      listenForLocalEvents(player);
      add_to_player_list (players_seated_locally, player->getId());
      notifySit(player, d_nickname);
    }
}

void GameServer::stand_up (Player *player)
{
  if (!player)
    return;
  if (player->getType() == Player::HUMAN)
    {
      //alright, we want to stand up as this player
      //convert the player from a human player back to a network player

      NetworkPlayer *new_p = new NetworkPlayer(*player);
      Playerlist::getInstance()->swap(player, new_p);
      stopListeningForLocalEvents(player);
      delete player;
      new_p->setConnected(false);
      notifyStand(new_p, d_nickname);
      remove_from_player_list (players_seated_locally, new_p->getId());
    }
  else if (player->getType() == Player::NETWORKED)
    {
      // do nothing
    }
  else // an ai player
    {
      stopListeningForLocalEvents(player);
      remove_from_player_list (players_seated_locally, player->getId());
      notifyStand(player, d_nickname);
    }
}

void GameServer::chat (std::string message)
{
  notifyChat(d_nickname + ":" + message);
}

void GameServer::notifyChat(std::string message)
{
  gotChatMessage(d_nickname, message);
  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    network_server->send((*i)->conn, MESSAGE_TYPE_CHATTED, message);
}

void GameServer::sendSeats(void *conn)
{
  Participant *part = findParticipantByConn(conn);
  if (!part)
    return;
  //send seatedness info for remote participants

  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    {
      if ((*i)->conn == part->conn)
	continue;

      for (std::list<guint32>::iterator j = (*i)->players.begin(); 
	   j != (*i)->players.end(); j++)
	{
	  Player *player = Playerlist::getInstance()->getPlayer(*j);
	  MessageType type;
	  switch (player->getId())
	    {
	    case 0: type = MESSAGE_TYPE_P1_SAT_DOWN; break;
	    case 1: type = MESSAGE_TYPE_P2_SAT_DOWN; break;
	    case 2: type = MESSAGE_TYPE_P3_SAT_DOWN; break;
	    case 3: type = MESSAGE_TYPE_P4_SAT_DOWN; break;
	    case 4: type = MESSAGE_TYPE_P5_SAT_DOWN; break;
	    case 5: type = MESSAGE_TYPE_P6_SAT_DOWN; break;
	    case 6: type = MESSAGE_TYPE_P7_SAT_DOWN; break;
	    case 7: type = MESSAGE_TYPE_P8_SAT_DOWN; break;
	    default:
		    return;
	    }
	  network_server->send(part->conn, type, (*i)->nickname);
	}
    }
  //send out seatedness info for local server
  for (std::list<guint32>::iterator j = players_seated_locally.begin(); 
       j != players_seated_locally.end(); j++)
    {
      Player *player = Playerlist::getInstance()->getPlayer(*j);
      MessageType type;
      switch (player->getId())
	{
	case 0: type = MESSAGE_TYPE_P1_SAT_DOWN; break;
	case 1: type = MESSAGE_TYPE_P2_SAT_DOWN; break;
	case 2: type = MESSAGE_TYPE_P3_SAT_DOWN; break;
	case 3: type = MESSAGE_TYPE_P4_SAT_DOWN; break;
	case 4: type = MESSAGE_TYPE_P5_SAT_DOWN; break;
	case 5: type = MESSAGE_TYPE_P6_SAT_DOWN; break;
	case 6: type = MESSAGE_TYPE_P7_SAT_DOWN; break;
	case 7: type = MESSAGE_TYPE_P8_SAT_DOWN; break;
	default:
		return;
	}
      network_server->send(part->conn, type, d_nickname);
    }
}

void GameServer::sendChatRoster(void *conn)
{
  Participant *part = findParticipantByConn(conn);
  if (!part)
    return;
  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    {
      if ((*i)->conn == part->conn)
	continue;
      network_server->send(part->conn, MESSAGE_TYPE_PARTICIPANT_CONNECTED, 
			   (*i)->nickname);
    }
  network_server->send(part->conn, MESSAGE_TYPE_PARTICIPANT_CONNECTED, 
		       d_nickname);
}

void GameServer::sendKillPlayer(Player *p)
{
  std::stringstream player;
  player << p->getId();
  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    network_server->send((*i)->conn, MESSAGE_TYPE_KILL_PLAYER, player.str());

  remote_player_died.emit(p);
}

void GameServer::sendTurnOrder()
{
  std::stringstream players;
  Playerlist *pl = Playerlist::getInstance();
  for (Playerlist::iterator it = pl->begin(); it != pl->end(); it++)
    players << (*it)->getId() << " ";
  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    network_server->send((*i)->conn, MESSAGE_TYPE_TURN_ORDER, players.str());
  playerlist_reorder_received.emit();
}

// End of file
