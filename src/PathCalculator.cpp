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

#include <stdlib.h>
#include <string.h>
#include <queue>
#include "PathCalculator.h"
#include "GameMap.h"
#include "path.h"
#include "stack.h"
#include "maptile.h"
#include "city.h"
#include "stacklist.h"
#include "armysetlist.h"
#include "armyprodbase.h"

using namespace std;
//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
#define debug(x)

void PathCalculator::populateNodeMap()
{
  Vector<int> start = stack->getPos();
  int width = GameMap::getWidth();
  int height = GameMap::getHeight();
  if (start.x >= width || start.x < 0)
    return;
  if (start.y >= height || start.y < 0)
    return;
  size_t length = width * height * sizeof (struct node);
  nodes = (struct node*) malloc (length);
  memset (nodes, 0, length);

  // Some notes concerning the path finding algorithm. The algorithm
  // uses two different lists. There is a nodes array, which contains how
  // many MP one needs to get to the location (x,y), and a process queue that
  // tells you at what point the number of movement points is calculated next.
  //
  // What we basically do is to start at the stack's position and calculate
  // the distance (i.e. MP needed to get there) in circles around the starting
  // position with the circles becoming increasingly larger. In detail, we
  // start by appending the starting position in the queue of positions to be
  // processed. Then, recursively, we take the first point in the queue and
  // recalculate the distance for all bordering tiles, assuming that we go
  // over this point and overwriting their current value if it is larger
  // than what we find now. Then, we append each modified tile to the queue of
  // tiles to be processed and continue. We stop when there are no more tiles
  // to process.
  //
  // Finally, all that is left is finding the minimum distance way from start
  // point to destination.

  // the conversion between x/y coordinates and index is (size is map size)
  // index = y*width + x    <=>    x = index % width;   y = index / width
  std::queue<Vector<int> > process;

  // initial filling of the nodes vector
  for (int i = 0; i < width*height; i++)
    {
      // -1 means don't know yet
      // -2 means can't go there at all
      // 0 or more is number of movement points needed to get there
      nodes[i].moves = -1;
      nodes[i].moves_left = 0;
      nodes[i].turns = 0;
      if (isBlocked(Vector<int>(i % width, i/width)))
	nodes[i].moves = -2;
    }
  int idx = start.toIndex();
  nodes[idx].moves = 0;
  nodes[idx].moves_left = stack->getMoves();
  nodes[idx].turns = 0;

  // now the main loop
  process.push(start);
  while (!process.empty())
    {
      Vector<int> pos = process.front();
      process.pop();                          // remove the first item

      std::list<Vector<int> > next = calcMoves(pos);
      for (std::list<Vector<int> >::iterator it = next.begin(); 
	   it != next.end(); it++)
	process.push(*it);
    }
}

PathCalculator::PathCalculator(Stack *s, bool zig)
:stack(s), flying(s->isFlying()), d_bonus(s->calculateMoveBonus()),
    land_reset_moves(s->getMaxLandMoves()),
    boat_reset_moves(s->getMaxBoatMoves()), zigzag(zig), on_ship(stack->hasShip()), delete_stack(false)
{
  populateNodeMap();
}

PathCalculator::PathCalculator(Player *p, Vector<int> src, const ArmyProdBase *prodbase, bool zig)
{
  stack = Stack::createNonUniqueStack(p, src);
  Army *army;
  if (!prodbase)
    {
      ArmyProto *proto = Armysetlist::getInstance()->getScout(p->getArmyset());
      if (!proto)
	return;
      army = Army::createNonUniqueArmy (*proto, p);
    }
  else
    army = Army::createNonUniqueArmy (*prodbase, p);

  if (!army)
    return;
  stack->push_back(army);
  flying = stack->isFlying();
  d_bonus = stack->calculateMoveBonus();
  land_reset_moves = stack->getMaxLandMoves();
  boat_reset_moves = stack->getMaxBoatMoves();
  zigzag = zig; 
  on_ship = stack->hasShip();
  delete_stack = true;
  populateNodeMap();
}

