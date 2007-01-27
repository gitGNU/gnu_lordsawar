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
#include "scroller.h"
#include "w_edit.h"
//#include "defs.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

Scroller::Scroller(PG_Widget* parent, PG_Rect rect, int id, SmallMap * smap, int x, int y)
    :PG_Button(parent, rect,"",id)
{
    debug("scroller constructor")
    d_timer=0;
    d_smap=smap;
    d_x=x;
    d_y=y;
    d_time=800;
    d_firsttime=true;
}

Scroller::~Scroller()
{
   debug("Scroller destructor!");
}

void Scroller::eventMouseEnter()
{  
    debug("mouse enter detected")

    d_timer=AddTimer(1);
    PG_Button::eventMouseEnter();
    return;
}

void Scroller::eventMouseLeave()
{
    debug("mouse exit detected")
    if (d_timer != 0)
    {
        RemoveTimer(d_timer);
        d_timer = 0;
    }
    PG_Button::eventMouseLeave();
    d_time=800;
    d_firsttime=true;
    return;
}

Uint32 Scroller::eventTimer(ID id, Uint32 interval)
{
    debug("Scroll start!")
    if (d_firsttime)
    {
        d_firsttime=false;
        SDL_Delay(400);
        return interval; 
    } 
    d_smap->inputFunction(d_x,d_y);            
    SDL_Delay(d_time);
    if (d_time!=100) d_time= d_time/2; 

    return interval;
}
