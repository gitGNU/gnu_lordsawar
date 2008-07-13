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



GameClient::GameClient()
{
}

GameClient::~GameClient()
{
}

void GameClient::start(std::string host, int port)
{
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

  network_connection->send(MESSAGE_TYPE_PING, "");
}

void GameClient::onConnectionLost()
{
  std::cerr << "connection lost" << std::endl;
}

void GameClient::onGotMessage(MessageType type, std::string payload)
{
  std::cerr << "got message of type " << type << std::endl;
  switch (type) {
  case MESSAGE_TYPE_PING:
    network_connection->send(MESSAGE_TYPE_PONG, "PONGOGOGO");
    break;

  case MESSAGE_TYPE_PONG:
    std::cerr << "sending join" << std::endl;
    network_connection->send(MESSAGE_TYPE_JOIN, "I wanna join");
    break;

  case MESSAGE_TYPE_SENDING_ACTIONS:
    gotActions(payload);
    break;
    
  case MESSAGE_TYPE_SENDING_MAP:
    gotScenario(payload);
    break;

  case MESSAGE_TYPE_JOIN:
    // FIXME: faulty server
    break;

  case MESSAGE_TYPE_SENDING_HISTORY:
    gotHistory(payload);
    break;
    
  }
}

void GameClient::gotScenario(const std::string &payload)
{
  std::string file = "network.sav";
  std::string path = File::getSavePath();
  path += file;
  
  std::ofstream f(path.c_str());
  f << payload;
  f.close();
  
  game_scenario_received.emit(path);
}

class ActionLoader 
{
public:
  bool loadAction(std::string tag, XML_Helper* helper)
  {
    if (tag == "networkaction") 
      {
	NetworkAction * action = new NetworkAction(helper);
	actions.push_back(action);
      }
    if (tag == "action")
      {
	NetworkAction *action = &*actions.back();
	action->setAction(Action::handle_load(helper));
      }
    return true;
  }

  std::list<NetworkAction *> actions;
};
  
void GameClient::gotActions(const std::string &payload)
{
  std::istringstream is(payload);

  ActionLoader loader;
  
  XML_Helper helper(&is);
  helper.registerTag("networkaction", sigc::mem_fun(loader, &ActionLoader::loadAction));
  helper.registerTag("action", sigc::mem_fun(loader, &ActionLoader::loadAction));
  helper.parse();

  for (std::list<NetworkAction *>::iterator i = loader.actions.begin(),
       end = loader.actions.end(); i != end; ++i)
  {
    NetworkAction *action = *i;
    std::cerr << "decoding action " << action->getAction()->getType() << std::endl;
    
    Player *p = action->getOwner();
    NetworkPlayer *np = dynamic_cast<NetworkPlayer *>(p);

    if (!np) {
      std::cerr << "warning: ignoring action for player " << p << std::endl;
      continue;
    }

    np->decodeAction(action->getAction());
  }

  for (std::list<NetworkAction *>::iterator i = loader.actions.begin(),
       end = loader.actions.end(); i != end; ++i)
    delete *i;
}

class HistoryLoader 
{
public:
  bool loadHistory(std::string tag, XML_Helper* helper)
  {
    if (tag == "networkhistory") 
      {
	NetworkHistory* history = new NetworkHistory(helper);
	histories.push_back(history);
      }
    if (tag == "history")
      {
	NetworkHistory *history = &*histories.back();
	history->setHistory(History::handle_load(helper));
      }
    return true;
  }

  std::list<NetworkHistory *> histories;
};
  

void GameClient::gotHistory(const std::string &payload)
{
  std::istringstream is(payload);

  HistoryLoader loader;
  
  XML_Helper helper(&is);
  helper.registerTag("history", sigc::mem_fun(loader, &HistoryLoader::loadHistory));
  helper.parse();

  for (std::list<NetworkHistory *>::iterator i = loader.histories.begin(),
       end = loader.histories.end(); i != end; ++i)
  {
    NetworkHistory *history = *i;
    std::cerr << "received history " << history->getHistory()->getType() << std::endl;
    
    //just add it to the player's history list.
    Player *p = Playerlist::getInstance()->getActiveplayer();
    p->getHistorylist()->push_back(history->getHistory());
  }

  for (std::list<NetworkHistory *>::iterator i = loader.histories.begin(),
       end = loader.histories.end(); i != end; ++i)
    delete *i;
}
