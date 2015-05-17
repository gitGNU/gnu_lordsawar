// Copyright (C) 2008 Ole Laursen
// Copyright (C) 2011, 2014 Ben Asselstine
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
#include <fstream>

#include "network-common.h"
#include "game-client.h"

#include "network-connection.h"
#include "File.h"
#include "action.h"
#include "network-action.h"
#include "network-history.h"
#include "playerlist.h"
#include "network_player.h"
#include "xmlhelper.h"
#include "ucompose.hpp"
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

void GameClient::start(Glib::ustring host, guint32 port, Glib::ustring profile_id, Glib::ustring nick)
{
  d_host = host;
  d_port = port;
  player_id = -1;
  setNickname(nick);
  setProfileId(profile_id);
  network_connection.reset(new NetworkConnection());
  network_connection->connected.connect(
    sigc::mem_fun(this, &GameClient::onConnected));
  network_connection->connection_lost.connect(
    sigc::mem_fun(this, &GameClient::onConnectionLost));
  network_connection->got_message.connect(
    sigc::mem_fun(this, &GameClient::onGotMessage));
  network_connection->connection_failed.connect
    (sigc::mem_fun(this->client_could_not_connect, &sigc::signal<void>::emit));
  network_connection->connectToHost(host, port);
}

void GameClient::onConnected() 
{
  std::cerr << "connected" << std::endl;
  d_connected = true;

  network_connection->send(MESSAGE_TYPE_PING, "");

  client_connected.emit();
}

void GameClient::onConnectionLost()
{
  std::cerr << "connection lost" << std::endl;
  if (d_connected)
    client_forcibly_disconnected.emit();
  else
    client_could_not_connect.emit();
}

void GameClient::sat_down(Player *player, Glib::ustring nickname)
{
  if (!player)
    return;
  if (player->getType() == Player::NETWORKED)
    dynamic_cast<NetworkPlayer*>(player)->setConnected(true);
  player_sits.emit(player, nickname);
}

void GameClient::name_changed (Player *player, Glib::ustring name)
{
  if (player)
    player_changes_name.emit(player, name);
}

void GameClient::type_changed (Player *player, int type)
{
  if (!player)
    return;
  //we don't change the type of player here.
  //the player is networked on the remote side, and not networked 
  //(e.g. ai_smart) on the local side.
  //we show the "ai smart" in the game lobby, but the player object is still
  //networked.
  player_changes_type.emit(player, type);
}

void GameClient::stood_up(Player *player, Glib::ustring nickname)
{
  if (!player)
    return;
  if (player->getType() == Player::HUMAN)
    {
      //this covers the "boot", or forcibly-stand case.
      NetworkPlayer *new_p = new NetworkPlayer(*player);
      Playerlist::getInstance()->swap(player, new_p);
      delete player;
      player = new_p;
      new_p->setConnected(false);
    }
  else if (player->getType() == Player::NETWORKED)
    dynamic_cast<NetworkPlayer*>(player)->setConnected(false);
  player_stands.emit(player, nickname);
}

bool GameClient::onGotMessage(int type, Glib::ustring payload)
{
  std::cerr << String::ucompose("got message of type %1", type) << std::endl;
  switch (MessageType(type)) {
  case MESSAGE_TYPE_PING:
    network_connection->send(MESSAGE_TYPE_PONG, "PONGOGOGO");
    break;

  case MESSAGE_TYPE_PONG:
    network_connection->send(MESSAGE_TYPE_PARTICIPANT_CONNECT, 
                             getNickname() + " " + getProfileId());
    break;

  case MESSAGE_TYPE_SENDING_ACTIONS:
    gotActions(payload);
    break;

  case MESSAGE_TYPE_SENDING_MAP:
    game_scenario_received.emit(payload);
    break;

  case MESSAGE_TYPE_PARTICIPANT_CONNECTED:
    std::cerr << String::ucompose("message: %1 has data: %2", type, payload) << std::endl;
    remote_participant_joins.emit(payload);
    break;

  case MESSAGE_TYPE_PARTICIPANT_DISCONNECTED:
    std::cerr << String::ucompose("message: %1 has data: %2", type, payload) << std::endl;
    remote_participant_departs.emit(payload);
    break;

  case MESSAGE_TYPE_ROUND_OVER:
    round_ends.emit();
    break;

  case MESSAGE_TYPE_REQUEST_SEAT_MANIFEST:
  case MESSAGE_TYPE_PARTICIPANT_CONNECT:
  case MESSAGE_TYPE_PARTICIPANT_DISCONNECT:
  case MESSAGE_TYPE_CHAT:
    //FIXME: faulty server.
    break;

  case MESSAGE_TYPE_CHATTED:
    gotChatMessage("", payload);
    break;
    //this is the client realizing that some other player joined the server

  case MESSAGE_TYPE_SENDING_HISTORY:
    gotHistories(payload);
    break;
  
  case MESSAGE_TYPE_SERVER_DISCONNECT:
    return false;
    break;

  case MESSAGE_TYPE_TURN_ORDER:
    gotTurnOrder (payload);
    break;

  case MESSAGE_TYPE_KILL_PLAYER:
    gotKillPlayer(Playerlist::getInstance()->getPlayer(atoi(payload.c_str())));
    break;

  case MESSAGE_TYPE_NEXT_PLAYER:
    start_player_turn.emit
      (Playerlist::getInstance()->getPlayer(atoi(payload.c_str())));
    break;

  case MESSAGE_TYPE_ROUND_START:
    round_begins.emit();
    break;

  case MESSAGE_TYPE_CHANGE_NICKNAME:
    nickname_changed.emit(d_nickname, payload);
    d_nickname = payload;
    break;

  case MESSAGE_TYPE_GAME_MAY_BEGIN:
    game_may_begin.emit();
    break;

  case MESSAGE_TYPE_OFF_PLAYER:
    gotOffPlayer(Playerlist::getInstance()->getPlayer(atoi(payload.c_str())));
    break;

  case MESSAGE_TYPE_LOBBY_ACTIVITY:
      {
        guint32 id;
        gint32 action;
        bool reported;
        Glib::ustring data;
        bool success = get_message_lobby_activity (payload, id, action, 
                                                   reported, data);
        if (success)
          {
            if (reported)
              {
                switch (action)
                  {
                  case LOBBY_MESSAGE_TYPE_SIT:
                    sat_down(Playerlist::getInstance()->getPlayer(id), data);
                    break;
                  case LOBBY_MESSAGE_TYPE_STAND:
                    stood_up(Playerlist::getInstance()->getPlayer(id), data);
                    break;
                  case LOBBY_MESSAGE_TYPE_CHANGE_NAME:
                    name_changed(Playerlist::getInstance()->getPlayer(id), 
                                 data);
                    break;
                  case LOBBY_MESSAGE_TYPE_CHANGE_TYPE:
                    type_changed(Playerlist::getInstance()->getPlayer(id), 
                                 atoi(data.c_str()));
                    break;
                  default:
                    break;
                  }
              }
          }
      }
    break;
  }
  return true;
}

