// Copyright (C) 2008 Ole Laursen
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

#ifndef NETWORK_CONNECTION_H
#define NETWORK_CONNECTION_H

#include "config.h"

#include <sigc++/signal.h>
#include <glibmm.h>
#include <giomm.h>
#include <string>

#include "network-common.h"

// a simple network connection for sending messages, encapsulates the protocol
class NetworkConnection
{
public:
  NetworkConnection(const Glib::RefPtr<Gio::SocketConnection> &conn);
  NetworkConnection();
  ~NetworkConnection();

  void connectToHost(std::string host, int port);

  sigc::signal<void> connected;
  sigc::signal<void> connection_lost;
  sigc::signal<void> connection_failed;
  sigc::signal<void> connection_received_data;
  sigc::signal<bool, int, std::string> got_message;

  void send(int type, const std::string &payload);
  void sendFile(int type, std::string filename);
  std::string get_peer_hostname();

  std::string getHost() const {return d_host;};
  guint32 getPort() const {return d_port;};
private:
  Glib::RefPtr<Gio::SocketClient> client; //this is client-side connections.
  Glib::RefPtr<Gio::SocketConnection> conn;
  Glib::RefPtr<Gio::DataInputStream> in;
  Glib::RefPtr<Gio::DataOutputStream> out;
  Glib::RefPtr<Glib::IOSource> source;
  sigc::connection d_connect_timer;
  char *payload;
  int payload_left;
  int payload_size;
  char header[MESSAGE_SIZE_BYTES];
  int header_left;
  int header_size;
  std::string d_host;
  guint32 d_port;

  void setup_connection();
  void on_connect_connected(Glib::RefPtr<Gio::AsyncResult> &result);
  gssize on_header_received(gssize len);
  gssize on_payload_received(gssize len);
  bool on_got_input(Glib::IOCondition cond);
  void tear_down_connection();
  bool on_connect_timeout();
};

#endif
