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
#include <list>

#include "gamelist-server.h"

#include "network-server.h"
#include "xmlhelper.h"
#include "Configuration.h"
#include "ucompose.hpp"
#include "gamelist.h"

GamelistServer * GamelistServer::s_instance = 0;

GamelistServer* GamelistServer::getInstance()
{
    if (s_instance == 0)
        s_instance = new GamelistServer();

    return s_instance;
}

void GamelistServer::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

GamelistServer::GamelistServer()
{
  Timing::instance().timer_registered.connect
    (sigc::mem_fun(*this, &GamelistServer::on_timer_registered));
  Gamelist::getInstance()->load();
}

GamelistServer::~GamelistServer()
{
  if (network_server.get() != NULL)
    {
      if (network_server->isListening())
        network_server->stop();
    }
}

void GamelistServer::reload()
{
  Gamelist::getInstance()->load();
}

bool GamelistServer::isListening()
{
  if (network_server.get() != NULL)
    return network_server->isListening();
  else
    return false;
}

void GamelistServer::start(int port)
{
  if (network_server.get() != NULL && network_server->isListening())
    return;
  network_server.reset(new NetworkServer());
  network_server->port_in_use.connect
    (sigc::mem_fun(port_in_use, &sigc::signal<void, int>::emit));
  network_server->got_message.connect
    (sigc::mem_fun(this, &GamelistServer::onGotMessage));
  network_server->connection_lost.connect
    (sigc::mem_fun(this, &GamelistServer::onConnectionLost));
  network_server->connection_made.connect
    (sigc::mem_fun(this, &GamelistServer::onConnectionMade));

  network_server->startListening(port);

}

bool GamelistServer::onGotMessage(void *conn, int type, std::string payload)
{
  std::cerr << "got message of type " << type << std::endl;
  switch (GlsMessageType(type)) 
    {
    case GLS_MESSAGE_HOST_NEW_GAME:
      break;
    case GLS_MESSAGE_HOST_NEW_RANDOM_GAME:
      break;
    case GLS_MESSAGE_ADVERTISE_GAME:
      break;
    case GLS_MESSAGE_UNADVERTISE_GAME:
      break;
    case GLS_MESSAGE_UNHOST_GAME:
      break;
    case GLS_MESSAGE_REQUEST_GAME_LIST:
      break;
    case GLS_MESSAGE_GAME_CREATED:
    case GLS_MESSAGE_GAME_LIST:
    case GLS_MESSAGE_GAME_UNHOSTED:
    case GLS_MESSAGE_COULD_NOT_HOST_GAME:
    case GLS_MESSAGE_COULD_NOT_UNHOST_GAME:
    case GLS_MESSAGE_COULD_NOT_ADVERTISE_GAME:
    case GLS_MESSAGE_COULD_NOT_UNADVERTISE_GAME:
    case GLS_MESSAGE_GAME_ADVERTISED:
    case GLS_MESSAGE_GAME_UNADVERTISED:
    case GLS_MESSAGE_COULD_NOT_GET_GAME_LIST:
      //faulty client
      break;
    }
  return true;
}

void GamelistServer::onConnectionMade(void *conn)
{
  std::cerr << "connection made" << std::endl;
}

void GamelistServer::onConnectionLost(void *conn)
{
  std::cerr << "connection lost" << std::endl;
}

sigc::connection GamelistServer::on_timer_registered(Timing::timer_slot s,
                                                     int msecs_interval)
{
    return Glib::signal_timeout().connect(s, msecs_interval);
}
// End of file
