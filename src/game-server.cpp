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


class NetworkAction;

struct Participant
{
  void *conn;
  std::list<Uint32> players;
  std::list<NetworkAction *> actions;
  std::list<NetworkHistory *> histories;
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
  d_nickname = nick;

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

  listenForActions();
  listenForHistories();
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
    join(conn);
    break;

  case MESSAGE_TYPE_P1_SIT:
    sit(conn, Playerlist::getInstance()->getPlayer(0));
    break;

  case MESSAGE_TYPE_P2_SIT:
    sit(conn, Playerlist::getInstance()->getPlayer(1));
    break;

  case MESSAGE_TYPE_P3_SIT:
    sit(conn, Playerlist::getInstance()->getPlayer(2));
    break;

  case MESSAGE_TYPE_P4_SIT:
    sit(conn, Playerlist::getInstance()->getPlayer(3));
    break;

  case MESSAGE_TYPE_P5_SIT:
    sit(conn, Playerlist::getInstance()->getPlayer(4));
    break;

  case MESSAGE_TYPE_P6_SIT:
    sit(conn, Playerlist::getInstance()->getPlayer(5));
    break;

  case MESSAGE_TYPE_P7_SIT:
    sit(conn, Playerlist::getInstance()->getPlayer(6));
    break;

  case MESSAGE_TYPE_P8_SIT:
    sit(conn, Playerlist::getInstance()->getPlayer(7));
    break;

  case MESSAGE_TYPE_P1_STAND:
    stand(conn, Playerlist::getInstance()->getPlayer(0));
    break;

  case MESSAGE_TYPE_P2_STAND:
    stand(conn, Playerlist::getInstance()->getPlayer(1));
    break;

  case MESSAGE_TYPE_P3_STAND:
    stand(conn, Playerlist::getInstance()->getPlayer(2));
    break;

  case MESSAGE_TYPE_P4_STAND:
    stand(conn, Playerlist::getInstance()->getPlayer(3));
    break;

  case MESSAGE_TYPE_P5_STAND:
    stand(conn, Playerlist::getInstance()->getPlayer(4));
    break;

  case MESSAGE_TYPE_P6_STAND:
    stand(conn, Playerlist::getInstance()->getPlayer(5));
    break;

  case MESSAGE_TYPE_P7_STAND:
    stand(conn, Playerlist::getInstance()->getPlayer(6));
    break;

  case MESSAGE_TYPE_P8_STAND:
    stand(conn, Playerlist::getInstance()->getPlayer(7));
    break;

  case MESSAGE_TYPE_PARTICIPANT_DISCONNECT:
    depart(conn);
    break;

  case MESSAGE_TYPE_PARTICIPANT_CONNECTED:
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
      MessageType type;
      for (std::list<Uint32>::iterator it = part->players.begin();
	   it != part->players.end(); it++)
	{
	  switch (*it)
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
	      if ((*i)->conn == conn)
		continue;
	      network_server->send((*i)->conn, type, "player stood up");
	    }
	}

      for (std::list<Participant *>::iterator i = participants.begin(),
	   end = participants.end(); i != end; ++i) 
	{
	  if ((*i)->conn == conn)
	    continue;
	  network_server->send((*i)->conn, MESSAGE_TYPE_PARTICIPANT_DISCONNECTED, "client departed");
	}

      remote_participant_disconnected.emit();
      participants.remove(part);
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

void GameServer::listenForActions()
{
  Playerlist *pl = Playerlist::getInstance();
  for (Playerlist::iterator i = pl->begin(); i != pl->end(); ++i)
    (*i)->acting.connect(sigc::mem_fun(this, &GameServer::onActionDone));
}

void GameServer::listenForHistories()
{
  Playerlist *pl = Playerlist::getInstance();
  for (Playerlist::iterator i = pl->begin(); i != pl->end(); ++i)
    (*i)->history_written.connect(sigc::mem_fun(this, &GameServer::onHistoryDone));
}

void GameServer::clearNetworkActionlist(std::list<NetworkAction*> actions)
{
  for (std::list<NetworkAction*>::iterator it = actions.begin();
       it != actions.end(); it++)
    {
      delete (*it);
    }
  actions.clear();
}

void GameServer::clearNetworkHistorylist(std::list<NetworkHistory*> histories)
{
  for (std::list<NetworkHistory*>::iterator it = histories.begin();
       it != histories.end(); it++)
    {
      delete (*it);
    }
  histories.clear();
}

void GameServer::onActionDone(NetworkAction *action)
{
  std::string desc = action->toString();
  std::cerr << "Game Server got " << desc <<"\n";

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

  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    {
      (*i)->histories.push_back(history);
      sendHistories(*i);
      clearNetworkHistorylist((*i)->histories);
    }
}

void GameServer::notifyJoin()
{
  remote_participant_joins.emit();
  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    {
      network_server->send((*i)->conn, MESSAGE_TYPE_PARTICIPANT_CONNECTED, "client joined");
    }
}

