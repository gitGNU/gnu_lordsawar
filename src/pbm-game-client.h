// Copyright (C) 2008 Ole Laursen
// Copyright (C) 2008 Ben Asselstine
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

#ifndef PBM_GAME_CLIENT_H
#define PBM_GAME_CLIENT_H

#include "config.h"

#include <list>
#include <memory>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
class XML_Helper;
class NetworkAction;
class NetworkHistory;

class GameScenario;
class Player;

#include "game-client-decoder.h"

class PbmGameClient: public GameClientDecoder
{
public:
  //! Returns the singleton instance.  Creates a new one if neccessary.
  static PbmGameClient* getInstance();

  //! Deletes the singleton instance.
  static void deleteInstance();

  bool loadWithHelper(XML_Helper &helper, Player *player);

protected:
  PbmGameClient();
  ~PbmGameClient();

private:
  //! A static pointer for the singleton instance.
  static PbmGameClient * s_instance;

};

#endif
