// Copyright (C) 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2007, 2008 Ben Asselstine
// Copyright (C) 2007, 2008 Ole Laursen
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

#include "NextTurn.h"
#include "playerlist.h"

using namespace std;
#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
//#define debug(x)

NextTurn::NextTurn(bool turnmode, bool random_turns)
    :d_turnmode(turnmode), d_random_turns (random_turns), d_stop(false)
{
  continuing_turn = false;
  
  Player *active = Playerlist::getActiveplayer();
  abort = srequestAbort.connect(sigc::mem_fun(active, &Player::abortTurn));
}

void NextTurn::stop() 
{
  d_stop = true;
  srequestAbort.emit();
}

void NextTurn::nextPlayer()
{
  Playerlist::getInstance()->nextPlayer();
  Player *active = Playerlist::getActiveplayer();
  abort.disconnect();
  abort = srequestAbort.connect(sigc::mem_fun(active, &Player::abortTurn));
}

NextTurn::~NextTurn()
{
}

