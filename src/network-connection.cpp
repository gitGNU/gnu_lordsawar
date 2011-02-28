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

#include "network-connection.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <giomm.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "timing.h"

#include "network-common.h"

void NetworkConnection::setup_connection()
{
  g_tcp_connection_set_graceful_disconnect((GTcpConnection*)conn->gobj(), true);
  in = Gio::DataInputStream::create(conn->get_input_stream());
  out = Gio::DataOutputStream::create(conn->get_output_stream());
  source = 
    Glib::IOSource::create 
    (conn->property_socket().get_value()->property_fd(), 
     Glib::IO_IN | Glib::IO_PRI | Glib::IO_ERR);
  source->connect(sigc::mem_fun(*this, &NetworkConnection::on_got_input));
  header_size = MESSAGE_SIZE_BYTES;
  header_left = header_size;
  source->attach(Glib::MainContext::get_default());
}

void NetworkConnection::tear_down_connection()
{
  if (conn)
    conn->clear_pending();
  if (source)
    source->destroy();
  if (conn)
    {
    if (conn->is_closed() == false)
      {
        conn->close();
        //conn.reset();
      }
    }
}

bool NetworkConnection::on_got_input(Glib::IOCondition cond)
{
  gssize len = -1;
  switch (cond)
    {
    case Glib::IO_IN:
    case Glib::IO_PRI:
      if (header_left > 0)
        len = on_header_received(in->fill (header_left));
      else if (payload_left > 0)
        len = on_payload_received(in->fill(payload_left));
      //break;  fallthrough here on purpose.
    case Glib::IO_ERR:
    case Glib::IO_HUP:
      if (len <= 0)
        {
          tear_down_connection();
          connection_lost.emit();
          return false;
        }
      break;
    default:
      return false;
    }
  return true;
}

NetworkConnection::NetworkConnection(const Glib::RefPtr<Gio::SocketConnection> &c)
{
  //okay, i've been asked to create a SERVER side network connection.
  client = Gio::SocketClient::create();
  client->set_protocol(Gio::SOCKET_PROTOCOL_TCP);
  if (c) 
    {
      conn = Gio::SocketConnection::create(c->property_socket());
      setup_connection();
    }
}

NetworkConnection::NetworkConnection()
{
  client = Gio::SocketClient::create();
  client->set_protocol(Gio::SOCKET_PROTOCOL_TCP);
}

NetworkConnection::~NetworkConnection()
{
  tear_down_connection();
}

void NetworkConnection::on_connect_connected(Glib::RefPtr<Gio::AsyncResult> &result)
{
  d_connect_timer.disconnect();
  try
    {
      conn = client->connect_to_host_finish (result);
    }
  catch(const Glib::Exception &ex)
    {
      connection_failed.emit();
      return;
    }
  if (conn)
    {
      //okay, i've been asked to create a CLIENT side network connection.
      setup_connection();
      connected.emit();
    }
}

gssize NetworkConnection::on_header_received(gssize len)
{
  if (len <= 0)
    return len;

  in->read(header + (header_size - header_left), len);
  header_left -= len;
  if (header_left > 0)
    return len;

  guint32 val;
  memcpy(&val, header, header_size);
  payload_size = g_ntohl(val);
  payload_left = payload_size;
  payload = (char *) malloc (payload_size);
  return len;
}

gssize NetworkConnection::on_payload_received(gssize len)
{
  if (len <= 0)
    return len;
  in->read(payload + (payload_size - payload_left), len);
  payload_left -= len;

  connection_received_data.emit();
  if (payload_left > 0)
    return len;

  int type = payload[1];
  bool keep_going = got_message.emit
    (type, std::string(payload + MESSAGE_PREAMBLE_EXTRA_BYTES,
                       payload_size - MESSAGE_PREAMBLE_EXTRA_BYTES));
  free (payload);
  payload = NULL;
  if (keep_going == false)
    {
      //time to go away
      return -1;
    }
  header_left = header_size; //set things up for the next header.
  return len;
}


void NetworkConnection::connectToHost(std::string host, int port)
{
  d_connect_timer = 
    Timing::instance().register_timer
    (sigc::mem_fun(this, &NetworkConnection::on_connect_timeout), 5000);
  client->connect_to_host_async 
    (host, port, sigc::mem_fun(*this, 
                               &NetworkConnection::on_connect_connected));
}

void NetworkConnection::sendFile(int type, const std::string filename)
{
  FILE *fileptr = fopen (filename.c_str(), "r");
  if (fileptr == NULL)
    return;

  struct stat statbuf;
  stat (filename.c_str(), &statbuf);
  // write the preamble
  gchar buf[MESSAGE_HEADER_SIZE];
  guint32 l = g_htonl(MESSAGE_PREAMBLE_EXTRA_BYTES + statbuf.st_size);
  memcpy(buf, &l, MESSAGE_SIZE_BYTES);
  buf[MESSAGE_SIZE_BYTES] = MESSAGE_PROTOCOL_VERSION;
  buf[MESSAGE_SIZE_BYTES + 1] = type;
  
  gsize bytessent = 0;
  bool wrote_all =  out->write_all ((const void*) buf, sizeof (buf), bytessent);

  char *buffer = (char*) malloc (statbuf.st_size);
  ssize_t bytesread = fread (buffer, 1, statbuf.st_size, fileptr);
  fclose (fileptr);
  wrote_all = out->write_all (buffer, bytesread, bytessent);
  free (buffer);
}

void NetworkConnection::send(int type, const std::string &payload)
{
  // write the preamble
  gchar buf[MESSAGE_HEADER_SIZE];
  guint32 l = g_htonl(MESSAGE_PREAMBLE_EXTRA_BYTES + payload.size());
  memcpy(buf, &l, MESSAGE_SIZE_BYTES);
  buf[MESSAGE_SIZE_BYTES] = MESSAGE_PROTOCOL_VERSION;
  buf[MESSAGE_SIZE_BYTES + 1] = type;
  
  gsize bytessent = 0;
  bool wrote_all = out->write_all (buf, sizeof (buf), bytessent);

  // write the payload
  wrote_all = out->write_all (payload.c_str(), payload.size(), bytessent);
}
  
bool NetworkConnection::on_connect_timeout()
{
  d_connect_timer.disconnect();
  connection_failed.emit();
  return Timing::STOP;
}

std::string NetworkConnection::get_peer_hostname()
{
  Glib::RefPtr<Gio::InetSocketAddress> iconn = Glib::wrap((GInetSocketAddress*)(conn->get_remote_address()->gobj()), true);
  Glib::ustring h = iconn->get_address()->to_string();
  size_t pos = h.rfind(':');
  if (pos == Glib::ustring::npos)
    return h;
  else
    return h.substr(pos + 1);
}
