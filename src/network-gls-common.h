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

#ifndef NETWORK_GLS_COMMON_H
#define NETWORK_GLS_COMMON_H

#include "config.h"

enum GlsMessageType 
{
  GLS_MESSAGE_HOST_NEW_GAME = 1,  //sent from client (lw host)
  GLS_MESSAGE_HOST_NEW_RANDOM_GAME = 2, //from client (lw host)
  GLS_MESSAGE_GAME_CREATED = 3, //from server
  GLS_MESSAGE_ADVERTISE_GAME = 4, //from client (lw host or client)
  GLS_MESSAGE_UNADVERTISE_GAME = 5, //from client (lw host or client)
  GLS_MESSAGE_UNHOST_GAME = 6, //from client (lw host)
  GLS_MESSAGE_REQUEST_GAME_LIST = 7, //from client (lw host or client)
  GLS_MESSAGE_GAME_LIST = 8, //from server
  GLS_MESSAGE_GAME_UNHOSTED = 9, //from server
  GLS_MESSAGE_COULD_NOT_HOST_GAME = 10, //from server
  GLS_MESSAGE_COULD_NOT_UNHOST_GAME = 11, //from server
  GLS_MESSAGE_COULD_NOT_ADVERTISE_GAME = 12, //from server
  GLS_MESSAGE_COULD_NOT_UNADVERTISE_GAME = 13, //from server
  GLS_MESSAGE_GAME_ADVERTISED = 14, //from server
  GLS_MESSAGE_GAME_UNADVERTISED = 15, //from server
  GLS_MESSAGE_COULD_NOT_GET_GAME_LIST = 16, //from server
  GLS_MESSAGE_REQUEST_RELOAD = 17, //from client
  GLS_MESSAGE_RELOADED = 18, //from server
  GLS_MESSAGE_COULD_NOT_RELOAD = 19, //from server
};

#endif
