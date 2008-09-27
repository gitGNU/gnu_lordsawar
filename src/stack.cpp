// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2000, Anluan O'Brien
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004 John Farrell
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>
#include <assert.h>
#include <algorithm>

#include "stack.h"
#include "playerlist.h"
#include "path.h"
#include "armysetlist.h"
#include "counter.h"
#include "army.h"
#include "citylist.h"
#include "templelist.h"
#include "hero.h"
#include "GameMap.h"
#include "vector.h"
#include "xmlhelper.h"
#include "FogMap.h"
#include "player.h"
#include "portlist.h"
#include "port.h"

std::string Stack::d_tag = "stack";
using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

Stack::Stack(Player* player, Vector<int> pos)
    : UniquelyIdentified(), Movable(pos), Ownable(player), d_defending(false), 
    d_parked(false), d_deleting(false)
{
    d_path = new Path();
}

Stack::Stack(Uint32 id, Player* player, Vector<int> pos)
    : UniquelyIdentified(id), Movable(pos), Ownable(player), 
    d_defending(false), d_parked(false), d_deleting(false)
{
    d_path = new Path();
}

Stack::Stack(Stack& s)
    : UniquelyIdentified(s), Movable(s), Ownable(s), 
    d_defending(s.d_defending), d_parked(s.d_parked), d_deleting(false)
{
    clear();
    d_path = new Path();
    //deep copy the other stack's armies
    
    for (iterator sit = s.begin(); sit != s.end(); sit++)
    {
	if ((*sit)->isHero())
	  {
	    Army *h = new Hero(dynamic_cast<Hero&>(**sit));
	    push_back(h);
	  }
	else
	  {
	    Army *a = new Army((**sit), (*sit)->getOwner());
	    push_back(a);
	  }
    }
}

Stack::Stack(XML_Helper* helper)
  : UniquelyIdentified(helper), Movable(helper), Ownable(helper), 
    d_deleting(false)
{
  helper->getData(d_defending, "defending");
  helper->getData(d_parked, "parked");

  helper->registerTag(Path::d_tag, sigc::mem_fun((*this), &Stack::load));
  helper->registerTag(Army::d_tag, sigc::mem_fun((*this), &Stack::load));
  helper->registerTag(Hero::d_tag, sigc::mem_fun((*this), &Stack::load));
}

Stack::~Stack()
{
  d_deleting = true;
  if (d_unique)
    sdying.emit(this);

  delete d_path;
  flClear();
}

void Stack::setPlayer(Player* p)
{
  // we need to change the armies' loyalties as well!!
  setOwner(p);
  for (iterator it = begin(); it != end(); it++)
    (*it)->setOwner(p);
}

void Stack::moveOneStep()
{
  debug("move_one_step()");

  Vector<int> dest = *getPath()->front();
  moveToDest(dest);

  //now remove first point of the path
  d_path->eraseFirstPoint();
}

void Stack::moveToDest(Vector<int> dest)
{
  Vector<int> pos = getPos();
    
  Uint32 maptype = GameMap::getInstance()->getTile(dest.x,dest.y)->getMaptileType();
  City* to_city = Citylist::getInstance()->getObjectAt(dest.x, dest.y);
  City* on_city = Citylist::getInstance()->getObjectAt(pos.x, pos.y);
  Port* on_port = Portlist::getInstance()->getObjectAt(pos.x, pos.y);
  bool on_water = (GameMap::getInstance()->getTile(pos.x,pos.y)->getMaptileType() == Tile::WATER);
  bool to_water = (GameMap::getInstance()->getTile(dest.x,dest.y)->getMaptileType() == Tile::WATER);
  bool ship_load_unload = false;
  //here we mark the armies as being on or off a boat
  if (!isFlying())
    {
      if ((on_water && to_city) || 
	  (on_water && on_port && !to_water) ||
	  ((on_city || on_port) && to_water))
	{
	  ship_load_unload = true;
	  for (Stack::iterator it = begin(); it != end(); it++)
	    {
	      if (to_water && 
		  ((*it)->getStat(Army::MOVE_BONUS) & Tile::WATER) == 0)
		(*it)->setInShip(true);
	      else
		(*it)->setInShip(false);
	    }
	}
    }
  else
    {
      for (Stack::iterator it = begin(); it != end(); it++)
	(*it)->setInShip(false);
    }

  //how many moves does the stack need to travel to dest?
  int needed_moves = calculateTileMovementCost(dest);

  for (Stack::iterator it = begin(); it != end(); it++)
    {
      if (ship_load_unload)
	(*it)->decrementMoves((*it)->getMoves());
      else 
	{
	  //maybe the army has a natural movement ability
	  if ((*it)->getStat(Army::MOVE_BONUS) & maptype && needed_moves > 1)
	    (*it)->decrementMoves(2);
	  else
	    (*it)->decrementMoves(needed_moves);
	}
    }

  //update position and status
  setPos(dest);

  setFortified(false);
  setDefending(false);
  setParked(false);

  //update fogmap
  getOwner()->getFogMap()->alterFogRadius(dest, getMaxSight(), FogMap::OPEN);
}