PathCalculator::PathCalculator(const Stack &s, bool zig)
{
  stack = new Stack(s);
  flying = stack->isFlying();
  d_bonus = stack->calculateMoveBonus();
  land_reset_moves = stack->getMaxLandMoves();
  boat_reset_moves = stack->getMaxBoatMoves();
  zigzag = zig; 
  on_ship = stack->hasShip();
  delete_stack = true;
  populateNodeMap();
}


PathCalculator::PathCalculator(const PathCalculator &p)
:stack(new Stack(*p.stack)), flying(p.flying), d_bonus(p.d_bonus),
    land_reset_moves(p.land_reset_moves),
    boat_reset_moves(p.boat_reset_moves), zigzag(p.zigzag), on_ship(p.on_ship),
    delete_stack(p.delete_stack)
{
  int width = GameMap::getWidth();
  int height = GameMap::getHeight();
  size_t length = width * height * sizeof (struct node);
  nodes = (struct node*) malloc (length);
  for (size_t i = 0; i < length; i++)
    nodes[i] = p.nodes[i];
}

bool PathCalculator::calcFinalMoves(Vector<int> pos, Vector<int> next)
{
  bool traversable = false;
  int mp;
  int dxy = nodes[pos.toIndex()].moves;   // always >= 0
  //am i blocked from entering sx,sy from pos?
  bool is_blocked_dir = isBlockedDir(pos, next);
  printf("checking %d,%d to %d,%d\n", pos.x, pos.y, next.x, next.y);
  //printf("flying is %d\n", flying);
  //printf("isblockeddir is %d\n", is_blocked_dir);
  printf("moves of source is %d\n", dxy);
  if (!flying && is_blocked_dir)
    return false;
  //flyers can't go through the void
  if (flying && is_blocked_dir &&
      GameMap::getInstance()->getTile(next)->getMaptileType() == Tile::VOID)
    return false;

  int dsxy = nodes[next.toIndex()].moves;
  printf("moves of dest is %d\n", dsxy);
  if (dsxy < -1)
    return false; //can't move there anyway

  if (zigzag == false)
    {
      Vector<int> diff = pos - next;
      if (diff.x && diff.y)
	return false;
    }
  int newDsxy = dxy;
  mp = pointsToMoveTo(pos, next);
  printf("number of moves to get from source to dest is %d\n", mp);
  if (mp < 0)
    mp = 0;
  if (!flying && load_or_unload(pos, next, on_ship) == true)
    {
      printf("moves left for %d,%d is %d\n", pos.x, pos.y, nodes[pos.toIndex()].moves_left);
    mp = nodes[pos.toIndex()].moves_left;
  printf("correction, number of moves to get from source to dest is %d\n", mp);
    }
  newDsxy += mp;
  if (newDsxy == -1)
    printf("new algo STILL put -1 in for moves\n");

  //printf("new value for source moves is %d\n", newDsxy);
  //printf("%d == -1 || %d > %d\n", dsxy, dsxy, newDsxy);
  if (dsxy == -1 || dsxy > newDsxy)
    {
      int idx = next.toIndex();
      nodes[idx].moves = newDsxy;
      nodes[idx].moves_left = nodes[pos.toIndex()].moves_left - mp;
      nodes[idx].turns = nodes[pos.toIndex()].turns;
      while (nodes[idx].moves_left <= 0)
	{
	  if (on_ship)
	    nodes[idx].moves_left += boat_reset_moves;
	  else
	    nodes[idx].moves_left += land_reset_moves;
	  nodes[idx].turns++;
	}

      // append the item to the queue
      traversable = true;
    }
  return traversable;
}

