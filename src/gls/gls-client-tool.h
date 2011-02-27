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

class Profile;
class RecentlyPlayedGameList;
class RecentlyPlayedGame;

class GlsClientTool
{
public:
    GlsClientTool(int port, bool show_list, std::list<std::string> unadvertise, bool advertise);
    virtual ~GlsClientTool();
private:
  Profile *new_profile;
  Profile *profile;
  bool d_show_list;
  std::list<std::string> d_unadvertise;
  bool d_advertise;

  //callbacks
  void on_got_list_response(RecentlyPlayedGameList *list, std::string err);
  void on_got_unadvertise_response(std::string id, std::string err);
  void on_got_advertise_response(std::string id, std::string err);
  void on_could_not_connect();
  void on_connected();
  void on_connection_lost();
  RecentlyPlayedGame* create_game();

};
