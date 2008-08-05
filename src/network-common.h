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
  MESSAGE_TYPE_P1_SIT = 6,
  MESSAGE_TYPE_P2_SIT = 7,
  MESSAGE_TYPE_P3_SIT = 8,
  MESSAGE_TYPE_P4_SIT = 9,
  MESSAGE_TYPE_P5_SIT = 10,
  MESSAGE_TYPE_P6_SIT = 11,
  MESSAGE_TYPE_P7_SIT = 12,
  MESSAGE_TYPE_P8_SIT = 13,
  MESSAGE_TYPE_PARTICIPANT_CONNECT = 14,
  MESSAGE_TYPE_P1_STOOD_UP = 15,
  MESSAGE_TYPE_P2_STOOD_UP = 16,
  MESSAGE_TYPE_P3_STOOD_UP = 17,
  MESSAGE_TYPE_P4_STOOD_UP = 18,
  MESSAGE_TYPE_P5_STOOD_UP = 19,
  MESSAGE_TYPE_P6_STOOD_UP = 20,
  MESSAGE_TYPE_P7_STOOD_UP = 21,
  MESSAGE_TYPE_P8_STOOD_UP = 22,
  MESSAGE_TYPE_PARTICIPANT_DISCONNECTED = 23,
  MESSAGE_TYPE_P1_SAT_DOWN = 24,
  MESSAGE_TYPE_P2_SAT_DOWN = 25,
  MESSAGE_TYPE_P3_SAT_DOWN = 26,
  MESSAGE_TYPE_P4_SAT_DOWN = 27,
  MESSAGE_TYPE_P5_SAT_DOWN = 28,
  MESSAGE_TYPE_P6_SAT_DOWN = 29,
  MESSAGE_TYPE_P7_SAT_DOWN = 30,
  MESSAGE_TYPE_P8_SAT_DOWN = 31,
  MESSAGE_TYPE_PARTICIPANT_CONNECTED = 32,
  MESSAGE_TYPE_P1_STAND = 33,
  MESSAGE_TYPE_P2_STAND = 34,
  MESSAGE_TYPE_P3_STAND = 35,
  MESSAGE_TYPE_P4_STAND = 36,
  MESSAGE_TYPE_P5_STAND = 37,
  MESSAGE_TYPE_P6_STAND = 38,
  MESSAGE_TYPE_P7_STAND = 39,
  MESSAGE_TYPE_P8_STAND = 40,
  MESSAGE_TYPE_PARTICIPANT_DISCONNECT = 41,
  MESSAGE_TYPE_SERVER_DISCONNECT = 42,
  MESSAGE_TYPE_CHAT = 43,
  MESSAGE_TYPE_CHATTED = 44,
  MESSAGE_TYPE_REQUEST_SEAT_MANIFEST = 45
};

#endif
