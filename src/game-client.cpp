// Copyright (C) 2008 Ole Laursen
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
#include <fstream>

#include "game-client.h"

#include "network-connection.h"
#include "File.h"
#include "action.h"
#include "network-action.h"
#include "network-history.h"
#include "playerlist.h"
#include "network_player.h"
#include "xmlhelper.h"
  
#include "real_player.h" 
#include "network_player.h" 


GameClient * GameClient::s_instance = 0;


GameClient* GameClient::getInstance()
{
    if (s_instance == 0)
        s_instance = new GameClient();

    return s_instance;
}

void GameClient::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

GameClient::GameClient()
{
  d_connected = false;
  player_id = -1;
}

GameClient::~GameClient()
{
  if (network_connection.get() != NULL)
    network_connection->send(MESSAGE_TYPE_PARTICIPANT_DISCONNECT, d_nickname);
}

void GameClient::start(std::string host, guint32 port, std::string nick)
{
  d_host = host;
  d_port = port;
  player_id = -1;
  setNickname(nick);
  network_connection.reset(new NetworkConnection());
  network_connection->connected.connect(
    sigc::mem_fun(this, &GameClient::onConnected));
  network_connection->connection_lost.connect(
    sigc::mem_fun(this, &GameClient::onConnectionLost));
  network_connection->got_message.connect(
    sigc::mem_fun(this, &GameClient::onGotMessage));
  network_connection->connectToHost(host, port);
}

void GameClient::onConnected() 
{
  std::cerr << "connected" << std::endl;
  d_connected = true;

  printf("okay, sending ping message from here.\n");
  network_connection->send(MESSAGE_TYPE_PING, "");

  client_connected.emit();
}

void GameClient::onConnectionLost()
{
  std::cerr << "connection lost" << std::endl;
  if (d_connected)
    client_disconnected.emit();
  else
    client_could_not_connect.emit();
}

void GameClient::sat_down(Player *player, std::string nickname)
{
  if (!player)
    return;
  if (player->getType() == Player::NETWORKED)
    dynamic_cast<NetworkPlayer*>(player)->setConnected(true);
  player_sits.emit(player, nickname);
}

void GameClient::stood_up(Player *player, std::string nickname)
{
  if (!player)
    return;
  if (player->getType() == Player::HUMAN)
    {
      //this covers the "boot", or forcibly-stand case.
      NetworkPlayer *new_p = new NetworkPlayer(*player);
      Playerlist::getInstance()->swap(player, new_p);
      delete player;
      new_p->setConnected(false);
    }
  else if (player->getType() == Player::NETWORKED)
    dynamic_cast<NetworkPlayer*>(player)->setConnected(false);
  player_stands.emit(player, nickname);
}

