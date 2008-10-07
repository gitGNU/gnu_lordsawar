// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2004 John Farrell
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
// Copyright (C) 2008 Ole Laursen
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

#include <stdlib.h>
#include <sstream>
#include <queue>

#include "path.h"
#include "army.h"
#include "GameMap.h"
#include "Location.h"
#include "citylist.h"
#include "city.h"
#include "stacklist.h"
#include "xmlhelper.h"
#include "stack.h"

std::string Path::d_tag = "path";

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

Path::Path()
{
  d_moves_exhausted_at_point = 0;
}

Path::Path(XML_Helper* helper)
{
    int i;
    std::istringstream sx, sy;
    std::string s;

    helper->getData(d_moves_exhausted_at_point, "moves_exhausted_at_point");
    helper->getData(i, "size");

    helper->getData(s, "x");
    sx.str(s);
    helper->getData(s, "y");
    sy.str(s);

    for (; i > 0; i--)
    {
        Vector<int> *p = new Vector<int>;

        sx >> p->x;
        sy >> p->y;
        push_back(p);
    }
}

Path::~Path()
{
    flClear();
}

bool Path::save(XML_Helper* helper) const
{
    bool retval = true;

    std::stringstream sx, sy;
    for (const_iterator it = begin(); it != end(); it++)
    {
        sx <<(*it)->x <<" ";
        sy <<(*it)->y <<" ";
    }

    retval &= helper->openTag(Path::d_tag);
    retval &= helper->saveData("size", size());
    retval &= helper->saveData("moves_exhausted_at_point", 
                               d_moves_exhausted_at_point);
    retval &= helper->saveData("x", sx.str());
    retval &= helper->saveData("y", sy.str());
    retval &= helper->closeTag();

    return retval;
}

Path::iterator Path::flErase(Path::iterator it)
{
    delete (*it);
    return erase (it);
}

void Path::flClear()
{
    for (iterator it = begin(); it != end(); it++)
    {
        delete (*it);
    }
    return clear();
}

bool Path::canMoveThere(const Stack* s, Vector<int> dest)
{
  d_bonus=s->calculateMoveBonus();
  Vector<int> pos = s->getPos();
  bool flying = s->isFlying();
  if (flying && isBlockedDir(pos.x, pos.y, dest.x, dest.y) &&
      GameMap::getInstance()->getTile(dest.x, dest.y)->getMaptileType()
      == Tile::VOID)
    return false;
  if (isBlocked(s, pos.x, pos.y, dest.x, dest.y))
    return false;
  if (isBlockedDir(pos.x, pos.y, dest.x, dest.y) && !flying)
    return false;
  return true;
}

bool Path::checkPath(Stack* s)
{
    if (empty())
        return true;
    
    Vector<int> dest = **(rbegin());
    debug("check_path() " << dest.x << "," << dest.y)
    bool blocked = false;
    d_bonus=s->calculateMoveBonus();

    for (iterator it = begin(); it != end(); it++)
    {
        if (isBlocked(s, (*it)->x, (*it)->y, dest.x, dest.y))
        {
            blocked = true;
            break;
        }
    }

    if (blocked)
    {
        debug("recalculating blocked path dest = " << dest.x << "," << dest.y)
        flClear();
        blocked = calculate(s, dest);
    }

    return !blocked;
}

//this is used to update the moves_exhausted_at_point variable
void Path::recalculate (Stack* s)
{
  if (size() == 0)
    return;

  // be careful to not go into cities that are now owned by the enemy
  reverse_iterator it = rbegin();
  for (; it != rend(); it++)
    {
      Vector<int> dest = **it;
      City *c = Citylist::getInstance()->getObjectAt(dest.x, dest.y);
      if (c && c->getOwner() != s->getOwner())
	continue;
      else
	break;
    }
  if (it == rend())
    {
      //well, it looks like all of our points were in enemy cities
      setMovesExhaustedAtPoint(0);
      flClear();
    }
  else
    {
      Vector<int> dest = **it;
      calculate(s, dest);
    }
  return;
}

