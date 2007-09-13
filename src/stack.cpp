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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

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
#include "hero.h"
#include "GameMap.h"
#include "vector.h"
#include "xmlhelper.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

Stack::Stack(Player* player, Vector<int> pos)
    : Object(pos), d_player(player), d_defending(false), d_deleting(false)
{
    d_path = new Path();
}

Stack::Stack(Stack& s)
    : Object(s), d_player(s.d_player), d_defending(s.d_defending),
     d_deleting(false)
{
    clear();
    d_path = new Path();
    //deep copy the other stack's armies
    for (iterator sit = s.begin(); sit != s.end(); sit++)
    {
        Army* a;
        Army* h;
	if ((*sit)->isHero())
	  {
	    h = new Hero(dynamic_cast<Hero&>(**sit));
	    push_back(h);
	  }
	else
	  {
	    a = new Army((**sit));
	    push_back(a);
	  }
    }
}

Stack::Stack(XML_Helper* helper)
: Object(helper), d_deleting(false)
{
  helper->getData(d_defending, "defending");

  int i;
  helper->getData(i, "player");
  if (i == -1)
    d_player = 0;
  else
    d_player = Playerlist::getInstance()->getPlayer(i);


  helper->registerTag("path", sigc::mem_fun((*this), &Stack::load));
  helper->registerTag("army", sigc::mem_fun((*this), &Stack::load));
  helper->registerTag("hero", sigc::mem_fun((*this), &Stack::load));
}

Stack::~Stack()
{
  d_deleting = true;
  sdying.emit(this);

  delete d_path;
  flClear();
}

void Stack::setPlayer(Player* p)
{
  // we need to change the armies' loyalties as well!!
  d_player = p;
  for (iterator it = begin(); it != end(); it++)
    (*it)->setPlayer(p);
}

bool Stack::moveOneStep()
{
  debug("move_one_step()");

  d_pos = **d_path->begin();

  d_defending = false;

  //now remove first point of the path
  d_path->flErase(d_path->begin());

  return true;
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
  Rectangle bounds = GameMap::get_boundary();

  Path p;
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
    if (is_inside(bounds, *i) && p.canMoveThere(this, *i))
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
  Army *strongest = 0;
  Uint32 highest_strength = 0;

  for (const_iterator it = begin(); it != end(); ++it)
    {
      // if hero
      if ((*it)->isHero())
	{
	  return *it;
	}
      else if ((*it)->getStat(Army::STRENGTH) > highest_strength)
	{
	  highest_strength = (*it)->getStat(Army::STRENGTH);
	  strongest = *it;
	}
    }
  return strongest;
}

Army* Stack::getStrongestHero() const
{
  Army *strongest = 0;
  Uint32 highest_strength = 0;

  for (const_iterator it = begin(); it != end(); ++it)
    {
      // if hero
      if ((*it)->isHero())
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

void Stack::group()
{
  if (empty())
    return;
  for (const_iterator it = begin(); it != end(); ++it)
    (*it)->setGrouped(true);
  return;
}

void Stack::ungroup()
{
  if (empty())
    return;
  for (const_iterator it = begin(); it != end(); ++it)
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
      if ((*it)->bless())
	count++;
    }
  return count;
}

bool Stack::enoughMoves() const
{
  Vector<int> p = **(d_path->begin());
  Uint32 needed = 0;

  Maptile* tile = GameMap::getInstance()->getTile(p.x, p.y);

  // find out how many MP we need for travelling, first
  if (tile->getBuilding() == Maptile::CITY)
    needed = 1;
  else
    for (const_iterator it = begin(); it != end(); it++)
      if ((*it)->getStat(Army::MOVE_BONUS) & tile->getMaptileType())
	needed = 2;
  if (needed == 0)
    needed = tile->getMoves();

  // now check if all armies fulfill this requirement
  for (const_iterator it = begin(); it != end(); it++)
    if ((*it)->getMoves() < needed)
      return false;

  return true;
}

bool Stack::canMove() const
{
  int tile_moves = getMinTileMoves(), group_moves = getGroupMoves();

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

bool Stack::isFriend(Player* player) const
{
  return d_player == player;
}

Uint32 Stack::getMaxSight() const
{
  Uint32 max = 0;
  for (const_iterator it = begin(); it != end(); it++)
    if ((*it)->getStat(Army::SIGHT) > max)
      max = (*it)->getStat(Army::SIGHT);

  return max;
}

void Stack::nextTurn()
{
  d_defending = false;
  for (iterator it = begin(); it != end(); it++)
    {
      (*it)->resetMoves();
      // TODO: should be moved in a more appropriate place => class Player
      if (d_player)
	d_player->withdrawGold((*it)->getUpkeep());
      (*it)->heal();
    }

  for (const_iterator it = begin(); it != end(); it++)
    if ((*it)->isHero())
      {
	std::list<Item*> backpack = dynamic_cast<Hero*>((*it))->getBackpack();
	std::list<Item*>::const_iterator item;
	for (item = backpack.begin(); item != backpack.end(); item++)
	  {
	    Player *p = d_player;
	    if ((*item)->getBonus(Item::DOUBLEMOVESTACK))
	      (*it)->setStat(Army::MOVES, (*it)->getMoves() * 2);
	    if ((*item)->getBonus(Item::ADD2GOLDPERCITY))
	      p->addGold(2 * Citylist::getInstance()->countCities(p));
	    if ((*item)->getBonus(Item::ADD3GOLDPERCITY))
	      p->addGold(3 * Citylist::getInstance()->countCities(p));
	    if ((*item)->getBonus(Item::ADD4GOLDPERCITY))
	      p->addGold(4 * Citylist::getInstance()->countCities(p));
	    if ((*item)->getBonus(Item::ADD5GOLDPERCITY))
	      p->addGold(5 * Citylist::getInstance()->countCities(p));
	  }
      }
}

bool Stack::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag("stack");
  retval &= helper->saveData("id", d_id);
  if (d_player)
    retval &= helper->saveData("player", d_player->getId());
  else
    retval &= helper->saveData("player", -1);
  retval &= helper->saveData("x", d_pos.x);
  retval &= helper->saveData("y", d_pos.y);
  retval &= helper->saveData("defending", d_defending);

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
  if (tag == "path")
    {
      d_path = new Path(helper);

      return true;
    }

  if (tag == "army")
    {
      Army* a = new Army(helper);
      a->setPlayer(d_player);
      push_back(a);

      return true;
    }

  if (tag == "hero")
    {
      Hero* h = new Hero(helper);
      h->setPlayer(d_player);
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
  for (Stack::const_iterator it = this->begin(); it != this->end(); it++)
    {
      if ((*it)->isGrouped() == false)
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
	  std::list<Item*> backpack = dynamic_cast<Hero*>((*it))->getBackpack();
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
// End of file
