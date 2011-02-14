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
#include "ucompose.hpp"
#include "game-parameters.h"

class NetworkAction;

struct Participant
{
  void *conn;
  std::list<GameParameters::Player> players;
  std::list<NetworkAction *> actions;
  std::list<NetworkHistory *> histories;
  std::string nickname;
  bool round_finished;
  bool departed;
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
  d_game_has_begun = false;
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

bool GameServer::onGotMessage(void *conn, MessageType type, std::string payload)
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
    sendChatRoster(conn);
    sendSeats(conn);
    if (gameHasBegun())
      network_server->send(conn, MESSAGE_TYPE_GAME_MAY_BEGIN, "");
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

  case MESSAGE_TYPE_LOBBY_ACTIVITY:
      {
        guint32 id;
        gint32 action;
        bool reported;
        Glib::ustring data;
        bool success = 
          get_message_lobby_activity (payload, id, action, reported, data);
        if (success)
          {
            if (reported == false) //player is /reporting/
              {
                switch (action)
                  {
                  case LOBBY_MESSAGE_TYPE_SIT:
                    sit(conn, Playerlist::getInstance()->getPlayer(id), data);
                    break;
                  case LOBBY_MESSAGE_TYPE_STAND:
                    stand(conn, Playerlist::getInstance()->getPlayer(id), data);
                    break;
                  case LOBBY_MESSAGE_TYPE_CHANGE_NAME:
                    change_name(conn, 
                                Playerlist::getInstance()->getPlayer(id), data);
                    break;
                  case LOBBY_MESSAGE_TYPE_CHANGE_TYPE:
                    change_type(conn, 
                                Playerlist::getInstance()->getPlayer(id), 
                                atoi(data.c_str()));
                    break;
                  default:
                    break;
                  }
              }
          }
      }
    break;

  case MESSAGE_TYPE_PARTICIPANT_DISCONNECTED:
    break;

  case MESSAGE_TYPE_SERVER_DISCONNECT:
  case MESSAGE_TYPE_CHATTED:
  case MESSAGE_TYPE_TURN_ORDER:
  case MESSAGE_TYPE_KILL_PLAYER:
  case MESSAGE_TYPE_ROUND_START:
  case MESSAGE_TYPE_CHANGE_NICKNAME:
  case MESSAGE_TYPE_GAME_MAY_BEGIN:
    //faulty client
    break;
  }
  return true;
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
      std::list<GameParameters::Player> players_to_stand = part->players;

      depart(conn);
      participants.remove(part);
      //tell everybode else that we've just stood up.
      for (std::list<GameParameters::Player>::iterator i = 
           players_to_stand.begin(); i != players_to_stand.end(); i++)
	notifyStand(Playerlist::getInstance()->getPlayer((*i).id), d_nickname);
      remote_participant_disconnected.emit();
      delete part;
    }
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
  Glib::ustring payload = 
    String::ucompose("%1 %2 %3 %4", player->getId(), LOBBY_MESSAGE_TYPE_SIT, 
                     1, nickname);
  player_sits.emit(player, nickname);

  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    {
      network_server->send((*i)->conn, MESSAGE_TYPE_LOBBY_ACTIVITY, payload);
      network_server->send((*i)->conn, MESSAGE_TYPE_CHATTED, 
			   nickname + " assumes control of " + 
			   player->getName() +".");
    }
  gotChatMessage("", nickname + " assumes control of " + 
		 player->getName() +".");
}

void GameServer::notifyTypeChange(Player *player, int type)
{
  if (!player)
    return;
  Glib::ustring payload = 
    String::ucompose("%1 %2 %3 %4", player->getId(), 
                     LOBBY_MESSAGE_TYPE_CHANGE_TYPE, 1, type);
  player_changes_type.emit(player, type);

  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    {
      network_server->send((*i)->conn, MESSAGE_TYPE_LOBBY_ACTIVITY, payload);
    }
}

void GameServer::notifyNameChange(Player *player, Glib::ustring name)
{
  if (!player)
    return;
  Glib::ustring payload = 
    String::ucompose("%1 %2 %3 %4", player->getId(), 
                     LOBBY_MESSAGE_TYPE_CHANGE_NAME, 1, name);
  player_changes_name.emit(player, name);

  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    {
      network_server->send((*i)->conn, MESSAGE_TYPE_LOBBY_ACTIVITY, payload);
    }
}