void GameServer::notifyDepart(void *conn)
{
  remote_participant_departs.emit();
  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    {
      if ((*i)->conn == conn)
	continue;
      network_server->send((*i)->conn, MESSAGE_TYPE_PARTICIPANT_DISCONNECTED, "client departed");
    }
}

void GameServer::notifySit(Player *player)
{
  if (!player)
    return;
  player_sits.emit(player);
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
      network_server->send((*i)->conn, type, "player joined");
    }
}

void GameServer::join(void *conn)
{
  bool new_participant = false;
  std::cout << "JOIN: " << conn << std::endl;

  Participant *part = findParticipantByConn(conn);
  if (!part) {
    part = new Participant;
    part->conn = conn;
    participants.push_back(part);
    new_participant = true;
  }
  if (new_participant)
    sendMap(part);

  notifyJoin();
}

void GameServer::depart(void *conn)
{
  std::cout << "DEPART: " << conn << std::endl;

  Participant *part = findParticipantByConn(conn);

  notifyDepart(conn);
  //we don't delete the participant, it gets deleted when it disconnects.
  //see onConnectionLost
}

void GameServer::sit(void *conn, Player *player)
{
  std::cout << "SIT: " << conn << " " << player << std::endl;

  if (!player || !conn)
    return;
  Participant *part = findParticipantByConn(conn);
  if (!part) 
    return;

  bool found = false;
  for (std::list<Uint32>::iterator i = part->players.begin(); 
       i != part->players.end(); i++)
    {
      if (*i == player->getId())
	{
	  found = true;
	  break;
	}
    }
  if (found == false)
    part->players.push_back(player->getId());

  if (player)
    dynamic_cast<NetworkPlayer*>(player)->setConnected(true);

  notifySit(player);
}

void GameServer::notifyStand(Player *player)
{
  if (!player)
    return;
  player_stands.emit(player);
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
    network_server->send((*i)->conn, type, "player departed");
}

void GameServer::stand(void *conn, Player *player)
{
  std::cout << "STAND: " << conn << " " << player << std::endl;
  if (!player || !conn)
    return;

  Participant *part = findParticipantByConn(conn);
  if (!part) 
    return;
  //remove player id from part.
  for (std::list<Uint32>::iterator i = part->players.begin(); 
       i != part->players.end(); i++)
    {
      if (*i == player->getId())
	{
	  part->players.erase (i);
	  break;
	}
    }

  if (player && player->getType() == Player::NETWORKED)
    dynamic_cast<NetworkPlayer*>(player)->setConnected(false);
  notifyStand(player);
}

void GameServer::gotRemoteActions(void *conn, const std::string &payload)
{
  gotActions(payload);
  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
    network_server->send((*i)->conn, MESSAGE_TYPE_SENDING_ACTIONS, payload);
}

void GameServer::gotRemoteHistory(void *conn, const std::string &payload)
{
  gotHistories(payload);
  for (std::list<Participant *>::iterator i = participants.begin(),
       end = participants.end(); i != end; ++i) 
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
      if ((*i)->getType() == Player::AI_FAST ||
	  (*i)->getType() == Player::AI_SMART ||
	  (*i)->getType() == Player::AI_DUMMY)
	connected = true;
      NetworkPlayer *new_p = new NetworkPlayer(**i);
      new_p->setConnected(connected);
      pl->swap(*i, new_p);
    }


  // send the map
  std::ostringstream os;
  XML_Helper helper(&os);
  d_game_scenario->saveWithHelper(helper);
  std::cerr << "sending map" << std::endl;
  network_server->send(part->conn, MESSAGE_TYPE_SENDING_MAP, os.str());

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
      for (std::list<Uint32>::iterator it = (*i)->players.begin();
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
  //alright, we want to sit down as this player
  //convert the network player to a human player
  dynamic_cast<NetworkPlayer*>(player)->setConnected(true);
  RealPlayer *new_p = new RealPlayer (*player);
  new_p->acting.connect(sigc::mem_fun(this, &GameServer::onActionDone));
  new_p->history_written.connect(sigc::mem_fun(this, &GameServer::onHistoryDone));
  Playerlist::getInstance()->swap(player, new_p);
  delete player;
  notifySit(new_p);
}

void GameServer::stand_up (Player *player)
{
  //alright, we want to stand up as this player
  //convert the player from a human player back to a network player

  NetworkPlayer *new_p = new NetworkPlayer(*player);
  Playerlist::getInstance()->swap(player, new_p);
  delete player;
  new_p->setConnected(false);
  new_p->acting.connect(sigc::mem_fun(this, &GameServer::onActionDone));
  new_p->history_written.connect(sigc::mem_fun(this, &GameServer::onHistoryDone));
  notifyStand(new_p);
}
// End of file
