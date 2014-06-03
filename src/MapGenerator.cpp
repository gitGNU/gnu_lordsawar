// Copyright (C) 2002 Vibhu Rishi
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004 David Barnsdale
// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2014 Ben Asselstine
// Copyright (C) 2008 Janek Kozicki
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

#include <iostream>
#include <math.h>  
#include <set>

//#include <boost/foreach.hpp>

#include "MapGenerator.h"
#include "army.h"
#include "GameMap.h"
#include "stack.h"
#include "path.h"
#include "File.h"
#include "citylist.h"
#include "city.h"
#include "roadlist.h"
#include "road.h"
#include "portlist.h"
#include "port.h"
#include "ruinlist.h"
#include "ruin.h"
#include "templelist.h"
#include "temple.h"
#include "bridgelist.h"
#include "bridge.h"
#include "armysetlist.h"
#include "tilesetlist.h"
#include "vector.h"
#include "RoadPathCalculator.h"
#include "cityset.h"
#include "overviewmap.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)
#define offmap(bx,by) (by<0)||(by>=d_height)||(bx<0)||(bx>=d_width)

//-------------------------------------------------------------------

MapGenerator::MapGenerator()
    //set reasonable default values
    :d_terrain(0), d_building(0), d_pswamp(2), d_pwater(25), d_pforest(3),
    d_phills(5), d_pmountains(5), d_nocities(11), d_notemples(9), d_noruins(20),
    d_nosignposts(30), cityset(NULL)

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
 *
 * See printMap() which is used for debugging maps.
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

    debug("Making random map:");
   
    // create the terrain
    debug("flatening plains");
    progress.emit(.090, _("flattening plains..."));
    makePlains();
    debug("raining water");
    progress.emit(.180, _("raining water..."));
    makeTerrain(Tile::WATER, d_pwater, true);  
    makeStreamer(Tile::WATER, d_pwater/3, 3);
    rescueLoneTiles(Tile::WATER,Tile::GRASS,true);
    makeRivers();
    verifyIslands();
    debug("raising hills");
    progress.emit(.270, _("raising hills..."));
    makeTerrain(Tile::HILLS, d_phills, false);
    debug("raising mountains");
    progress.emit(.360, _("raising mountains..."));
    makeTerrain(Tile::MOUNTAIN, d_pmountains, false);
    makeStreamer(Tile::MOUNTAIN, d_pmountains/3, 3);
    rescueLoneTiles(Tile::MOUNTAIN,Tile::GRASS,false);
    surroundMountains(0, d_width, 0, d_height);
    debug("planting forest");
    progress.emit(.450, _("planting forests..."));
    makeTerrain(Tile::FOREST, d_pforest, false);
    debug("watering swamps");
    progress.emit(.540, _("watering swamps..."));
    makeTerrain(Tile::SWAMP, d_pswamp, false);
    debug("normalizing terrain");
    progress.emit(.630, _("normalizing terrain..."));
    normalize();

    // place buildings
    debug("building cities");
    progress.emit(.720, _("building cities..."));
    makeCities(d_nocities);

    if (roads)
      {
	debug("paving roads");
	progress.emit(.810, _("paving roads..."));
	makeRoads();
      }
    rescueLoneTiles(Tile::MOUNTAIN,Tile::HILLS,false);

    debug("ruining ruins");
    progress.emit(.810, _("ruining ruins..."));
    makeBuildings(Maptile::RUIN,d_noruins);
    debug("spawning temples");
    progress.emit(.900, _("spawning temples..."));
    makeBuildings(Maptile::TEMPLE,d_notemples);
    debug("building bridges");
    if (roads == true)
      {
        progress.emit(.950, _("building bridges..."));
        makeBridges();
      }
    debug("raising signs");
    progress.emit(.990, _("raising signs..."));
    makeBuildings(Maptile::SIGNPOST,d_nosignposts);

    debug("Done making map.");

//    printMap();
}
        
#define  NORTH_SOUTH_BRIDGE 1
#define  EAST_WEST_BRIDGE 2

void MapGenerator::placeBridge(Vector<int> pos, int type)
{
  Bridgelist *bl = Bridgelist::getInstance();
  if (type == NORTH_SOUTH_BRIDGE)
    {
      d_building[pos.y*d_width + pos.x] = Maptile::BRIDGE;
      d_building[(pos.y + 1)*d_width + pos.x] = Maptile::BRIDGE;
      bl->add(new Bridge(Vector<int>(pos.x, pos.y)));
      bl->add(new Bridge(Vector<int>(pos.x+1, pos.y)));
    }
  else if (type == EAST_WEST_BRIDGE)
    {
      d_building[pos.y*d_width + pos.x] = Maptile::BRIDGE;
      d_building[pos.y*d_width + pos.x + 1] = Maptile::BRIDGE;
      bl->add(new Bridge(Vector<int>(pos.x, pos.y)));
      bl->add(new Bridge(Vector<int>(pos.x, pos.y+1)));
    }
  GameMap::getInstance()->calculateBlockedAvenues();
}

bool MapGenerator::findBridgePurpose(Vector<int> pos, int type, 
				     Vector<int> &src, Vector<int> &dest)
{
  if (type == EAST_WEST_BRIDGE)
    {
      src = GameMap::getInstance()->findNearestObjectToTheWest(pos);
      dest = GameMap::getInstance()->findNearestObjectToTheEast(pos);
    }
  else if (type == NORTH_SOUTH_BRIDGE)
    {
      src = GameMap::getInstance()->findNearestObjectToTheNorth(pos);
      dest = GameMap::getInstance()->findNearestObjectToTheSouth(pos);
    }
  if (src == Vector<int>(-1,-1) || dest == Vector<int>(-1,-1))
    return false;
  return true;
}

bool MapGenerator::canPlaceBridge(Vector<int> pos, int type, Vector<int> &src, Vector<int> &dest)
{
  if (d_building[pos.y*d_width + pos.x] == Maptile::NONE &&
      findBridgePurpose(pos, type, src, dest) == true)
    return true;
  return false;
}

