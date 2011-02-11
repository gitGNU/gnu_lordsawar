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

#include <stdio.h> //benfix
#include "network-server.h"
#include <iostream>
#include <sigc++/bind.h>
#include <giomm.h>

#include "network-common.h"
#include "network-connection.h"

NetworkServer::NetworkServer()
{
  connections.clear();
}

NetworkServer::~NetworkServer()
{
  //for (std::list<NetworkConnection *>::iterator i = connections.begin(),
         //end = connections.end(); i != end; ++i)
    //delete *i;
  
  if (server)
    {
      server->stop();
    }
}

void NetworkServer::startListening(int port)
{
  server = Gio::SocketService::create();
  server->add_inet_port (port);

  server->signal_incoming().connect(sigc::mem_fun(*this, &NetworkServer::gotClientConnection));
  server->start();
}

void NetworkServer::send(void *c, MessageType type, const std::string &payload)
{
  NetworkConnection *conn = static_cast<NetworkConnection *>(c);
  if (type == MESSAGE_TYPE_SENDING_MAP)
    conn->sendFile(type, payload);
  else
    conn->send(type, payload);
}

bool NetworkServer::gotClientConnection(const Glib::RefPtr<Gio::SocketConnection>& c, const Glib::RefPtr<Glib::Object>& source_object)
{
  if (c) 
    {
      NetworkConnection *conn = new NetworkConnection(c);
      connections.push_back(conn);

      conn->connection_lost.connect
        (sigc::bind(sigc::mem_fun
                    (connection_lost, 
                     &sigc::signal<void, void *>::emit), conn));

      conn->connected.connect
        (sigc::bind(sigc::mem_fun(connection_made, 
                                  &sigc::signal<void, void *>::emit), conn));
      
      conn->got_message.connect
        (sigc::bind<0>(sigc::mem_fun(got_message, 
                                     &sigc::signal<void, void *, 
                                     MessageType, std::string>::emit), conn));

      // bind ourselves too, so we can delete the connection
      conn->connection_lost.connect
        (sigc::bind(sigc::mem_fun(this, 
                                  &NetworkServer::onConnectionLost), conn));
      return true;
    }
  return false;
}

void NetworkServer::onConnectionLost(NetworkConnection *conn)
{
  if (std::find (connections.begin(), connections.end(), conn) != connections.end())
    connections.remove(conn);
  delete conn;
}
  
bool NetworkServer::isListening()
{
  return server->is_active();
}
