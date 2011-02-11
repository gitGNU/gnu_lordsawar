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

#include "network-common.h"

void NetworkConnection::read_in_more_message_header()
{
  printf("reading in %d more bytes of header\n", header_left);
  in->fill_async ((sigc::mem_fun(*this, 
                                 &NetworkConnection::on_header_received)),
                 header_left);
}

void NetworkConnection::read_in_more_message_payload()
{
  printf("reading in %d more bytes of payload\n", payload_left);
  in->fill_async(sigc::mem_fun(*this, 
                               &NetworkConnection::on_payload_received),
                 payload_left);
}

void NetworkConnection::setup_connection()
{
  g_tcp_connection_set_graceful_disconnect((GTcpConnection*)conn->gobj(), true);
  in = Gio::DataInputStream::create(conn->get_input_stream());
  out = Gio::DataOutputStream::create(conn->get_output_stream());
}

NetworkConnection::NetworkConnection(const Glib::RefPtr<Gio::SocketConnection> &c)
{
  printf("okay, i've been asked to create a SERVER side network connection.\n");
  client = Gio::SocketClient::create();
  client->set_protocol(Gio::SOCKET_PROTOCOL_TCP);
  if (c) 
    {
      printf("here1\n");
      conn = Gio::SocketConnection::create(c->property_socket());
      printf("here2\n");
      setup_connection();
      printf("here3\n");
      header_size = MESSAGE_SIZE_BYTES;
      printf("here4\n");
      header_left = header_size;
      printf("here5 %p\n", header);
      read_in_more_message_header();
    }
}

NetworkConnection::NetworkConnection()
{
  client = Gio::SocketClient::create();
  client->set_protocol(Gio::SOCKET_PROTOCOL_TCP);
}

NetworkConnection::~NetworkConnection()
{
}

void NetworkConnection::on_connect_connected(Glib::RefPtr<Gio::AsyncResult> &result)
{
  try
    {
      conn = client->connect_to_host_finish (result);
    }
  catch(const Glib::Exception &ex)
    {
      return;
    }
  if (conn)
    {
      setup_connection();
      connected.emit();
    }
}

void NetworkConnection::on_header_received(Glib::RefPtr<Gio::AsyncResult> &result)
{
  gssize len = -1;
  try
    {
      len = in->fill_finish(result);
    }
  catch(const Glib::Exception &ex)
    {
      connection_lost();
      return;
    }

  printf("got %d bytes\n", len);
  if (len <= 0)
    {
      connection_lost();
      return;
    }

  in->read(header + (header_size - header_left), len);
  header_left -= len;
  if (header_left)
    printf("still need %d more bytes of header\n", header_left);
  if (header_left > 0)
    return read_in_more_message_header();

  guint32 val;
  memcpy(&val, header, header_size);
  payload_size = g_ntohl(val);
  payload_left = payload_size;
  payload = (char *) malloc (payload_size);
  read_in_more_message_payload();
}

void NetworkConnection::on_payload_received(Glib::RefPtr<Gio::AsyncResult> &result)
{
  gssize len = -1;
  try
    {
      len = in->fill_finish(result);
    }
  catch(const Glib::Exception &ex)
    {
      connection_lost();
      return;
    }

  printf("got %d bytes\n", len);
  if (len <= 0)
    {
      connection_lost();
      return;
    }
  in->read(payload + (payload_size - payload_left), len);
  payload_left -= len;
  if (payload_left)
    printf("still need %d more bytes of payload\n", payload_left);

  connection_received_data.emit();
  if (payload_left > 0)
    return read_in_more_message_payload();

  MessageType type = MessageType(payload[1]);
  got_message.emit(type, 
                   std::string(payload + MESSAGE_PREAMBLE_EXTRA_BYTES,
                               payload_size - MESSAGE_PREAMBLE_EXTRA_BYTES));
  free (payload);
  payload = NULL;
}


void NetworkConnection::connectToHost(std::string host, int port)
{
  client->connect_to_host_async 
    (host, port, sigc::mem_fun(*this, 
                               &NetworkConnection::on_connect_connected));
}

void NetworkConnection::sendFile(MessageType type, const std::string filename)
{
  if (conn->has_pending())
    {
      printf("we have pending!!!\n");
    }
  //if (conn->is_closed())
    //return;
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
  
  printf("writing out %d bytes\n", sizeof (buf));
  gsize bytessent = 0;
  bool wrote_all =  out->write_all ((const void*) buf, sizeof (buf), bytessent);
  printf("written %d\n", wrote_all ? bytessent: -1);

  //std::cerr << "sending file " << type <<" of length " << MESSAGE_PREAMBLE_EXTRA_BYTES + statbuf.st_size << " to " << gnet_inetaddr_get_name(conn->inetaddr) << std::endl;
  char *buffer = (char*) malloc (statbuf.st_size);
  ssize_t bytesread = fread (buffer, 1, statbuf.st_size, fileptr);
  fclose (fileptr);
  printf("writing out %d bytes\n", bytesread);
  wrote_all = out->write_all (buffer, bytesread, bytessent);
  printf("written %d\n", wrote_all ? bytessent : -1);
  free (buffer);
  //header_size = MESSAGE_SIZE_BYTES;
  //header_left = header_size;
  //read_in_more_message_header();
}

void NetworkConnection::send(MessageType type, const std::string &payload)
{
  if (conn->has_pending())
    {
      printf("we have pending!!!\n");
    }
  printf("Sending message %d\n", type);
  //if (conn->is_closed())
    //return;
  // write the preamble
  gchar buf[MESSAGE_HEADER_SIZE];
  guint32 l = g_htonl(MESSAGE_PREAMBLE_EXTRA_BYTES + payload.size());
  memcpy(buf, &l, MESSAGE_SIZE_BYTES);
  buf[MESSAGE_SIZE_BYTES] = MESSAGE_PROTOCOL_VERSION;
  buf[MESSAGE_SIZE_BYTES + 1] = type;
  
  printf("writing out size bytes now\n");
  gsize bytessent = 0;
  bool wrote_all = out->write_all (buf, sizeof (buf), bytessent);
  printf("written %d\n", wrote_all ? bytessent : -1);

  //std::cerr << "sending message " << type <<" of length " << MESSAGE_PREAMBLE_EXTRA_BYTES + payload.size() << " to " << gnet_inetaddr_get_name(conn->inetaddr) << std::endl;

  // write the payload
  printf("writing out payload now\n");
  wrote_all = out->write_all (payload.c_str(), payload.size(), bytessent);
  printf("written %d\n", wrote_all ? bytessent : -1);
  printf("done\n");
  header_size = MESSAGE_SIZE_BYTES;
  header_left = header_size;
  read_in_more_message_header();
}
