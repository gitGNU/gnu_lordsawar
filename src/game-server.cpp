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
#include "action.h"


class Action;

struct Participant
{
  void *conn;
  Player *player;
  std::list<Action *> actions;
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
  network_server->got_message.connect(
    sigc::mem_fun(this, &GameServer::onGotMessage));
  network_server->connection_lost.connect(
    sigc::mem_fun(this, &GameServer::onConnectionLost));

  int port = 12345;
  network_server->startListening(port);

  listenForActions();
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

  case MESSAGE_TYPE_JOIN:
    join(conn);
    break;
    
  case MESSAGE_TYPE_SENDING_MAP:
    // should never occur
    break;
  }
}

void GameServer::onConnectionLost(void *conn)
{
  std::cerr << "connection lost" << std::endl;
  
  Participant *part = findParticipantByConn(conn);
  if (part)
    participants.remove(part);
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
    (*i)->action_done.connect(sigc::mem_fun(this, &GameServer::onActionDone));
}

void GameServer::onActionDone(Action *action)
{
  for (std::list<Participant *>::iterator i = participants.begin(),
         end = participants.end(); i != end; ++i) {
    (*i)->actions.push_back(action);
    sendActions(*i);
    (*i)->actions.clear();
  }

#if 0
  delete action;
#endif
}

void GameServer::join(void *conn)
{
  std::cout << "JOIN: " << conn << std::endl;
  
  Participant *part = findParticipantByConn(conn);
  if (!part) {
    part = new Participant;
    part->conn = conn;
    part->player = 0;
    participants.push_back(part);
  }

  sendMap(part);
}

void GameServer::gotActions(void *conn, const std::string &payload)
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
  
  for (std::list<Action *>::iterator i = part->actions.begin(),
         end = part->actions.end(); i != end; ++i)
    (**i).save(&helper);

  helper.closeTag();
    
  std::cerr << "sending actions" << std::endl;
  network_server->send(part->conn, MESSAGE_TYPE_SENDING_ACTIONS, os.str());
}


// End of file
