//  This program is free software; you can redistribute it d_terrainand/or modify
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

#include <stdlib.h>
#include <iostream>
#include <math.h>  
#include <deque>

#include "MapGenerator.h"
#include "File.h"
#include "defs.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)
#define offmap(x,y) (y<0)||(y>=d_height)||(x<0)||(x>=d_width)

const int maxlink = 300;

using namespace std;


/* This struct saves data about ports to be placed. The data it contains
 * are the id of the land or sea which the port belongs to, if the port has
 * already been placed and a possible location for the port.
 */

struct portneeded
{
    portneeded();
    ~portneeded();

    int landid;
    int seaid;
    bool hasPort;
    int x,y;
};

portneeded::portneeded()
    :landid(0), seaid(0), hasPort(false), x(-1), y(0)
{
}

portneeded::~portneeded()
{}

//-------------------------------------------------------------------

MapGenerator::MapGenerator()
    //set reasonable default values
    :d_terrain(0), d_building(0), d_l_mass(0), d_pswamp(2), d_pwater(25), d_pforest(3),
    d_phills(5), d_pmountains(5), d_nocities(11), d_notemples(9), d_noruins(20),
    d_nosignposts(30), d_nostones(30)

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
    if (d_l_mass)
        delete[] d_l_mass;    
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

int MapGenerator::setNoStones (int nostones)
{
    if (nostones < 0)
        return -1;

    int tmp = d_nostones;
    d_nostones = nostones;
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
void MapGenerator::makeMap(int width, int height)
{
    d_width = width;
    d_height = height;

    //initialize terrain and building arrays
    d_terrain = new Tile::Type[width*height];
    d_building = new Maptile::Building[width*height];
    d_l_mass = new int [width*height];
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            d_building[i*width + j] = Maptile::NONE;

    cout <<_("Making random map") <<endl;
   
    // create the terrain
    cout <<_("Flatening Plains  ... 10%") <<endl;
    makePlains();
    cout <<_("Raining Water     ... 30%") <<endl;
    makeTerrain(Tile::WATER, d_pwater, true);  
    cout <<_("Raising Hills     ... 20%") <<endl;
    makeTerrain(Tile::HILLS, d_phills, false);
    cout <<_("Raising Mountains ... 30%") <<endl;
    makeTerrain(Tile::MOUNTAIN, d_pmountains, false);
    cout <<_("Planting Forest   ... 40%") <<endl;
    makeTerrain(Tile::FOREST, d_pforest, false);
    cout <<_("Watering Swamps   ... 50%") <<endl;
    makeTerrain(Tile::SWAMP, d_pswamp, false);
    cout <<_("Normalizing       ... 60%") <<endl;
    normalize();
           
    // analyse the location of continents/seas
    int nmrofSeas=0, nmrLands=0;
    continents(nmrLands, nmrofSeas);
    findRoutes();

    // place buildings
    cout <<_("Building Cities   ... 70%") <<endl;
    makeCities(d_nocities);
    cout <<_("Ruining Ruins     ... 80%") <<endl;
    makeBuildings(Maptile::RUIN,d_noruins);
    cout <<_("Raising Signs     ... 88%") <<endl;
    makeBuildings(Maptile::SIGNPOST,d_nosignposts);
    cout <<_("Placing Stones    ... 89%") <<endl;
    makeBuildings(Maptile::STONE,d_nostones);
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
        seekPlain(x, y);


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
                    seekPlain(x, y);
                else
                    break;
            }
        }
    }
}