bool Stack::isGrouped()
{
  if (empty())
    return false;

  for (const_iterator it = begin(); it != end(); ++it)
    if ((*it)->isGrouped() == false)
      return false;

  return true;
}

// return the maximum moves of this stack by checking the moves of each army
Uint32 Stack::getGroupMoves() const
{
  if (empty())
    return 0;

  assert(!empty());

  int min = -1;

  for (const_iterator it = begin(); it != end(); ++it)
    if ((*it)->isGrouped())
      {
	if (min == -1)
	  min = int((*it)->getMoves());
	else
	  min = std::min(min, int((*it)->getMoves()));
      }

  if (min <= -1)
    return 0;
  return min;
}

int Stack::getMinTileMoves() const
{
  GameMap *map = GameMap::getInstance();
  Rectangle bounds = map->get_boundary();

  std::vector<Vector<int> > tiles;
  tiles.push_back(Vector<int>(getPos().x + 1, getPos().y - 1));
  tiles.push_back(Vector<int>(getPos().x,     getPos().y - 1));
  tiles.push_back(Vector<int>(getPos().x - 1, getPos().y - 1));
  tiles.push_back(Vector<int>(getPos().x + 1, getPos().y + 1));
  tiles.push_back(Vector<int>(getPos().x,     getPos().y + 1));
  tiles.push_back(Vector<int>(getPos().x - 1, getPos().y + 1));
  tiles.push_back(Vector<int>(getPos().x + 1, getPos().y));
  tiles.push_back(Vector<int>(getPos().x - 1, getPos().y));

  int min = -1;

  for (std::vector<Vector<int> >::iterator i = tiles.begin(), end = tiles.end();
       i != end; ++i)
    if (is_inside(bounds, *i))
      {
	int v = map->getTile(i->x, i->y)->getMoves();
	if (min == -1)
	  min = v;
	else
	  min = std::min(min, v);
      }

  return min;
}

// decrement each armys moves by needed moves to travel

void Stack::decrementMoves(Uint32 moves)
{
  debug("decrement_moves()");

  for (iterator it = begin(); it != end(); it++)
    {
      (*it)->decrementMoves(moves);
    }
}

// Purpose: Return the strongest army of a group
// Note: If a hero is present return it. If there are two similar armies
// (two of the same strength, or two heroes) return the first in the sequence.

Army* Stack::getStrongestArmy() const
{
  assert(!empty());
  return getStrongestArmy(false);
}

Army* Stack::getStrongestHero() const
{
  return getStrongestArmy(true);
}

Army* Stack::getStrongestArmy(bool hero) const
{
  Army *strongest = 0;
  Uint32 highest_strength = 0;

  for (const_iterator it = begin(); it != end(); ++it)
    {
      if (((*it)->isHero() && hero) || !hero)
	{
	  if ((*it)->getStat(Army::STRENGTH) > highest_strength)
	    {
	      highest_strength = (*it)->getStat(Army::STRENGTH);
	      strongest = *it;
	    }
	}
    }
  return strongest;
}

Army *Stack::getArmyById(Uint32 id) const
{
  for (Stack::const_iterator i = begin(), e = end(); i != e; ++i)
    if ((*i)->getId() == id)
      return *i;
  
  return 0;
}

Uint32 Stack::countGroupedArmies() const
{
  Uint32 count = 0;
  if (empty())
    return 0;
  for (const_iterator it = begin(); it != end(); ++it)
    if ((*it)->isGrouped() == true)
      count++;
  return count;
}

void Stack::group()
{
  if (empty())
    return;
  for (iterator it = begin(); it != end(); ++it)
    (*it)->setGrouped(true);
  return;
}

void Stack::ungroup()
{
  if (empty())
    return;
  for (iterator it = begin(); it != end(); ++it)
    (*it)->setGrouped(false);
  //set first army to be in the group
  (*(begin()))->setGrouped(true);
  return;
}

