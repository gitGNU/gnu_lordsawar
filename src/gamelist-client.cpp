// Copyright (C) 2011 Ben Asselstine
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

#include "gamelist-client.h"

#include "network-connection.h"
#include "network-gls-common.h"
#include "xmlhelper.h"
#include "ucompose.hpp"
#include "profile.h"
#include "profilelist.h"
#include "advertised-game.h"
#include "recently-played-game-list.h"
#include "recently-played-game.h"
  
//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

GamelistClient * GamelistClient::s_instance = 0;

GamelistClient* GamelistClient::getInstance()
{
    if (s_instance == 0)
        s_instance = new GamelistClient();

    return s_instance;
}

void GamelistClient::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

GamelistClient::GamelistClient()
{
}

GamelistClient::~GamelistClient()
{
}

void GamelistClient::start(std::string host, guint32 port, Profile *p)
{
  d_host = host;
  d_port = port;
  d_profile_id = p->getId();
  network_connection.reset(new NetworkConnection());
  network_connection->connected.connect(
    sigc::mem_fun(this, &GamelistClient::onConnected));
  network_connection->connection_lost.connect(
    sigc::mem_fun(this, &GamelistClient::onConnectionLost));
  network_connection->got_message.connect(
    sigc::mem_fun(this, &GamelistClient::onGotMessage));
  network_connection->connection_failed.connect
    (sigc::mem_fun(this->client_could_not_connect, &sigc::signal<void>::emit));
  network_connection->connectToHost(host, port);
}

void GamelistClient::onConnected() 
{
  debug("GamelistClient connected");

  d_connected = true;
  client_connected.emit();
}

void GamelistClient::onConnectionLost()
{
  debug("GamelistClient connection lost");
  if (d_connected)
    client_forcibly_disconnected.emit();
  else
    client_could_not_connect.emit();
}

bool GamelistClient::onGotMessage(int type, std::string payload)
{
  size_t pos;
  debug("GamelistClient got message of type " << type);
  switch (GlsMessageType(type)) 
    {
    case GLS_MESSAGE_GAME_CREATED:
      break;
    case GLS_MESSAGE_GAME_LIST:
        {
          std::istringstream is(payload);
          XML_Helper helper(&is);
          helper.registerTag
            (RecentlyPlayedGameList::d_tag, 
             sigc::mem_fun(*this, &GamelistClient::loadRecentlyPlayedGameList));
          helper.parse();
          received_game_list.emit(d_recently_played_game_list, "");
        }
      break;
    case GLS_MESSAGE_COULD_NOT_GET_GAME_LIST:
      received_game_list.emit(NULL, payload);
      break;
    case GLS_MESSAGE_GAME_UNHOSTED:
      break;
    case GLS_MESSAGE_COULD_NOT_HOST_GAME:
      break;
    case GLS_MESSAGE_COULD_NOT_UNHOST_GAME:
      break;
    case GLS_MESSAGE_COULD_NOT_ADVERTISE_GAME:
        {
          pos = payload.find(' ');
          if (pos == std::string::npos)
            return false;
          received_advertising_response.emit(payload.substr(0, pos - 1), 
                                             payload.substr(pos + 1));
        }
      break;
    case GLS_MESSAGE_COULD_NOT_UNADVERTISE_GAME:
        {
          pos = payload.find(' ');
          if (pos == std::string::npos)
            return false;
          received_advertising_removal_response.emit
            (payload.substr(0, pos - 1), payload.substr(pos + 1));
        }
      break;
    case GLS_MESSAGE_GAME_ADVERTISED:
      received_advertising_response.emit(payload, "");
      break;
    case GLS_MESSAGE_GAME_UNADVERTISED:
      received_advertising_response.emit(payload, "");
      break;
    case GLS_MESSAGE_RELOADED:
      received_reload_response.emit("");
      break;
    case GLS_MESSAGE_COULD_NOT_RELOAD:
      received_reload_response.emit(payload);
      break;
    case GLS_MESSAGE_HOST_NEW_GAME:
    case GLS_MESSAGE_HOST_NEW_RANDOM_GAME:
    case GLS_MESSAGE_ADVERTISE_GAME:
    case GLS_MESSAGE_UNADVERTISE_GAME:
    case GLS_MESSAGE_UNHOST_GAME:
    case GLS_MESSAGE_REQUEST_GAME_LIST:
    case GLS_MESSAGE_REQUEST_RELOAD:
      //faulty server
      break;
    }
  return true;
}

void GamelistClient::disconnect()
{
  d_connected = false;
}
  
void GamelistClient::request_game_list()
{
  network_connection->send(GLS_MESSAGE_REQUEST_GAME_LIST, d_profile_id);
}

void GamelistClient::request_advertising(RecentlyPlayedGame *game)
{
  if (game)
    {
      RecentlyPlayedNetworkedGame *ng = 
        dynamic_cast<RecentlyPlayedNetworkedGame*>(game);
      Profile *p = Profilelist::getInstance()->findProfileById(d_profile_id);
      AdvertisedGame *g = new AdvertisedGame(*ng, p);
      std::ostringstream os;
      XML_Helper helper(&os);
      helper.begin(LORDSAWAR_RECENTLY_HOSTED_VERSION);
      helper.openTag("advertising_request");
      g->saveEntry(&helper);
      helper.closeTag();
      network_connection->send(GLS_MESSAGE_ADVERTISE_GAME, os.str());
      delete g;
    }
}

void GamelistClient::request_advertising_removal(std::string scenario_id)
{
  network_connection->send(GLS_MESSAGE_UNADVERTISE_GAME, 
                           d_profile_id + " " + scenario_id);

}
             
bool GamelistClient::loadRecentlyPlayedGameList(std::string tag, XML_Helper *helper)
{
  if (tag == RecentlyPlayedGameList::d_tag)
    {
      d_recently_played_game_list = new RecentlyPlayedGameList(helper);
      return true;
    }
  else if (tag == RecentlyPlayedGame::d_tag)
    {
      RecentlyPlayedGame *g = RecentlyPlayedGame::handle_load(helper);
      d_recently_played_game_list->push_back(g);
      return true;
    }
  return false;
}

void GamelistClient::request_reload()
{
  network_connection->send(GLS_MESSAGE_REQUEST_RELOAD, "");
}
