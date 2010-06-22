// Copyright (C) 2009, 2010 Ben Asselstine
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

#include "RoadPathCalculator.h"
#include "PathCalculator.h"
#include "stack.h"
#include "armysetlist.h"
#include "tileset.h"
#include "army.h"

using namespace std;
#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
//#define debug(x)

RoadPathCalculator::RoadPathCalculator(Vector<int> starting_point)
{
  stack = new Stack(NULL, starting_point);

  ArmyProto *basearmy = ArmyProto::createScout();
  Army *a = Army::createNonUniqueArmy(*basearmy);
  delete basearmy;
  stack->add(a);
  path_calculator = new PathCalculator(stack, false);
}
RoadPathCalculator::RoadPathCalculator(const RoadPathCalculator &r)
{
  stack = new Stack(*r.stack);
  path_calculator = new PathCalculator(*r.path_calculator);
}

RoadPathCalculator::~RoadPathCalculator()
{
  delete stack;
  delete path_calculator;
}
Path* RoadPathCalculator::calculate(Vector<int> dest)
{
  guint32 moves = 0, turns = 0, left = 0;
  return path_calculator->calculate(dest, moves, turns, left, false);
}