bool Stack::hasHero() const
{
  for (const_iterator it = begin(); it != end(); it++)
    if ((*it)->isHero())
      return true;

  return false;
}

Army* Stack::getFirstHero() const
{
  for (const_iterator it = begin(); it != end(); it++)
    if ((*it)->isHero())
      return (*it);

  return 0;
}

void Stack::getHeroes(std::vector<Uint32>& dst) const
{
  debug("getHeroes - stack = " << this)
    for (const_iterator it = begin(); it != end(); ++it)
      {
	// if hero - add it to the vector
	debug("Army type: " << (*it)->getType())
	  if ((*it)->isHero() && (*it)->getHP() > 0)
	    dst.push_back((*it)->getId());
      }
}

int Stack::bless()
{
  int count = 0;
  for (iterator it = begin(); it != end(); it++)
    {
      Temple *temple = Templelist::getInstance()->getObjectAt(getPos());
      if ((*it)->bless(temple))
	count++;
    }
  return count;
}

Uint32 Stack::calculateTileMovementCost(Vector<int> pos) const
{
  Maptile* tile = GameMap::getInstance()->getTile(pos);
  Uint32 moves = tile->getMoves();
  Uint32 bonus = calculateMoveBonus();
  if (bonus & tile->getMaptileType() && moves > 1)
    moves = 2;
  else if (isFlying() && moves > 1)
    moves = 2;
  return moves;
}

bool Stack::enoughMoves() const
{
  if (d_path->size() == 0)
    return true; //we have enough moves to move nowhere!

  Vector<int> p = **(d_path->begin());
  Uint32 needed = calculateTileMovementCost(p);

  // now check if all armies fulfill this requirement
  for (const_iterator it = begin(); it != end(); it++)
    if ((*it)->getMoves() < needed)
      return false;

  return true;
}

bool Stack::canMove() const
{
  int tile_moves = getMinTileMoves();
  int group_moves = getGroupMoves();

  assert (tile_moves != -1);
  return group_moves > 0 && tile_moves >= 0 && group_moves >= tile_moves;
}

Army* Stack::getFirstUngroupedArmy() const
{
  for (const_iterator it = begin(); it != end(); it++)
    {
      if (!(*it)->isGrouped())
	{
	  return *it;
	}
    }
  return 0;
}

Uint32 Stack::getMaxSight() const
{
  Uint32 max = 0;
  for (const_iterator it = begin(); it != end(); it++)
    if ((*it)->getStat(Army::SIGHT) > max)
      max = (*it)->getStat(Army::SIGHT);

  return max;
}

void Stack::payUpkeep(Player *p)
{
  for (iterator it = begin(); it != end(); ++it)
      p->withdrawGold((*it)->getUpkeep());
}

void Stack::nextTurn()
{
  Uint32 movement_multiplier = 1;
  setParked(false);

  //count the number of items that double the movement in the stack.
  for (const_iterator it = begin(); it != end(); it++)
    if ((*it)->isHero())
      {
	std::list<Item*> backpack = dynamic_cast<Hero*>((*it))->getBackpack();
	std::list<Item*>::const_iterator item;
	for (item = backpack.begin(); item != backpack.end(); item++)
	  {
	    if ((*item)->getBonus(Item::DOUBLEMOVESTACK))
	      movement_multiplier*=2;
	  }
      }

  //set the multipler on all armies in the stack
  for (const_iterator it = begin(); it != end(); it++)
    (*it)->setStat(Army::MOVES_MULTIPLIER, movement_multiplier);

  if (d_defending == true)
    setFortified(true);

  for (iterator it = begin(); it != end(); ++it)
    {
      (*it)->resetMoves();
      (*it)->heal();
    }

  //recalculate paths
  getPath()->recalculate(this);

}

bool Stack::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag(Stack::d_tag);
  retval &= helper->saveData("id", d_id);
  retval &= helper->saveData("x", getPos().x);
  retval &= helper->saveData("y", getPos().y);
  if (d_owner)
    retval &= helper->saveData("owner", d_owner->getId());
  else
    retval &= helper->saveData("owner", -1);
  retval &= helper->saveData("defending", d_defending);
  retval &= helper->saveData("parked", d_parked);


  //save path
  retval &= d_path->save(helper);

  //save armies
  for (const_iterator it = begin(); it != end(); it++)
    retval &= (*it)->save(helper);

  retval &= helper->closeTag();

  return retval;
}

