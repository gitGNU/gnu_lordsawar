// Copyright (C) 2008, 2011, 2014 Ben Asselstine
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

#ifndef PBM_GAME_SERVER_H
#define PBM_GAME_SERVER_H

#include <config.h>

#include <memory>
#include <glibmm.h>
#include <list>
#include <sigc++/trackable.h>

class NetworkAction;
class NetworkHistory;
class XML_Helper;
class Player;

class PbmGameServer: public sigc::trackable
{
public:
    //!the topmost tag for a lordsawar turn file.
    static Glib::ustring d_tag; 
        
  void start();

  bool endTurn(Glib::ustring turnfile, bool &broken);
  
  //Statics
  //! Returns the singleton instance.  Creates a new one if neccessary.
  static PbmGameServer * getInstance();

  //! Deletes the singleton instance.
  static void deleteInstance();

  static bool upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version);
  static void support_backward_compatibility();
protected:
  PbmGameServer();
  ~PbmGameServer();
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

  //! A static pointer for the singleton instance.
  static PbmGameServer * s_instance;
};

#endif