bool PathCalculator::calcMoves(Vector<int> pos, Vector<int> next)
{
  bool traversable = false;
  int mp;
  int dxy = nodes[pos.toIndex()].moves;   // always >= 0
  //am i blocked from entering sx,sy from pos?
  bool is_blocked_dir = isBlockedDir(pos, next);
  if (!flying && is_blocked_dir)
    return false;
  //flyers can't go through the void
  if (flying && is_blocked_dir &&
      GameMap::getInstance()->getTile(next)->getMaptileType() == Tile::VOID)
    return false;

  int dsxy = nodes[next.toIndex()].moves;
  if (dsxy < -1)
    return false; //can't move there anyway

  if (zigzag == false)
    {
      Vector<int> diff = pos - next;
      if (diff.x && diff.y)
	return false;
    }
  int newDsxy = dxy;
  mp = pointsToMoveTo(pos, next);
  if (mp < 0)
    mp = 0;
  if (!flying && load_or_unload(pos, next, on_ship) == true)
    mp = nodes[pos.toIndex()].moves_left;
  newDsxy += mp;

  if (dsxy == -1 || dsxy > newDsxy)
    {
      int idx = next.toIndex();
      nodes[idx].moves = newDsxy;
      nodes[idx].moves_left = nodes[pos.toIndex()].moves_left - mp;
      nodes[idx].turns = nodes[pos.toIndex()].turns;
      while (nodes[idx].moves_left <= 0)
	{
	  if (on_ship)
	    nodes[idx].moves_left += boat_reset_moves;
	  else
	    nodes[idx].moves_left += land_reset_moves;
	  nodes[idx].turns++;
	}

      // append the item to the queue
      traversable = true;
    }
  return traversable;
}

bool PathCalculator::calcFinalMoves(Vector<int> pos)
{
  bool traversable = false;
  int width = GameMap::getWidth();
  int height = GameMap::getHeight();
  for (int sx = pos.x-1; sx <= pos.x+1; sx++)
    {
      if (sx < 0 || sx >= width)
	continue;

      for (int sy = pos.y-1; sy <= pos.y+1; sy++)
	{
	  if (sy < 0 || sy >= height)
	    continue;

	  Vector<int> next = Vector<int>(sx, sy);
	  if (pos == next)
	    continue;

	  if (nodes[next.toIndex()].moves <= -1)
	    continue;
	  if (calcMoves(next, pos))
	    traversable = true;

	}
    }
  return traversable;
}

std::list<Vector<int> > PathCalculator::calcMoves(Vector<int> pos)
{
  int width = GameMap::getWidth();
  int height = GameMap::getHeight();
  std::list<Vector<int> > process;
  for (int sx = pos.x-1; sx <= pos.x+1; sx++)
    {
      if (sx < 0 || sx >= width)
	continue;

      for (int sy = pos.y-1; sy <= pos.y+1; sy++)
	{
	  if (sy < 0 || sy >= height)
	    continue;

	  Vector<int> next = Vector<int>(sx, sy);
	  if (pos == next)
	    continue;

	  if (calcMoves(pos, next) == true)
	    process.push_back(next);

	}
    }
  return process;
}

bool PathCalculator::load_or_unload(Vector<int> src, Vector<int> dest, bool &on_ship)
{
  //do we load or unload if we step from SRC to DEST?
  Vector<int> old = stack->getPos();
  stack->setPos(src);
  bool retval = stack->isMovingToOrFromAShip(dest, on_ship);
  stack->setPos(old);
  return retval;
}
int PathCalculator::pointsToMoveTo(Vector<int> pos, Vector<int> next) const
{
  guint32 moves;
  const Maptile* tile = GameMap::getInstance()->getTile(next);

  if (pos == next) //probably shouldn't happen
    return 0;

  moves = tile->getMoves();

  // does everything in the stack have a bonus to move onto this square?
  if (tile->getMaptileType() & d_bonus && moves != 1)
    return 2;

  return moves;
}

PathCalculator::~PathCalculator()
{
  free (nodes);
  if (delete_stack)
    delete stack;
}
//am i blocked from entering destx,desty from x,y when i'm not flying?
bool PathCalculator::isBlockedDir(Vector<int> pos, Vector<int> next)
{
  int diffx = next.x - pos.x;
  int diffy = next.y - pos.y;
  if (diffx >= -1 && diffx <= 1 && diffy >= -1 && diffy <= 1) 
    {
      int idx = 0;
      if (diffx == -1 && diffy == -1)
	idx = 0;
      else if (diffx == -1 && diffy == 0)
	idx = 1;
      else if (diffx == -1 && diffy == 1)
	idx = 2;
      else if (diffx == 0 && diffy == 1)
	idx = 3;
      else if (diffx == 0 && diffy == -1)
	idx = 4;
      else if (diffx == 1 && diffy == -1)
	idx = 5;
      else if (diffx == 1 && diffy == 0)
	idx = 6;
      else if (diffx == 1 && diffy == 1)
	idx = 7;
      else
	return false;
      return GameMap::getInstance()->getTile(pos)->d_blocked[idx];
    }

  return false;
}

