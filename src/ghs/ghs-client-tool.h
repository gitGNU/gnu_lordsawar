//
//  Copyright (C) 2011 Ben Asselstine
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

#include "config.h"
#include <memory>
#include <string>
#include <list>
#include <iostream>
#include <glib.h>
#include <gtkmm.h>
#include <sigc++/trackable.h>
#include <sigc++/functors/mem_fun.h>

#ifndef GHS_CLIENT_TOOL_H
#define GHS_CLIENT_TOOL_H
class Profile;
class RecentlyPlayedGameList;
class RecentlyPlayedGame;

class GhsClientTool
{
public:
    GhsClientTool(std::string host, int port, Profile *p, bool show_list, bool reload, std::string unhost, std::string file, bool terminate);
    virtual ~GhsClientTool();
private:
  Profile *new_profile;
  Profile *profile;
  std::string d_host;
  bool d_show_list;
  bool d_reload;
  std::string d_unhost;
  std::string d_file_to_host;
  bool d_terminate;
  guint32 request_count;

  //callbacks
  void on_got_list_response(RecentlyPlayedGameList *list, std::string err);
  void on_got_reload_response(std::string err);
  void on_got_unhost_response(std::string id, std::string err);

  void on_got_host_game_response(std::string scenario_id, std::string err, std::string file);
  void on_game_hosted(std::string scenario_id, guint32 port, std::string err);
  void on_could_not_connect();
  void on_connected();
  void on_connection_lost();
};
#endif
