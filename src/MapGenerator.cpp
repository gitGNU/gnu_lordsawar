// Copyright (C) 2002 Vibhu Rishi
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004 David Barnsdale
// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2004, 2005 Andrea Patton
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
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
#include <iostream>
#include <math.h>  
#include <deque>
#include "vector.h"

#include "MapGenerator.h"
#include "GameMap.h"
#include "stack.h"
#include "path.h"
#include "File.h"
#include "defs.h"
#include "citylist.h"
#include "roadlist.h"
#include "portlist.h"
#include "armysetlist.h"
#include "army.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)
#define offmap(bx,by) (by<0)||(by>=d_height)||(bx<0)||(bx>=d_width)

using namespace std;

//-------------------------------------------------------------------

MapGenerator::MapGenerator()
    //set reasonable default values
    :d_terrain(0), d_building(0), d_pswamp(2), d_pwater(25), d_pforest(3),
    d_phills(5), d_pmountains(5), d_nocities(11), d_notemples(9), d_noruins(20),
    d_nosignposts(30)

{
    d_xdir[0]=0;d_xdir[1]=-1;d_xdir[2]=-1;d_xdir[3]=-1;d_xdir[4]=0;d_xdir[5]=1;d_xdir[6]=1;d_xdir[7]=1;
    d_ydir[0]=-1;d_ydir[1]=-1;d_ydir[2]=0;d_ydir[3]=1;d_ydir[4]=1;d_ydir[5]=1;d_ydir[6]=0;d_ydir[7]=-1;
}

MapGenerator::~MapGenerator()
{
    if (d_terrain)
        delete[] d_terrain;
    if (d_building)
        delete[] d_building;
}

int MapGenerator::setNoCities(int nocities)
{
    if (nocities <= 0)
        return -1;

    int tmp = d_nocities;
    d_nocities = nocities;
    return tmp;
}

int MapGenerator::setNoRuins(int noruins)
{
    if (noruins < 0)
        return -1;

    int tmp = d_noruins;
    d_noruins = noruins;
    return tmp;
}

int MapGenerator::setNoSignposts(int nosignposts)
{
    if (nosignposts < 0)
        return -1;

    int tmp = d_nosignposts;
    d_nosignposts = nosignposts;
    return tmp;
}

int MapGenerator::setNoTemples(int notemples)
{
    if (notemples < 0)
        return -1;

    int tmp = d_notemples;
    d_notemples = notemples;
    return tmp;
}

void MapGenerator::setPercentages(int pwater, int pforest, int pswamp,
                                    int phills, int pmountains)
{
    if ((pswamp < 0) || (pwater < 0) || (pforest < 0) || (phills < 0)
        || (pmountains < 0))
        return;

    if (pswamp + pwater + pforest + phills + pmountains > 100)
        return;

    d_pswamp = pswamp;
    d_pwater = pwater;
    d_pforest = pforest;
    d_phills = phills;
    d_pmountains = pmountains;
}

/** 
 * Generates a random map . The map is stored as a char array of size 
 * 100x100. Each character stands for something. We use :
 * M = mountains
 * h = hills
 * ~ = water
 * $ = forest
 * . = plains
 * _ = swamps
 * C = city/castle
 * r = ruins
 * T = temple
 *   = nothing
 * c = part of city/castle
 * See the TileMapTypes enum at the beginning of the class definition.
 */