Uint32 Path::calculateToCity (Stack *s, Location *c, bool zigzag)
{
  int min_dist = -1;
  Vector<int> shortest = c->getPos();

  for (unsigned int i = 0; i < c->getSize(); i++)
    for (unsigned int j = 0; j < c->getSize(); j++)
      {
	int distance = dist (s->getPos(), c->getPos() + Vector<int>(i, j));
	if (distance > 0)
	  {
	    if (distance < min_dist || min_dist == -1)
	      {
		min_dist = distance;
		shortest = c->getPos() + Vector<int>(i, j);
	      }
	  }
      }
  int mp = calculate(s, shortest, zigzag);
  if (mp <= 0)
    {
      //okay.. try really hard
      min_dist = -1;
      for (unsigned int i = 0; i < c->getSize(); i++)
	for (unsigned int j = 0; j < c->getSize(); j++)
	  {
	    int dist = calculate(s, c->getPos() + Vector<int>(i, j), zigzag);
	    if (dist > 0)
	      {
		if (dist < min_dist || min_dist == -1)
		  {
		    min_dist = dist;
		    shortest = c->getPos() + Vector<int>(i, j);
		  }
	      }
	  }
      mp = calculate(s, shortest, zigzag);
    }
  return mp;
}

Uint32 Path::calculate (Stack* s, Vector<int> dest, bool zigzag)
{
  int mp;
  Vector<int> start = s->getPos();
  debug("path from "<<start.x<<","<<start.y<<" to "<<dest.x<<","<<dest.y)

    flClear();
  //can we get there with one move?
  if (dist(s->getPos(), dest) == 1 && canMoveThere(s, dest) == true)
    {
      Vector<int> *p = new Vector<int>(dest);
      push_back(p);
      if (s->canMove() == true)
	setMovesExhaustedAtPoint(1);
      else
	setMovesExhaustedAtPoint(0);
      return 1;
    }
  int width = GameMap::getWidth();
  int height = GameMap::getHeight();
  d_bonus = s->calculateMoveBonus();
  //if (isBlocked(s, dest.x, dest.y, dest.x, dest.y))
  //{
  //s->setMovesExhaustedAtPoint(0);
  //return 0;
  //}

  // Some notes concerning the path finding algorithm. The algorithm
  // uses two different lists. There is a distance array, which contains how
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
  int length = width*height;
  int distance[length];
  std::queue<Vector<int> > process;
  bool flying = s->isFlying();

  // initial filling of the distance vector
  for (int i = 0; i < width*height; i++)
    {
      // -1 means don't know yet
      // -2 means can't go there at all
      // 0 or more is number of movement points needed to get there
      distance[i] = -1;
      if (isBlocked(s, i % width, i/width, dest.x, dest.y))
	distance[i] = -2;
    }
  distance[start.y*width+start.x] = 0;

  // now the main loop
  process.push(Vector<int>(start.x, start.y));
  while (!process.empty())
    {
      Vector<int> pos = process.front();
      process.pop();                          // remove the first item

      int dxy = distance[pos.y*width+pos.x];   // always >= 0

      for (int sx = pos.x-1; sx <= pos.x+1; sx++)
	{
	  if (sx < 0 || sx >= width)
	    continue;

	  for (int sy = pos.y-1; sy <= pos.y+1; sy++)
	    {
	      if (sy < 0 || sy >= height)
		continue;

	      if (sx == pos.x && sy == pos.y)
		continue;

	      //am i blocked from entering sx,sy from pos?
	      if (!flying && isBlockedDir(pos.x, pos.y, sx, sy))
		continue;
	      //flyers can't go through the void
	      if (flying && isBlockedDir(pos.x, pos.y, sx, sy) &&
		  GameMap::getInstance()->getTile(sx, sy)->getMaptileType()
		  == Tile::VOID)
		continue;

	      int dsxy = distance[sy*width+sx];
	      if (dsxy < -1)
		continue; // can't move there anyway

	      if (zigzag == false)
		{
		  Vector<int> diff = pos - Vector<int>(sx, sy);
		  if (diff.x && diff.y)
		    continue;
		}
	      int newDsxy = dxy;
	      mp = pointsToMoveTo(s, pos.x, pos.y, sx, sy);
	      if (mp >= 0)
		newDsxy += mp;

	      if (dsxy == -1 || dsxy > newDsxy)
		{
		  distance[sy*width+sx] = newDsxy;

		  // append the item to the queue
		  process.push(Vector<int>(sx, sy));
		}
	    }
	}
    }

  // The distance array is now completely populated.
  // What we have to do now is find the shortest path to the destination.
  // We do that by starting at the destination and moving at each step to
  // the neighbour closest to the start.

  int dist = distance[dest.y * width + dest.x];
  if (dist < 0)
    {
      setMovesExhaustedAtPoint(0);
      return 0;
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

  int x = dest.x;
  int y = dest.y;

  while (dist > 0)
    {
      Vector<int> *p = new Vector<int>(x,y);
      push_front(p);

      int min = dist;
      int rx = x;
      int ry = y;
      for (std::list<Vector<int> >::iterator it = diffs.begin();
	   it != diffs.end(); it++)
	{
	  int newx = x + (*it).x;//diffs[i][0];
	  int newy = y + (*it).y;//diffs[i][1];
	  if (newx < 0 || newx == width || newy < 0 || newy == height)
	    continue;
	  //isBlockedDir is needed to catch crossings from land to sea when not thru a port/city
	  if (!flying && isBlockedDir(x, y, newx, newy))
	    continue;

	  dist = distance[newy*width+newx];
	  if (dist >= 0 && dist < min)
	    {
	      rx = newx;
	      ry = newy;
	      min = dist;
	    }
	}
      // found the best spot to go to from
      x = rx;
      y = ry;
      dist = min;
    }

  //calculate when the waypoints show no more movement possible
  Uint32 pathcount = 0;
  Uint32 moves_left = s->getGroupMoves();
  for (iterator it = begin(); it != end(); it++)
    {
      Uint32 moves = s->calculateTileMovementCost(**it);
      if (moves_left >= moves)
	moves_left -= moves;
      else
	break;
      pathcount++;
    }
  setMovesExhaustedAtPoint(pathcount);

  debug("...done");
  return distance[dest.y * width + dest.x];
}

void Path::eraseFirstPoint()
{
  flErase(begin());

  if (getMovesExhaustedAtPoint() > 0)
    setMovesExhaustedAtPoint(getMovesExhaustedAtPoint()-1);
}



//am i blocked from entering destx,desty from x,y when i'm not flying?
bool Path::isBlockedDir(int x, int y, int destx, int desty) const
{
  int diffx = destx - x;
  int diffy = desty - y;
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
      return GameMap::getInstance()->getTile(x, y)->d_blocked[idx];
    }

  return false;
}

