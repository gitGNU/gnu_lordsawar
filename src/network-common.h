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
  MESSAGE_TYPE_SENDING_HISTORY = 5,
  MESSAGE_TYPE_P1_JOIN = 6,
  MESSAGE_TYPE_P2_JOIN = 7,
  MESSAGE_TYPE_P3_JOIN = 8,
  MESSAGE_TYPE_P4_JOIN = 9,
  MESSAGE_TYPE_P5_JOIN = 10,
  MESSAGE_TYPE_P6_JOIN = 11,
  MESSAGE_TYPE_P7_JOIN = 12,
  MESSAGE_TYPE_P8_JOIN = 13,
  MESSAGE_TYPE_VIEWER_JOIN = 14,
  MESSAGE_TYPE_P1_DEPARTED = 15,
  MESSAGE_TYPE_P2_DEPARTED = 16,
  MESSAGE_TYPE_P3_DEPARTED = 17,
  MESSAGE_TYPE_P4_DEPARTED = 18,
  MESSAGE_TYPE_P5_DEPARTED = 19,
  MESSAGE_TYPE_P6_DEPARTED = 20,
  MESSAGE_TYPE_P7_DEPARTED = 21,
  MESSAGE_TYPE_P8_DEPARTED = 22,
  MESSAGE_TYPE_VIEWER_DEPARTED = 23,
  MESSAGE_TYPE_P1_JOINED = 24,
  MESSAGE_TYPE_P2_JOINED = 25,
  MESSAGE_TYPE_P3_JOINED = 26,
  MESSAGE_TYPE_P4_JOINED = 27,
  MESSAGE_TYPE_P5_JOINED = 28,
  MESSAGE_TYPE_P6_JOINED = 29,
  MESSAGE_TYPE_P7_JOINED = 30,
  MESSAGE_TYPE_P8_JOINED = 31,
  MESSAGE_TYPE_VIEWER_JOINED = 32,
};

#endif