bool MapGenerator::seekPlain(int& x, int& y)
{
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

    std::deque<PG_Point> tiles;
    
    // fill the list with initial values; the rand is there to avoid a bias
    // (i.e. prefer a certain direction)
    for (int dir = rand() % 8, i = 0; i < 8; i++, dir = (dir+1)%8)
        tiles.push_back(PG_Point(x + d_xdir[dir], y + d_ydir[dir]));
    
    // now loop until all tiles were checked (should hardly happen)
    while (!tiles.empty())
    {
        PG_Point p = tiles.front();
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
            
            tiles.push_back(PG_Point(newx, newy));
            tiles.push_back(PG_Point(newx, p.y));
            tiles.push_back(PG_Point(p.x, newy));
        }
        else
        {
            if (abs(dx) > abs(dy) && dx > 0)        // right border
                tiles.push_back(PG_Point(p.x + 1, p.y));
            else if (abs(dx) > abs(dy) && dx < 0)   //left border
                tiles.push_back(PG_Point(p.x - 1, p.y));
            else if (abs(dx) < abs(dy) && dy > 0)   // top border
                tiles.push_back(PG_Point(p.x, p.y + 1));
            else if (abs(dx) < abs(dy) && dy < 0)   // lower border
                tiles.push_back(PG_Point(p.x, p.y - 1));
        }
    }

    // if this line is ever reached, we haven't found a free grass tile
    // (should only happen under really exceptional circumstances)
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
        placePorts( x, y, city_count);
        if((d_terrain[y*d_width +x] == Tile::WATER
            || d_terrain[y*d_width + x+1] == Tile::WATER 
            || d_terrain[(y+1)*d_width + x] == Tile::WATER
            || d_terrain[(y+1)*d_width + x+1] == Tile::WATER)
            && (iterations < 10))
        {
            iterations++;
            continue;
        }
        
        // check if we can put the building
        if(!canPutBuilding(x,y) &&
            ((iterations < 10) || (d_terrain[y*d_width+x] != Tile::WATER)))
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
	if ((found_place == false) && 
			((b == Maptile::TEMPLE) || (b == Maptile::RUIN)))
		found_place = true;
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
    // if the building is on water, return false
    if (d_terrain[y*d_width +x] == Tile::WATER)
        return false;
        
    //if the building is close to the map boundaries, return false
    if ((x < 1) || (x > d_width-4) || (y < 1) || (y > d_height-4))
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

void MapGenerator::placePorts(int x,int y, int& city_count)
{
    // sea tile => abort
    if (d_l_mass[y*d_width + x] < 0)
        return;

    if (d_portneed.empty())
        return;
    
    // else look for the appropriate connection
    portneeded& pn = d_portneed[0][0];
    for (unsigned int i = 0; i < d_portneed.size(); i++)
        for (unsigned int j = 0; j < d_portneed[i].size(); j++)
        {
            if (!d_portneed[i][j].hasPort &&
                    d_portneed[i][j].landid == d_l_mass[y*d_width + x])
            {
                pn = d_portneed[i][j];

                // double break
                i = d_portneed.size();
                break;
            }
        }

    // Now move around the cost and try to place a city at random.
    // First, try it clockwise, then counterclockwise. If nothing works, then
    // try to move some single steps.

    // first, try clockwise direction
    x = pn.x;
    y = pn.y;

    
    int lastwat = 0;
    walkCoast(x, y, pn.seaid, rand() % 40, lastwat);
    pn.hasPort = tryToPlaceCity(x, y, city_count);
    if (pn.hasPort)
        return;

    // now try it anticlockwise
    x = pn.x;
    y = pn.y;
    walkCoastAntiClock(x, y, pn.seaid, rand() % 40, lastwat);
    pn.hasPort = tryToPlaceCity(x, y, city_count);
    if (pn.hasPort)
        return;

    // now try in single steps clockwise
    x = pn.x;
    y = pn.y;
    for (int i = 0; (i < 5) && (!pn.hasPort); i++)
    {
        walkCoast(x, y, pn.seaid, 1, lastwat);
        pn.hasPort = tryToPlaceCity(x, y, city_count);
    }
    if (pn.hasPort)
        return;

    // still no city placement? Try one last round anticlockwise
    for (int i = 0; (i < 5) && (!pn.hasPort); i++)
    {
        walkCoastAntiClock(x, y, pn.seaid, 1, lastwat);
        pn.hasPort = tryToPlaceCity(x, y, city_count);
    }

    // give up
    debug("gave up "<<x<<' '<<y<<endl)
}



/* 
 * walkCoast
 * moves along a coast line. Goes in a clock wise direction
 * around a lake.
 *
 * Fails under certain setups like this (W=water, G=grass, 1=grass, x=route)
 *
 * WWWWWWWWWW
 * WWWWWxWWWW
 * xxxxxWxxxx
 * GGGGG1GGGG
 *
 * While it would be possible to place a city at location 1, the algorithm
 * ignores it.
 *
 * We assume that we start on a non-water tile and then always check
 * from a starting direction clockwise until we find the first piece
 * of water with the given seaid and then the first piece of land with
 * the given landid, move forward and start again...
 */


