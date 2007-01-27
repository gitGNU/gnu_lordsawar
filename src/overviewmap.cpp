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
#include <pgdraw.h>
#include <pgapplication.h>

#include "overviewmap.h"
#include "stacklist.h"
#include "citylist.h"
#include "ruinlist.h"
#include "templelist.h"
#include "city.h"
#include "ruin.h"
#include "temple.h"
#include "playerlist.h"
#include "player.h"
#include "GameMap.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

OverviewMap::OverviewMap(PG_Widget* parent, PG_Rect rect)
    :PG_Widget(parent, rect, true), d_staticMap(0)
{
    createStaticMap();
}

OverviewMap::~OverviewMap()
{
    SDL_FreeSurface(d_staticMap);
}

void OverviewMap::createStaticMap(Uint16 xsize, Uint16 ysize)
{
    if (d_staticMap != 0)
        SDL_FreeSurface(d_staticMap);

    if (!xsize || !ysize)
    {
        xsize = my_width;
        ysize = my_height;
    }
    
    // calculate the width and height relations between pixels and maptiles
    d_xpixels = (xsize-2) * (1.0 / GameMap::getWidth());
    d_ypixels = (ysize-2) * (1.0 / GameMap::getHeight());
    
    SDL_PixelFormat* fmt = PG_Application::GetScreen()->format;
    d_staticMap = SDL_CreateRGBSurface(SDL_SWSURFACE, xsize-2, 
            ysize-2, fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask,
            fmt->Bmask, fmt->Amask);
    
    // Draw map
    for(int i = 0; i < d_staticMap->w; i++)
        for(int j = 0; j < d_staticMap->h; j++)
        {
            int x = int(i / d_xpixels);
            int y = int(j / d_ypixels);

	    if (GameMap::getInstance()->getTile(x,y)->getBuilding() == Maptile::ROAD)
                 PG_Draw::SetPixel(i, j, PG_Color(255,255,255), d_staticMap);
	    else
	    {
                 SDL_Color c = GameMap::getInstance()->getTile(x,y)->getColor();
                 PG_Draw::SetPixel(i, j, PG_Color(c), d_staticMap);
	    }
        }
}

void OverviewMap::eventDraw(SDL_Surface* surface, const PG_Rect& rect)
{
    debug("eventDraw()");

    // During the whole drawing stuff, ALWAYS consider that 
    // there is an offset of 1 between map coordinates and coordinates
    // of the surface when drawing. I will implcitely assume this during this
    // function.

    //    SDL_mutexP(d_lock);
    
    PG_Rect r(1, 1, d_staticMap->w, d_staticMap->h);
    SDL_BlitSurface(d_staticMap, 0, surface, &r);

    // minimum size for typical features is 1
    int size = (int(d_xpixels) > 1? int(d_xpixels) : 1);

    // Draw all cities as rectangles around the city location. They consist
    // of an outer rectangle in black and inner ones in player colors.
    for (Citylist::iterator it = Citylist::getInstance()->begin();
        it != Citylist::getInstance()->end(); it++)
    {    
        SDL_Color c = (*it).getPlayer()->getColor();
        PG_Point pos = (*it).getPos();
        pos = mapToSurface(pos);

        // Fill a rect with the player colors
        for (int i=0; i < size; i++) 
            DrawRectWH(pos.x + i, pos.y + i, 2*size - (i*2), 2*size - (i*2), PG_Color(c));

        // if we have space, draw a black rect around the city
        if (size > 1)
            DrawRectWH(pos.x, pos.y, 2*size, 2*size, PG_Color(0, 0, 0)); 
    }

    // Draw ruins as yellow boxes
    for (Ruinlist::iterator it = Ruinlist::getInstance()->begin();
        it != Ruinlist::getInstance()->end(); it++)
    {
        PG_Point pos = (*it).getPos();
	PG_Color ruinColor;
        pos = mapToSurface(pos);
	if ((*it).isSearched() == true)
	   ruinColor = PG_Color(0xa1,0x4e,0x11);
	else
	   ruinColor = PG_Color(255,255,0);
	

        for (int i=0; i < size; i++) 
            DrawRectWH(pos.x + i, pos.y + i, 2*size - (i*2), 2*size - (i*2), ruinColor);
    }

    // Draw temples as magenta pi's
    for (Templelist::iterator it = Templelist::getInstance()->begin();
        it != Templelist::getInstance()->end(); it++)
    {
        PG_Point pos = (*it).getPos();
        pos = mapToSurface(pos);

        // Note: The old version had a lot of bug/error checking here (when drawing
        // outside map borders). This should not create serious problems (at least
        // I think), but as a reminder.
        DrawHLine(pos.x - size, pos.y - size, 3*size, PG_Color(255,0,255));
        DrawVLine(pos.x - size, pos.y - size, 3*size, PG_Color(255,0,255));
        DrawVLine(pos.x + size, pos.y - size, 3*size, PG_Color(255,0,255));
    }

    // Draw stacks as crosses using the player color
    for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
        pit != Playerlist::getInstance()->end(); pit++)
    {
        Stacklist* mylist = (*pit)->getStacklist();
        SDL_Color c = (*pit)->getColor();
        
        for (Stacklist::iterator it= mylist->begin(); it != mylist->end(); it++)
        {
            PG_Point pos = (*it)->getPos();

            //don't draw stacks in cities, they could hardly be identified
            Maptile* mytile = GameMap::getInstance()->getTile(pos.x, pos.y);
            if (mytile->getBuilding() == Maptile::CITY)
                continue;

            pos = mapToSurface(pos);
            DrawHLine(pos.x - 2*size, pos.y, 4*size, PG_Color(c));
            DrawVLine(pos.x, pos.y - 2*size, 4*size, PG_Color(c));
        }
    }

    //    SDL_mutexV(d_lock);
}

void OverviewMap::eventSizeWidget(Uint16 w, Uint16 h)
{
    createStaticMap(w, h);
    Redraw();
}

PG_Point OverviewMap::mapFromScreen(PG_Point pos)
{
    // subtract the widget's location and 1 for the border
    int x = int((pos.x - my_xpos - 1) / d_xpixels);
    int y = int((pos.y - my_ypos - 1) / d_ypixels);
    
    if (x >= GameMap::getWidth())
        x = GameMap::getWidth() - 1;

    if (y >= GameMap::getHeight())
        y = GameMap::getHeight() - 1;
    
    return PG_Point(x,y);
}

PG_Point OverviewMap::mapToSurface(PG_Point pos)
{
    if (pos.x < 0 || pos.y < 0 || pos.x > GameMap::getWidth()
            || pos.y > GameMap::getHeight())
    {
        std::cerr <<"OverviewMap:: mapping to surface with invalid point\n";
        return PG_Point(-1,-1);
    }
    
    int x = 1 + int(pos.x * d_xpixels);
    int y = 1 + int(pos.y * d_ypixels);

    if (d_xpixels > 2)
        // try to take the center position of the pixel
        x += int(0.5 * d_xpixels);

    if (d_ypixels > 2)
        y += int(0.5 * d_ypixels);
    
    return PG_Point(x, y);
}