void MapGenerator::makeBridges()
{
  Glib::ustring orig_tileset = GameMap::getInstance()->getTilesetName();
  Glib::ustring orig_shieldset = GameMap::getInstance()->getShieldsetName();
  Glib::ustring orig_cityset = GameMap::getInstance()->getCitysetName();
  GameMap::deleteInstance();
  Citylist::deleteInstance();
  Roadlist::deleteInstance();
  Ruinlist::deleteInstance();
  Templelist::deleteInstance();
  Portlist::deleteInstance();
  Bridgelist::deleteInstance();

  GameMap::setWidth(d_width);
  GameMap::setHeight(d_height);
  GameMap::getInstance("default", "default", "default")->fill(this);

  //the game map class smooths the map, so let's take what it smoothed.
  for (int y = 0; y < d_height; y++)
    for (int x = 0; x < d_width; x++)
      d_terrain[y*d_width + x] = 
        GameMap::getInstance()->getTile(x, y)->getType();

  //load up the roadlist, and stuff.

  for (int y = 0; y < d_height; y++)
    for (int x = 0; x < d_width; x++)
      {
	if (d_building[y*d_width + x] == Maptile::CITY)
	  Citylist::getInstance()->add
	    (new City(Vector<int>(x,y), cityset->getCityTileWidth()));
	else if (d_building[y*d_width + x] == Maptile::ROAD)
	  Roadlist::getInstance()->add(new Road(Vector<int>(x,y)));
	else if (d_building[y*d_width + x] == Maptile::RUIN)
	  Ruinlist::getInstance()->add
	    (new Ruin(Vector<int>(x,y), cityset->getRuinTileWidth()));
	else if (d_building[y*d_width + x] == Maptile::TEMPLE)
	  Templelist::getInstance()->add
	    (new Temple(Vector<int>(x,y), cityset->getTempleTileWidth()));
	else if (d_building[y*d_width + x] == Maptile::PORT)
	  Portlist::getInstance()->add(new Port(Vector<int>(x,y)));
      }
  GameMap::getInstance()->calculateBlockedAvenues();

  Vector<int> src, dest;
  std::vector<std::pair<int , Vector<int> > >  bridges;
  bridges = findBridgePlaces();
  for (std::vector<std::pair<int, Vector<int> > >::iterator it = bridges.begin();
       it != bridges.end(); it++)
    {
      Vector<int> pos = (*it).second + Vector<int>(1,1);
      Vector<int> edge1; 
      Vector<int> edge2; 
      if ((*it).first == NORTH_SOUTH_BRIDGE)
	{
	  edge1 = pos - Vector<int>(0, 1);
	  edge2 = pos + Vector<int>(0, 2);
	}
      else if ((*it).first == EAST_WEST_BRIDGE)
	{
	  edge1 = pos - Vector<int>(1, 0);
	  edge2 = pos + Vector<int>(2, 0);
	}
      if (offmap(edge1.x, edge1.y) || offmap(edge2.x, edge2.y))
	continue;
      if (canPlaceBridge((*it).second + Vector<int>(1,1), (*it).first, src, 
			 dest) == true)
	{
	  int leg1 = tryRoad (src, edge1);
	  int leg2 = tryRoad (dest, edge2);
	  int shortcut = tryRoad (src, dest);
	  bool construct_bridge_and_roads = true;
	  if (leg1 <= 0 || leg2 <= 0)
	    construct_bridge_and_roads = false;
	  if (shortcut > 0 && (leg1 + leg2 + 2) > shortcut)
	    construct_bridge_and_roads = false;

	  if (construct_bridge_and_roads)
	    {
	      makeRoad(src, edge1);
	      makeRoad(dest, edge2);
	      placeBridge(pos, (*it).first);
	    }
	}
	progress.emit(.950, _("paving bridges..."));
    }

  Roadlist::deleteInstance();
  Ruinlist::deleteInstance();
  Templelist::deleteInstance();
  GameMap::deleteInstance();
  Citylist::deleteInstance();
  Portlist::deleteInstance();
  Bridgelist::deleteInstance();
  GameMap::getInstance(orig_tileset, orig_shieldset, orig_cityset);
}

void MapGenerator::printMap(int j, int i)
{
    char ch='?';
    bool adom_convention=true; // well, except mountains
    switch(d_terrain[j*d_width + i])
    {
        case Tile::MOUNTAIN:  ch=adom_convention ? 'M' : 'M';break; // mountains
        case Tile::HILLS   :  ch=adom_convention ? '~' : 'h';break; // hills
        case Tile::WATER   :  ch=adom_convention ? '=' : '~';break; // water
        case Tile::FOREST  :  ch=adom_convention ? '&' : '$';break; // forest
        case Tile::GRASS   :  ch=adom_convention ? '.' : '.';break; // plains
        case Tile::SWAMP   :  ch=adom_convention ? '"' : '_';break; // swamps

            // cannot print those, actually because they don't exist in Tile::Type
            //     ch='C';break; // city/castle
            //     ch='r';break; // ruins
            //     ch='T';break; // temple
            //     ch=' ';break; // nothing
            //     ch='c';break; // part of city/castle
    }
    std::cout << ch;
}

