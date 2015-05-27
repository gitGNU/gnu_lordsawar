// Copyright (C) 2008 Ole Laursen
// Copyright (C) 2011, 2014 Ben Asselstine
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

#include <config.h>

#include <queue>
#include <sigc++/signal.h>
#include <glibmm.h>
#include <giomm.h>
#include <gtkmm.h>
#include <glibmm/threads.h>
#include "network-common.h"

//! A simple network connection for sending messages, encapsulates the protocol
class NetworkConnection
{
public:

  NetworkConnection(const Glib::RefPtr<Gio::SocketConnection> &conn);
  NetworkConnection();
  ~NetworkConnection();

  void connectToHost(Glib::ustring host, int port);

  sigc::signal<void> connected;
  sigc::signal<void> connection_lost;
  sigc::signal<void> connection_failed;
  sigc::signal<void> connection_received_data;
  sigc::signal<bool, int, Glib::ustring> got_message;
  sigc::signal<void> queue_flushed;
  sigc::signal<void> torn_down;

  void send(int type, const Glib::ustring &payload);
  void sendFile(int type, const Glib::ustring &filename);
  Glib::ustring get_peer_hostname();

  void tear_down_connection();
  void disconnect();
  Glib::ustring getHost() const {return d_host;};
  guint32 getPort() const {return d_port;};

  //Glib::Threads::Thread * consumer;
  void send_queued_messages();

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
  Glib::ustring d_host;
  guint32 d_port;

  Glib::Threads::Mutex mutex;
  Glib::Threads::Cond cond_push;
  Glib::Threads::Cond cond_pop;

  bool d_stop;

  struct Message
    {
      int type;
      Glib::ustring payload;
    };
  std::queue<struct Message> messages;

  void setup_connection();
  void on_connect_connected(Glib::RefPtr<Gio::AsyncResult> &result);
  gssize on_header_received(gssize len);
  gssize on_payload_received(gssize len);
  bool on_got_input(Glib::IOCondition cond);
  bool on_connect_timeout();

  void queue_message(int type, const Glib::ustring &payload);
  bool sendMessage(int type, const Glib::ustring &payload);
  void sendFileMessage(int type, Glib::ustring filename);

};

#endif
