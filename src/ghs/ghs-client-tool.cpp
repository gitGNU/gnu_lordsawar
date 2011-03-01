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
#include <iostream>
#include <glib.h>
#include <gtkmm.h>
#include <sigc++/trackable.h>
#include <sigc++/functors/mem_fun.h>
#include "gamehost-client.h"
#include "ucompose.hpp"
#include "profilelist.h"
#include "profile.h"
#include "recently-played-game-list.h"
#include "recently-played-game.h"

#include "ghs-client-tool.h"

GhsClientTool::GhsClientTool(std::string host, int port, bool show_list, bool reload)
{
  request_count = 0;
  d_show_list = show_list;
  d_reload = reload;
  GamehostClient *gamehostclient = GamehostClient::getInstance();
  Profilelist *plist = Profilelist::getInstance();
  new_profile = NULL;
  if (plist->size() > 0)
    profile = plist->front();
  else
    {
      new_profile = new Profile("admin");
      profile = new_profile;
    } 
  gamehostclient->client_could_not_connect.connect
    (sigc::mem_fun(*this, &GhsClientTool::on_could_not_connect));
  gamehostclient->client_connected.connect
    (sigc::mem_fun(*this, &GhsClientTool::on_connected));
  gamehostclient->client_forcibly_disconnected.connect
    (sigc::mem_fun(*this, &GhsClientTool::on_connection_lost));
  gamehostclient->start(host, port, profile);
}

GhsClientTool::~GhsClientTool()
{
  GamehostClient::deleteInstance();
  if (new_profile)
    delete new_profile;
}

void GhsClientTool::on_got_list_response(RecentlyPlayedGameList *l, std::string err)
{
  request_count--;
  if (err != "")
    {
      std::cerr << err << std::endl;
      if (request_count == 0)
        Gtk::Main::quit();
      return;
    }
  Glib::ustring s = String::ucompose(ngettext ("Listing %1 game", 
                                               "Listing %1 games", l->size()),
                                    l->size());
  std::cout << s << std::endl;
  for (RecentlyPlayedGameList::iterator i = l->begin(); i != l->end(); i++)
    {
      std::cout << std::endl;
      RecentlyPlayedNetworkedGame *g = 
        dynamic_cast<RecentlyPlayedNetworkedGame*>(*i);
      std::cout << _("Id:") << " " << g->getId() << std::endl;
      std::cout << _("Name:") << " " << g->getName() << std::endl;
      std::cout << _("Host:") << " " << g->getHost() << std::endl;
      std::cout << _("Port:") << " " << g->getPort() << std::endl;
      std::cout << _("Profile:") << " " << g->getProfileId() << std::endl;
    }
  delete l;
  if (request_count == 0)
    Gtk::Main::quit();
  return;
}

void GhsClientTool::on_got_reload_response(std::string err)
{
  request_count--;
  if (err != "")
    std::cerr << err << std::endl;
  if (request_count == 0)
    Gtk::Main::quit();
}

void GhsClientTool::on_could_not_connect()
{
  std::cerr << _("Could not connect to game list server") << std::endl;
  exit(1);
}

void GhsClientTool::on_connection_lost()
{
  std::cerr << _("Server went away unexpectedly") << std::endl;
  exit(1);
}

void GhsClientTool::on_connected()
{
  GamehostClient *gamehostclient = GamehostClient::getInstance();
  if (d_show_list)
    {
      gamehostclient->received_game_list.connect
        (sigc::mem_fun(*this, &GhsClientTool::on_got_list_response));
      request_count++;
      gamehostclient->request_game_list();
    }

  if (d_reload)
    {
      request_count++;
      gamehostclient->received_reload_response.connect
        (sigc::mem_fun(*this, &GhsClientTool::on_got_reload_response));
      gamehostclient->request_reload();
    }

}