bool Stack::load(std::string tag, XML_Helper* helper)
{
  if (tag == Path::d_tag)
    {
      d_path = new Path(helper);

      return true;
    }

  if (tag == Army::d_tag)
    {
      Army* a = new Army(helper);
      a->setOwner(d_owner);
      push_back(a);

      return true;
    }

  if (tag == Hero::d_tag)
    {
      Hero* h = new Hero(helper);
      h->setOwner(d_owner);
      push_back(h);

      return true;
    }

  return false;
}

void Stack::flClear()
{
  for (iterator it = begin(); it != end(); it++)
    delete (*it);
  clear();
}

Stack::iterator Stack::flErase(Stack::iterator object)
{
  delete (*object);
  return erase(object);
}

Uint32 Stack::calculateMoveBonus() const
{
  Uint32 d_bonus = 0;

  bool landed = false;
  Uint32 bonus;
  // check to see if we're all flying
  int num_landedhero = 0;
  int num_flyer = 0;
  int num_landedother = 0;
  if (size() == 0)
    return 0;
  bool grouped = countGroupedArmies() > 0;
  for (const_iterator it = this->begin(); it != this->end(); it++)
    {
      if ((*it)->isGrouped() == false && grouped)
	continue;
      bonus = (*it)->getStat(Army::MOVE_BONUS);
      if (bonus == Tile::GRASS || (bonus & Tile::WATER) == 0 || 
	  (bonus & Tile::FOREST) == 0 || (bonus & Tile::HILLS) == 0 ||
	  (bonus & Tile::MOUNTAIN) == 0 || (bonus & Tile::SWAMP) == 0)
	{
	  landed = true;
	  if ((*it)->isHero())
	    num_landedhero++;
	  else
	    num_landedother++;
	}
      else
	num_flyer++;

    }
  //if we're all flying or we have enough flyers to carry landbound heroes
  if (landed == false ||
      (num_landedother == 0 && num_landedhero <= num_flyer)) 
    {
      d_bonus = Tile::GRASS | Tile::WATER | Tile::FOREST | Tile::HILLS |
	Tile::MOUNTAIN | Tile::SWAMP;
      return d_bonus;
    }

  //or maybe we have an item that lets us all fly
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it)->isHero())
	{
	  Hero *h = dynamic_cast<Hero*>(*it);
	  std::list<Item*> backpack = h->getBackpack();
	  std::list<Item*>::const_iterator item;
	  for (item = backpack.begin(); item != backpack.end(); item++)
	    {
	      if ((*item)->getBonus(Item::FLYSTACK))
		{
		  d_bonus = Tile::GRASS | Tile::WATER | Tile::FOREST | 
		    Tile::HILLS | Tile::MOUNTAIN | Tile::SWAMP;
		  return d_bonus;
		}
	    }
	}
    }

  //calculate move bonuses for non-flying stacks
  for (Stack::const_iterator it = this->begin(); it != this->end(); it++)
    {
      if ((*it)->isGrouped() == false)
	continue;
      bonus = (*it)->getStat(Army::MOVE_BONUS);

      //only forest and hills extend to all other units in the stack
      d_bonus |= bonus & (Tile::HILLS | Tile::FOREST);

    }
  return d_bonus;
}

bool Stack::isFlying () const
{
  Uint32 d_bonus = calculateMoveBonus();
  if (d_bonus == (Tile::GRASS | Tile::WATER | Tile::FOREST | Tile::HILLS |
		  Tile::MOUNTAIN | Tile::SWAMP))
    return true;
  else
    return false;
}

/*if any stack member is in a boat, then the whole stack appears to be in
 * a boat */
bool Stack::hasShip () const
{
  for (Stack::const_iterator it = this->begin(); it != this->end(); it++)
    {
      if ((*it)->isGrouped() == false)
	continue;
      if ((*it)->getStat(Army::SHIP))
	return true;
    }
  return false;
}

Uint32 getFightOrder(std::list<Uint32> values, Uint32 value)
{
  Uint32 count = 0;
  for (std::list<Uint32>::const_iterator it = values.begin(); 
       it != values.end(); it++)
    {
      count++;
      if (count == value)
	return (*it);
    }
  return 0;
}