void MapGenerator::makeMap(int width, int height, bool roads)
{
    d_width = width;
    d_height = height;

    //initialize terrain and building arrays
    d_terrain = new Tile::Type[width*height];
    d_building = new Maptile::Building[width*height];
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            d_building[i*width + j] = Maptile::NONE;

    cout <<_("Making random map") <<endl;
   
    // create the terrain
    cout <<_("Flatening Plains  ... 10%") <<endl;
    makePlains();
    cout <<_("Raining Water     ... 30%") <<endl;
    makeTerrain(Tile::WATER, d_pwater, true);  
    makeStreamer(Tile::WATER, d_pwater/3, 3);
    cout <<_("Raising Hills     ... 20%") <<endl;
    makeTerrain(Tile::HILLS, d_phills, false);
    cout <<_("Raising Mountains ... 30%") <<endl;
    makeTerrain(Tile::MOUNTAIN, d_pmountains, false);
    makeStreamer(Tile::MOUNTAIN, d_pmountains/3, 3);
    cout <<_("Planting Forest   ... 40%") <<endl;
    makeTerrain(Tile::FOREST, d_pforest, false);
    cout <<_("Watering Swamps   ... 50%") <<endl;
    makeTerrain(Tile::SWAMP, d_pswamp, false);
    cout <<_("Normalizing       ... 60%") <<endl;
    normalize();
           
    // place buildings
    cout <<_("Building Cities   ... 70%") <<endl;
    makeCities(d_nocities);

    if (roads)
      {
	cout <<_("Paving Roads      ... 75%") <<endl;
	makeRoads();
      }

    cout <<_("Ruining Ruins     ... 80%") <<endl;
    makeBuildings(Maptile::RUIN,d_noruins);
    cout <<_("Raising Signs     ... 88%") <<endl;
    makeBuildings(Maptile::SIGNPOST,d_nosignposts);
    cout <<_("Spawning temples  ... 90%") <<endl;
    makeBuildings(Maptile::TEMPLE,d_notemples);
    cout <<_("Done making map   ... 100%") <<endl;
}

const Tile::Type* MapGenerator::getMap(int& width, int& height) const
{
    width = d_width;
    height = d_height;
    return d_terrain;
}

const Maptile::Building* MapGenerator::getBuildings(int& width, int& height) const
{
    width = d_width;
    height = d_height;
    return d_building;
}

void MapGenerator::makePlains()
{
    for(int j = 0; j < d_height; j++)
        for(int i = 0; i < d_width; i++)
            d_terrain[j*d_width + i] = Tile::GRASS;
}

/**
 * Makes Terrains.
 * The algorithm is as follows :
 * 1. Find a random starting location
 * 2. chose a random direction , if x is the starting location, the direction
 *    can be from 0-7 as follows :
 *    +-+-+-+
 *    |0|1|2|
 *    +-+-+-+
 *    |7|x|3|
 *    +-+-+-+
 *    |6|5|4|
 *    +-+-+-+
 * 3. Check if there is some other terrain there ('.' = plains is okay)
 * 4. Move one tile in this direction, mutate the tile and continue with 2
 * 5. If we hit a dead end (all directions non-grass), continue with 1
 */
void MapGenerator::makeTerrain(Tile::Type t, int percent, bool contin)
{
    // calculate the total number of tiles for this terrain
    int terrain = d_width*d_height*percent / 100;
    int placed = 0;  // total of current terrain placed so far 
    
    while(placed != terrain)
    {
        // find a random starting position
        int x = rand() % d_width;
        int y = rand() % d_height;
        if (seekPlain(x, y) == false)
          continue;


        // now go on until we hit a dead end
        while (placed < terrain)
        {
            // if we are on grass, modify this tile first
            if (d_terrain[y*d_width + x] == Tile::GRASS)
            {
                d_terrain[y*d_width + x] = t;
                placed++;
                continue;
            }
            
            // from a random direction, check all directions for further progress
            int loop = 0;
            for (int dir = rand()%8; loop < 8; loop++, dir = (dir+1)%8)
            {
                int tmpx = x + d_xdir[dir];
                int tmpy = y + d_ydir[dir];
                
                // reject invalid data
                if (offmap(tmpx, tmpy) || d_terrain[tmpy*d_width + tmpx] != Tile::GRASS)
                    continue;

                // else move our region of interest by one tile
                x = tmpx;
                y = tmpy;
                d_terrain[y*d_width + x] = t;
                placed++;
                break;
            }

            // we have hit a dead end, i.e. we are only surrounded by non-grass
            // tiles. Either choose a new random starting point or, if contin
            // is set, find a close one via seekPlain()
            if (loop == 8)
            {
                if (contin)
                  {
                    if (seekPlain(x, y) == false)
                      continue;
                  }
                else
                    break;
            }
        }
    }
}

