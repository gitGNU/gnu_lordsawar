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

#ifndef GAME_STATION_H
#define GAME_STATION_H

#include "config.h"

#include <memory>
#include <list>
#include <map>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <sigc++/connection.h>

#include "game-client-decoder.h"

class GameStation: public GameClientDecoder 
{
public:
        
  sigc::signal<void, std::string> remote_participant_joins;
  sigc::signal<void, Player*, std::string> player_sits;
  sigc::signal<void, Player*, std::string> player_stands;
  sigc::signal<void, std::string> remote_participant_departs;
  sigc::signal<void> playerlist_reorder_received;

protected:
  GameStation();
  ~GameStation();

  virtual void onActionDone(NetworkAction *action) = 0;
  virtual void onHistoryDone(NetworkHistory *history) = 0;

  void clearNetworkActionlist(std::list<NetworkAction*> &actions);
  void clearNetworkHistorylist(std::list<NetworkHistory*> &histories);

  void listenForLocalEvents(Player *p);
  void stopListeningForLocalEvents(Player *p);

private:
  std::map<Uint32, sigc::connection> action_listeners;
  std::map<Uint32, sigc::connection> history_listeners;
};

#endif