void GameClient::onGotMessage(MessageType type, std::string payload)
{
  printf("hey the client got a message!\n");
  std::cerr << "got message of type " << type << std::endl;
  switch (type) {
  case MESSAGE_TYPE_PING:
    network_connection->send(MESSAGE_TYPE_PONG, "PONGOGOGO");
    break;

  case MESSAGE_TYPE_PONG:
    network_connection->send(MESSAGE_TYPE_PARTICIPANT_CONNECT, d_nickname);
    break;

  case MESSAGE_TYPE_SENDING_ACTIONS:
    gotActions(payload);
    break;

  case MESSAGE_TYPE_SENDING_MAP:
    gotScenario(payload);
    break;

  case MESSAGE_TYPE_PARTICIPANT_CONNECTED:
    remote_participant_joins.emit(payload);
    break;
  case MESSAGE_TYPE_PARTICIPANT_DISCONNECTED:
    remote_participant_departs.emit(payload);
    break;

  case MESSAGE_TYPE_REQUEST_SEAT_MANIFEST:
  case MESSAGE_TYPE_P1_SIT:
  case MESSAGE_TYPE_P2_SIT:
  case MESSAGE_TYPE_P3_SIT:
  case MESSAGE_TYPE_P4_SIT:
  case MESSAGE_TYPE_P5_SIT:
  case MESSAGE_TYPE_P6_SIT:
  case MESSAGE_TYPE_P7_SIT:
  case MESSAGE_TYPE_P8_SIT:
  case MESSAGE_TYPE_P1_STAND:
  case MESSAGE_TYPE_P2_STAND:
  case MESSAGE_TYPE_P3_STAND:
  case MESSAGE_TYPE_P4_STAND:
  case MESSAGE_TYPE_P5_STAND:
  case MESSAGE_TYPE_P6_STAND:
  case MESSAGE_TYPE_P7_STAND:
  case MESSAGE_TYPE_P8_STAND:
  case MESSAGE_TYPE_PARTICIPANT_CONNECT:
  case MESSAGE_TYPE_PARTICIPANT_DISCONNECT:
  case MESSAGE_TYPE_CHAT:
  case MESSAGE_TYPE_ROUND_OVER:
    //FIXME: faulty server.
    break;

  case MESSAGE_TYPE_CHATTED:
    gotChatMessage("", payload);
    break;
    //this is the client realizing that some other player joined the server
  case MESSAGE_TYPE_P1_SAT_DOWN:
    sat_down(Playerlist::getInstance()->getPlayer(0), payload);
    break;
  case MESSAGE_TYPE_P2_SAT_DOWN:
    sat_down(Playerlist::getInstance()->getPlayer(1), payload);
    break;
  case MESSAGE_TYPE_P3_SAT_DOWN:
    sat_down(Playerlist::getInstance()->getPlayer(2), payload);
    break;
  case MESSAGE_TYPE_P4_SAT_DOWN:
    sat_down(Playerlist::getInstance()->getPlayer(3), payload);
    break;
  case MESSAGE_TYPE_P5_SAT_DOWN:
    sat_down(Playerlist::getInstance()->getPlayer(4), payload);
    break;
  case MESSAGE_TYPE_P6_SAT_DOWN:
    sat_down(Playerlist::getInstance()->getPlayer(5), payload);
    break;
  case MESSAGE_TYPE_P7_SAT_DOWN:
    sat_down(Playerlist::getInstance()->getPlayer(6), payload);
    break;
  case MESSAGE_TYPE_P8_SAT_DOWN:
    sat_down(Playerlist::getInstance()->getPlayer(7), payload);
    break;
  case MESSAGE_TYPE_P1_STOOD_UP:
    stood_up(Playerlist::getInstance()->getPlayer(0), payload);
    break;
  case MESSAGE_TYPE_P2_STOOD_UP:
    stood_up(Playerlist::getInstance()->getPlayer(1), payload);
    break;
  case MESSAGE_TYPE_P3_STOOD_UP:
    stood_up(Playerlist::getInstance()->getPlayer(2), payload);
    break;
  case MESSAGE_TYPE_P4_STOOD_UP:
    stood_up(Playerlist::getInstance()->getPlayer(3), payload);
    break;
  case MESSAGE_TYPE_P5_STOOD_UP:
    stood_up(Playerlist::getInstance()->getPlayer(4), payload);
    break;
  case MESSAGE_TYPE_P6_STOOD_UP:
    stood_up(Playerlist::getInstance()->getPlayer(5), payload);
    break;
  case MESSAGE_TYPE_P7_STOOD_UP:
    stood_up(Playerlist::getInstance()->getPlayer(6), payload);
    break;
  case MESSAGE_TYPE_P8_STOOD_UP:
    stood_up(Playerlist::getInstance()->getPlayer(7), payload);
    break;

  case MESSAGE_TYPE_SENDING_HISTORY:
    gotHistories(payload);
    break;
  
  case MESSAGE_TYPE_SERVER_DISCONNECT:
    client_disconnected.emit();
    break;

  case MESSAGE_TYPE_TURN_ORDER:
    gotTurnOrder (payload);
    break;

  case MESSAGE_TYPE_KILL_PLAYER:
    gotKillPlayer(Playerlist::getInstance()->getPlayer(atoi(payload.c_str())));
    break;

  case MESSAGE_TYPE_ROUND_START:
    round_begins.emit();
    break;
  }
}

void GameClient::gotKillPlayer(Player *player)
{
  player->kill();
}  

void GameClient::onHistoryDone(NetworkHistory *history)
{
  std::string desc = history->toString();
  std::cerr << "Game Client got " << desc <<"\n";

  if (history->getHistory()->getType() == History::PLAYER_VANQUISHED)
    local_player_died(history->getOwner());

  histories.push_back(history);
  sendHistories();
  clearNetworkHistorylist(histories);
}

