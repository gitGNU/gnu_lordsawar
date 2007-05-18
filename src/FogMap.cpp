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

    d_fogmap = new TYPE[d_width*d_height];

    fill(COVERED);
}

FogMap::FogMap(XML_Helper* helper)
{
    std::string types;
    
    helper->getData(d_width, "width");
    helper->getData(d_height, "height");
    helper->getData(types, "map");

    //create the map
    d_fogmap = new TYPE[d_width*d_height];

    for (int y = 0; y < d_height; y++)
    {
        for (int x = 0; x < d_width; x++)
        {
            //due to the circumstances, types is a long stream of
            //numbers, so read it character for character (no \n's or so)
            d_fogmap[y*d_width + x] = static_cast<TYPE>(types[y*d_width + x]);
        }
    }
}


FogMap::~FogMap()
{
    delete[] d_fogmap;
}

bool FogMap::fill(TYPE type)
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

FogMap::TYPE FogMap::getFogTile(Vector<int> pos) const
{
    return d_fogmap[pos.y * d_width + pos.x];
}

void FogMap::alterFogRadius(Vector<int> pt, int radius, TYPE new_type)
{
    int x = pt.x - radius;
    int y = pt.y - radius;
    for (int i = 0; i < 2*radius; i++)
    {
        for (int j = 0; y < 2*radius; y++)
        {
            if ((x+i) < 0 || (y+j) < 0 || (x+i) >= d_width || (y+j) >= d_height)
                continue;
            d_fogmap[(y+j)*d_width + (x+i)] = new_type;
        }
    }
}

// End of file
