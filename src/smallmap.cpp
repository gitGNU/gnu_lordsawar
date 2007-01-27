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
#include "smallmap.h"
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
#include "config.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

SmallMap::SmallMap(PG_Widget* parent, PG_Rect rect, PG_Rect viewsize)
    :OverviewMap(parent, rect), d_pressed(false), d_r(50), d_g(50), d_b(50),
    d_add(true), d_timerID(0)
{
    d_lock = SDL_CreateMutex();
    d_viewrect = new PG_Rect(1, 1, viewsize.w, viewsize.h);

    restartTimer();
}

SmallMap::~SmallMap()
{
    interruptTimer();
    delete d_viewrect;
    SDL_DestroyMutex(d_lock);
}

void SmallMap::changeResolution(PG_Rect viewsize)
{
    if (d_viewrect)
        delete(d_viewrect);
    
    d_viewrect = new PG_Rect(1, 1, viewsize.w, viewsize.h);
}

void SmallMap::interruptTimer()
{
    if (d_timerID != 0)
        RemoveTimer(d_timerID);

    d_timerID = 0;
}

void SmallMap::restartTimer()
{
#ifndef FL_NO_TIMERS
    if (d_timerID == 0)
        d_timerID = AddTimer(TIMER_SMALLMAP_REFRESH);
#else
    return;
#endif
}

Uint32 SmallMap::eventTimer(ID id, Uint32 interval)
{
    PG_Application::ClearOldMousePosition();
    
    // change viewrect's color
    if (d_r == 255) d_add = false;
    if (d_r == 160) d_add = true;
 
    if (d_add) {++d_r; ++d_g; ++d_b;}
    else {--d_r; --d_g; --d_b;} 
    
    if (SDL_LockMutex(d_lock))
    {
        std::cerr <<"Error while locking mutex!\n";
        // uhm, let's return here
        PG_TimerObject::eventTimer(id, interval);
        return interval;
    }

    drawViewrect();
    Update();
    PG_TimerObject::eventTimer(id, interval);

    PG_Application::DrawCursor();

    if (SDL_UnlockMutex(d_lock))
        std::cerr <<"Error while unlocking mutex!\n";
    
    return interval;
}

// do some animation of the viewrect
void SmallMap::drawViewrect()
{
    DrawBorder(PG_Rect(0,0,my_width,my_height), 1);

    // Draw the rectangle that shows the viewed part of the map
    PG_Point pos = mapToSurface(PG_Point(d_viewrect->x, d_viewrect->y));
            
    DrawRectWH(pos.x, pos.y, int(d_viewrect->my_width*d_xpixels),
               int(d_viewrect->my_height*d_ypixels), PG_Color(d_r, d_g, d_b));
    SDL_UpdateRect(GetWidgetSurface(), pos.x, pos.y,
               int(d_viewrect->my_width*d_xpixels), int(d_viewrect->my_height*d_ypixels));
}

bool SmallMap::changedViewrect(PG_Widget *widget)
{
#if 0
    debug("changedViewrect()");
    SDL_Rect* rect = (SDL_Rect*)clientdata;
    d_viewrect->my_xpos = rect->x;
    d_viewrect->my_ypos = rect->y;
#endif
    Redraw();
    return true;
}

void SmallMap::inputFunction(int arrowx, int arrowy)
{
    keycheck = 1;

    int x = d_viewrect->my_xpos + arrowx;
    int y = d_viewrect->my_ypos + arrowy;

    setViewrect(x, y);
}

void SmallMap::setViewrect(int x, int y)
{
    //the x/y coordinates are absolute to the whole programm, so subtract the
    //position of the widget and a bit more so the mouse isn't in the top left
    //corner

    // This "if" is neccessary ,because a eventKeyDown ( value == 1 ) can't handle the next lines.
    if (keycheck == 0) // eventMouseButton ( value == 0 ).
    {
        PG_Point pos = mapFromScreen(PG_Point(x,y));
        x = pos.x;
        y = pos.y;
    }

    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x > (GameMap::getWidth() - d_viewrect->my_width)) 
        x = GameMap::getWidth() - d_viewrect->my_width;
    if (y > (GameMap::getHeight() - d_viewrect->my_height))
        y = GameMap::getHeight() - d_viewrect->my_height;

    d_viewrect->my_xpos = x;
    d_viewrect->my_ypos = y;

    PG_Application::ClearOldMousePosition();
    Redraw();
    schangingViewrect.emit(true);
    PG_Application::DrawCursor();
}

void SmallMap::eventDraw(SDL_Surface* surface, const PG_Rect& rect)
{
    // the only thing that is not inherited from OverviewMap is the
    // redrawing of the viewrect.
    debug("eventDraw()");

    if (SDL_LockMutex(d_lock))
    {
        std::cerr <<"Error while locking mutex!\n";
        return;
    }

    OverviewMap::eventDraw(surface, rect);

    drawViewrect();

    if (SDL_UnlockMutex(d_lock))
        std::cerr <<"Error while unlocking mutex!\n";
}

// Pressed in the drawing area to set viewrect

bool SmallMap::eventMouseButtonDown(const SDL_MouseButtonEvent* event)
{
    keycheck = 0;

    if(event->button == SDL_BUTTON_LEFT)
    {
        // the mouse position should be something like the center
        // of the new viewrect position
        int xpos = event->x - int(d_xpixels*d_viewrect->my_width/2);
        int ypos = event->y - int(d_ypixels*d_viewrect->my_height/2);

        setViewrect(xpos, ypos);
    }

    return true;
}

// This eventhandler is called when moving the mouse with pressed button in
// the drawing area to move viewrect
bool SmallMap::eventMouseMotion(const SDL_MouseMotionEvent* event) 
{ 
    if (event->state == SDL_PRESSED)
    {
        keycheck = 0;

        // the mouse position should be something like the center
        // of the new viewrect position
        int xpos = event->x - int(d_xpixels*d_viewrect->my_width/2);
        int ypos = event->y - int(d_ypixels*d_viewrect->my_height/2);

        setViewrect(xpos, ypos); 
    }

    return true; 
} 