bool PathCalculator::isBlocked(Stack *s, Vector<int> pos)
{
  const Maptile* tile = GameMap::getInstance()->getTile(pos);

  // Return true on every condition which may prevent the stack from
  // entering the tile, which are...

  // ...enemy stacks which stand in the way...
  Stack* target = GameMap::getEnemyStack(pos);
  if (target)
    return true;

  //...enemy cities
  // saves some computation time here
  if (tile->getBuilding() == Maptile::CITY)
    {
      City* c = GameMap::getCity(pos);
      if (c && (c->getOwner() != s->getOwner()))
	return true;
    }

  //no obstacles??? well, then...
  return false;
}

bool PathCalculator::isBlocked(Vector<int> pos)
{
  return isBlocked(stack, pos);
}

bool PathCalculator::canMoveThere(Vector<int> dest)
{
  Vector<int> pos = stack->getPos();
  if (flying && isBlockedDir(pos, dest) &&
      GameMap::getInstance()->getTile(dest)->getMaptileType()
      == Tile::VOID)
    return false;
  if (isBlocked(pos))
    {
      //psst. if it's our last step we can step into cities.
      //psst. if it's our last step we can step onto enemy stacks
      //psst.  if it's our last step and we're a computer player who was diligently walking around friendly stacks, we can merge on our last step
      return false;
    }
  if (isBlockedDir(pos, dest) && !flying)
    return false;
  return true;
}

int PathCalculator::calculate(Vector<int> dest, bool zigzag)
{
  int retval = 0;
  guint32 moves = 0;
  guint32 turns = 0;
  Path *p = calculate(dest, moves, turns, zigzag);
  if (p->size() == 0)
    retval = -1;
  delete p;
  if (retval == 0)
    retval = (int) moves;
  return retval;
}

Path* PathCalculator::calculate(Vector<int> dest, guint32 &moves, guint32 &turns, bool zigzag)
{
  Path *path = new Path();
  int width = GameMap::getWidth();
  int height = GameMap::getHeight();
  if (dest.x >= width || dest.x < 0)
    return path;
  if (dest.y >= height || dest.y < 0)
    return path;


  int idx = dest.toIndex();
  struct node orig_dest = nodes[idx];
  //now change dest node
  if (nodes[idx].moves == -2)
    {
      nodes[idx].moves = -1;
      nodes[idx].moves_left = 0;
      nodes[idx].turns = 0;
      bool traversable = calcFinalMoves(dest);
      if (traversable == false)
	{
	  nodes[idx] = orig_dest;
	  return path;
	}
    }

  // The nodes array is now completely populated.
  // What we have to do now is find the shortest path to the destination.
  // We do that by starting at the destination and moving at each step to
  // the neighbour closest to the start.

  int dist = nodes[idx].moves;
  if (dist < 0)
    {
      path->setMovesExhaustedAtPoint(0);
      moves = 0;
      turns = 0;
      nodes[idx] = orig_dest;
      return path;
    }

  // choose the order in which we process directions so as to favour
  // diagonals over straight lines
  std::list<Vector<int> > diffs;
  if (zigzag)
    {
      diffs.push_back(Vector<int>(-1, -1));
      diffs.push_back(Vector<int>(-1, 1));
      diffs.push_back(Vector<int>(1, -1));
      diffs.push_back(Vector<int>(1, 1));
      diffs.push_back(Vector<int>(1, 0));
      diffs.push_back(Vector<int>(-1, 0));
      diffs.push_back(Vector<int>(0, -1));
      diffs.push_back(Vector<int>(0, 1));
    }
  else
    {
      diffs.push_back(Vector<int>(1, 0));
      diffs.push_back(Vector<int>(-1, 0));
      diffs.push_back(Vector<int>(0, -1));
      diffs.push_back(Vector<int>(0, 1));
    }

  Vector<int> pos = dest;
  while (dist > 0)
    {
      path->push_front(pos);

      int min = dist;
      Vector<int> minpos = pos;
      for (std::list<Vector<int> >::iterator it = diffs.begin();
	   it != diffs.end(); it++)
	{
	  Vector<int> next = pos + (*it);
	  if (next.x < 0 || next.x == width || next.y < 0 || next.y == height)
	    continue;
	  //isBlockedDir is needed to catch crossings from land to sea when not thru a port/city
	  if (!flying && isBlockedDir(pos, next))
	    continue;

	  dist = nodes[next.toIndex()].moves;
	  if (dist >= 0 && dist < min)
	    {
	      min = dist;
	      minpos = next;
	    }
	}
      // found the best spot to go to from
      pos = minpos;
      dist = min;
    }

  //calculate when the waypoints show no more movement possible
  guint32 pathcount = 0;
  guint32 moves_left = stack->getMoves();
  for (Path::iterator it = path->begin(); it != path->end(); it++)
    {
      guint32 moves = stack->calculateTileMovementCost(*it);
      if (moves_left >= moves)
	moves_left -= moves;
      else
	break;
      pathcount++;
    }
  path->setMovesExhaustedAtPoint(pathcount);

  debug("...done");

  moves = nodes[idx].moves;
  turns = nodes[idx].turns;

  //change dest back
  nodes[idx] = orig_dest;
  return path;
}
void PathCalculator::dumpNodeMap(Vector<int> dest)
{
  int width = GameMap::getWidth();
  int height = GameMap::getHeight();
  printf ("==2=====================================\n");
  for (int i = 0; i < width; i++)
    {
    for (int j = 0; j < height; j++)
      {
	int moves = nodes[j*width+i].moves;
	if (stack->getPos() == Vector<int>(i,j))
	  printf("0");
	else if (dest == Vector<int>(i, j))
	  printf("1");
	else if (moves == -2)
	  printf ("Z");
	else if (moves == -1)
	  printf ("X");
	else
	  printf("%c", (moves % 26) + 'a');
      }
    printf ("\n");
      }
  printf ("=======================================\n");
  return;
} 
bool PathCalculator::compareNodeMaps(void *map)
{
  bool found = false;
  int width = GameMap::getWidth();
  int height = GameMap::getHeight();
  struct node * othernodes = (struct node *) map;
  for (int i = 0; i < width; i++)
    for (int j = 0; j < height; j++)
      {
	if (othernodes[j*width+i].moves != nodes[j*width+i].moves)
	  {
	  found = true;
	  break;
	  }
      }
  return found;
}
bool PathCalculator::isReachable(Vector<int> pos)
{
  return nodes[pos.toIndex()].moves >= 0;
}