bool MapGenerator::walkCoast(int& x,int& y, int sea_id, int dist, int& lastwat)
{
    for (int steps = 0; steps < dist; steps++)
    {
        int loop, dir;

        // first loop until we find the first water tile with the sea's id
        for (loop = 0, dir = lastwat; loop < 8; loop++, dir = (dir+1)%8)
        {
            if (dir < 0)
              continue;
            int nx = x + d_xdir[dir];
            int ny = y + d_ydir[dir];

            // invalid points, we abort when close to the border
            if (offmap(nx,ny))
                return false;
            
            // found it
            if (d_l_mass[ny*d_width + nx] == sea_id)
                break;
        }

        // didn't find the requested sea => abort
        if (loop == 8)
        {
            debug("Didn't find sea with id " <<sea_id)
            return false;
        }

        // continue looping until we find land again
        for (loop = 0; loop < 8; loop++, dir = (dir+1)%8)
        {
            if (dir < 0)
              continue;
            int nx = x + d_xdir[dir];
            int ny = y + d_ydir[dir];

            // invalid point, see above
            if (offmap(nx,ny))
                return false;

            if (d_l_mass[ny*d_width + nx] > 0) 
                break;
        }

        // we are on a 1x1 island => abort
        if (loop == 8)
        {
            debug("Island too small")
            return false;
        }

        // move to the new location
        x += d_xdir[dir];
        y += d_ydir[dir];

        // UL: To ensure we _move_ clockwise, set the start for the next water-
        // searching loop to point to the last water tile we found this time
        // -1 comes from the fact that the last water tile comes one direction
        // before the first land tile, another -(1|2) comes from the idea that we
        // move one step in direction dir. Draw it on a paper or ask me. :)
        lastwat = (dir - 2 - (dir%2))%8;
    }

    return true;
}



/** 
 * walkCoastAntiClock
 * moves along a coast line in the opposite direction   
 * to walkCoast. may miss the odd promentary.
 */