bool Stack::armyCompareFightOrder (const Army *lhs, const Army *rhs)  
{
  std::list<Uint32> lhs_fight_order = lhs->getOwner()->getFightOrder();
  std::list<Uint32> rhs_fight_order = rhs->getOwner()->getFightOrder();
  Uint32 lhs_rank = getFightOrder (lhs_fight_order, lhs->getTypeId());
  Uint32 rhs_rank = getFightOrder (rhs_fight_order, rhs->getTypeId());
  //if (lhs_rank == rhs_rank)
    //return lhs->getId() < rhs->getId();
  return lhs_rank < rhs_rank; 
}

bool armyCompareGrouped (const Army *lhs, const Army *rhs)  
{
  return lhs->isGrouped() < rhs->isGrouped(); 
}


void Stack::sortForViewing (bool reverse)
{
  sort(armyCompareFightOrder);
  sort(armyCompareGrouped);
  if (reverse)
    std::reverse(begin(), end());
}

void Stack::setFortified(bool fortified)
{
  if (empty())
    return;
  for (iterator it = begin(); it != end(); it++)
    (*it)->setFortified(false);

  (*begin())->setFortified(fortified);
}

bool Stack::getFortified()
{
  if (empty())
    return false;
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getFortified())
	return true;
    }
  return false;
}

Uint32 Stack::getUpkeep()
{
  Uint32 upkeep = 0;
  for (iterator it = begin(); it != end(); it++)
    upkeep += (*it)->getUpkeep();
  return upkeep;
}

bool Stack::canJoin(const Stack *stack) const
{
  Uint32 joinSize = countGroupedArmies();
  if (joinSize == 0)
    joinSize = size();

  /* this is necessary because human players "group" armies in stacks,
   * while the computer player does not.
   */
  if ((stack->size() + joinSize) > MAX_STACK_SIZE)
    return false;

  return true;

}

std::vector<Uint32> Stack::determineReachableArmies(Vector<int> dest)
{
  std::vector<Uint32> ids;
  //try each army individually to see if it reaches
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getMoves() > 0)
	{
	  Stack *stack = Stack::createNonUniqueStack(getOwner(), getPos());
	  stack->push_back(*it);
	  if (stack->getGroupMoves() >= 
	      stack->getPath()->calculate(stack, dest))
	    ids.push_back((*it)->getId());
	  stack->clear();
	  delete stack;
	  //fixme: can't delete stacks like this because it deletes the army
	}
    }
  if (ids.size() == 0)
    return ids;

  //now try to see if any army units can tag along
  for (iterator it = begin(); it != end(); it++)
    {
      //skip over armies that are already known to be reachable
      if (find(ids.begin(), ids.end(), (*it)->getId()) != ids.end())
	continue;
      if ((*it)->getMoves() > 0)
	{
	  Stack *stack = Stack::createNonUniqueStack(getOwner(), getPos());
	  stack->push_back(*it);
	  //also push back the rest of the known reachables
	  std::vector<Uint32>::iterator iit = ids.begin();
	  for (; iit != ids.end(); iit++)
	    {
	      Army *army = getArmyById(*iit);
	      if (army)
		stack->push_back(army);
	    }
	  if (stack->getGroupMoves() >= 
	      stack->getPath()->calculate(stack, dest))
	    ids.push_back((*it)->getId());
	  stack->clear();
	  delete stack;
	}
    }

  return ids;
}

Uint32 Stack::countArmiesBlessedAtTemple(Uint32 temple_id)
{
  Uint32 blessed = 0;
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->blessedAtTemple(temple_id))
	blessed++;
    }
    return blessed;
}
	
Uint32 Stack::scout(Player *p, Vector<int> src, Vector<int> dest, 
		    const ArmyProdBase *prodbase)
{
  Stack *stack = Stack::createNonUniqueStack(p, src);

  Army *army;
  if (!prodbase)
    {
      ArmyProto *proto = Armysetlist::getInstance()->getScout(p->getArmyset());
      if (!proto)
	return 0;
      army = Army::createNonUniqueArmy (*proto, p);
    }
  else
    army = Army::createNonUniqueArmy (*prodbase, p);

  if (!army)
    return 0;
  stack->push_back(army);
  Uint32 mp = stack->getPath()->calculate(stack, dest);
  delete stack;
  return mp;
}

Uint32 Stack::scout(Stack *stack, Vector<int> dest)
{
  Stack *scout_stack = new Stack(*stack);
  Uint32 mp = scout_stack->getPath()->calculate(scout_stack, dest);
  delete scout_stack;
  return mp;
}

Stack* Stack::createNonUniqueStack(Player *player, Vector<int> pos)
{
  return new Stack(0, player, pos);
}

// End of file
