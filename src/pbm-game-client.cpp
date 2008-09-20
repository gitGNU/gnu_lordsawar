// Copyright (C) 2008 Ole Laursen
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

#include <iostream>
#include <fstream>

#include "pbm-game-client.h"

#include "File.h"
#include "action.h"
#include "network-action.h"
#include "network-history.h"
#include "playerlist.h"
#include "network_player.h"
#include "xmlhelper.h"



PbmGameClient * PbmGameClient::s_instance = 0;


PbmGameClient* PbmGameClient::getInstance()
{
    if (s_instance == 0)
        s_instance = new PbmGameClient();

    return s_instance;
}

void PbmGameClient::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}


PbmGameClient::PbmGameClient()
{
}

PbmGameClient::~PbmGameClient()
{
}

bool PbmGameClient::loadWithHelper(XML_Helper &helper, Player *p)
{
  ActionLoader actionloader;
  HistoryLoader historyloader;
  bool broken = false;
  helper.registerTag(NetworkAction::d_tag, sigc::mem_fun(actionloader, &ActionLoader::loadAction));
  helper.registerTag(Action::d_tag, sigc::mem_fun(actionloader, &ActionLoader::loadAction));
  helper.registerTag(NetworkHistory::d_tag, sigc::mem_fun(historyloader, &HistoryLoader::loadHistory));
  helper.registerTag(History::d_tag, sigc::mem_fun(historyloader, &HistoryLoader::loadHistory));
  if (!helper.parse())
    broken = true;

  int num = decodeActions(actionloader.actions, p);
  printf ("decoded %d actions\n", num);
  num = decodeHistories(historyloader.histories);
  printf ("decoded %d histories\n", num);
  return broken;
}
