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
  player_id = -1;
}

GameClient::~GameClient()
{
}

void GameClient::startAsPlayer(std::string host, int port, int id) 
{
  network_connection.reset(new NetworkConnection());
  network_connection->connected.connect(
    sigc::mem_fun(this, &GameClient::onConnected));
  network_connection->connection_lost.connect(
    sigc::mem_fun(this, &GameClient::onConnectionLost));
  network_connection->got_message.connect(
    sigc::mem_fun(this, &GameClient::onGotMessage));

  network_connection->connectToHost(host, port);
};

void GameClient::start(std::string host, int port)
{
  player_id = -1;
  startAsPlayer(host, port, player_id);
}

void GameClient::onConnected() 
{
  std::cerr << "connected" << std::endl;

  network_connection->send(MESSAGE_TYPE_PING, "");

  client_connected.emit();
}

void GameClient::onConnectionLost()
{
  std::cerr << "connection lost" << std::endl;
  client_disconnected.emit();
}

void GameClient::joined(Player *player)
{
  if (player)
    {
      //now turning a network player into a human player
      dynamic_cast<NetworkPlayer*>(player)->setConnected(true);
      RealPlayer *new_p = new RealPlayer (*player);
      Playerlist::getInstance()->swap(player, new_p);
      delete player;
      listenForActions(new_p);
      listenForHistories(new_p);
      remote_player_connected.emit(new_p);
    }
  else
    remote_player_connected.emit(player);
}

void GameClient::departed(Player *player)
{
  if (player)
    {
      //now turning a human player into a network player
      NetworkPlayer *new_p = new NetworkPlayer(*player);
      Playerlist::getInstance()->swap(player, new_p);
      delete player;
      listenForActions(new_p);
      listenForHistories(new_p);
      new_p->setConnected(false);
      remote_player_connected.emit(new_p);
    }
  else
    remote_player_disconnected.emit(player);
}

void GameClient::onGotMessage(MessageType type, std::string payload)
{
  std::cerr << "got message of type " << type << std::endl;
  switch (type) {
  case MESSAGE_TYPE_PING:
    network_connection->send(MESSAGE_TYPE_PONG, "PONGOGOGO");
    break;

  case MESSAGE_TYPE_PONG:
    std::cerr << "sending join for player number " <<player_id << std::endl;
    switch (player_id)
      {
      case 0: type = MESSAGE_TYPE_P1_JOIN; break;
      case 1: type = MESSAGE_TYPE_P2_JOIN; break;
      case 2: type = MESSAGE_TYPE_P3_JOIN; break;
      case 3: type = MESSAGE_TYPE_P4_JOIN; break;
      case 4: type = MESSAGE_TYPE_P5_JOIN; break;
      case 5: type = MESSAGE_TYPE_P6_JOIN; break;
      case 6: type = MESSAGE_TYPE_P7_JOIN; break;
      case 7: type = MESSAGE_TYPE_P8_JOIN; break;
      case -1: type = MESSAGE_TYPE_VIEWER_JOIN; break;
      default:
	      return;
      }
    network_connection->send(type, "I wanna join");
    break;

  case MESSAGE_TYPE_SENDING_ACTIONS:
    gotActions(payload);
    break;
    
  case MESSAGE_TYPE_SENDING_MAP:
    gotScenario(payload);
    if (player_id > -1)
      {
	Player *player = Playerlist::getInstance()->getPlayer(player_id);
	listenForActions(player);
	listenForHistories(player);
      }
    break;

  case MESSAGE_TYPE_P1_JOIN:
  case MESSAGE_TYPE_P2_JOIN:
  case MESSAGE_TYPE_P3_JOIN:
  case MESSAGE_TYPE_P4_JOIN:
  case MESSAGE_TYPE_P5_JOIN:
  case MESSAGE_TYPE_P6_JOIN:
  case MESSAGE_TYPE_P7_JOIN:
  case MESSAGE_TYPE_P8_JOIN:
  case MESSAGE_TYPE_VIEWER_JOIN:
  case MESSAGE_TYPE_P1_DEPART:
  case MESSAGE_TYPE_P2_DEPART:
  case MESSAGE_TYPE_P3_DEPART:
  case MESSAGE_TYPE_P4_DEPART:
  case MESSAGE_TYPE_P5_DEPART:
  case MESSAGE_TYPE_P6_DEPART:
  case MESSAGE_TYPE_P7_DEPART:
  case MESSAGE_TYPE_P8_DEPART:
  case MESSAGE_TYPE_VIEWER_DEPART:
    //FIXME: faulty server.
    break;

    //this is the client realizing that some other player joined the server
  case MESSAGE_TYPE_P1_JOINED:
    joined(Playerlist::getInstance()->getPlayer(0));
    break;
  case MESSAGE_TYPE_P2_JOINED:
    joined(Playerlist::getInstance()->getPlayer(1));
    break;
  case MESSAGE_TYPE_P3_JOINED:
    joined(Playerlist::getInstance()->getPlayer(2));
    break;
  case MESSAGE_TYPE_P4_JOINED:
    joined(Playerlist::getInstance()->getPlayer(3));
    break;
  case MESSAGE_TYPE_P5_JOINED:
    joined(Playerlist::getInstance()->getPlayer(4));
    break;
  case MESSAGE_TYPE_P6_JOINED:
    joined(Playerlist::getInstance()->getPlayer(5));
    break;
  case MESSAGE_TYPE_P7_JOINED:
    joined(Playerlist::getInstance()->getPlayer(6));
    break;
  case MESSAGE_TYPE_P8_JOINED:
    joined(Playerlist::getInstance()->getPlayer(7));
    break;
  case MESSAGE_TYPE_VIEWER_JOINED:
    joined(NULL);
    break;
    //this is the client realizing that a remote player has disconnected
  case MESSAGE_TYPE_P1_DEPARTED:
    departed(Playerlist::getInstance()->getPlayer(0));
    break;
  case MESSAGE_TYPE_P2_DEPARTED:
    departed(Playerlist::getInstance()->getPlayer(1));
    break;
  case MESSAGE_TYPE_P3_DEPARTED:
    departed(Playerlist::getInstance()->getPlayer(2));
    break;
  case MESSAGE_TYPE_P4_DEPARTED:
    departed(Playerlist::getInstance()->getPlayer(3));
    break;
  case MESSAGE_TYPE_P5_DEPARTED:
    departed(Playerlist::getInstance()->getPlayer(4));
    break;
  case MESSAGE_TYPE_P6_DEPARTED:
    departed(Playerlist::getInstance()->getPlayer(5));
    break;
  case MESSAGE_TYPE_P7_DEPARTED:
    departed(Playerlist::getInstance()->getPlayer(6));
    break;
  case MESSAGE_TYPE_P8_DEPARTED:
    departed(Playerlist::getInstance()->getPlayer(7));
    break;
  case MESSAGE_TYPE_VIEWER_DEPARTED:
    departed(NULL);
    break;

  case MESSAGE_TYPE_SENDING_HISTORY:
    gotHistories(payload);
    break;
    
  }
}