/**
 * Makes streaming terrain features.
 * The algorithm is as follows :
 * 1. Find a random starting location
 * 2. chose a random direction , if x is the starting location, the direction
 *    can be from 0-7 as follows :
 *    +-+-+-+
 *    |0|1|2|
 *    +-+-+-+
 *    |7|x|3|
 *    +-+-+-+
 *    |6|5|4|
 *    +-+-+-+
 * 3. Drop the tile and move in the direction
 * 4. Change the direction every so often
 * 5. Keep doing this until we go off the map or we've dropped enough tiles
 *
 */
void MapGenerator::makeStreamer(Tile::Type t, int percent, int thick)
{
    // calculate the total number of tiles for this terrain
    int terrain = d_width*d_height*percent / 100;
    int placed = 0;  // total of current terrain placed so far 
    int dir;
    int i;
    
    while(placed < terrain)
    {
        // find a random starting position
        int x = rand() % d_width;
        int y = rand() % d_height;
        if (seekPlain(x, y) == false)
          continue;
        dir = rand()%8; // pick a random direction
        // now go on until we hit a dead end
        while (placed < terrain)
        {
            // if we are on grass, modify this tile first
            if (d_terrain[y*d_width + x] == Tile::GRASS)
            {
                d_terrain[y*d_width + x] = t;
                placed++;
                continue;
            }
            
            if (rand() % 2 == 0)
              {
                if (rand() % 2 == 0)
                  {
                    dir++;
                    if (dir > 7)
                      dir = 0;
                  }
                else
                  {
                    dir--;
                    if (dir < 0)
                      dir = 7;
                  }
            }

            {
                int tmpx = x + d_xdir[dir];
                int tmpy = y + d_ydir[dir];
                
                // reject invalid data
                if (offmap(tmpx, tmpy))// || d_terrain[tmpy*d_width + tmpx] != Tile::GRASS)
                    break;

                // else move our region of interest by one tile
                x = tmpx;
                y = tmpy;
                d_terrain[y*d_width + x] = t;
                placed++;
                switch (dir)
                  {
                    case 1: case 2: case 6: case 5:
                      {
                        for (i = 1; i <= thick ; i++)
                          {
                            if (offmap(x+i, y))
                              continue;
                            d_terrain[y*d_width + x+i] = t;
                            placed++;
                          }
                      }
                      break;
                    case 7: case 3: case 0: case 4:
                      {
                        for (i = 1; i <= thick; i++)
                          {
                            if (offmap(x, y+i))
                              continue;
                            d_terrain[(y+i)*d_width + x] = t;
                            placed++;
                          }
                      }
                      break;
                  }
            }

        }
    }

}

