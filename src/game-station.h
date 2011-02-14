// Copyright (C) 2008, 2011 Ben Asselstine
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
  sigc::signal<void, Player*> local_player_moved;
  sigc::signal<void, Player*> local_player_died;
  sigc::signal<void, Player*> local_player_starts_move;
  sigc::signal<void, Player*, int> player_changes_type;
  sigc::signal<void, Player*, Glib::ustring> player_changes_name;
  sigc::signal<void, Glib::ustring, Glib::ustring> nickname_changed;
  sigc::signal<void, Player*> player_gets_turned_off;
  sigc::signal<void> round_begins;
  sigc::signal<void> game_may_begin;

  void listenForLocalEvents(Player *p);
protected:
  GameStation();
  virtual ~GameStation();

  virtual void onActionDone(NetworkAction *action) = 0;
  virtual void onHistoryDone(NetworkHistory *history) = 0;

  void clearNetworkActionlist(std::list<NetworkAction*> &actions);
  void clearNetworkHistorylist(std::list<NetworkHistory*> &histories);

  void stopListeningForLocalEvents(Player *p);

  static bool get_message_lobby_activity (std::string payload, 
                                          guint32 &player_id, 
                                          gint32 &action, bool &reported,
                                          Glib::ustring &nickname);

private:
  std::map<guint32, sigc::connection> action_listeners;
  std::map<guint32, sigc::connection> history_listeners;
};

#endif