Path *PathCalculator::calculateToCity (City *c, guint32 &moves, guint32 &turns, bool zigzag)
{
  int min_dist = -1;
  Vector<int> shortest = c->getPos();
  bool checkJoin = stack->getOwner() == c->getOwner();

  for (unsigned int i = 0; i < c->getSize(); i++)
    for (unsigned int j = 0; j < c->getSize(); j++)
      {
	if (checkJoin == true)
	  {
	    Stack *other_stack = GameMap::getStack(c->getPos() + Vector<int>(i,j));
	    if (other_stack && GameMap::canJoin(stack,other_stack) == false)
	      continue;
	  }
	int distance = dist (stack->getPos(), c->getPos() + Vector<int>(i, j));
	if (distance > 0)
	  {
	    if (distance < min_dist || min_dist == -1)
	      {
		min_dist = distance;
		shortest = c->getPos() + Vector<int>(i, j);
	      }
	  }
      }
  Path *p = calculate(shortest, moves, turns, zigzag);
  if (p->size() > 0)
    return p;
  delete p;

  //okay.. try really hard
  min_dist = -1;
  for (unsigned int i = 0; i < c->getSize(); i++)
    for (unsigned int j = 0; j < c->getSize(); j++)
      {
	if (checkJoin == true)
	  {
	    Stack *other_stack = GameMap::getStack(c->getPos() + Vector<int>(i,j));
	    if (other_stack && GameMap::canJoin(stack, other_stack) == false)
	      continue;
	  }
	p = calculate(c->getPos() + Vector<int>(i,j), moves, turns, zigzag);
	int dist = (int) moves;
	delete p;
	if (dist > 0)
	  {
	    if (dist < min_dist || min_dist == -1)
	      {
		min_dist = dist;
		shortest = c->getPos() + Vector<int>(i, j);
	      }
	  }
      }
  return calculate(shortest, moves, turns, zigzag);
}

