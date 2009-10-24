// Copyright (C) 2009 Ben Asselstine
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
#include <list>
#include <gtkmm.h>
#include "vector.h"
class Stack;
class Player;
/**
 * This class is about managing a set stacks that share a tile on the map.
 * 
 * A stacktile object is a temporary object that holds a number of non-empty 
 * stack objects belonging to the same player where their total number of 
 * armies doesn't exceed 8.  The actual value is related to the 
 * MAX_ARMIES_ON_A_SINGLE_TILE constant.
 *
 * This means that there can be up to 8 army units belong to the green player,
 * and another 8 army units belonging to the red player.  Stacks of differing 
 * sides only share a tile when fighting.
 *
 * The stacktile object is not saved to disk, instead it is reconstituted 
 * based on the loading of stacklists.
 *
 * Like map backpack objects that exist on every tile, whether they have any 
 * items in them or not, a stacktile object exists on every tile whether any 
 * stacks are present or not.  the parent object in both cases is the maptile 
 * object.
 *
 * A stack doesn't know what stacktile it's in but it knows Where it is on the 
 * map.  game map has a quick lookup of position to stacktile.
 */
//! Allow a bunch of stacks to share a single tile.

struct StackTileRecord
{
  guint32 stack_id;
  guint32 player_id;
};

class StackTile: public std::list<StackTileRecord>
{
public:
    StackTile(Vector<int> pos);
    ~StackTile();
    bool canAdd(Stack *stack);
    bool leaving(Stack *stack);
    void arriving(Stack *stack);
    void add(Stack *stack);
    bool removeDeadStack(Stack *stack);
    bool removeLivingStack(Stack *stack, StackTile *dest);
    Stack *getFriendlyStack(Player *owner);
    std::list<Stack *> getFriendlyStacks(Player *owner);
    Stack *getEnemyStack(Player *notowner);
    Stack *getStack();
    Stack *getOtherStack(Stack *stack);

    bool join(Stack *receiver);
    bool join(Stack *receiver, Stack *joiner);
    bool contains(guint32 stack_id);


    Stack *group(Player *owner);
    void ungroup(Player *owner);
    void setDefending(Player *owner, bool defending);
    void setParked(Player *owner, bool parked);
private:
    Vector<int> tile;
    guint32 countNumberOfArmies(Player *owner);
    StackTile::iterator findStack(Stack *s);
};