bool MapGenerator::seekPlain(int& x, int& y)
{
    int orig_x = x;
    int orig_y = y;
    /* The algorithm here uses a large list of tiles to be checked.
     * In the beginning, it is filled with the tiles surrounding the starting
     * tile. Each tile is then checked if it contains grass. If not, all
     * surrounding tiles are added to the list (we have to take some care to
     * avoid infinite loops).
     *
     * Another way of describing it: The algorithm checks all tiles around the
     * position for the existence of grass. It then checks the tiles in larger
     * and larger circles around the position until it finds a grass tile.
     */
    if (d_terrain[y*d_width + x] == Tile::GRASS)
        return true;

    std::deque<Vector<int> > tiles;
    
    // fill the list with initial values; the rand is there to avoid a bias
    // (i.e. prefer a certain direction)
    for (int dir = rand() % 8, i = 0; i < 8; i++, dir = (dir+1)%8)
        tiles.push_back(Vector<int>(x + d_xdir[dir], y + d_ydir[dir]));
    
    // now loop until all tiles were checked (should hardly happen)
    while (!tiles.empty())
    {
        Vector<int> p = tiles.front();
        tiles.pop_front();

        if (offmap(p.x, p.y))
            continue;
        
        // if we have found a patch of grass, we are lucky and return
        if (d_terrain[p.y*d_width + p.x] == Tile::GRASS)
        {
            x = p.x;
            y = p.y;
            return true;
        }
        
        // not found? Well, then append the surrounding tiles. To avoid double-
        // checking (and therefore an infinite loop), only certain surrounding
        // tiles are appended. See the following sketch:
        //
        //              ebbbe
        //              b   b
        //              b x b
        //              b   b
        //              ebbbe
        //
        // This is a circle of radius 2 around the position x. In the case of border
        // tiles, only the tile directly outerwards is appended, the edge tiles 
        // (which can be identified by distx == disty) append two new border and
        // one new edge tile.
        int dx = p.x - x;
        int dy = p.y - y;
        
        // edge tile; append three new tiles
        if (abs(dx) == abs(dy))
        {
            int newx = p.x - 1;
            int newy = p.y - 1;

            if (dx > 0)
                newx = p.x + 1;
            if (dy > 0)
                newy = p.y + 1;
            
            tiles.push_back(Vector<int>(newx, newy));
            tiles.push_back(Vector<int>(newx, p.y));
            tiles.push_back(Vector<int>(p.x, newy));
        }
        else
        {
            if (abs(dx) > abs(dy) && dx > 0)        // right border
                tiles.push_back(Vector<int>(p.x + 1, p.y));
            else if (abs(dx) > abs(dy) && dx < 0)   //left border
                tiles.push_back(Vector<int>(p.x - 1, p.y));
            else if (abs(dx) < abs(dy) && dy > 0)   // top border
                tiles.push_back(Vector<int>(p.x, p.y + 1));
            else if (abs(dx) < abs(dy) && dy < 0)   // lower border
                tiles.push_back(Vector<int>(p.x, p.y - 1));
        }
    }

    // if this line is ever reached, we haven't found a free grass tile
    // (should only happen under really exceptional circumstances)
    x = orig_x;
    y = orig_y;
    return false;
}

void MapGenerator::makeCities(int cities)
{

    int city_count = 0;
    int iterations = 0;
    
    // place the cities
    while(city_count < cities)
    {
        int x = rand()%(d_width-2);
        int y = rand()%(d_height-2);
        if((d_terrain[y*d_width +x] == Tile::WATER
            || d_terrain[y*d_width + x+1] == Tile::WATER 
            || d_terrain[(y+1)*d_width + x] == Tile::WATER
            || d_terrain[(y+1)*d_width + x+1] == Tile::WATER
            || d_terrain[y*d_width +x] == Tile::MOUNTAIN
            || d_terrain[y*d_width + x+1] == Tile::MOUNTAIN
            || d_terrain[(y+1)*d_width + x] == Tile::MOUNTAIN
            || d_terrain[(y+1)*d_width + x+1] == Tile::MOUNTAIN)
            && (iterations < 1000))
        {
            iterations++;
            continue;
        }
        
        // check if we can put the building
        if ((!canPutBuilding(x, y) || !canPutBuilding(x + 1, y) ||
            !canPutBuilding(x, y + 1) || !canPutBuilding(x + 1,y + 1)) &&
            ((iterations < 1000)))
        {
            iterations++;
            continue;
        }
        
        putCity(x, y, city_count);
        iterations=0;
    }
    
}

bool MapGenerator::canPutCity(int x,int y)
{
    if ((canPutBuilding(x,y))&&(canPutBuilding(x+1,y))&&(canPutBuilding(x,y+1))&&(canPutBuilding(x+1,y+1)))
        return true;
    else
        return false;
}

void MapGenerator::putCity(int x, int y, int& city_count)
{
        d_building[y*d_width + x] = Maptile::CITY;

        //cities shall only sit on grass tiles
        d_terrain[y*d_width + x] = Tile::GRASS;
        d_terrain[y*d_width + x+1]     = Tile::GRASS;
        d_terrain[(y+1)*d_width + x]   = Tile::GRASS;
        d_terrain[(y+1)*d_width + x+1] = Tile::GRASS;

        city_count++;
}

