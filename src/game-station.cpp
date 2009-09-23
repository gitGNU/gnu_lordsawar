// Copyright (C) 2008 Ben Asselstine
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

#include "game-station.h"

#include "network-action.h"
#include "network-history.h"


GameStation::GameStation()
{
}

GameStation::~GameStation()
{
}

void GameStation::clearNetworkActionlist(std::list<NetworkAction*> &a)
{
  for (std::list<NetworkAction*>::iterator it = a.begin();
       it != a.end(); it++)
    {
      delete (*it);
    }
  a.clear();
}

void GameStation::clearNetworkHistorylist(std::list<NetworkHistory*> &h)
{
  for (std::list<NetworkHistory*>::iterator it = h.begin(); 
       it != h.end(); it++)
    {
      delete (*it);
    }
  h.clear();
}

void GameStation::listenForLocalEvents(Player *p)
{
  sigc::connection connection;
  connection = p->acting.connect(sigc::mem_fun(this, 
					       &GameStation::onActionDone));
  action_listeners[p->getId()] = connection;
  connection = p->history_written.connect
    (sigc::mem_fun(this, &GameStation::onHistoryDone));
  history_listeners[p->getId()] = connection;
}

void GameStation::stopListeningForLocalEvents(Player *p)
{
  sigc::connection connection;
  std::map<guint32, sigc::connection>::iterator it;
  it = action_listeners.find(p->getId());
  if (it != action_listeners.end())
    {
      connection = (*it).second;
      connection.disconnect();
      action_listeners.erase(it);
    }
  it = history_listeners.find(p->getId());
  if (it != history_listeners.end())
    {
      connection = (*it).second;
      connection.disconnect();
      history_listeners.erase(it);
    }
}
// End of file
