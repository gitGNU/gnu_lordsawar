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


class NetworkAction;

struct Participant
{
  void *conn;
  Player *player;
  std::list<NetworkAction *> actions;
  std::list<NetworkHistory *> histories;
};


GameServer::GameServer()
{
}

GameServer::~GameServer()
{
  for (std::list<Participant *>::iterator i = participants.begin(),
         end = participants.end(); i != end; ++i)
    delete *i;
}

void GameServer::start()
{
  network_server.reset(new NetworkServer());
  network_server->got_message.connect
    (sigc::mem_fun(this, &GameServer::onGotMessage));
  network_server->connection_lost.connect
    (sigc::mem_fun(this, &GameServer::onConnectionLost));

  int port = LORDSAWAR_PORT;
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
    gotActions(conn, payload);
    break;

  case MESSAGE_TYPE_SENDING_MAP:
    // should never occur
    break;

  case MESSAGE_TYPE_SENDING_HISTORY:
    gotHistory(conn, payload);
    break;

  case MESSAGE_TYPE_P1_JOIN:
    join(conn, Playerlist::getInstance()->getPlayer(0));
    break;

  case MESSAGE_TYPE_P2_JOIN:
    join(conn, Playerlist::getInstance()->getPlayer(1));
    break;

  case MESSAGE_TYPE_P3_JOIN:
    join(conn, Playerlist::getInstance()->getPlayer(2));
    break;

  case MESSAGE_TYPE_P4_JOIN:
    join(conn, Playerlist::getInstance()->getPlayer(3));
    break;

  case MESSAGE_TYPE_P5_JOIN:
    join(conn, Playerlist::getInstance()->getPlayer(4));
    break;

  case MESSAGE_TYPE_P6_JOIN:
    join(conn, Playerlist::getInstance()->getPlayer(5));
    break;

  case MESSAGE_TYPE_P7_JOIN:
    join(conn, Playerlist::getInstance()->getPlayer(6));
    break;

  case MESSAGE_TYPE_P8_JOIN:
    join(conn, Playerlist::getInstance()->getPlayer(7));
    break;

  }
}

void GameServer::onConnectionLost(void *conn)
{
  std::cerr << "connection lost" << std::endl;

  Participant *part = findParticipantByConn(conn);
  if (part)
    {
      client_disconnected.emit(part->player);
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

void GameServer::join(void *conn, Player *player)
{
  std::cout << "JOIN: " << conn << std::endl;

  Participant *part = findParticipantByConn(conn);
  if (!part) {
    part = new Participant;
    part->conn = conn;
    participants.push_back(part);
  }
  part->player = player;

  sendMap(part);
  client_connected.emit(player);
}

void GameServer::gotActions(void *conn, const std::string &payload)
{
}

void GameServer::gotHistory(void *conn, const std::string &payload)
{
}

void GameServer::sendMap(Participant *part)
{
  Playerlist *pl = Playerlist::getInstance();

  // first hack the players so the player type we serialize is right
  std::vector<Uint32> player_types;
  for (Playerlist::iterator i = pl->begin(); i != pl->end(); ++i) {
    player_types.push_back((*i)->getType());
    (*i)->setType(Player::NETWORKED);
  }


  // send the map
  std::ostringstream os;
  XML_Helper helper(&os);
  Game::getScenario()->saveWithHelper(helper);
  std::cerr << "sending map" << std::endl;
  network_server->send(part->conn, MESSAGE_TYPE_SENDING_MAP, os.str());


  // unhack the players
  std::vector<Uint32>::iterator j = player_types.begin();
  for (Playerlist::iterator i = pl->begin(); i != pl->end(); ++i, ++j) {
    (*i)->setType(Player::Type(*j));
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
      if (player == (*i)->player)
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

// End of file
