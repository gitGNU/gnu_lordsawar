// Copyright (C) 2008 Ole Laursen
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

#ifndef NETWORK_COMMON_H
#define NETWORK_COMMON_H

#include "config.h"


// the network protocol

#define MESSAGE_SIZE_BYTES 4
#define MESSAGE_PREAMBLE_EXTRA_BYTES 2
#define MESSAGE_PROTOCOL_VERSION 1

enum MessageType {
  MESSAGE_TYPE_PING = 1,
  MESSAGE_TYPE_PONG = 2,
  MESSAGE_TYPE_SENDING_MAP = 3,
  MESSAGE_TYPE_SENDING_ACTIONS = 4,
  MESSAGE_TYPE_JOIN = 5
};

#endif
