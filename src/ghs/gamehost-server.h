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

#ifndef GAMEHOST_SERVER_H
#define GAMEHOST_SERVER_H

#include "config.h"

#include <memory>
#include <list>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>

#include "timing.h"
#include "network-ghs-common.h"

class NetworkServer;
class XML_Helper;

class GamehostServer
{
public:
        
  //! Returns the singleton instance.  Creates a new one if neccessary.
  static GamehostServer * getInstance();

  //! Deletes the singleton instance.
  static void deleteInstance();

  bool isListening();
  void start(int port);

  void reload();

  sigc::signal<void, int> port_in_use;

protected:
  GamehostServer();
  ~GamehostServer();

private:
  std::auto_ptr<NetworkServer> network_server;

  bool onGotMessage(void *conn, int type, std::string message);
  void onConnectionLost(void *conn);
  void onConnectionMade(void *conn);
  sigc::connection on_timer_registered(Timing::timer_slot s, int msecs_interval);

  void sendList(void *conn);

  //! A static pointer for the singleton instance.
  static GamehostServer * s_instance;
};

#endif
