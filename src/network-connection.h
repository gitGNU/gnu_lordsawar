// Copyright (C) 2008 Ole Laursen
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

#ifndef NETWORK_CONNECTION_H
#define NETWORK_CONNECTION_H

#include "config.h"

#include <sigc++/signal.h>
#include <string>

#include "network-common.h"

class _GConn;
class _GConnEvent;

// a simple network connection for sending messages, encapsulates the protocol
class NetworkConnection
{
public:
  NetworkConnection(_GConn *conn = 0);
  ~NetworkConnection();

  void connectToHost(std::string host, int port);

  sigc::signal<void> connected;
  sigc::signal<void> connection_lost;
  sigc::signal<void> connection_received_data;
  sigc::signal<void, MessageType, std::string> got_message;

  void send(MessageType type, const std::string &payload);

  
  // private callback
  void gotConnectionEvent(_GConn* conn, _GConnEvent* event);

private:
  _GConn *conn;
  bool receiving_message;
};

#endif