bool MapGenerator::walkCoastAntiClock(int& x,int& y, int sea_id, int dist, int& lastwat)
{
    for (int steps = 0; steps < dist; steps++)
    {
        int loop, dir;

        // first loop until we find the first water tile with the sea's id
        for (loop = 0, dir = lastwat; loop < 8; loop++, dir = (dir-1)%8)
        {
            int nx = x + d_xdir[dir];
            int ny = y + d_ydir[dir];

            // invalid points, we abort when close to the border
            if (offmap(nx,ny))
                return false;
            
            // found it
            if (d_l_mass[ny*d_width + nx] == sea_id)
                break;
        }

        // didn't find the requested sea => abort
        if (loop == 8)
        {
            debug("Didn't find sea with id " <<sea_id)
            return false;
        }

        // continue looping until we find land again
        for (loop = 0; loop < 8; loop++, dir = (dir-1)%8)
        {
            int nx = x + d_xdir[dir];
            int ny = y + d_ydir[dir];

            // invalid point, see above
            if (offmap(nx,ny))
                return false;

            if (d_l_mass[ny*d_width + nx] > 0) 
                break;
        }

        // we are on a 1x1 island => abort
        if (loop == 8)
        {
            debug("Island too small")
            return false;
        }

        // move to the new location
        x += d_xdir[dir];
        y += d_ydir[dir];

        lastwat = (dir + 2 + (dir%2))%8;
    }

    return true;
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


void MapGenerator::continents(int& nmrLands, int& nmrSeas)
{   
    int l_id = 0;   // id of the current continent to set
    int s_id = 0;   // id of the current sea to set

    if (!d_terrain)
        return;
    
    // first, initialize the d_l_mass array
    if (!d_l_mass)
        d_l_mass = new int[d_width*d_height];
            
    for (int i = 0; i < d_width*d_height; i++)
            d_l_mass[i] = 0;

    /* The algorithm is quite simple.
     * 1. We just go through all tiles until we find one which has not been
     *    assigned a land/sea id yet (i.e. d_l_mass is set to zero). We append
     *    it to the process list.
     * 2. The we continue to check for each process tile if it is valid, set it's
     *    land/sea id correspondingly (if it is valid) and append all surrounding
     *    tiles to the list of tiles to process. We end this one if there are no
     *    more tiles to process (then we have processed all tiles of that
     *    continent/sea)
     * 3. Restart with 1. until there are no more tiles untouched.
     */
    while(true)
    {
        // Step 1
        int x=-1, y=-1;
        for (int i = 0; i < d_width; i++)
            for (int j = 0; j < d_height; j++)
                if (d_l_mass[j*d_width + i] == 0)
                {
                    x = i;
                    y = j;
                    break;
                }


        // all tiles have been processed
        if (x == -1)
        {
            nmrLands = l_id;
            nmrSeas = abs(s_id);
            return;
        }


        // Step 2
        // first the setup
        std::deque<PG_Point> process;
        bool land;
        
        process.push_back(PG_Point(x, y));
        if (d_terrain[y*d_width + x] == Tile::WATER)
        {
            land = false;
            s_id--;
        }
        else
        {
            land = true;
            l_id++;
        }
        
        // then the loop
        while (!process.empty())
        {
            PG_Point p = process.front();
            process.pop_front();
            
            // first check for validity
            if (offmap(p.x, p.y) || d_l_mass[p.y*d_width + p.x] != 0)
                continue;
            if (land && d_terrain[p.y*d_width + p.x] == Tile::WATER)
                continue;
            if (!land && d_terrain[p.y * d_width + p.x] != Tile::WATER)
                continue;

            // set the tile and append all surrounding tiles
            if (land)
                d_l_mass[p.y*d_width + p.x] = l_id;
            else
                d_l_mass[p.y*d_width + p.x] = s_id;

            for (int i = 0; i < 8; i++)
                process.push_back(PG_Point(p.x + d_xdir[i], p.y + d_ydir[i]));
        }
    }
}

void MapGenerator::findRoutes()// Lands is short for LandMasses
{
    /* Here, we save notifications where we have to put ports lateron. This
     * goes in two steps. First, we assign one port to do to each land/sea
     * junction. In a second run, we remove obsolete entries (lakes within one
     * continent) by demanding that a valid sea has to border at least two
     * continents.
     */
    portneeded pn;
    vecports* vec;

    if (!d_l_mass || !d_terrain)
        return;

    for (int x = 0; x < d_width; x++)
        for (int y = 0; y < d_height; y++)
        {
            // we are not interested in water tiles as base point
            if (d_l_mass[y*d_width + x] < 0)
                continue;
            
            for (int i = 0; i < 8; i++)
            {
                int locx = x + d_xdir[i];
                int locy = y + d_ydir[i];
                vec = 0;

                if (locx >= d_width || locx < 0 || locy > d_height || locy < 0)
                    continue;

                // only check borders with water tiles
                if (d_l_mass[locy*d_width + locx] > 0)
                    continue;

                // Now we have found a land/sea border. Let's look if an
                // entry already exists or add another one if neccessary.
                
                // look if an entry with the given seaid already exists
                for (unsigned int i = 0; i < d_portneed.size(); i++)
                    if (d_portneed[i][0].seaid == d_l_mass[locy*d_width + locx])
                    {
                        vec = &d_portneed[i];
                        break;
                    }

                // If not, we can be damn sure that this entry does not exist yet.
                // So only continue checking if we found entries with the seaid.
                if (vec)
                    for (unsigned int i = 0; i < vec->size(); i++)
                        if ((*vec)[i].landid == d_l_mass[y*d_width + x])
                            continue;

                // We haven't found an entry => add a new one
                pn.seaid = d_l_mass[locy*d_width + locx];
                pn.landid = d_l_mass[y*d_width + x];
                pn.hasPort = false;
                pn.x = x;
                pn.y = y;

                if (vec)
                    vec->push_back(pn);
                else
                {
                    vecports vp;
                    vp.push_back(pn);
                    d_portneed.push_back(vp);
                }
            }
        }
         

    // now the second step
    for (vecs2d::iterator it = d_portneed.begin(); it != d_portneed.end(); it++)
        if ((*it).size() == 1)
        {
            it = d_portneed.erase(it);
            if (it == d_portneed.end())
                break;
        }

}