void GameClient::listenForActions(Player *player)
{
  if (!player)
    return;
  player->acting.connect(sigc::mem_fun(this, &GameClient::onActionDone));
}

void GameClient::listenForHistories(Player *player)
{
  if (!player)
    return;
 player->history_written.connect(sigc::mem_fun(this, &GameClient::onHistoryDone));
}

void GameClient::clearNetworkActionlist(std::list<NetworkAction*> actions)
{
    for (std::list<NetworkAction*>::iterator it = actions.begin();
        it != actions.end(); it++)
      {
	delete (*it);
      }
    actions.clear();
}

void GameClient::clearNetworkHistorylist(std::list<NetworkHistory*> histories)
{
    for (std::list<NetworkHistory*>::iterator it = histories.begin();
        it != histories.end(); it++)
      {
	delete (*it);
      }
    histories.clear();
}

void GameClient::onHistoryDone(NetworkHistory *history)
{
  std::string desc = history->toString();
  std::cerr << "Game Client got " << desc <<"\n";

  histories.push_back(history);
  sendHistories();
  clearNetworkHistorylist(histories);
}

void GameClient::onActionDone(NetworkAction *action)
{
  std::string desc = action->toString();
  std::cerr << "Game Client got " << desc <<"\n";

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
    (**i).save(&helper);

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
  MessageType type;
  switch (player->getId())
    {
    case 0: type = MESSAGE_TYPE_P1_JOIN; break;
    case 1: type = MESSAGE_TYPE_P2_JOIN; break;
    case 2: type = MESSAGE_TYPE_P3_JOIN; break;
    case 3: type = MESSAGE_TYPE_P4_JOIN; break;
    case 4: type = MESSAGE_TYPE_P5_JOIN; break;
    case 5: type = MESSAGE_TYPE_P6_JOIN; break;
    case 6: type = MESSAGE_TYPE_P7_JOIN; break;
    case 7: type = MESSAGE_TYPE_P8_JOIN; break;
    default:
	     return;
    }
  network_connection->send(type, "I wanna join");
}

void GameClient::stand_up (Player *player)
{
  MessageType type;
  switch (player->getId())
    {
    case 0: type = MESSAGE_TYPE_P1_DEPART; break;
    case 1: type = MESSAGE_TYPE_P2_DEPART; break;
    case 2: type = MESSAGE_TYPE_P3_DEPART; break;
    case 3: type = MESSAGE_TYPE_P4_DEPART; break;
    case 4: type = MESSAGE_TYPE_P5_DEPART; break;
    case 5: type = MESSAGE_TYPE_P6_DEPART; break;
    case 6: type = MESSAGE_TYPE_P7_DEPART; break;
    case 7: type = MESSAGE_TYPE_P8_DEPART; break;
    default:
	     return;
    }
  network_connection->send(type, "I wanna depart");
}