void MapGenerator::makeBuildings(Maptile::Building b, int building)
{
    int i, j, x, y;
    int iterations = 10;
    bool found_place = false;

   //If number of iterations is smaller 10, look for a suitable
   //place. If this number is exceeded, place the temple on an
   //island if neccessary.
    for (i = 0; i < building; i++)
    {        
	for (j = 0; j < iterations; j++)
	{
             x = rand()%d_width;
             y = rand()%d_height;
        
             if (canPutBuilding(x, y) == true)
             {
		 found_place = true;
		 break;
             }
	}

	if (found_place == true)
	{
             d_terrain[y*d_width + x] = Tile::GRASS;
             d_building[y*d_width + x] = b;
	     found_place = false;
	}
    }
}

/** 
 * canPutBuilding
 * Checks if we can put a building at the specified place. 
 * If we are on a square with water, we cannot put it
 * nor if it is too close.
 * Checks for neighboring buildings yet sometimes these
 * are bang next to each other - why?
 * UL: Propably too many tries, then the algorithm forces the
 * building to be placed.
 */

bool MapGenerator::canPutBuilding(int x,int y)
{
    // if the building is on water or mountains, return false
    if (d_terrain[y*d_width +x] != Tile::GRASS )
        return false;
        
    //if the building is close to the map boundaries, return false
    if ((x < 3) || (x > (d_width-3)) || (y < 3) || (y > (d_height-3)))
        return false;
        
    //if there is another building too close, return false
    for (int locx = x-3; locx <= x+3; locx++)
        for (int locy = y-3; locy <= y+3; locy++)
        {
            if (offmap(locx, locy))
                continue;
            if (d_building[locy*d_width + locx] != Maptile::NONE)
                return false;
        }  
    // everything okay here! return true
    return true;
    
}

bool MapGenerator::tryToPlaceCity(int px,int py ,int& city_count)
{
    // first, try to place the city at the given location
    if (canPutCity(px, py))
    { 
        putCity(px, py, city_count);
        return true;
    } 
    
    // else try all surrounding squares
    for (int dir = 0; dir < 8; dir++)
        if (canPutCity(px + d_xdir[dir], py + d_ydir[dir]))
        {
            putCity(px + d_xdir[dir], py + d_ydir[dir], city_count);
            return true;
        }

    return false;                   
}

void MapGenerator::normalize()
{
    std::map<Uint32,Uint32> ajacentTer;
    Tile::Type curTer=Tile::NONE, ajTer=Tile::NONE;
    
    // Go through every tile bar the outer edge
    for(int globy = 1; globy < (d_height-2); globy++)
        for(int globx = 1; globx < (d_width-2); globx++)
        {
            curTer = d_terrain[globy*d_width + globx];

            // reset all counters
            ajacentTer[Tile::GRASS] = 0;
            ajacentTer[Tile::WATER] = 0;
            ajacentTer[Tile::FOREST] = 0;
            ajacentTer[Tile::HILLS] = 0;
            ajacentTer[Tile::MOUNTAIN] = 0;
            ajacentTer[Tile::SWAMP] = 0;

            // count how many neighbours of each type we have
            for(int locx = globx - 1; locx <= globx+1; locx++)
                for(int locy = globy - 1; locy <= globy+1; locy++)
                { 
                     ajTer = d_terrain[locy*d_width +locx];
                     ajacentTer[ajTer] += 1;
                }

            // we have counted our own tile as well
            ajacentTer[curTer] -= 1;

            if (curTer==Tile::WATER) // For the moment only water is normalized
            {
                if (ajacentTer[curTer]==0)
                    d_terrain[globy*d_width +globx] = Tile::GRASS;
                else if ((ajacentTer[curTer]==1) && (rand()%100 < 95 ))
                    d_terrain[globy*d_width +globx] = Tile::GRASS;
                else if ((ajacentTer[curTer]==2) && (rand()%100 < 70 ))
                    d_terrain[globy*d_width +globx] = Tile::GRASS;
                else if ((ajacentTer[curTer]==3) && (rand()%100 < 40 ))
                    d_terrain[globy*d_width +globx] = Tile::GRASS;
            }
            else 
            {
                if (ajacentTer[Tile::WATER]==8)
                    d_terrain[globy*d_width +globx] = Tile::WATER;
                else if ((ajacentTer[Tile::WATER]==7) && (rand()%100 < 70 ))
                    d_terrain[globy*d_width +globx] = Tile::WATER;
                else if ((ajacentTer[Tile::WATER]==6) && (rand()%100 < 40 ))
                    d_terrain[globy*d_width +globx] = Tile::WATER;
             }
        }
}

