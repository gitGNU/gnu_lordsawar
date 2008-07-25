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

#ifndef PBM_GAME_SERVER_H
#define PBM_GAME_SERVER_H

#include "config.h"

#include <memory>
#include <list>
#include <sigc++/trackable.h>

class NetworkAction;
class NetworkHistory;
class XML_Helper;
class Player;

class PbmGameServer: public sigc::trackable
{
public:
        
  PbmGameServer();
  ~PbmGameServer();

  void start();

  bool endTurn(std::string turnfile, bool &broken);
  
private:
  std::list<NetworkAction*> d_actions;
  std::list<NetworkHistory*> d_histories;
  void listenForActions();
  void listenForHistories();
  void onActionDone(NetworkAction *action);
  void onHistoryDone(NetworkHistory *history);

  void clearNetworkActionlist();
  void clearNetworkHistorylist();
  bool dumpActionsAndHistories(XML_Helper *helper);
};

#endif
