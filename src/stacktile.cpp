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
#include "stacktile.h"
#include "stack.h"
#include "defs.h"
#include "vector.h"
#include "player.h"
#include "stacklist.h"
#include "playerlist.h"

StackTile::StackTile(Vector<int> pos)
  :tile(pos)
{
}

StackTile::~StackTile()
{
}

bool StackTile::canAdd(Stack *stack)
{
  if (stack->size() == 0)
    return false;
  if (countNumberOfArmies(stack->getOwner()) + stack->size() > MAX_ARMIES_ON_A_SINGLE_TILE)
    return false;
  return true;
}

bool StackTile::leaving(Stack *stack)
{
  iterator it = findStack(stack);
  if (it == end())
    return false;
  erase(it);
  return true;
}

void StackTile::arriving(Stack *stack)
{
  add(stack);
}

void StackTile::add(Stack *stack)
{
  iterator it = findStack(stack);
  if (it != end())
    return;  //it's already here.
  struct StackTileRecord rec;
  rec.stack_id = stack->getId();
  rec.player_id = stack->getOwner()->getId();
  push_back(rec);
  //i could stack->setpos here, but i prefer to let Stack::moveToDest do that because it's movement related, and this class is not movement related.
}

bool StackTile::removeDeadStack(Stack *s)
{
  iterator it = findStack(s);
  if (it == end())
    return false;
  if (s->size() > 0)
    return false; //stack not dead yet!
  erase(it);
  return true;
}

bool StackTile::removeLivingStack(Stack *s, StackTile *dest)
{
  iterator it = findStack(s);
  if (it == end())
    return false;
  if (dest->canAdd(s) == false)
    return false;
  erase(it);
  dest->add(s);
  return true;
}

guint32 StackTile::countNumberOfArmies(Player *owner)
{
  guint32 count = 0;
  for (iterator it = begin(); it != end(); it++)
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

Stack *StackTile::getStack()
{
  if (size() > 0)
    {
      Player *p = Playerlist::getInstance()->getPlayer(front().player_id);
      return p->getStacklist()->getStackById(front().stack_id);
    }
  return NULL;
}

std::list<Stack *> StackTile::getFriendlyStacks(Player *owner)
{
  std::list<Stack *> stacks;
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it).player_id != owner->getId())
	continue;
      Stack *stack = owner->getStacklist()->getStackById((*it).stack_id);
      if (stack)
	stacks.push_back(stack);
    }
  return stacks;
}
Stack *StackTile::getFriendlyStack(Player *owner)
{
  //return just one of the stacks located here, but owned by OWNER
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it).player_id != owner->getId())
	continue;
      Stack *stack = owner->getStacklist()->getStackById((*it).stack_id);
      if (stack)
	return stack;
    }
  return NULL;
}

Stack *StackTile::getEnemyStack(Player *owner)
{
  //return just one of the stacks located here, but not owned by OWNER
  for (iterator it = begin(); it != end(); it++)
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


//! join all of the stacks located here that are owned by OWNER with S
bool StackTile::join(Stack *receiver, Stack *joiner)
{
  iterator it = findStack(receiver);
  if (it == end())
    return false;
  it = findStack(joiner);
  if (it == end())
    return false;
  if (receiver->canJoin(joiner) == false)
    return false;
  receiver->join(joiner);
  erase(findStack(joiner));
  return true;
}

bool StackTile::join(Stack *receiver)
{
  iterator it = findStack(receiver);
  if (it == end())
    return false;
  std::list<Stack *> joiner;
  for (iterator it = begin(); it != end(); it++)
    {
      if (receiver->getId() == (*it).stack_id)
	continue;
      Stack *stack = 
	receiver->getOwner()->getStacklist()->getStackById((*it).stack_id);
      if (stack->getOwner() == receiver->getOwner())
	joiner.push_back(stack);
    }
  for (std::list<Stack *>::iterator i = joiner.begin(); i != joiner.end(); i++)
    {
      if (receiver->canJoin(*i) == false)
	return false;
      else
	receiver->join(*i);
      erase(findStack(*i));
    }
  return true;
}
    
Stack *StackTile::getOtherStack(Stack *stack)
{
  if (findStack(stack) == end())
    return NULL;
  for (iterator it = begin(); it != end(); it++)
    {
      if (stack->getId() != (*it).stack_id)
	{
	  //great, this is a stack id that isn't us.
	  //now go get the stack from somebody's stacklist.
	  if (stack->getOwner()->getId() != (*it).player_id)
	    continue;
	  Player *p = stack->getOwner();
	  Stack *other = p->getStacklist()->getStackById((*it).stack_id);
	  if (other)
	    return other;
	}
    }
  return NULL;
}
    
bool StackTile::contains(guint32 id)
{
  for (iterator it = begin(); it != end(); it++)
    if ((*it).stack_id == id)
      return true;
  return false;
}

//! merge all of the armies owned by OWNER into a single stack
Stack *StackTile::group(Player *owner)
{
  Stack *stack = NULL;
  std::list<Stack*> stacks = getFriendlyStacks(owner);
  if (stacks.size() > 0)
    stack = stacks.front();
  for (std::list<Stack*>::iterator i = stacks.begin(); i != stacks.end(); i++)
    {
      if (*i == stack)
	continue;
      owner->stackJoin(stack, *i);
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
    (*it)->setDefending(defending);
}

void StackTile::setParked(Player *owner, bool parked)
{
  std::list<Stack *> stks = getFriendlyStacks(owner);
  for (std::list<Stack *>::iterator it = stks.begin(); it != stks.end(); it++)
    (*it)->setParked(parked);
}
