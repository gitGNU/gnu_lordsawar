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

#include <iostream>
#include <sstream>

#include "FogMap.h"
#include "GameMap.h"

#include "playerlist.h"
#include "xmlhelper.h"

using namespace std;

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<flush;}
#define debug(x)


FogMap::FogMap()
{
    debug("FogMap()");
    d_width = GameMap::getInstance()->getWidth();
    d_height = GameMap::getInstance()->getHeight();

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
            d_fogmap[y*d_width + x] = static_cast<FogType>(types[y*d_width + x]);
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

    retval &= helper->openTag("fogmap");
    retval &= helper->saveData("width", d_width);
    retval &= helper->saveData("height", d_height);
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
// End of file
