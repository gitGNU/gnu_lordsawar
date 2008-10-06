//  Copyright (C) 2007, 2008 Ben Asselstine
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

#include <iostream>
#include <sstream>

#include "FogMap.h"
#include "SightMap.h"

#include "playerlist.h"
#include "xmlhelper.h"
#include "GameScenarioOptions.h"

std::string FogMap::d_tag = "fogmap";

using namespace std;

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<flush;}
#define debug(x)


FogMap::FogMap(int width, int height)
{
    debug("FogMap()");
    d_width = width;
    d_height = height;

    d_fogmap = new FogType[d_width*d_height];

    fill(OPEN);
}

FogMap::FogMap(XML_Helper* helper)
{
    std::string types;
    
    helper->getData(d_width, "width");
    helper->getData(d_height, "height");
    helper->getData(types, "map");

    //create the map
    d_fogmap = new FogType[d_width*d_height];

    for (int y = 0; y < d_height; y++)
    {
        for (int x = 0; x < d_width; x++)
        {
            //due to the circumstances, types is a long stream of
            //numbers, so read it character for character (no \n's or so)
            d_fogmap[y*d_width + x] = FogType(types[y*d_width + x] - '0');
        }
    }
}

FogMap::FogMap(const FogMap& fogmap)
    :d_width(fogmap.d_width), d_height(fogmap.d_height)
{
    //create the map
    d_fogmap = new FogType[d_width*d_height];

    for (int y = 0; y < d_height; y++)
    {
        for (int x = 0; x < d_width; x++)
        {
            d_fogmap[y*d_width + x] = fogmap.d_fogmap[y*d_width + x];
        }
    }
}

FogMap::~FogMap()
{
    delete[] d_fogmap;
}

bool FogMap::fill(FogType type)
{
    for (int i = 0; i < d_width*d_height; i++)
        d_fogmap[i] = type;

    return true;
}

bool FogMap::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(FogMap::d_tag);
    retval &= helper->saveData("width", d_width);
    retval &= helper->saveData("height", d_height);

    std::stringstream types;
    types << std::endl;
    for (int y = 0; y < d_height; y++)
    {
        for (int x = 0; x < d_width; x++)
        {
            types << static_cast<int>(d_fogmap[y*d_width + x]);
        }
        types << std::endl;
    }

    retval &= helper->saveData("map", types.str());
    retval &= helper->closeTag();

    return retval;
}

FogMap::FogType FogMap::getFogTile(Vector<int> pos) const
{
    return d_fogmap[pos.y * d_width + pos.x];
}

void FogMap::alterFogRadius(Vector<int> pt, int radius, FogType new_type)
{
    int x = pt.x - radius;
    int y = pt.y - radius;
    int size = 2 * radius + 1;
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if ((x+i) < 0 || (y+j) < 0 || (x+i) >= d_width || (y+j) >= d_height)
                continue;
            d_fogmap[(y+j)*d_width + (x+i)] = new_type;
        }
    }
}

void FogMap::alterFogRectangle(Vector<int> pt, int height, int width, FogType new_type)
{
    int x = pt.x;
    int y = pt.y;
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if ((x+i) < 0 || (y+j) < 0 || (x+i) >= d_width || (y+j) >= d_height)
                continue;
            d_fogmap[(y+j)*d_width + (x+i)] = new_type;
        }
    }
}

bool FogMap::isCompletelyObscuredFogTile(Vector<int> pos)
{
  bool foggyTile;
  for (int i = pos.x - 1; i <= pos.x + 1; i++)
    for (int j = pos.y - 1; j <= pos.y + 1; j++)
      {
	foggyTile = false;
	if (i == pos.x && j == pos.y)
	  continue;
	if (i < 0 || j < 0 || 
	    i >= FogMap::getWidth() || j >= FogMap::getHeight())
	  foggyTile = true;
	else
	  {
	    Vector<int> pos = Vector<int>(i, j);
	    foggyTile = FogMap::isFogged(pos);
	  }
	if (foggyTile == false)
	  return false;
      }
  return true;
}

bool FogMap::isLoneFogTile(Vector<int> pos)
{
  bool west_open = false;
  bool east_open = false;
  //are east-west adjacent squares open?
  if (pos.x + 1 >= d_width || d_fogmap[pos.y*d_width + pos.x + 1] == OPEN)
    west_open = true;
  if (pos.x - 1 < 0 || d_fogmap[pos.y*d_width + pos.x - 1] == OPEN)
    east_open = true;
  bool north_open = false;
  bool south_open = false;
  //are north-south adjacent squares open?
  if (pos.y + 1 >= d_height || d_fogmap[(pos.y+1)*d_width + pos.x] == OPEN)
    south_open = true;
  if (pos.y - 1 < 0 || d_fogmap[(pos.y-1)*d_width + pos.x] == OPEN)
    north_open = true;
  if (east_open && west_open)
    return true;
  if (north_open && south_open)
    return true;
  return false;
}

void FogMap::smooth()
{
    for (int y = 0; y < d_height; y++)
    {
        for (int x = 0; x < d_width; x++)
        {
            if (d_fogmap[y*d_width + x] == CLOSED)
              {
		Vector<int> pos;
		pos.x = x;
		pos.y = y;
                if (isLoneFogTile (pos))
                  d_fogmap[y*d_width + x] = OPEN;
              }
        }
    }
}

bool FogMap::isFogged(Vector <int> pos)
{
  //is this tile visible, or not?
  FogMap *fogmap = Playerlist::getActiveplayer()->getFogMap();
  if (fogmap->getFogTile(pos) == FogMap::CLOSED)
    return true;
  if (Playerlist::getActiveplayer())
    if (Playerlist::getActiveplayer()->getType() != Player::HUMAN &&
	GameScenarioOptions::s_hidden_map == true)
      return true;
                
  if (fogmap->isLoneFogTile(pos) == true)
    return false;

  return false;
}
	
void FogMap::alterFog(SightMap *sightmap)
{
  return alterFogRectangle(sightmap->pos, sightmap->h, sightmap->w, OPEN);
}
// End of file
