// Copyright (C) 2008 Ole Laursen
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

#include "network-server.h"
#include <iostream>
#include <sigc++/bind.h>
#include <gnet.h>

#include "network-common.h"
#include "network-connection.h"

NetworkServer::NetworkServer()
{
  server = 0;
}

NetworkServer::~NetworkServer()
{
  //for (std::list<NetworkConnection *>::iterator i = connections.begin(),
         //end = connections.end(); i != end; ++i)
    //delete *i;
  
  if (server)
    gnet_server_delete(server);
}

static void
server_helper(GServer* server, GConn* conn, gpointer user_data)
{
  static_cast<NetworkServer *>(user_data)->gotClientConnection(conn);
}

void NetworkServer::startListening(int port)
{
  // listen on all interfaces
  GInetAddr *iface = 0;

  server = gnet_server_new(iface, port, &server_helper, this);
  if (!server)
    ; // FIXME: report error
}

void NetworkServer::send(void *c, MessageType type, const std::string &payload)
{
  NetworkConnection *conn = static_cast<NetworkConnection *>(c);
  conn->send(type, payload);
}


void NetworkServer::gotClientConnection(GConn* c)
{
  if (c) {
    NetworkConnection *conn = new NetworkConnection(c);
    connections.push_back(conn);

    conn->connection_lost.connect(
      sigc::bind(sigc::mem_fun(connection_lost, &sigc::signal<void, void *>::emit), conn));

    conn->connected.connect(
      sigc::bind(sigc::mem_fun(connection_made, &sigc::signal<void, void *>::emit), conn));
    conn->got_message.connect(
      sigc::bind<0>(sigc::mem_fun(got_message, &sigc::signal<void, void *, MessageType, std::string>::emit), conn));

    // bind ourselves too, so we can delete the connection
    conn->connection_lost.connect(
      sigc::bind(sigc::mem_fun(this, &NetworkServer::onConnectionLost), conn));
  }
  else
    ; // FIXME: report error
}

void NetworkServer::onConnectionLost(NetworkConnection *conn)
{
  delete conn;
}
  
bool NetworkServer::isListening()
{
  if (server)
    return true;
  return false;
}