void GameClient::onActionDone(NetworkAction *action)
{
  std::string desc = action->toString();
  std::cerr << "Game Client got " << desc <<"\n";

  if (action->getAction()->getType() == Action::END_TURN)
    local_player_moved(action->getOwner());
  if (action->getAction()->getType() == Action::INIT_TURN)
    local_player_starts_move(action->getOwner());

  actions.push_back(action);
  sendActions();
  clearNetworkActionlist(actions);

}
void GameClient::sendActions()
{
  std::ostringstream os;
  XML_Helper helper(&os);

  helper.begin("1");
  helper.openTag("actions");

  for (std::list<NetworkAction *>::iterator i = actions.begin(),
       end = actions.end(); i != end; ++i)
    {
      (*i)->save(&helper);
    }

  helper.closeTag();

  std::cerr << "sending actions" << std::endl;
  network_connection->send(MESSAGE_TYPE_SENDING_ACTIONS, os.str());
}

void GameClient::sendHistories()
{
  std::ostringstream os;
  XML_Helper helper(&os);

  helper.begin("1");
  helper.openTag("histories");

  for (std::list<NetworkHistory *>::iterator i = histories.begin(),
       end = histories.end(); i != end; ++i)
    (**i).save(&helper);

  helper.closeTag();

  network_connection->send(MESSAGE_TYPE_SENDING_HISTORY, os.str());
}

void GameClient::sit_down (Player *player)
{
  if (!player)
    return;
  MessageType type;
  switch (player->getId())
    {
    case 0: type = MESSAGE_TYPE_P1_SIT; break;
    case 1: type = MESSAGE_TYPE_P2_SIT; break;
    case 2: type = MESSAGE_TYPE_P3_SIT; break;
    case 3: type = MESSAGE_TYPE_P4_SIT; break;
    case 4: type = MESSAGE_TYPE_P5_SIT; break;
    case 5: type = MESSAGE_TYPE_P6_SIT; break;
    case 6: type = MESSAGE_TYPE_P7_SIT; break;
    case 7: type = MESSAGE_TYPE_P8_SIT; break;
    default:
	    return;
    }
  RealPlayer *new_p = new RealPlayer (*player);
  Playerlist::getInstance()->swap(player, new_p);
  stopListeningForLocalEvents(player);
  listenForLocalEvents(new_p);
  delete player;
  network_connection->send(type, d_nickname);
}

void GameClient::stand_up (Player *player)
{
  if (!player)
    return;
  MessageType type;
  switch (player->getId())
    {
    case 0: type = MESSAGE_TYPE_P1_STAND; break;
    case 1: type = MESSAGE_TYPE_P2_STAND; break;
    case 2: type = MESSAGE_TYPE_P3_STAND; break;
    case 3: type = MESSAGE_TYPE_P4_STAND; break;
    case 4: type = MESSAGE_TYPE_P5_STAND; break;
    case 5: type = MESSAGE_TYPE_P6_STAND; break;
    case 6: type = MESSAGE_TYPE_P7_STAND; break;
    case 7: type = MESSAGE_TYPE_P8_STAND; break;
    default:
	    return;
    }
  //now turning a human player into a network player
  NetworkPlayer *new_p = new NetworkPlayer(*player);
  Playerlist::getInstance()->swap(player, new_p);
  stopListeningForLocalEvents(new_p);
  delete player;
  new_p->setConnected(false);
  network_connection->send(type, d_nickname);
}

void GameClient::chat(std::string message)
{
  network_connection->send(MESSAGE_TYPE_CHAT, d_nickname + ":" + message);
}
  
void GameClient::request_seat_manifest()
{
  network_connection->send(MESSAGE_TYPE_REQUEST_SEAT_MANIFEST, "");
}
    
void GameClient::gotTurnOrder (std::string payload)
{
  std::list<guint32> player_ids;
  std::stringstream players;
  players.str(payload);

  int ival;
  while (players.eof() == false)
    {
      ival = -1;
      players >> ival;
      if (ival != -1)
	player_ids.push_back(ival);
    }
  Playerlist::getInstance()->reorder(player_ids);
  playerlist_reorder_received.emit();
}

void GameClient::sendRoundOver()
{
  network_connection->send(MESSAGE_TYPE_ROUND_OVER, "");
}
