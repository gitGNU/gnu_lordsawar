// Copyright (C) 2008 Ben Asselstine
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
#include <sstream>
#include <list>

#include "pbm-game-server.h"

#include "game.h"
#include "xmlhelper.h"
#include "GameScenario.h"
#include "playerlist.h"
#include "player.h"
#include "network-action.h"
#include "network-history.h"
#include "Configuration.h"


PbmGameServer::PbmGameServer()
{
}

PbmGameServer::~PbmGameServer()
{
  clearNetworkActionlist();
  clearNetworkHistorylist();
}

void PbmGameServer::start()
{
  listenForActions();
  listenForHistories();
}

void PbmGameServer::listenForActions()
{
  Playerlist *pl = Playerlist::getInstance();
  for (Playerlist::iterator i = pl->begin(); i != pl->end(); ++i)
    (*i)->acting.connect(sigc::mem_fun(this, &PbmGameServer::onActionDone));
}

void PbmGameServer::listenForHistories()
{
  Playerlist *pl = Playerlist::getInstance();
  for (Playerlist::iterator i = pl->begin(); i != pl->end(); ++i)
    (*i)->history_written.connect(sigc::mem_fun(this, &PbmGameServer::onHistoryDone));
}

void PbmGameServer::clearNetworkActionlist()
{
    for (std::list<NetworkAction*>::iterator it = d_actions.begin();
        it != d_actions.end(); it++)
      delete (*it);
    d_actions.clear();
}

void PbmGameServer::clearNetworkHistorylist()
{
    for (std::list<NetworkHistory*>::iterator it = d_histories.begin();
        it != d_histories.end(); it++)
      delete (*it);
    d_histories.clear();
}

void PbmGameServer::onActionDone(NetworkAction *action)
{
  std::string desc = action->toString();
  std::cerr << "Play By Mail Game Server got " << desc <<"\n";

  d_actions.push_back(action);
}

void PbmGameServer::onHistoryDone(NetworkHistory *history)
{
  std::string desc = history->toString();
  std::cerr << "Play By Mail Game Server got " << desc <<"\n";
  d_histories.push_back(history);
}

bool PbmGameServer::dumpActionsAndHistories(XML_Helper *helper)
{
  for (std::list<NetworkHistory *>::iterator i = d_histories.begin(),
       end = d_histories.end(); i != end; ++i)
    (**i).save(helper);
  for (std::list<NetworkAction *>::iterator i = d_actions.begin(),
       end = d_actions.end(); i != end; ++i)
    (**i).save(helper);
  return true;
}

bool PbmGameServer::endTurn(std::string turnfile, bool &broken)
{
  bool retval = true;
  XML_Helper helper(turnfile, std::ios::out, Configuration::s_zipfiles);
  retval &= helper.openTag("lordsawarturn");
  broken = dumpActionsAndHistories(&helper);
  helper.closeTag();
  helper.close();
  return retval;
}
// End of file