void MapGenerator::printMap()
{
    for(int j = 0; j < d_height; j++)
    {
        for(int i = 0; i < d_width; i++)
            printMap(j,i);
        std::cout << "\n";
    }
    std::cout << "\n";
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

void MapGenerator::connectWithWater(Vector<int> from, Vector<int> to)
{
    Vector<float> delta = from - to;
    if (dist<float>(from,to) > (float)(d_width*0.4))
        // we don't want to mess up whole map with straight lines
        return;

    int kind(rand()%4);
    delta /= length(delta)*2;
    for(Vector<float>path = Vector<float>(from)+delta*4 ; dist<float>(path,Vector<float>(to)-delta*4) > 0.5 ; path -= delta)
    {
        int j = (int)(path.x);
        int i = (int)(path.y);

        if(rand()%3 == 0) 
            kind = rand()%4;
        switch(kind)
        {
            case 0:
                if((!(offmap(i,j))) && (!(offmap(i-1,j-1))))
                {
                    d_terrain[(j  )*d_width + i  ] = Tile::WATER;
                    d_terrain[(j-1)*d_width + i-1] = Tile::WATER;
                    d_terrain[(j  )*d_width + i-1] = Tile::WATER;
                    d_terrain[(j-1)*d_width + i  ] = Tile::WATER;
                }; break;

            case 1:
                if((!(offmap(i,j))) && (!(offmap(i+1,j+1))))
                {
                    d_terrain[(j  )*d_width + i  ] = Tile::WATER;
                    d_terrain[(j+1)*d_width + i+1] = Tile::WATER;
                    d_terrain[(j  )*d_width + i+1] = Tile::WATER;
                    d_terrain[(j+1)*d_width + i  ] = Tile::WATER;
                }; break;

            case 2:
                if((!(offmap(i,j))) && (!(offmap(i-1,j+1))))
                {
                    d_terrain[(j  )*d_width + i  ] = Tile::WATER;
                    d_terrain[(j+1)*d_width + i-1] = Tile::WATER;
                    d_terrain[(j  )*d_width + i-1] = Tile::WATER;
                    d_terrain[(j+1)*d_width + i  ] = Tile::WATER;
                }; break;

            case 3:
                if((!(offmap(i,j))) && (!(offmap(i+1,j-1))))
                {
                    d_terrain[(j  )*d_width + i  ] = Tile::WATER;
                    d_terrain[(j-1)*d_width + i+1] = Tile::WATER;
                    d_terrain[(j  )*d_width + i+1] = Tile::WATER;
                    d_terrain[(j-1)*d_width + i  ] = Tile::WATER;
                }; break;
        }
    }
}

void MapGenerator::findAreasOf(Tile::Type THIS_TILE,std::vector<std::vector<int> >& box,int& how_many)
{
    box.resize(d_height);
    for(int j = 0; j < d_height; j++)
        box[j].resize(d_width,0);

    // find all enclosed areas by scanning the map
    // distinct areas have different numbers in box
    for(int j = 1; j < d_height-1; j++)
        for(int i = 1; i < d_width-1; i++)
            if (box[j][i]==0 &&
                    d_terrain[j*d_width + i] == THIS_TILE &&
                    (
                     (d_terrain[(j-1)*d_width + i-1] == THIS_TILE &&
                      d_terrain[(j  )*d_width + i-1] == THIS_TILE &&
                      d_terrain[(j-1)*d_width + i  ] == THIS_TILE)  ||

                     (d_terrain[(j-1)*d_width + i  ] == THIS_TILE &&
                      d_terrain[(j-1)*d_width + i+1] == THIS_TILE &&
                      d_terrain[(j  )*d_width + i+1] == THIS_TILE) ||

                     (d_terrain[(j  )*d_width + i+1] == THIS_TILE &&
                      d_terrain[(j+1)*d_width + i+1] == THIS_TILE &&
                      d_terrain[(j+1)*d_width + i  ] == THIS_TILE) ||

                     (d_terrain[(j+1)*d_width + i  ] == THIS_TILE &&
                      d_terrain[(j+1)*d_width + i-1] == THIS_TILE &&
                      d_terrain[(j  )*d_width + i-1] == THIS_TILE))
               )
            {
                box[j][i]=++how_many+3000;
                int counter=1;
                while(counter != 0)
                {
                    counter=0;
                    for(int J = 1; J < d_height-1; J++)
                        for(int I = 1; I < d_width-1; I++)
                        {
                            if(d_terrain[J*d_width + I] == THIS_TILE &&
                                    box[J][I]    ==0 &&
                                    (box[J-1][I  ]==how_many+3000 ||
                                     box[J  ][I-1]==how_many+3000 ||
                                     box[J  ][I+1]==how_many+3000 ||
                                     box[J+1][I  ]==how_many+3000))
                            {
                                ++counter;
                                box[J][I]=how_many+2000;
                            }
                        }
                    for(int J = 0; J < d_height; J++)
                        for(int I = 0; I < d_width; I++)
                        {
                            if (box[J][I]==how_many+3000)
                                box[J][I]=how_many;
                            if (box[J][I]==how_many+2000)
                                box[J][I]=how_many+3000;
                        }
                }
            }
}

void MapGenerator::verifyIslands()
{
    int how_many=0;
    std::vector<std::vector<int> > box;
    findAreasOf(Tile::GRASS,box,how_many);

    // count the size of each area
    std::vector<float> counts;
    counts.resize(how_many+2,0);
    for(int j = 0; j < d_height; j++)
        for(int i = 0; i < d_width; i++)
            if(box[j][i] != 0)
                counts[box[j][i]] += 1;

    // find four largest land areas
    std::set<int> largest;largest.clear();
    int max;
    for(int z=0 ; z<4 ; ++z)
    {
        max = -1;
        for(size_t i=0 ; i<counts.size() ; ++i)
        {
            if(counts[i] > max && largest.find(counts[i]) == largest.end())
                max = counts[i];
        }
        largest.insert(max);
    }

    // largest are good. Also one/third of all others is good:
    std::set<int> good(largest);
    for(size_t i=0 ; i<counts.size() ; ++i)
        if(rand()%3 == 0) // that's one/third here
            good.insert(counts[i]);

    // now, eliminate all land that is not good
    for(int I=0 ; I<(int)(counts.size()) ; ++I)
        if(good.find(counts[I]) == good.end())
            for(int j = 1; j < d_height-1; j++)
                for(int i = 1; i < d_width-1; i++)
                    if(box[j][i] == I)
                        d_terrain[j*d_width + i] = Tile::WATER;
}

void MapGenerator::makeRivers()
{
    // river style:
    //  1 - plenty of short rivers and islands
    //  2 - longer rivers, less islands
    //  3 - even longer rivers, even less islands
    int river_style=rand()%3+1;

    // count how many separate bodies of water were found
    int how_many;

    int iter=0; // avoid deadlocks
    while(++iter < 20)
    {
        how_many=0;

        std::vector<std::vector<int> > box;
        
        findAreasOf(Tile::WATER,box,how_many);

        // this loop allows maximum 3 distinctly separated bodies of water
        // so no need to continue the algorithm
        if(how_many<4)
            break;

        // find two biggest bodies of water, and calculate centers for all of them
        std::vector< Vector<float> > centers;
        centers.resize(how_many+2,Vector<float>(0,0));
        std::vector<float> counts;
        counts.resize(how_many+2,0);
        for(int j = 0; j < d_height; j++)
            for(int i = 0; i < d_width; i++)
                if(box[j][i] != 0)
                {
                    counts[box[j][i]] += 1;
                    centers[box[j][i]] += Vector<float>(j,i);
                }
        // divide sum by counts to get a center
        int max_count=0,max_count_2=0;
        for(int h = 0; h < how_many+2; ++h)
        {
            if(counts[h]>0)
            {
                centers[h] /= counts[h];
                if(max_count < (int)(counts[h]))
                    max_count = (int)(counts[h]);
                if(max_count_2 < (int)(counts[h]) && (int)(counts[h]) != max_count)
                    max_count_2 = (int)(counts[h]);
                int J=(int)(centers[h].x), I=(int)(centers[h].y);
                if(box[J][I] != h)
                // center doesn't necessarily fall on water tile, so fix this.
                {
                    //      // for debugging...
                    //      box[J][I]+=5000;
                    int i_up=0,i_dn=0,j_up=0,j_dn=0;
                    while((I+i_up <  d_width-1 ) && (box[J     ][I+i_up] != h)) ++i_up;
                    while((I-i_dn >  0         ) && (box[J     ][I-i_dn] != h)) ++i_dn;
                    while((J+j_up <  d_height-1) && (box[J+j_up][I     ] != h)) ++j_up;
                    while((J-j_dn >  0         ) && (box[J-j_dn][I     ] != h)) ++j_dn;

                    int shortest = std::min( std::min(i_up,i_dn) , std::min(j_up,j_dn));

                    if(shortest == i_up && I+i_up <  d_width)
                        centers[h] = Vector<float>( J      , I+i_up );
                    else
                        if(shortest == i_dn && I-i_dn >= 0      )
                            centers[h] = Vector<float>( J      , I-i_dn );
                        else
                            if(shortest == j_up && J+j_up <  d_height)
                                centers[h] = Vector<float>( J+j_up , I      );
                            else
                                if(shortest == j_dn && J-j_dn >= 0       )
                                    centers[h] = Vector<float>( J+j_dn , I      );
                                else
                                {
                                    std::cout << "Sages are wondering about unforeseen mysteries behind the edge of the world.\n";
                                    counts[h] = -1; // that's ok, but an interesting case. I'd like to see a map with such water :)
                                    // FIXME - can you make a message box here?
                                    //MessageBox("Message from author: this is algorithmically a very interesting map, please make screenshot and send to cosurgi@gmail.com");
                                }
                }
                //      // for debugging...
                //      box[(int)(centers[h].x)][(int)(centers[h].y)]+=4000;
            }
        }

        // determine what are the biggest bodies of water here
        int the_biggest_area=0,second_biggest_area=0;
        for(int h = 0; h < how_many+2; ++h)
        {
            if(counts[h]==max_count   && max_count   != 0)
                the_biggest_area = h;
            if(counts[h]==max_count_2 && max_count_2 != 0)
                second_biggest_area = h;
        }

        // find shortest distances between areas
        std::vector<std::vector<std::pair<float, std::pair<Vector<int>, Vector<int> > > > > distances; // I would prefer boost::tuple, but oh well...
        distances.resize(how_many+2);
        for(int h = 0; h < how_many+2; ++h)
        {
            distances[h].resize(how_many+3,std::make_pair(0,std::make_pair(Vector<int>(0,0),Vector<int>(0,0))));
            for(int k = h+1; k < how_many+2; ++k)
            {
                if(counts[h] > 0 && counts[k] > 0) // h and k are two different areas
                {
                    // find tile from area h closest to the center of k 
                    float min_dist = d_height*d_height;
                    float min_h_j=0,min_h_i=0;
                    for(int j = 1; j < d_height-1; j++)
                        for(int i = 1; i < d_width-1; i++)
                            if(box[j][i] == h)
                            {
                                float dj = j - centers[k].x;
                                float di = i - centers[k].y;
                                float dist = dj*dj + di*di;
                                if(dist < min_dist)
                                {
                                    min_dist = dist;
                                    min_h_j = j;
                                    min_h_i = i;
                                }
                            }

                    // then find tile from area k closest to that tile from h
                    min_dist = d_height * d_height;
                    float min_k_j=0,min_k_i=0;
                    for(int j = 1; j < d_height-1; j++)
                        for(int i = 1; i < d_width-1; i++)
                            if(box[j][i] == k)
                            {
                                float dj = j - min_h_j;
                                float di = i - min_h_i;
                                float dist = dj*dj + di*di;
                                if(dist < min_dist)
                                {
                                    min_dist = dist;
                                    min_k_j = j;
                                    min_k_i = i;
                                }
                            }

                    if (min_k_j != 0 && 
                            min_h_j != 0 && 
                            min_k_i != 0 && 
                            min_h_i != 0)
                    {
                        float dj = min_k_j - min_h_j;
                        float di = min_k_i - min_h_i;
                        distances[h][k] = std::make_pair(dj*dj + di*di , std::make_pair(Vector<int>(min_h_j,min_h_i) , Vector<int>(min_k_j,min_k_i)) );
                    }
                }
            }
        }

        for(int connect_some_closest=0; connect_some_closest<14; connect_some_closest+=river_style)
        {
            // if river_style is 1 then
            //   connect 10 closest to each other, and 4 closest to two biggest bodies of water
            // otherwise skip some - connect fewer of them.
            int closest_h=-1,closest_k=-1,min=d_height*d_height;
            int start_h=0;
            if(connect_some_closest < 2 ) start_h=the_biggest_area;
            else
                if(connect_some_closest < 4) start_h=second_biggest_area;
            for(int h = start_h; h < ((connect_some_closest >= 4) ? (how_many+2) : start_h+1); ++h)
                for(int k = h+1; k < how_many+2; ++k)
                    if(counts[h] > 0 && counts[k] > 0)
                        if(distances[h][k].first > 0 && min > distances[h][k].first)
                        {
                            min = distances[h][k].first;
                            closest_h = h;
                            closest_k = k;
                        }
            if (closest_h != -1 &&
                closest_k != -1)
            {
                connectWithWater(distances[closest_h][closest_k].second.first , distances[closest_h][closest_k].second.second);
                // mark as done:
                distances[closest_h][closest_k].first = d_height*d_height;
            }
        }
        //          // for debugging...   print whole box
        //          std::cerr << how_many << " separate bodies of water found.\n";
        //          std::cerr << the_biggest_area << " is the biggest\n";
        //          std::cerr << second_biggest_area << " is second in size\n";
        //          std::vector<int> a;
        //          BOOST_FOREACH(a,box)
        //          {
        //              BOOST_FOREACH(int i,a)
        //              {
        //                  if(i<4000)
        //                      std::cout << i << " ";
        //                  else
        //                      if(i<5000)
        //                      std::cout << "X" << " ";
        //                      else
        //                      if(i<7000)
        //                      std::cout << "%" << " ";
        //                      else
        //                      if(i<14000)
        //                      std::cout << "!" << " ";
        //                      else
        //                      std::cout << "|" << " ";
        //              }
        //              std::cout << "\n";
        //          }
        //          std::cout << "\n";

    };
    //if(how_many>1)
        //std::cout << "There are " << how_many << (how_many<4?(Glib::ustring(" seas")):(Glib::ustring(" lakes"))) << " on this map.\n";
    //else
        //std::cout << "There is 1 sea on this map.\n";
    //std::cout << "River style was: " << river_style << "\n";
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

bool MapGenerator::inhospitableTerrain(int x, int y, unsigned int width)
{
  for (unsigned int i = 0; i < width; i++)
    for (unsigned int j = 0; j < width; j++)
      if (d_terrain[(y+i)*d_width +(x+j)] == Tile::WATER ||
	  d_terrain[(y+i)*d_width +(x+j)] == Tile::MOUNTAIN)
	return true;
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
        if (inhospitableTerrain(x, y, cityset->getCityTileWidth()) && (iterations < 1000))
        {
            iterations++;
            continue;
        }
        
        // check if we can put the building
        if (!canPutCity(x, y) && iterations < 1000)
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
  for (unsigned int i = 0; i < cityset->getCityTileWidth(); i++)
    for (unsigned int j = 0; j < cityset->getCityTileWidth(); j++)
      if (canPutBuilding(x+i,y+j) == false)
        return false;
        
  return true;
}

void MapGenerator::putCity(int x, int y, int& city_count)
{
        d_building[y*d_width + x] = Maptile::CITY;

        //cities shall only sit on grass tiles
	for (unsigned int i = 0; i < cityset->getCityTileWidth(); i++)
	  for (unsigned int j = 0; j < cityset->getCityTileWidth(); j++)
	    d_terrain[(y+i)*d_width + (x+j)] = Tile::GRASS;
        //cities cannot neighbor with mountain tiles
        for (int Y = -1; Y <= (int)cityset->getCityTileWidth(); ++Y )
            for (int X = -1; X <= (int)cityset->getCityTileWidth(); ++X)
                if (d_terrain[(y+Y)*d_width + x+X] == Tile::MOUNTAIN)
                    d_terrain[(y+Y)*d_width + x+X] = Tile::HILLS;

        city_count++;
}

void MapGenerator::makeBuildings(Maptile::Building b, int building)
{
    int i, j, x, y;
    int iterations = 10;
    bool found_place = false;

    unsigned int width = 1;

    switch (b)
      {
      case Maptile::CITY:
	width = cityset->getCityTileWidth(); break;
      case Maptile::RUIN:
	width = cityset->getRuinTileWidth(); break;
      case Maptile::TEMPLE:
	width = cityset->getTempleTileWidth(); break;
      case Maptile::NONE:
      case Maptile::SIGNPOST:
      case Maptile::ROAD:
      case Maptile::PORT:
      case Maptile::BRIDGE:
	width = 1;
	break;
      }

   //If number of iterations is smaller 10, look for a suitable
   //place. If this number is exceeded, place the temple on an
   //island if neccessary.
    for (i = 0; i < building; i++)
    {        
	for (j = 0; j < iterations; j++)
	{
             x = rand()%d_width;
             y = rand()%d_height;
        
	     found_place = true;
	     for (unsigned int k = 0; k < width; k++)
	       for (unsigned int l = 0; l < width; l++)
		 if (canPutBuilding(x+k, y+l) == false)
		   found_place = false;

	     if (found_place == true)
	       break;
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

    int tooclose;
    tooclose = GameMap::calculateTilesPerOverviewMapTile(d_width, d_height);
    tooclose++;
    //if the building is close to the map boundaries, return false
    if (x <= tooclose || x >= (d_width - tooclose) || 
        y <= tooclose || y >= (d_height - tooclose))
        return false;

    int dist = (int)cityset->getCityTileWidth() + tooclose;
    //if there is another building too close, return false
    for (int locx = x-dist; locx <= x+dist; locx++)
        for (int locy = y-dist; locy <= y+dist; locy++)
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
    std::map<guint32,guint32> ajacentTer;
    Tile::Type curTer=Tile::NONE, ajTer=Tile::NONE;

    // that was 40 before. Now with rivers, the smaller the value - the more connected rivers we got.
    int center_tiles = rand()%40;
    //std::cerr << center_tiles << "\% chance of disconnecting rivers.\n";

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
                else if ((ajacentTer[curTer]==3) && (rand()%100 < center_tiles ))
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
	  pl->add(new Port(Vector<int>(x, y)));
	  calculateBlockedAvenue(x, y);
	  return true;
	}
    }
  return false;
}

int MapGenerator::tryRoad(Vector<int> src, Vector<int>dest)
{
  return tryRoad (src.x, src.y, dest.x, dest.y);
}

int MapGenerator::tryRoad(int src_x, int src_y, int dest_x, int dest_y)
{
  Vector<int> src(src_x, src_y);
  Vector<int> dest(dest_x, dest_y);

  Path *p = new Path();
  Stack s(NULL, src);

  ArmyProto *basearmy = ArmyProto::createScout();
  Army *a = Army::createNonUniqueArmy(*basearmy);
  delete basearmy;
  s.push_back(a);
  // try to get there with a scout
  guint32 moves = p->calculate(&s, dest, false);

  delete p;
  return (int)moves;
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

  RoadPathCalculator rpc(src);
  Path *p = rpc.calculate(dest);

  if (p->size() > 0)
    {
      Roadlist *rl = Roadlist::getInstance();
      for (Path::iterator it = p->begin(); it != p->end(); it++)
	{
	  int x = (*it).x;
	  int y = (*it).y;
	  if (gm->getTile(x, y)->getType() == Tile::WATER &&
	      gm->getTile(x, y)->getBuilding() != Maptile::BRIDGE)
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
		  rl->add(new Road(Vector<int>(x, y)));
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

  ArmyProto *basearmy = ArmyProto::createScout();
  Army *a = Army::createNonUniqueArmy(*basearmy);
  delete basearmy;
  s.push_back(a);
  // try to get there with a scout
  if (p->calculate(&s, dest, true) <= 0)
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

  ArmyProto *basearmy = ArmyProto::createScout();
  Army *a = Army::createNonUniqueArmy(*basearmy);
  delete basearmy;
  s.push_back(a);
  guint32 moves = p->calculate(&s, dest, false);

  if (moves <= 0)
    {
      s.clear();
      delete a;
      ArmyProto *basearmy = ArmyProto::createBat();
      a = Army::createNonUniqueArmy(*basearmy);
      delete basearmy;
      s.push_back(a);
      moves = p->calculate(&s, dest, false);
    }

  if (moves != 0)
    {
      Path::iterator it = p->begin();
      Path::iterator nextit = it;
      nextit++;
      for ( ; nextit != p->end(); it++, nextit++)
	{
	  int x = (*it).x;
	  int y = (*it).y;
	  int nextx = (*nextit).x;
	  int nexty = (*nextit).y;
	  if (d_terrain[y*d_width + x] == Tile::MOUNTAIN)
	    {
	      d_terrain[y*d_width +x] = Tile::HILLS;
	      Maptile *t = new Maptile(x, y, Tile::HILLS, NULL);
	      gm->setTile(x, y, t);
	      calculateBlockedAvenue(x, y);
	    }
	  if (d_terrain[y*d_width + x] == Tile::WATER &&
	      d_terrain[nexty*d_width + nextx] != Tile::WATER)
	    {
	      if (placePort(x, y) == true)
		{
		  if (isAccessible(src_x, src_y, dest.x, dest.y))
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
		  if (isAccessible(src_x, src_y, dest.x, dest.y))
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

std::vector<std::pair<int , Vector<int> > > MapGenerator::findBridgePlaces()
{
    std::vector<std::pair<int , Vector<int> > > result;
    result.clear();

    for(int j = 1; j < d_height-5; j++)
        for(int i = 1; i < d_width-5; i++)
        {
            if (
                d_terrain[(j  )*d_width + i+1] != Tile::WATER &&
                d_terrain[(j+1)*d_width + i+1] == Tile::WATER &&
                d_terrain[(j+2)*d_width + i+1] == Tile::WATER &&
                d_terrain[(j+3)*d_width + i+1] != Tile::WATER &&
                d_terrain[(j+1)*d_width + i  ] == Tile::WATER &&
                d_terrain[(j+2)*d_width + i  ] == Tile::WATER &&
                d_terrain[(j+1)*d_width + i+2] == Tile::WATER &&
                d_terrain[(j+2)*d_width + i+2] == Tile::WATER
                )
            {
                int count_left =
                (int)(d_terrain[(j  )*d_width + i  ] == Tile::WATER) +
                (int)(d_terrain[(j+1)*d_width + i  ] == Tile::WATER) +
                (int)(d_terrain[(j+2)*d_width + i  ] == Tile::WATER) +
                (int)(d_terrain[(j+3)*d_width + i  ] == Tile::WATER) +
                (int)(d_terrain[(j  )*d_width + i-1] == Tile::WATER) +
                (int)(d_terrain[(j+1)*d_width + i-1] == Tile::WATER) +
                (int)(d_terrain[(j+2)*d_width + i-1] == Tile::WATER) +
                (int)(d_terrain[(j+3)*d_width + i-1] == Tile::WATER);
                int count_right =
                (int)(d_terrain[(j  )*d_width + i+2] == Tile::WATER) +
                (int)(d_terrain[(j+1)*d_width + i+2] == Tile::WATER) +
                (int)(d_terrain[(j+2)*d_width + i+2] == Tile::WATER) +
                (int)(d_terrain[(j+3)*d_width + i+2] == Tile::WATER) +
                (int)(d_terrain[(j  )*d_width + i+3] == Tile::WATER) +
                (int)(d_terrain[(j+1)*d_width + i+3] == Tile::WATER) +
                (int)(d_terrain[(j+2)*d_width + i+3] == Tile::WATER) +
                (int)(d_terrain[(j+3)*d_width + i+3] == Tile::WATER);
                
                if(count_left > 5 && count_right > 5)
                    result.push_back(std::make_pair(1, Vector<int>(i,j) ));
            }
            if (
                d_terrain[(j+1)*d_width + i  ] != Tile::WATER &&
                d_terrain[(j+1)*d_width + i+1] == Tile::WATER &&
                d_terrain[(j+1)*d_width + i+2] == Tile::WATER &&
                d_terrain[(j+1)*d_width + i+3] != Tile::WATER &&
                d_terrain[(j  )*d_width + i+1] == Tile::WATER &&
                d_terrain[(j  )*d_width + i+2] == Tile::WATER &&
                d_terrain[(j+2)*d_width + i+1] == Tile::WATER &&
                d_terrain[(j+2)*d_width + i+2] == Tile::WATER
                )
            {
                int count_top =
                (int)(d_terrain[(j  )*d_width + i  ] == Tile::WATER) +
                (int)(d_terrain[(j  )*d_width + i+1] == Tile::WATER) +
                (int)(d_terrain[(j  )*d_width + i+2] == Tile::WATER) +
                (int)(d_terrain[(j  )*d_width + i+3] == Tile::WATER) +
                (int)(d_terrain[(j-1)*d_width + i  ] == Tile::WATER) +
                (int)(d_terrain[(j-1)*d_width + i+1] == Tile::WATER) +
                (int)(d_terrain[(j-1)*d_width + i+2] == Tile::WATER) +
                (int)(d_terrain[(j-1)*d_width + i+3] == Tile::WATER);

                int count_bottom =
                (int)(d_terrain[(j+2)*d_width + i  ] == Tile::WATER) +
                (int)(d_terrain[(j+2)*d_width + i+1] == Tile::WATER) +
                (int)(d_terrain[(j+2)*d_width + i+2] == Tile::WATER) +
                (int)(d_terrain[(j+2)*d_width + i+3] == Tile::WATER) +
                (int)(d_terrain[(j+3)*d_width + i  ] == Tile::WATER) +
                (int)(d_terrain[(j+3)*d_width + i+1] == Tile::WATER) +
                (int)(d_terrain[(j+3)*d_width + i+2] == Tile::WATER) +
                (int)(d_terrain[(j+3)*d_width + i+3] == Tile::WATER);

                if(count_top > 5 && count_bottom > 5)
                    result.push_back(std::make_pair(2, Vector<int>(i,j) ));
            }
        }
    // randomize
    std::random_shuffle(result.begin(),result.end());

    // remove those that are too close to each other
    std::set<int> bad;bad.clear();
    for(size_t r = 0; r<result.size() ; ++r)
        for(size_t s = r+1; s<result.size() ; ++s)
            if(dist(Vector<float>(result[r].second),Vector<float>(result[s].second)) < 4.5)
                bad.insert(r);
    std::vector<std::pair<int , Vector<int> > > filter;filter.clear();
    for(size_t r = 0; r<result.size() ; ++r)
        if(bad.find(r) == bad.end())
            filter.push_back(result[r]);
    result=filter;

    return result;
}

void MapGenerator::makeRoads()
{
  Glib::ustring orig_tileset = GameMap::getInstance()->getTilesetName();
  Glib::ustring orig_shieldset = GameMap::getInstance()->getShieldsetName();
  Glib::ustring orig_cityset = GameMap::getInstance()->getCitysetName();
  GameMap::deleteInstance();
  Citylist::deleteInstance();
  Roadlist::deleteInstance();
  Portlist::deleteInstance();

  GameMap::setWidth(d_width);
  GameMap::setHeight(d_height);
  GameMap::getInstance("default", "default", cityset->getBaseName())->fill(this);
  Roadlist::getInstance();
  //the game map class smooths the map, so let's take what it smoothed.
  for (int y = 0; y < d_height; y++)
    for (int x = 0; x < d_width; x++)
      d_terrain[y*d_width + x] = 
	GameMap::getInstance()->getTile(x, y)->getType();

  for (int y = 0; y < d_height; y++)
    for (int x = 0; x < d_width; x++)
      {
	if (d_building[y*d_width + x] == Maptile::CITY)
	  Citylist::getInstance()->add
	    (new City(Vector<int>(x,y), cityset->getCityTileWidth()));
      }
  GameMap::getInstance()->calculateBlockedAvenues();

  Citylist *cl = Citylist::getInstance();
  for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
    {
      if (rand() % 2 == 0)
	continue;
      City *c = cl->getNearestCityPast((*it)->getPos(), 13);
      Vector<int> dest = c->getPos();
      Vector<int> src = (*it)->getPos();
      //does it already have a road going to it?
      if (Roadlist::getInstance()->getNearestObjectBefore(dest, 
							  c->getSize() + 1))
	continue;

      makeRoad(src, dest);
      progress.emit(.810, _("paving roads..."));
    }
  
  //make all cities accessible by allowing movement to a central city
  Vector<int> pos = GameMap::getCenterOfMap();
  City *center = cl->getNearestCity(pos);
  for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
    {
      if (center == *it)
	continue;
      if (isAccessible(center->getPos(), (*it)->getPos()) == false)
	{
	makeAccessible(center->getPos(), (*it)->getPos());
	}
      progress.emit(.810, _("paving roads..."));
    }

  Roadlist::deleteInstance();
  GameMap::deleteInstance();
  Citylist::deleteInstance();
  Portlist::deleteInstance();
  GameMap::getInstance(orig_tileset, orig_shieldset, orig_cityset);
}

void MapGenerator::rescueLoneTiles(Tile::Type FIND_THIS, Tile::Type REPLACE, bool grow)
{
    int box[3][3];
    memset (box, 0, sizeof (box));

    if(grow)
    {
        for(int j = 1; j < d_height-1; j++)
            for(int i = 1; i < d_width-1; i++)
            {
                if (d_terrain[j*d_width + i] == FIND_THIS &&
                   (d_terrain[(j-1)*d_width + i-1] == FIND_THIS &&
                    d_terrain[(j  )*d_width + i-1] == FIND_THIS &&
                    d_terrain[(j-1)*d_width + i  ] != FIND_THIS))
                    d_terrain[(j-1)*d_width + i  ] =  FIND_THIS;
            }
    }

    for(int iteration=0; iteration <8 ;++iteration)
    {
        for(int j = 0; j < d_height; j++)
            for(int i = 0; i < d_width; i++)
            {
                if(d_terrain[j*d_width + i] == FIND_THIS)
                { 
                    for (int I = -1; I <= +1; ++I)
                        for (int J = -1; J <= +1; ++J)
                            if (!(offmap(i+I,j+J)))
                                box[J+1][I+1] = (d_terrain[(j+J)*d_width + (i+I)] == d_terrain[j*d_width + i]);
                            else
                                box[J+1][I+1] = 0;

                    if (!box[0][2] && !box[1][2] && /***********/ 
                        /***********/  box[1][1] &&  box[2][1] && 
                        !box[0][0] && !box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/ !box[1][2] && !box[2][2] && 
                         box[0][1] &&  box[1][1] && /***********/
                        /***********/ !box[1][0] && !box[2][0])
                            d_terrain[j*d_width + i] = REPLACE;
                    if (!box[0][2] && /***********/ !box[2][2] && 
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                        /***********/  box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/  box[1][2] && /***********/
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                        !box[0][0] && /***********/ !box[2][0])
                            d_terrain[j*d_width + i] = REPLACE;

                    if (/***********/ !box[1][2] && /***********/ 
                        /***********/  box[1][1] &&  box[2][1] && 
                        !box[0][0] && !box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/ !box[1][2] && /***********/ 
                         box[0][1] &&  box[1][1] && /***********/
                        /***********/ !box[1][0] && !box[2][0])
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/ /***********/ !box[2][2] && 
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                        /***********/  box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/  box[1][2] && /***********/
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                        /***********/ /***********/ !box[2][0])
                            d_terrain[j*d_width + i] = REPLACE;

                    if (!box[0][2] && !box[1][2] && /***********/ 
                        /***********/  box[1][1] &&  box[2][1] && 
                        /***********/ !box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/ !box[1][2] && !box[2][2] && 
                         box[0][1] &&  box[1][1] && /***********/
                        /***********/ !box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (!box[0][2] && /***********/ /***********/ 
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                        /***********/  box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/  box[1][2] && /***********/
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                        !box[0][0]    /***********/ /***********/)
                            d_terrain[j*d_width + i] = REPLACE;

                    if (/***********/ !box[1][2] && /***********/ 
                        !box[0][1] &&  box[1][1] &&  box[2][1] && 
                        /***********/ !box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/ !box[1][2] && /***********/ 
                         box[0][1] &&  box[1][1] && !box[2][1] &&
                        /***********/ !box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/ !box[1][2] && /***********/ 
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                        /***********/  box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/  box[1][2] && /***********/
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                        /***********/ !box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;

                    if ( box[0][2] && !box[1][2] && /***********/
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                        /***********/ !box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/ !box[1][2] &&  box[2][2] && 
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                        /***********/ !box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/ !box[1][2] && /***********/ 
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                         box[0][0] && !box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/ !box[1][2] && /***********/  
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                        /***********/ !box[1][0] &&  box[2][0])
                            d_terrain[j*d_width + i] = REPLACE;


                    if ( box[0][2] && !box[1][2] &&  box[2][2] &&  
                         box[0][1] &&  box[1][1] &&  box[2][1] && 
                         box[0][0] && !box[1][0] &&  box[2][0])
                            d_terrain[j*d_width + i+(rand()%2?+1:-1)] = FIND_THIS;
                    if ( box[0][2] &&  box[1][2] &&  box[2][2] &&  
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                         box[0][0] &&  box[1][0] &&  box[2][0])
                            d_terrain[(j+(rand()%2?+1:-1))*d_width + i] = FIND_THIS;

                    if ( box[0][2] && !box[1][2] && !box[2][2] &&  
                         box[0][1] &&  box[1][1] && !box[2][1] && 
                        !box[0][0] &&  box[1][0] &&  box[2][0])
                            d_terrain[j*d_width + i] = REPLACE;
                    if (!box[0][2] && !box[1][2] &&  box[2][2] &&  
                        !box[0][1] &&  box[1][1] &&  box[2][1] && 
                         box[0][0] &&  box[1][0] && !box[2][0])
                            d_terrain[j*d_width + i] = REPLACE;
                    if ( box[0][2] &&  box[1][2] && !box[2][2] &&  
                        !box[0][1] &&  box[1][1] &&  box[2][1] && 
                        !box[0][0] && !box[1][0] &&  box[2][0])
                            d_terrain[j*d_width + i] = REPLACE;
                    if (!box[0][2] &&  box[1][2] &&  box[2][2] &&  
                         box[0][1] &&  box[1][1] && !box[2][1] && 
                         box[0][0] && !box[1][0] && !box[2][0])
                            d_terrain[j*d_width + i] = REPLACE;
                }
            }
    }
}

void MapGenerator::surroundMountains(int minx, int maxx, int miny, int maxy)
{
  for(int j = miny; j < maxy; j++)
    for(int i = minx; i < maxx; i++)
      if(d_terrain[j*d_width + i] == Tile::MOUNTAIN)
	for(int J = -1; J <= +1; ++J)
	  for(int I = -1; I <= +1; ++I)
	    if((!(offmap(i+I,j+J))) &&
	       (d_terrain[(j+J)*d_width + (i+I)] != Tile::MOUNTAIN))
	      {
		if(d_terrain[(j+J)*d_width + (i+I)] != Tile::WATER)
		  d_terrain[(j+J)*d_width + (i+I)] = Tile::HILLS;
		else 
		  // water has priority here, there was some work done to conenct bodies of water
		  // so don't break those connections.
		  d_terrain[(j  )*d_width + (i  )] = Tile::HILLS;
	      }
}
