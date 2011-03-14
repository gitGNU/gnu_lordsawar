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

#include "gamehost-client.h"

#include "network-connection.h"
#include "network-ghs-common.h"
#include "xmlhelper.h"
#include "ucompose.hpp"
#include "profile.h"
#include "profilelist.h"
#include "recently-played-game-list.h"
#include "recently-played-game.h"
  
//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

GamehostClient * GamehostClient::s_instance = 0;

GamehostClient* GamehostClient::getInstance()
{
    if (s_instance == 0)
        s_instance = new GamehostClient();

    return s_instance;
}

void GamehostClient::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

GamehostClient::GamehostClient()
{
  network_connection.reset();
}

GamehostClient::~GamehostClient()
{
}

void GamehostClient::start(std::string host, guint32 port, Profile *p)
{
  d_host = host;
  d_port = port;
  d_profile_id = p->getId();
  network_connection.reset(new NetworkConnection());
  network_connection->connected.connect(
    sigc::mem_fun(this, &GamehostClient::onConnected));
  network_connection->connection_lost.connect(
    sigc::mem_fun(this, &GamehostClient::onConnectionLost));
  network_connection->got_message.connect(
    sigc::mem_fun(this, &GamehostClient::onGotMessage));
  network_connection->connection_failed.connect
    (sigc::mem_fun(this->client_could_not_connect, &sigc::signal<void>::emit));
  network_connection->connectToHost(host, port);
}

void GamehostClient::onConnected() 
{
  debug("GamehostClient connected");

  d_connected = true;
  client_connected.emit();
}

void GamehostClient::onConnectionLost()
{
  debug("GamehostClient connection lost");
  if (d_connected)
    client_forcibly_disconnected.emit();
  else
    client_could_not_connect.emit();
}

bool GamehostClient::onGotMessage(int type, std::string payload)
{
  size_t pos;
  debug("GamehostClient got message of type " << type);
  switch (GhsMessageType(type)) 
    {
    case GHS_MESSAGE_AWAITING_MAP:
      received_host_response.emit(payload, "");
      break;
    case GHS_MESSAGE_GAME_UNHOSTED:
      received_unhost_response.emit(payload, "");
      break;
    case GHS_MESSAGE_COULD_NOT_HOST_GAME:
        {
          pos = payload.find(' ');
          if (pos == std::string::npos)
            return false;
          received_host_response.emit(payload.substr(0, pos), 
                                      payload.substr(pos + 1));
        }
      break;
    case GHS_MESSAGE_COULD_NOT_UNHOST_GAME:
        {
          pos = payload.find(' ');
          if (pos == std::string::npos)
            return false;
          received_unhost_response.emit(payload.substr(0, pos), 
                                        payload.substr(pos + 1));
        }
      break;
    case GHS_MESSAGE_GAME_LIST:
        {
          std::istringstream is(payload);
          XML_Helper helper(&is);
          helper.registerTag
            (RecentlyPlayedGameList::d_tag, 
             sigc::mem_fun(*this, &GamehostClient::loadRecentlyPlayedGameList));
          helper.parse();
          received_game_list.emit(d_recently_played_game_list, "");
        }
      break;
    case GHS_MESSAGE_COULD_NOT_GET_GAME_LIST:
      received_game_list.emit(NULL, payload);
      break;
    case GHS_MESSAGE_RELOADED:
      received_reload_response.emit("");
      break;
    case GHS_MESSAGE_COULD_NOT_RELOAD:
      received_reload_response.emit(payload);
      break;
    case GHS_MESSAGE_COULD_NOT_READ_MAP:
    case GHS_MESSAGE_COULD_NOT_START_GAME:
        {
          pos = payload.find(' ');
          if (pos == std::string::npos)
            return false;
          received_map_response.emit(payload.substr(0, pos), 0,
                                     payload.substr(pos + 1));
        }
      break;
    case GHS_MESSAGE_GAME_HOSTED:
        {
          std::string scenario_id;
          guint32 port = 0;
          std::stringstream spayload;
          spayload.str(payload);
          spayload >> scenario_id;
          spayload >> port;
          received_map_response.emit(scenario_id, port, "");
        }
      break;
    case GHS_MESSAGE_SENDING_MAP:
    case GHS_MESSAGE_REQUEST_GAME_LIST:
    case GHS_MESSAGE_REQUEST_RELOAD:
    case GHS_MESSAGE_HOST_NEW_GAME:
    case GHS_MESSAGE_UNHOST_GAME:
    case GHS_MESSAGE_REQUEST_TERMINATION:
      //faulty server
      break;
    }
  return true;
}

void GamehostClient::disconnect()
{
  if (network_connection.get())
    network_connection->disconnect();
  d_connected = false;
}
  
void GamehostClient::request_game_list()
{
  network_connection->send(GHS_MESSAGE_REQUEST_GAME_LIST, d_profile_id);
}

bool GamehostClient::loadRecentlyPlayedGameList(std::string tag, XML_Helper *helper)
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

void GamehostClient::request_reload()
{
  network_connection->send(GHS_MESSAGE_REQUEST_RELOAD, "");
} 

void GamehostClient::request_game_unhost(std::string scenario_id)
{
  network_connection->send(GHS_MESSAGE_UNHOST_GAME, 
                           d_profile_id + " " + scenario_id);

}
  
void GamehostClient::request_game_host(std::string scenario_id)
{
  Profile *profile = Profilelist::getInstance()->findProfileById(d_profile_id);
  //dump the profile to a string
  std::ostringstream os;
  XML_Helper helper(&os);
  helper.begin(LORDSAWAR_RECENTLY_HOSTED_VERSION);
  profile->save(&helper);
  helper.close();
  // os.str() is the first part that contains the profile object.
  // it is followed by the scenario id, outside of any tags.
  std::string data = os.str() + scenario_id;
  network_connection->send(GHS_MESSAGE_HOST_NEW_GAME, os.str() + scenario_id);
  return;
}

void GamehostClient::send_map(GameScenario *game_scenario)
{
  std::string tmpfile = "lw.XXXX";
  int fd = Glib::file_open_tmp(tmpfile, "lw.XXXX");
  close(fd);
  tmpfile += SAVE_EXT;
  game_scenario->saveGame(tmpfile);
  send_map_file(tmpfile);
  File::erase(tmpfile);
}

void GamehostClient::send_map_file(std::string file)
{
  network_connection->sendFile(GHS_MESSAGE_SENDING_MAP, file);
}

void GamehostClient::request_server_terminate()
{
  network_connection->send(GHS_MESSAGE_REQUEST_TERMINATION, "");
}
