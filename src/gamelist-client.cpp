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
  std::cerr << "connected" << std::endl;

  d_connected = true;
  client_connected.emit();
}

void GamelistClient::onConnectionLost()
{
  std::cerr << "connection lost" << std::endl;
  if (d_connected)
    client_forcibly_disconnected.emit();
  else
    client_could_not_connect.emit();
}

bool GamelistClient::onGotMessage(int type, std::string payload)
{
  std::cerr << "got message of type " << type << std::endl;
  switch (GlsMessageType(type)) 
    {
    case GLS_MESSAGE_GAME_CREATED:
      break;
    case GLS_MESSAGE_GAME_LIST:
      break;
    case GLS_MESSAGE_GAME_UNHOSTED:
      break;
    case GLS_MESSAGE_COULD_NOT_HOST_GAME:
      break;
    case GLS_MESSAGE_COULD_NOT_UNHOST_GAME:
      break;
    case GLS_MESSAGE_COULD_NOT_ADVERTISE_GAME:
      break;
    case GLS_MESSAGE_COULD_NOT_UNADVERTISE_GAME:
      break;
    case GLS_MESSAGE_HOST_NEW_GAME:
    case GLS_MESSAGE_HOST_NEW_RANDOM_GAME:
    case GLS_MESSAGE_ADVERTISE_GAME:
    case GLS_MESSAGE_UNADVERTISE_GAME:
    case GLS_MESSAGE_UNHOST_GAME:
    case GLS_MESSAGE_REQUEST_GAME_LIST:
      //faulty server
      break;
    }
  return true;
}

void GamelistClient::disconnect()
{
  d_connected = false;
}