Participant *GameServer::findParticipantByPlayerId(guint32 id)
{
  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i)
    {
      for (std::list<GameParameters::Player>::iterator j = 
           (*i)->players.begin(); j != (*i)->players.end(); j++)
        {
          if ((*j).id == id)
            return *i;
        }
    }

  return 0;
}

Participant *GameServer::findParticipantByNick(Glib::ustring nickname)
{
  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i)
    if ((*i)->nickname == nickname)
      return *i;

  return 0;
}

Glib::ustring GameServer::make_nickname_unique(Glib::ustring nickname)
{
  Glib::ustring new_nickname;
  int count = 0;
  //okay, does this nickname appear twice?
  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i)
    {
      if ((*i)->nickname == nickname)
        count++;
    }
  if (nickname == d_nickname)
    count++;
  if (count <= 1)
    return nickname;
  count = 2;
  while (1)
    {
      new_nickname = String::ucompose("%1-%2", nickname, count);
      Participant *part = findParticipantByNick(new_nickname);
      if (!part)
        break;
      count++;
      if (count == 1000)
        break;
    }
  if (count == 1000)
    return "";
  return new_nickname;
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
    part->departed = false;
    part->round_finished = false;
    new_participant = true;
  }
  if (new_participant)
    sendMap(part);


  Glib::ustring new_nickname = make_nickname_unique(nickname);
  if (new_nickname != "")
    {
      if (new_nickname != nickname)
        {
          part->nickname = new_nickname;
          network_server->send(conn, MESSAGE_TYPE_CHANGE_NICKNAME, 
                               new_nickname);
        }
      notifyJoin(new_nickname);
    }
      
}

void GameServer::depart(void *conn)
{
  Participant *part = findParticipantByConn(conn);
  if (part && part->departed == false)
    {
      std::cout << "DEPART: " << conn << std::endl;
      notifyDepart(conn, part->nickname);
      part->departed = true;
    }
  //we don't delete the participant, it gets deleted when it disconnects.
  //see onConnectionLost
}

