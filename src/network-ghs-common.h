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

#ifndef NETWORK_GHS_COMMON_H
#define NETWORK_GHS_COMMON_H

#include "config.h"

enum GhsMessageType 
{
  GHS_MESSAGE_HOST_NEW_GAME = 1,  //sent from client (lw host)
  GHS_MESSAGE_HOST_NEW_RANDOM_GAME = 2, //from client (lw host)
  GHS_MESSAGE_GAME_CREATED = 3, //from server
  GHS_MESSAGE_UNHOST_GAME = 4, //from client (lw host)
  GHS_MESSAGE_GAME_UNHOSTED = 5, //from server
  GHS_MESSAGE_COULD_NOT_HOST_GAME = 6, //from server
  GHS_MESSAGE_COULD_NOT_UNHOST_GAME = 7, //from server
  GHS_MESSAGE_REQUEST_RELOAD = 8, //from client
  GHS_MESSAGE_RELOADED = 9, //from server
  GHS_MESSAGE_COULD_NOT_RELOAD = 10, //from server
  GHS_MESSAGE_GAME_LIST = 11, //from server
  GHS_MESSAGE_COULD_NOT_GET_GAME_LIST = 12, //from server
  GHS_MESSAGE_REQUEST_GAME_LIST = 13, //from client (lw host or client)
};

#endif