void MapGenerator::calculateBlockedAvenue(int x, int y)
{
  for (int i = x - 1; i <= x + 1; i++)
    {
      for (int j = y - 1; j <= y + 1; j++)
	{
	  if (i < 0 || i >= d_width)
	    continue;
	  if (j < 0 || j >= d_height)
	    continue;
	  GameMap::getInstance()->calculateBlockedAvenue(i, j);
	}
    }
}
bool MapGenerator::placePort(int x, int y)
{
  //if (Citylist::getInstance()->getNearestCity(Vector<int>(x, y), 2) == NULL)
    {
      if (d_building[y*d_width + x] == 0)
	{
	  Portlist *pl = Portlist::getInstance();
	  d_building[y*d_width + x] = Maptile::PORT;
	  pl->push_back(Port(Vector<int>(x, y)));
	  calculateBlockedAvenue(x, y);
	  return true;
	}
    }
  return false;
}

bool MapGenerator::makeRoad(Vector<int> src, Vector<int>dest)
{
  return makeRoad (src.x, src.y, dest.x, dest.y);
}

bool MapGenerator::makeRoad(int src_x, int src_y, int dest_x, int dest_y)
{
  bool retval = true;
  GameMap *gm = GameMap::getInstance();
  Vector<int> src(src_x, src_y);
  Vector<int> dest(dest_x, dest_y);

  Path *p = new Path();
  Stack s(NULL, src);

  Armysetlist *al = Armysetlist::getInstance();
  Uint32 armyset = al->getArmysetId("Default");
  const Army* basearmy = Armysetlist::getInstance()->getArmy(armyset, 1);
  Army *a = new Army(*basearmy, NULL);
  s.push_back(a);
  // try to get there with a scout
  Uint32 moves = p->calculate(&s, dest, false);

  if (moves != 0)
    {
      Roadlist *rl = Roadlist::getInstance();
      for (Path::iterator it = p->begin(); it != p->end(); it++)
	{
	  int x = (**it).x;
	  int y = (**it).y;
	  if (gm->getTile(x, y)->getMaptileType() == Tile::WATER)
	    {
	      retval = false;
	      break;
	    }
	  Citylist *cl = Citylist::getInstance();
	  if (cl->getObjectAt(x, y) == NULL)
	    {
	      if (d_building[y*d_width + x] == 0)
		{
		  d_building[y*d_width + x] = Maptile::ROAD;
		  rl->push_back(Road(Vector<int>(x, y)));
		  calculateBlockedAvenue(x, y);
		}
	    }
	}

    }
  else
    retval = false;
  delete p;

  return retval;
}

bool MapGenerator::isAccessible(Vector<int> src, Vector<int> dest)
{
  return isAccessible (src.x, src.y, dest.x, dest.y);
}

bool MapGenerator::isAccessible (int src_x, int src_y, int dest_x, int dest_y)
{
  bool retval = true;
  Vector<int> src(src_x, src_y);
  Vector<int> dest(dest_x, dest_y);

  Path *p = new Path();
  Stack s(NULL, src);

  Armysetlist *al = Armysetlist::getInstance();
  Uint32 armyset = al->getArmysetId("Default");
  const Army* basearmy = Armysetlist::getInstance()->getArmy(armyset, 1);
  Army *a = new Army(*basearmy, NULL);
  s.push_back(a);
  // try to get there with a scout
  if (p->calculate(&s, dest, true) == 0)
    retval = false;
  delete p;

  return retval;
}

bool MapGenerator::makeAccessible(Vector<int> src, Vector<int> dest)
{
  return makeAccessible(src.x, src.y, dest.x, dest.y);
}

