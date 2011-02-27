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
#include "recently-played-game-list.h"
#include "recently-played-game.h"
#include "hosted-game.h"
#include "advertised-game.h"

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

void GamelistServer::sendList(void *conn)
{
  std::ostringstream os;
  XML_Helper helper(&os);
  helper.begin("1");
  RecentlyPlayedGameList *l = Gamelist::getInstance()->getList();
  l->save(&helper);
  network_server->send(conn, GLS_MESSAGE_GAME_LIST, os.str());
  delete l;
}

void GamelistServer::unadvertise(std::string profile_id, std::string scenario_id, std::string &err)
{
  HostedGame *g = Gamelist::getInstance()->findGameByScenarioId(scenario_id);
  if (!g)
    {
      err = _("no such game with that scenario id");
      return;
    }
  if (g->getAdvertisedGame()->getProfileId() != profile_id)
    {
      err = _("permission denied");
      return;
    }
  Gamelist::getInstance()->remove(g);
  Gamelist::getInstance()->save();
  delete g;
  return;
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
        {
          std::istringstream is(payload);
          XML_Helper helper(&is);
          helper.registerTag
            (RecentlyPlayedGameList::d_tag, 
             sigc::bind(sigc::mem_fun(*this, &GamelistServer::loadAdvertisedGame), conn));
          helper.parse();
        }
      break;
    case GLS_MESSAGE_UNADVERTISE_GAME:
        {
          size_t pos;
          std::string err;
          pos = payload.find(' ');
          if (pos == std::string::npos)
            return false;
          unadvertise(payload.substr(0, pos - 1), payload.substr(pos + 1),
                      err);
          network_server->send(conn, GLS_MESSAGE_GAME_UNADVERTISED, 
                               payload.substr(pos + 1) + " " + err);
        }
      break;
    case GLS_MESSAGE_UNHOST_GAME:
      break;
    case GLS_MESSAGE_REQUEST_GAME_LIST:
      sendList(conn);
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
             
bool GamelistServer::loadAdvertisedGame(std::string tag, XML_Helper *helper, void *conn)
{
  if (tag == AdvertisedGame::d_tag)
    {
      AdvertisedGame *a = new AdvertisedGame(helper);
      //printf("profile is %p\n", a->getProfile());
      //HostedGame* g = new HostedGame(a);
      HostedGame *h = Gamelist::getInstance()->findGameByScenarioId(a->getId());
      if (h)
        {
          //replace?
          if (a->getProfileId() != h->getAdvertisedGame()->getProfileId())
            {
              network_server->send(conn, GLS_MESSAGE_GAME_ADVERTISED,
                                   a->getId() + " " + _("permission denied"));
              return true;
            }
          std::replace(Gamelist::getInstance()->begin(), 
                       Gamelist::getInstance()->end(), h, new HostedGame(a));
          Gamelist::getInstance()->save();
              
          network_server->send(conn, GLS_MESSAGE_GAME_ADVERTISED,
                               a->getId() + " "); //no error
          return true;
        }
      else
        {
          bool success = Gamelist::getInstance()->add(new HostedGame(a));
          Gamelist::getInstance()->save();
          if (!success)
              network_server->send(conn, GLS_MESSAGE_GAME_ADVERTISED,
                                   a->getId() + " " + 
                                   _("could not advertised game"));
          else
              network_server->send(conn, GLS_MESSAGE_GAME_ADVERTISED,
                                   a->getId() + " "); //no error

        }
      return true;
    }
  return false;
}
// End of file
