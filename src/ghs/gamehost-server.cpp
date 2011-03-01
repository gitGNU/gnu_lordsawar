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
#include <fstream>
#include <sstream>
#include <list>

#include "gamehost-server.h"

#include "network-server.h"
#include "xmlhelper.h"
#include "Configuration.h"
#include "ucompose.hpp"
#include "gamelist.h"
#include "recently-played-game-list.h"
#include "recently-played-game.h"
#include "hosted-game.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

GamehostServer * GamehostServer::s_instance = 0;

GamehostServer* GamehostServer::getInstance()
{
    if (s_instance == 0)
        s_instance = new GamehostServer();

    return s_instance;
}

void GamehostServer::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

GamehostServer::GamehostServer()
{
  Timing::instance().timer_registered.connect
    (sigc::mem_fun(*this, &GamehostServer::on_timer_registered));
  Gamelist::getInstance()->load();
  Gamelist::getInstance()->pruneGames();
  Gamelist::getInstance()->pingGames();
}

GamehostServer::~GamehostServer()
{
  if (network_server.get() != NULL)
    {
      if (network_server->isListening())
        network_server->stop();
    }
}

void GamehostServer::reload()
{
  Gamelist::getInstance()->load();
}

bool GamehostServer::isListening()
{
  if (network_server.get() != NULL)
    return network_server->isListening();
  else
    return false;
}

void GamehostServer::start(int port)
{
  if (network_server.get() != NULL && network_server->isListening())
    return;
  network_server.reset(new NetworkServer());
  network_server->port_in_use.connect
    (sigc::mem_fun(port_in_use, &sigc::signal<void, int>::emit));
  network_server->got_message.connect
    (sigc::mem_fun(this, &GamehostServer::onGotMessage));
  network_server->connection_lost.connect
    (sigc::mem_fun(this, &GamehostServer::onConnectionLost));
  network_server->connection_made.connect
    (sigc::mem_fun(this, &GamehostServer::onConnectionMade));

  network_server->startListening(port);

}

void GamehostServer::sendList(void *conn)
{
  std::ostringstream os;
  XML_Helper helper(&os);
  RecentlyPlayedGameList *l;
  if (network_server->is_local_connection(conn))
    l = Gamelist::getInstance()->getList(false);
  else
    l = Gamelist::getInstance()->getList(true);

  l->save(&helper);
  network_server->send(conn, GHS_MESSAGE_GAME_LIST, os.str());
  delete l;
}

bool GamehostServer::onGotMessage(void *conn, int type, std::string payload)
{
  debug("got message of type " << type);
  switch (GhsMessageType(type)) 
    {
    case GHS_MESSAGE_HOST_NEW_GAME:
      break;
    case GHS_MESSAGE_HOST_NEW_RANDOM_GAME:
      break;
    case GHS_MESSAGE_UNHOST_GAME:
      break;
    case GHS_MESSAGE_REQUEST_GAME_LIST:
      sendList(conn);
      break;
    case GHS_MESSAGE_REQUEST_RELOAD:
      if (network_server->is_local_connection(conn))
        {
          Gamelist::getInstance()->load();
          network_server->send(conn, GHS_MESSAGE_RELOADED, "");
        }
      else
          network_server->send(conn, GHS_MESSAGE_COULD_NOT_RELOAD, 
                               _("permission denied"));
      break;
    case GHS_MESSAGE_GAME_LIST:
    case GHS_MESSAGE_COULD_NOT_RELOAD:
    case GHS_MESSAGE_RELOADED:
    case GHS_MESSAGE_GAME_CREATED:
    case GHS_MESSAGE_GAME_UNHOSTED:
    case GHS_MESSAGE_COULD_NOT_HOST_GAME:
    case GHS_MESSAGE_COULD_NOT_UNHOST_GAME:
    case GHS_MESSAGE_COULD_NOT_GET_GAME_LIST:
      //faulty client
      break;
    }
  return true;
}

void GamehostServer::onConnectionMade(void *conn)
{
  debug("connection made");
  printf("pinging games now.\n");
  Gamelist::getInstance()->pruneGames();
  Gamelist::getInstance()->pingGames();
}

void GamehostServer::onConnectionLost(void *conn)
{
  debug("connection lost");
}

sigc::connection GamehostServer::on_timer_registered(Timing::timer_slot s,
                                                     int msecs_interval)
{
    return Glib::signal_timeout().connect(s, msecs_interval);
}
             
// End of file
