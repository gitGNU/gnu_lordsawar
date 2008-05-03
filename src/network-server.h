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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include "config.h"

#include <map>
#include <sigc++/signal.h>
#include <string>

#include "network-common.h"

class _GServer;
class _GConn;
class NetworkConnection;

class NetworkServer
{
public:
  NetworkServer();
  ~NetworkServer();

  void startListening(int port);
  void send(void *conn, MessageType type, const std::string &payload);

  sigc::signal<void, void *, MessageType, std::string> got_message;
  sigc::signal<void, void *> connection_lost;
  
  // private callback
  void gotClientConnection(_GConn* conn);

private:
  void onConnectionLost(NetworkConnection *conn);
  
  _GServer *server;
  std::list<NetworkConnection *> connections;
};

#endif
