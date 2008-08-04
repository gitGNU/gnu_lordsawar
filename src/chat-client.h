// Copyright (C) 2008 Ben Asselstine
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

#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include "config.h"

#include <list>
#include <memory>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>

class ChatClient: public sigc::trackable
{
public:
  ChatClient(std::string nick = "guest");
  ~ChatClient();

  void gotChatMessage(const std::string nickname, const std::string &payload);
  void setNickname(std::string nick) {d_nickname = nick;};
  sigc::signal<void, std::string, std::string> chat_message_received;
protected:

  std::string d_nickname;
};

#endif
