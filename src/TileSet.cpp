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

#include <SDL_image.h>
#include <sigc++/functors/mem_fun.h>

#include "TileSet.h"

#include "File.h"
#include "xmlhelper.h"

using namespace std;

#include <iostream>
//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

// used for loading purposes
static int my_counter = 0;

TileSet::TileSet(string tileName)
{
    debug("TileSet(): " << fileName)

    my_counter = 0;
                           
    //Get mapset picture
    d_surface = File::getMapsetPicture(tileName, tileName + ".png");
    d_nepic = File::getMapsetPicture(tileName, "ne_bridge.png");
    d_nwpic = File::getMapsetPicture(tileName, "nw_bridge.png");
    
    XML_Helper helper(File::getMapset(tileName), ios::in, false);
    helper.registerTag("tileset", sigc::mem_fun((*this), &TileSet::loadTileSet));
    helper.registerTag("tile", sigc::mem_fun((*this), &TileSet::loadTile));
    helper.parse();
}

TileSet::~TileSet()
{
    if (d_surface)
        SDL_FreeSurface(d_surface);
    if (d_nepic)
        SDL_FreeSurface(d_nepic);
    if (d_nwpic)
        SDL_FreeSurface(d_nwpic);
    for (unsigned int i=0; i < size(); i++)
        delete (*this)[i];
}

Uint32 TileSet::getIndex(Tile::Type type) const
{
    for (Uint32 i = 0; i < size(); i++)
        if (type == (*this)[i]->getType())
            return i;

    // catch errors?
    return 0;
}


SDL_Surface* TileSet::getDiagPic(int pic) const
{
    if (pic == 0)
        return d_nwpic;
    if (pic == 1)
        return d_nepic;
    return 0;
}

bool TileSet::loadTile(string, XML_Helper* helper)
{
    debug("loadTile()")

    // create a new tile with the information we got
    Tile* tile = new Tile(helper);
    createTiles(tile, my_counter);
    this->push_back(tile);

    my_counter++;
    return true;
}

bool TileSet::loadTileSet(string, XML_Helper* helper)
{
    helper->getData(d_name, "name"); 
    helper->getData(d_info, "info");
    helper->getData(d_tileSize, "tilesize");
    return true;
}

// createTiles creates all corners out of the 8 given tiles

void TileSet::createTiles(Tile* tile, int row)
{
    SDL_Rect src;
    src.w = d_tileSize/2;
    src.h = d_tileSize/2;

    const SDL_Surface* screen = SDL_GetVideoSurface();
    SDL_PixelFormat* fmt = screen->format;
    SDL_Surface* tmp = SDL_CreateRGBSurface(SDL_HWSURFACE, 32, 32, fmt->BitsPerPixel,
                    fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
    
    for (int i=0; i < 8; i++)
    {
        // corner 1
        src.x = i*d_tileSize;
        src.y = row*d_tileSize;
        SDL_BlitSurface(d_surface, &src, tmp, 0);
        tile->setSurface(i*4, SDL_DisplayFormat(tmp));

        // corner 2
        src.x = i*d_tileSize + d_tileSize/2;
        src.y = row*d_tileSize;
        SDL_BlitSurface(d_surface, &src, tmp, 0);
        tile->setSurface(i*4+1, SDL_DisplayFormat(tmp));

        // corner 3
        src.x = i*d_tileSize;
        src.y = row*d_tileSize + d_tileSize/2;
        SDL_BlitSurface(d_surface, &src, tmp, 0);
        tile->setSurface(i*4+2, SDL_DisplayFormat(tmp));

        // corner 4
        src.x = i*d_tileSize + d_tileSize/2;
        src.y = row*d_tileSize + d_tileSize/2;
        SDL_BlitSurface(d_surface, &src, tmp, 0);
        tile->setSurface(i*4+3, SDL_DisplayFormat(tmp));
    }

    SDL_FreeSurface(tmp);
}

// End of file
