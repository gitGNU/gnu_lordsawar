// Copyright (C) 2009, 2014, 2015 Ben Asselstine
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
#include <assert.h>
#include "stacktile.h"
#include "stack.h"
#include "defs.h"
#include "vector.h"
#include "player.h"
#include "stacklist.h"
#include "playerlist.h"
#include "Tile.h"

StackTile::StackTile(Vector<int> pos)
  :tile(pos)
{
}

bool StackTile::canAdd(const Stack *stack)
{
  //it's not a bug if the stack is already on this tile.
  //two or more stacks have to be on the same stacktile to join.
  //it's not a bug if more than MAX_ARMIES_ON_A_SINGLE_TILE is exceeded
  //temporarily.  too large stacks can pass through, but not stay.
  if (findStack(stack) != end())
    return true;
  return canAdd(stack->size(), stack->getOwner());
}
    
bool StackTile::canAdd(guint32 siz, Player *owner)
{
  if (siz == 0)
    return false;
  if (countNumberOfArmies(owner) + siz > MAX_ARMIES_ON_A_SINGLE_TILE)
    return false;
  return true;
}

bool StackTile::leaving(Stack *stack)
{
  bool first = true;
  while (1)
    {
      iterator it = findStack(stack);
      if (it == end())
	{
	  if (first)
	    return false;
	  else
	    break;
	}
      erase(it);
    }
  return true;
}

void StackTile::arriving(Stack *stack)
{
  add(stack);
}

void StackTile::add(Stack *stack)
{
  iterator it = findStack(stack);
  if (it != end()) //we replace existing entries, to make this work sans game.
    leaving(stack);
  struct StackTileRecord rec;
  rec.stack_id = stack->getId();
  rec.player_id = stack->getOwner()->getId();
  if (rec.stack_id == 4322)
  printf("4322 on %d,%d has an owner of %d\n", tile.x, tile.y, rec.player_id);
  push_back(rec);
  //i could stack->setpos here, but i prefer to let Stack::moveToDest do that because it's movement related, and this class is not movement related.
}

guint32 StackTile::countNumberOfArmies(Player *owner) const
{
  guint32 count = 0;
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it).player_id == owner->getId())
	{
	  Stack *stack = owner->getStacklist()->getStackById((*it).stack_id);
	  if (stack)
	    count += stack->size();
	}
    }
  return count;
}

StackTile::iterator StackTile::findStack(Stack *s)
{
  for (iterator it = begin(); it != end(); it++)
    if (s->getId() == (*it).stack_id)
      return it;
  return end();
}

StackTile::const_iterator StackTile::findStack(const Stack *s) const
{
  for (const_iterator it = begin(); it != end(); it++)
    if (s->getId() == (*it).stack_id)
      return it;
  return end();
}

Stack *StackTile::getStack() const
{
  if (size() > 0)
    {
      StackTileRecord rec = front();
      Player *p = Playerlist::getInstance()->getPlayer(rec.player_id);
      return p->getStacklist()->getStackById(rec.stack_id);
    }
  return NULL;
}

std::list<Stack *> StackTile::getStacks() const
{
  std::list<Stack *> stacks;
  for (const_iterator it = begin(); it != end(); it++)
    {
      Playerlist *pl = Playerlist::getInstance();
      for (Playerlist::iterator i = pl->begin(); i != pl->end(); i++)
	{
	  Stack *stack = (*i)->getStacklist()->getStackById((*it).stack_id);
	  if (stack)
	    stacks.push_back(stack);
	}
    }
  return stacks;
}

std::list<Stack *> StackTile::getFriendlyStacks(Player *owner) const
{
  std::list<Stack *> stacks;
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it).player_id != owner->getId())
	continue;
      Stack *stack = owner->getStacklist()->getStackById((*it).stack_id);
      if (stack)
	stacks.push_back(stack);
    }
  return stacks;
}
Stack *StackTile::getFriendlyStack(Player *owner) const
{
  //return just one of the stacks located here, but owned by OWNER
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it).player_id != owner->getId())
	continue;
      Stack *stack = owner->getStacklist()->getStackById((*it).stack_id);
      if (stack)
	return stack;
    }
  return NULL;
}

Stack *StackTile::getEnemyStack(Player *owner) const
{
  //return just one of the stacks located here, but not owned by OWNER
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it).player_id == owner->getId())
	continue;
      Player *p = Playerlist::getInstance()->getPlayer((*it).player_id);
      Stack *stack = p->getStacklist()->getStackById((*it).stack_id);
      if (stack)
	return stack;
    }
  return NULL;
}

std::list<Stack *> StackTile::getEnemyStacks(Player *owner) const
{
  std::list<Stack *> stacks;
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it).player_id == owner->getId())
	continue;
      Player *p = Playerlist::getInstance()->getPlayer((*it).player_id);
      Stack *stack = p->getStacklist()->getStackById((*it).stack_id);
      if (stack)
	stacks.push_back(stack);
    }
  return stacks;
}

bool StackTile::contains(guint32 id) const
{
  for (const_iterator it = begin(); it != end(); it++)
    if ((*it).stack_id == id)
      return true;
  return false;
}

Stack *StackTile::group(Player *owner)
{
  return groupStacks(owner, NULL);
}

void StackTile::group(Player *owner, Stack *stack)
{
  groupStacks(owner, stack);
  return;
}

Stack *StackTile::groupStacks(Player *owner, Stack *stack)
{
  std::list<Stack*> stacks = getFriendlyStacks(owner);
  if (stack == NULL)
    {
      if (stacks.size() > 0)
	stack = stacks.front();
    }
  else if (findStack(stack) == end())
    return NULL;

  for (std::list<Stack*>::iterator i = stacks.begin(); i != stacks.end(); i++)
    {
      if (*i == stack)
	continue;
      bool joined = owner->stackJoin(stack, *i);
      assert (joined == true);
    }
  return stack;
}

//! split all of the armies owned by OWNER into a stack by themselves
void StackTile::ungroup(Player *owner)
{
  std::list<Stack*> stacks = getFriendlyStacks(owner);
  std::list<Army *> armies;
  for (std::list<Stack*>::iterator i = stacks.begin(); i != stacks.end(); i++)
    {
      bool first = true;
      for (Stack::iterator j = (*i)->begin(); j != (*i)->end(); j++)
	{
	  if (first == true)
	    {
	      //skip one army of every stack because it doesn't need a new one.
	      first = false;
	      continue;
	    }
	  else
	    {
	      owner->stackSplitArmy(*i, *j);
	      j = (*i)->begin();
	    }
	}
    }
}
    
void StackTile::setDefending(Player *owner, bool defending)
{
  std::list<Stack *> stks = getFriendlyStacks(owner);
  for (std::list<Stack *>::iterator it = stks.begin(); it != stks.end(); it++)
    {
      if ((*it)->getDefending() != defending)
        {
          if (defending)
            (*it)->getOwner()->stackDefend(*it);
          else
            (*it)->getOwner()->stackUndefend(*it);
        }
    }
}

void StackTile::setParked(Player *owner, bool parked)
{
  std::list<Stack *> stks = getFriendlyStacks(owner);
  for (std::list<Stack *>::iterator it = stks.begin(); it != stks.end(); it++)
    {
      if ((*it)->getParked() != parked)
        {
          if (parked)
            (*it)->getOwner()->stackPark((*it));
          else
            (*it)->getOwner()->stackUnpark((*it));
        }
    }
}
