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

#include "tileset.h"

#include "File.h"
#include "xmlhelper.h"

using namespace std;

#include <iostream>
//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

TileSet::TileSet(XML_Helper *helper)
{
    helper->getData(d_name, "name"); 
    helper->getData(d_info, "info");
    helper->getData(d_tileSize, "tilesize");
    helper->registerTag("tile", sigc::mem_fun((*this), &TileSet::loadTile));
    helper->registerTag("smallmap", sigc::mem_fun((*this), &TileSet::loadTile));
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

void TileSet::instantiatePixmaps()
{
  //Get mapset picture
  d_surface = File::getMapsetPicture(d_dir, d_dir + ".png");
  d_nepic = File::getMapsetPicture(d_dir, "ne_bridge.png");
  d_nwpic = File::getMapsetPicture(d_dir, "nw_bridge.png");
  for (unsigned int i = 0; i < size(); i++)
    createTiles((*this)[i], i);
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

bool TileSet::loadTile(string tag, XML_Helper* helper)
{
    debug("loadTile()")

    if (tag == "tile")
      {
	// create a new tile with the information we got
	Tile* tile = new Tile(helper);
	this->push_back(tile);

	return true;
      }

    if (tag == "smallmap")
      {
	Uint32 i;
	SDL_Color color;
	color.unused = 0;
	Tile *tile = this->back();
	helper->getData(i, "red");      color.r = i;
	helper->getData(i, "green");    color.g = i;
	helper->getData(i, "blue");     color.b = i;
	tile->setColor(color);

	helper->getData(i, "pattern");
	Tile::Pattern pattern = static_cast<Tile::Pattern>(i);
	tile->setPattern(pattern);
    
	if (pattern != Tile::SOLID)
	  {
	    helper->getData(i, "2nd_red");      color.r = i;
	    helper->getData(i, "2nd_green");    color.g = i;
	    helper->getData(i, "2nd_blue");     color.b = i;
	    tile->setSecondColor(color);
	    if (pattern != Tile::STIPPLED && pattern != Tile::SUNKEN)
	      {
		helper->getData(i, "3rd_red");      color.r = i;
		helper->getData(i, "3rd_green");    color.g = i;
		helper->getData(i, "3rd_blue");     color.b = i;
		tile->setThirdColor(color);
	      }
	  }
	return true;
      }

    return false;
}

// createTiles creates all corners out of the 8 given tiles

void TileSet::createTiles(Tile* tile, int row)
{
    SDL_Rect src;
    src.w = d_tileSize/2;
    src.h = d_tileSize/2;

    const SDL_Surface* screen = SDL_GetVideoSurface();
    SDL_PixelFormat* fmt = screen->format;
    SDL_Surface* tmp = SDL_CreateRGBSurface(SDL_HWSURFACE, d_tileSize / 2, 
					    d_tileSize / 2, fmt->BitsPerPixel,
					    fmt->Rmask, fmt->Gmask, 
					    fmt->Bmask, fmt->Amask);
    
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