void GameClient::gotOffPlayer(Player *player)
{
  player_gets_turned_off.emit(player);
  GameParameters::Player p;
  p.id = player->getId();
  p.type = GameParameters::Player::OFF;
  p.name = player->getName();
  Playerlist::getInstance()->syncPlayer(p);
}  

void GameClient::gotKillPlayer(Player *player)
{
  player->kill();
}  

void GameClient::onHistoryDone(NetworkHistory *history)
{
  Glib::ustring desc = history->toString();
  std::cerr << String::ucompose("Game Client got %1", desc) << std::endl;

  if (history->getHistory()->getType() == History::PLAYER_VANQUISHED)
    local_player_died(history->getOwner());

  histories.push_back(history);
  sendHistories();
  clearNetworkHistorylist(histories);
}

void GameClient::onActionDone(NetworkAction *action)
{
  Glib::ustring desc = action->toString();
  std::cerr << String::ucompose("Game Client got %1", desc) << std::endl;

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

  //std::cerr << "sending actions" << std::endl;
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

void GameClient::sit_or_stand (Player *player, bool sit)
{
  if (!player)
    return;
  Glib::ustring payload = 
    String::ucompose("%1 %2 %3 %4", player->getId(), 
                     sit ? LOBBY_MESSAGE_TYPE_SIT : LOBBY_MESSAGE_TYPE_STAND, 
                     0, 
                     d_nickname);
  network_connection->send(MESSAGE_TYPE_LOBBY_ACTIVITY, payload);
  if (sit)
    {
      RealPlayer *new_p = new RealPlayer (*player);
      Playerlist::getInstance()->swap(player, new_p);
      stopListeningForLocalEvents(player);
      listenForLocalEvents(new_p);
      delete player;
      payload = String::ucompose("%1 %2 %3 %4", new_p->getId(), 
                     LOBBY_MESSAGE_TYPE_CHANGE_TYPE, 0, new_p->getType());
      network_connection->send(MESSAGE_TYPE_LOBBY_ACTIVITY, payload);
    }
  else
    {
      NetworkPlayer *new_p = new NetworkPlayer(*player);
      Playerlist::getInstance()->swap(player, new_p);
      stopListeningForLocalEvents(player);
      delete player;
      new_p->setConnected(false);
    }

}

void GameClient::change_name (Player *player, Glib::ustring name)
{
  Glib::ustring payload = 
    String::ucompose("%1 %2 %3 %4", player->getId(), 
                     LOBBY_MESSAGE_TYPE_CHANGE_NAME, 0, name);
  network_connection->send(MESSAGE_TYPE_LOBBY_ACTIVITY, payload);
}

void GameClient::change_type (Player *player, int type)
{
  Glib::ustring payload = 
    String::ucompose("%1 %2 %3 %4", player->getId(), 
                     LOBBY_MESSAGE_TYPE_CHANGE_TYPE, 0, type);
  network_connection->send(MESSAGE_TYPE_LOBBY_ACTIVITY, payload);
}

void GameClient::sit_down (Player *player)
{
  sit_or_stand (player, true);
}

void GameClient::stand_up (Player *player)
{
  sit_or_stand (player, false);
}

void GameClient::chat(Glib::ustring message)
{
  network_connection->send(MESSAGE_TYPE_CHAT, d_nickname + ":" + message);
}
  
void GameClient::request_seat_manifest()
{
  network_connection->send(MESSAGE_TYPE_REQUEST_SEAT_MANIFEST, "");
}
    
void GameClient::gotTurnOrder (Glib::ustring payload)
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

void GameClient::disconnect()
{
  d_connected = false;
  if (network_connection.get() != NULL)
    network_connection->send(MESSAGE_TYPE_PARTICIPANT_DISCONNECT, d_nickname);
}