bool GameServer::player_already_sitting(Player *p)
{
  //check if the player p is already sitting down as a participant.
  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    {
      for (std::list<GameParameters::Player>::iterator j = 
           (*i)->players.begin(); j != (*i)->players.end(); j++)
	{
	  if (p->getId() == (*j).id)
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

  add_to_player_list(part->players, player->getId(), player->getName(),
                     GameParameters::player_type_to_player_param(player->getType()));

  if (player)
    dynamic_cast<NetworkPlayer*>(player)->setConnected(true);

  notifySit(player, nickname);
}

void GameServer::change_name(void *conn, Player *player, Glib::ustring name)
{
  std::cout << "CHANGE NAME: " << conn << " " << player << " " << name << std::endl;

  if (!player || !conn)
    return;
  Participant *part = findParticipantByConn(conn);
  if (!part) 
    return;

  update_player_name (part->players, player->getId(), name);
  name_change(player, name);
}

void GameServer::change_type(void *conn, Player *player, int type)
{
  std::cout << "CHANGE TYPE: " << conn << " " << player << " " << type << std::endl;

  if (!player || !conn)
    return;
  Participant *part = findParticipantByConn(conn);
  if (!part) 
    return;

  update_player_type (part->players, player->getId(), (guint32) type);
  notifyTypeChange(player, type);
}

void GameServer::notifyStand(Player *player, std::string nickname)
{
  if (!player)
    return;
  Glib::ustring payload = 
    String::ucompose("%1 %2 %3 %4", player->getId(), 
                     LOBBY_MESSAGE_TYPE_STAND, 1, nickname);
  player_stands.emit(player, nickname);

  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    {
      network_server->send((*i)->conn, MESSAGE_TYPE_LOBBY_ACTIVITY, payload);
      network_server->send((*i)->conn, MESSAGE_TYPE_CHATTED, 
			   nickname + " relinquishes control of " + 
			   player->getName() +".");
    }
  gotChatMessage("", nickname + " relinquishes control of " + 
		 player->getName() +".");
}

bool
GameServer::update_player_type (std::list<GameParameters::Player> &list, guint32 id, guint32 type)
{
  bool found = false;
  for (std::list<GameParameters::Player>::iterator i = list.begin(); i != list.end(); i++)
    {
      if ((*i).id == id)
	{
	  found = true;
          (*i).type = GameParameters::Player::Type(type);
	  break;
	}
    }
  return found;
}

bool
GameServer::update_player_name (std::list<GameParameters::Player> &list, guint32 id, Glib::ustring name)
{
  bool found = false;
  for (std::list<GameParameters::Player>::iterator i = list.begin(); i != list.end(); i++)
    {
      if ((*i).id == id)
	{
	  found = true;
          (*i).name = name;
	  break;
	}
    }
  return found;
}

bool
GameServer::add_to_player_list(std::list<GameParameters::Player> &list, guint32 id, Glib::ustring name, guint32 type)
{
  bool found = false;
  for (std::list<GameParameters::Player>::iterator i = list.begin(); i != list.end(); i++)
    {
      if ((*i).id == id)
	{
	  found = true;
          (*i).type = GameParameters::Player::Type(type);
          (*i).name = name;
	  break;
	}
    }
  if (found == false)
    {
      GameParameters::Player p;
      p.id = id;
      p.type = GameParameters::Player::Type(type);
      p.name = name;
      list.push_back(p);
    }
  return found;
}

bool
GameServer::remove_from_player_list(std::list<GameParameters::Player> &list, guint32 id)
{
  //remove player id from part.
  for (std::list<GameParameters::Player>::iterator i = list.begin(); 
       i != list.end(); i++)
    {
      if ((*i).id == id)
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
  tmpfile += SAVE_EXT;

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
      for (std::list<GameParameters::Player>::iterator it = 
           (*i)->players.begin(); it != (*i)->players.end(); it++)
	{
	  if ((*it).id == player->getId())
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
      player = new_p;
      add_to_player_list (players_seated_locally, new_p->getId(),
                          new_p->getName(), 
                          GameParameters::player_type_to_player_param(new_p->getType()));
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
      add_to_player_list (players_seated_locally, player->getId(),
                          player->getName(),
                          GameParameters::player_type_to_player_param(player->getType()));
      notifySit(player, d_nickname);
    }
  notifyTypeChange(player, player->getType());
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
      player = new_p;
      new_p->setConnected(false);
      notifyStand(new_p, d_nickname);
      remove_from_player_list (players_seated_locally, new_p->getId());
    }
  else if (player->getType() == Player::NETWORKED)
    {
      //this is the forcibly booted, made to stand-up case. .
      Participant *part = findParticipantByPlayerId(player->getId());
      stand(part->conn, player, part->nickname);
    }
  else // an ai player
    {
      stopListeningForLocalEvents(player);
      remove_from_player_list (players_seated_locally, player->getId());
      notifyStand(player, d_nickname);
    }
  notifyTypeChange(player, GameParameters::Player::NETWORKED);
}

void GameServer::name_change (Player *player, Glib::ustring name)
{
  if (!player)
    return;
  player->rename(name);
  update_player_name (players_seated_locally, player->getId(), name);
  notifyNameChange(player, name);
}

void GameServer::type_change (Player *player, int type)
{
  if (!player)
    return;
  update_player_type (players_seated_locally, player->getId(), (guint32) type);
  notifyTypeChange(player, type);
  player_changes_type.emit(player, type);
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

void GameServer::sendSeat(void *conn, GameParameters::Player player, Glib::ustring nickname)
{
  Glib::ustring payload = String::ucompose ("%1 %2 %3 %4", player.id, 
                                            LOBBY_MESSAGE_TYPE_SIT, 1, 
                                            nickname);
  network_server->send(conn, MESSAGE_TYPE_LOBBY_ACTIVITY, payload);

  payload = String::ucompose ("%1 %2 %3 %4", player.id, 
                              LOBBY_MESSAGE_TYPE_CHANGE_TYPE, 1, player.type);
  network_server->send(conn, MESSAGE_TYPE_LOBBY_ACTIVITY, payload);

  payload = String::ucompose ("%1 %2 %3 %4", player.id, 
                              LOBBY_MESSAGE_TYPE_CHANGE_NAME, 1, 
                              player.name);
  network_server->send(conn, MESSAGE_TYPE_LOBBY_ACTIVITY, payload);
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

      for (std::list<GameParameters::Player>::iterator j = 
           (*i)->players.begin(); j != (*i)->players.end(); j++)
        sendSeat(conn, (*j), (*i)->nickname);
    }
  //send out seatedness info for local server
  for (std::list<GameParameters::Player>::iterator j = 
       players_seated_locally.begin(); j != players_seated_locally.end(); j++)
    sendSeat(conn, (*j), d_nickname);
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

bool GameServer::gameHasBegun()
{
  return d_game_has_begun;
}

void GameServer::notifyClientsGameMayBeginNow()
{
  d_game_has_begun = true;
  //notify everyone that the game can finally start.
  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    network_server->send((*i)->conn, MESSAGE_TYPE_GAME_MAY_BEGIN, "");
}

// End of file
