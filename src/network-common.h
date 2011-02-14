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

#ifndef NETWORK_COMMON_H
#define NETWORK_COMMON_H

#include "config.h"


// the network protocol

#define MESSAGE_SIZE_BYTES 4
#define MESSAGE_PREAMBLE_EXTRA_BYTES 2
#define MESSAGE_PROTOCOL_VERSION 1
#define MESSAGE_HEADER_SIZE (MESSAGE_SIZE_BYTES + MESSAGE_PREAMBLE_EXTRA_BYTES)

enum LobbyActionType {
  LOBBY_MESSAGE_TYPE_SIT = -1,
  LOBBY_MESSAGE_TYPE_CHANGE_NAME = 0,
  LOBBY_MESSAGE_TYPE_STAND = 1,
  LOBBY_MESSAGE_TYPE_CHANGE_TYPE = 2,
};

enum MessageType {
  MESSAGE_TYPE_PING = 1,
  MESSAGE_TYPE_PONG = 2,
  MESSAGE_TYPE_SENDING_MAP = 3,
  MESSAGE_TYPE_SENDING_ACTIONS = 4,
  MESSAGE_TYPE_SENDING_HISTORY = 5,
  MESSAGE_TYPE_PARTICIPANT_CONNECT = 6,
  MESSAGE_TYPE_PARTICIPANT_DISCONNECTED = 7,
  MESSAGE_TYPE_PARTICIPANT_CONNECTED = 8,
  MESSAGE_TYPE_PARTICIPANT_DISCONNECT = 9,
  MESSAGE_TYPE_SERVER_DISCONNECT = 10,
  MESSAGE_TYPE_CHAT = 11,
  MESSAGE_TYPE_CHATTED = 12,
  MESSAGE_TYPE_REQUEST_SEAT_MANIFEST = 13,
  MESSAGE_TYPE_TURN_ORDER = 14,
  MESSAGE_TYPE_KILL_PLAYER = 15,
  MESSAGE_TYPE_ROUND_OVER = 16,
  MESSAGE_TYPE_ROUND_START = 17,
  MESSAGE_TYPE_LOBBY_ACTIVITY = 18,
  MESSAGE_TYPE_CHANGE_NICKNAME = 19,
  MESSAGE_TYPE_GAME_MAY_BEGIN = 20,
  MESSAGE_TYPE_OFF_PLAYER = 21,
};

#endif
