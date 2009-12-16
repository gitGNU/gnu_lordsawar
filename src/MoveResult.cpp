// Copyright (C) 2004 John Farrell
// Copyright (C) 2005 Ulf Lorenz
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

#include <iostream>
#include "MoveResult.h"
#include "fight.h"
#include "stack.h"
#include "path.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
//#define debug(x)

MoveResult::MoveResult()
    : d_fight(false), d_stepCount(0), d_out_of_moves(false), 
    d_reached_end(false), d_treachery(false), d_considered_treachery(false),
    d_too_large_stack_in_the_way(false), d_fightResult(Fight::DRAW)
{
}

MoveResult::~MoveResult()
{
}

void MoveResult::fillData(Stack *s, int stepCount)
{
  if (s->getPath()->size() == 0)
    d_reached_end = true;

  if (s->enoughMoves() == false)
    d_out_of_moves = true;

  d_stepCount = stepCount;
}

void MoveResult::setFightResult(Fight::Result fightResult)
{
    d_fight = true;
    d_fightResult = fightResult;
}
// End of file
