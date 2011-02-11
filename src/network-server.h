// Copyright (C) 2008 Ole Laursen
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

#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include "config.h"

#include <map>
#include <giomm.h>
#include <giomm/socketservice.h>
#include <glibmm.h>
#include <sigc++/signal.h>
#include <string>

#include "network-common.h"

class NetworkConnection;

class NetworkServer
{
public:
  NetworkServer();
  ~NetworkServer();

  bool isListening();
  void startListening(int port);
  void send(void *conn, MessageType type, const std::string &payload);

  sigc::signal<bool, void *, MessageType, std::string> got_message;
  sigc::signal<void, void *> connection_made;
  sigc::signal<void, void *> connection_lost;
  
  // private callback
  bool gotClientConnection(const Glib::RefPtr<Gio::SocketConnection>& c, const Glib::RefPtr<Glib::Object>& source_object);

private:
  void onConnectionLost(NetworkConnection *conn);
  
  Glib::RefPtr<Gio::SocketService> server;
  std::list<NetworkConnection *> connections;
};

#endif