bool MapGenerator::makeAccessible(int src_x, int src_y, int dest_x, int dest_y)
{
  bool retval = true;
  GameMap *gm = GameMap::getInstance();
  Vector<int> src(src_x, src_y);
  Vector<int> dest(dest_x, dest_y);

  Path *p = new Path();
  Stack s(NULL, src);

  Armysetlist *al = Armysetlist::getInstance();
  Uint32 armyset = al->getArmysetId("Default");
  const Army* basearmy = Armysetlist::getInstance()->getArmy(armyset, 16);
  Army *a = new Army(*basearmy, NULL);
  s.push_back(a);
  // try to get there with a giant bat
  Uint32 moves = p->calculate(&s, dest, false);

  if (moves != 0)
    {
      Path::iterator it = p->begin();
      Path::iterator nextit = it;
      nextit++;
      for ( ; nextit != p->end(); it++, nextit++)
	{
	  int x = (**it).x;
	  int y = (**it).y;
	  int nextx = (**nextit).x;
	  int nexty = (**nextit).y;
	  if (d_terrain[y*d_width + x] == Tile::MOUNTAIN)
	    {
	      d_terrain[y*d_width +x] = Tile::GRASS;
	      Maptile *t = new Maptile(gm->getTileset(), 
				       x, y, Tile::GRASS, NULL);
	      gm->setTile(x, y, t);
	      calculateBlockedAvenue(x, y);
	    }
	  if (d_terrain[y*d_width + x] == Tile::WATER &&
	      d_terrain[nexty*d_width + nextx] != Tile::WATER)
	    {
	      if (placePort(x, y) == true)
		{
		  if (isAccessible(x, y, dest.x, dest.y))
		    {
		      retval = true;
		      break;
		    }
		}
	    }
	  else if (d_terrain[y*d_width + x] != Tile::WATER &&
		   d_terrain[nexty*d_width + nextx] == Tile::WATER)
	    {
	      if (placePort(nextx, nexty) == true)
		{
		  if (isAccessible(nextx, nexty, dest.x, dest.y))
		    {
		      retval = true;
		      break;
		    }
		}
	    }

	}
    }
  else
    retval = false;
  delete p;

  return retval;
}
void MapGenerator::makeRoads()
{
  GameMap::deleteInstance();
  Citylist::deleteInstance();
  Roadlist::deleteInstance();
  Portlist::deleteInstance();

  GameMap::setWidth(d_width);
  GameMap::setHeight(d_height);
  GameMap::getInstance("default", "default", "default")->fill(this);
  Roadlist::getInstance();
  //the game map class smooths the map, so let's take what it smoothed.
  for (int y = 0; y < d_height; y++)
    for (int x = 0; x < d_width; x++)
      d_terrain[y*d_width + x] = 
	GameMap::getInstance()->getTile(x, y)->getMaptileType();

  for (int y = 0; y < d_height; y++)
    for (int x = 0; x < d_width; x++)
      {
	if (d_building[y*d_width + x] == Maptile::CITY)
	  Citylist::getInstance()->push_back(City(Vector<int>(x,y)));
      }
  GameMap::getInstance()->calculateBlockedAvenues();

  Citylist *cl = Citylist::getInstance();
  for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
    {
      if (rand() % 2 == 0)
	continue;
      City *c = cl->getNearestCityPast((*it).getPos(), 13);
      Vector<int> dest = c->getPos();
      Vector<int> src = (*it).getPos();
      //does it already have a road going to it?
      if (Roadlist::getInstance()->getNearestObjectBefore(dest, 
							  c->getSize() + 1))
	continue;

      makeRoad(src, dest);
    }

  //make all cities accessible by allowing movement to a central city
  Vector<int> pos = cl->calculateCenterOfTerritory(NULL);
  City *center = cl->getNearestCity(pos);
  for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
    {
      if (center == &*it)
	continue;
      if (isAccessible(center->getPos(), (*it).getPos()) == false)
	makeAccessible(center->getPos(), (*it).getPos());
    }

  Roadlist::deleteInstance();
  GameMap::deleteInstance();
  Citylist::deleteInstance();
  Portlist::deleteInstance();
}