bool Path::isBlocked(const Stack* s, int x, int y, int destx, int desty) const
{
  const Maptile* tile = GameMap::getInstance()->getTile(x,y);

  // Return true on every condition which may prevent the stack from
  // entering the tile, which are...

  // TODO: you can extract quite some amount of time here with a clever
  // search algorithm for stacklist
  Stack* target = Stacklist::getObjectAt(x,y);
  if (target)
    {
      // ...enemy stacks which stand in the way...
      if ((s->getOwner() != target->getOwner())
	  && ((x != destx) || (y != desty)))
	return true;

      //the computer walks around any too-big stacks of it's own
      //unless it's the final destination.
      if (s->getOwner() == target->getOwner()
	  && s->canJoin(target) == false && 
	  s->getOwner()->getType() != Player::HUMAN) 
	{
	  if (Vector<int>(destx, desty) != Vector<int>(x,y))
	    return true;
	}

    }

  //...enemy cities
  // saves some computation time here
  if (tile->getBuilding() == Maptile::CITY)
    {
      City* c = Citylist::getInstance()->getObjectAt(x,y);
      if (c && (c->getOwner() != s->getOwner())
	  && ((x != destx) || (y != desty)))
	return true;
    }

  //no obstacles??? well, then...
  return false;
}

int Path::pointsToMoveTo(const Stack *s, int x, int y, int destx, int desty) const
{
  Uint32 moves;
  const Maptile* tile = GameMap::getInstance()->getTile(destx,desty);

  if (x == destx && y == desty) //probably shouldn't happen
    return 0;

  moves = tile->getMoves();

  // does everything in the stack have a bonus to move onto this square?
  if (tile->getMaptileType() & d_bonus && moves != 1)
    return 2;

  return moves;
}

// End of file
