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

#include "network-connection.h"
#include <iostream>
#include <cstring>
#include <gnet.h>

#include "network-common.h"

static void
event_helper(GConn* conn, GConnEvent* event, gpointer user_data) 
{
  static_cast<NetworkConnection *>(user_data)->gotConnectionEvent(conn, event);
}


NetworkConnection::NetworkConnection(_GConn *conn)
{
  this->conn = conn;
  if (conn) {
    gnet_conn_set_callback(conn, &event_helper, this);
    receiving_message = false;
    gnet_conn_readn(conn, MESSAGE_SIZE_BYTES);
  }
}

NetworkConnection::~NetworkConnection()
{
  if (conn)
    {
      gnet_conn_delete(conn);
      conn = 0;
    }
}

void NetworkConnection::connectToHost(std::string host, int port)
{
  conn = gnet_conn_new(host.c_str(), port, &event_helper, this);
  if (!conn)
    exit(1);
    ; // FIXME: report error

  gnet_conn_connect(conn);
  gnet_conn_set_watch_error(conn, true);
  gnet_conn_timeout(conn, 30 * 1000);
}

void NetworkConnection::send(MessageType type, const std::string &payload)
{
  if (gnet_conn_is_connected(conn) == false)
    return;
  // write the preamble
  gchar buf[MESSAGE_SIZE_BYTES + MESSAGE_PREAMBLE_EXTRA_BYTES];
  guint32 l = g_htonl(MESSAGE_PREAMBLE_EXTRA_BYTES + payload.size());
  memcpy(buf, &l, MESSAGE_SIZE_BYTES);
  buf[MESSAGE_SIZE_BYTES] = MESSAGE_PROTOCOL_VERSION;
  buf[MESSAGE_SIZE_BYTES + 1] = type;
  
  gnet_conn_write(conn, buf, MESSAGE_SIZE_BYTES + MESSAGE_PREAMBLE_EXTRA_BYTES);

  std::cerr << "sending length " << MESSAGE_PREAMBLE_EXTRA_BYTES + payload.size() << " to " << gnet_inetaddr_get_name(conn->inetaddr) << std::endl;

  // write the payload
  if (!payload.empty())
    gnet_conn_write(conn, const_cast<gchar *>(payload.data()), payload.size());
}

void NetworkConnection::gotConnectionEvent(GConn* conn, GConnEvent* event)
{
  switch (event->type)
  {
  case GNET_CONN_CONNECT:
    gnet_conn_timeout(conn, 0); // stop timeout
    receiving_message = false;
    gnet_conn_readn(conn, MESSAGE_SIZE_BYTES);
    connected.emit();
    break;
    
  case GNET_CONN_READ:
    if (receiving_message)
    {
      if (event->length < 2)
      {
	printf ("misbehaving server!\n");
        // misbehaving server
        gnet_conn_disconnect(conn);
        break;
      }

      // protocol version is ignored for now
      //int protocol_version = event->buffer[0];
      MessageType type = MessageType(event->buffer[1]);
      
      got_message.emit(type, std::string(event->buffer + MESSAGE_PREAMBLE_EXTRA_BYTES,
                                         event->length - MESSAGE_PREAMBLE_EXTRA_BYTES));
      receiving_message = false;
      gnet_conn_readn(conn, MESSAGE_SIZE_BYTES);
    }
    else
    {
      g_assert(event->length == MESSAGE_SIZE_BYTES);
      guint32 val;
      memcpy(&val, event->buffer, 4);
      int message_size = g_ntohl(val);
      
      std::cerr << "going to read length " << message_size << std::endl;
      receiving_message = true;
      gnet_conn_readn(conn, message_size);
      connection_received_data.emit();
    }
    break;

  case GNET_CONN_WRITE:
    break;

  case GNET_CONN_CLOSE:
  case GNET_CONN_TIMEOUT:
  case GNET_CONN_ERROR:
    connection_lost.emit();
    gnet_conn_delete(conn);
    break;

  default:
    g_assert_not_reached ();
  }
}
