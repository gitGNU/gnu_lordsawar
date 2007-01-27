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

#include "Smallmap.h"

#include "../GameMap.h"
#include "../citylist.h"
#include "../ruinlist.h"
#include "../templelist.h"
#include "../stacklist.h"
#include "../playerlist.h"

E_Smallmap::E_Smallmap(PG_Widget* parent, PG_Rect rect, int tilex, int tiley)
    :PG_Widget(parent, rect, true), d_pressed(false)
{
    d_rect.w = tilex;
    d_rect.h = tiley;
    d_rect.x = d_rect.y = 0;
}

E_Smallmap::~E_Smallmap()
{
}

void E_Smallmap::centerView(const PG_Point pos)
{
    // the coordinates give us the center of the view
    // if the view is already centered, do nothing
    if ((d_rect.x == pos.x - d_rect.w/2) && (d_rect.y == pos.y - d_rect.h/2))
        return;

    // center the view appropriately
    d_rect.x = pos.x - d_rect.w/2;
    d_rect.y = pos.y - d_rect.h/2;

    // catch possible errors
    if (d_rect.x < 0)
        d_rect.x = 0;
    if (d_rect.y < 0)
        d_rect.y = 0;
    if (d_rect.x + d_rect.w > GameMap::getWidth())
        d_rect.x = GameMap::getWidth() - d_rect.w;
    if (d_rect.y + d_rect.h > GameMap::getHeight())
        d_rect.y = GameMap::getHeight() - d_rect.h;
    
    // and emit the signal
    Redraw();
    schangingView.emit(PG_Point(d_rect.x+d_rect.w/2, d_rect.y+d_rect.h/2));
}

void E_Smallmap::setView(int x, int y)
{
    // the coordinates x and y are given in absolute values (absolute to the
    // program), we need to modify them a bit
    x -= my_xpos + 1;
    y -= my_ypos + 1;

    // change the viewrect
    centerView(PG_Point(x, y));
}

void E_Smallmap::eventDraw(SDL_Surface* surface, const PG_Rect& rect)
{
    // During drawng, always consider that there is an offset (due to the
    // border) between map coordinates and surface coordinates of +1
    // (i.e. map coordinate (x,y) maps to surface coordinate (x+1, y+1).
    // I will implicitely assume this in the drawing code here.
    
    // First, draw the terrain
    int width = GameMap::getWidth();
    int height = GameMap::getHeight();
    GameMap* map = GameMap::getInstance();
    
    for (int i = 0; i < width; i++)
        for (int j = 0; j < height; j++)
        {
	    if (map->getTile(i,j)->getBuilding() == Maptile::ROAD)
                 SetPixel(i+1, j+1, PG_Color(255, 255, 255));
	    else
	    {
                 SDL_Color c = map->getTile(i, j)->getColor();
                 SetPixel(i+1, j+1, PG_Color(c.r, c.g, c.b));
	    }
        }

    // draw cities as a rect of 4x4 pixels in black with an inner 2x2 rect
    // of player color
    Citylist* cl = Citylist::getInstance();
    for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
    {
        SDL_Color c = (*it).getPlayer()->getColor();
        PG_Point pos = (*it).getPos();

        DrawRectWH(pos.x, pos.y, 4, 4, PG_Color(0,0,0));
        DrawRectWH(pos.x+1, pos.y+1, 2, 2, PG_Color(c));
    }

    // draw ruins as yellow boxes
    Ruinlist* rl = Ruinlist::getInstance();
    for (Ruinlist::iterator it = rl->begin(); it != rl->end(); it++)
    {
        PG_Point pos = (*it).getPos();
        DrawRectWH(pos.x, pos.y, 3, 3, PG_Color(255, 255, 0));
    }

    // draw temples as magenta pi's
    Templelist* tl = Templelist::getInstance();
    for (Templelist::iterator it = tl->begin(); it != tl->end(); it++)
    {
        PG_Point pos = (*it).getPos();
        DrawHLine(pos.x, pos.y, 3, PG_Color(255, 0, 255));
        DrawVLine(pos.x, pos.y+1, 2, PG_Color(255, 0, 255));
        DrawVLine(pos.x+2, pos.y+1, 2, PG_Color(255, 0, 255));
    }

    // draw stacks as crosses in the player color
    Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::iterator it = pl->begin(); it != pl->end(); it++)
    {
        Stacklist* sl = (*it)->getStacklist();
        SDL_Color c = (*it)->getColor();
        
        for (Stacklist::iterator it = sl->begin(); it != sl->end(); it++)
        {
            PG_Point pos = (*it)->getPos();
            
            // don't draw stacks on cities, doesn't look good
            if (map->getTile(pos.x, pos.y)->getBuilding() == Maptile::CITY)
                continue;
            
            DrawHLine(pos.x, pos.y+1, 3, PG_Color(c));
            DrawVLine(pos.x+1, pos.y, 3, PG_Color(c));
        }
    }

    // draw the border and the viewrect, drawing over everything we have done so
    // far. This way, we don't need to care for drawing over the border
    DrawBorder(PG_Rect(0, 0, my_width, my_height), 1);
    DrawRectWH(d_rect.x+1, d_rect.y+1, d_rect.w, d_rect.h, PG_Color(200, 200, 100));
}

void E_Smallmap::eventMouseLeave()
{
    d_pressed = false;
}

bool E_Smallmap::eventMouseMotion(const SDL_MouseMotionEvent* ev)
{
    if (d_pressed)
        setView(ev->x, ev->y);

    return true;
}

bool E_Smallmap::eventMouseButtonDown(const SDL_MouseButtonEvent* ev)
{
    if (ev->button == SDL_BUTTON_LEFT)
    {
        d_pressed = true;
        setView(ev->x, ev->y);
    }
    
    return true;
}

bool E_Smallmap::eventMouseButtonUp(const SDL_MouseButtonEvent* ev)
{
    if (ev->button == SDL_BUTTON_LEFT)
        d_pressed = false;
    
    return true;
}
