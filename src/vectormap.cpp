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
#include "vectormap.h"
#include "stacklist.h"
#include "citylist.h"
#include "ruinlist.h"
#include "templelist.h"
#include "city.h"
#include "stack.h"
#include "armysetlist.h"
#include "army.h"
#include "GameMap.h"
#include "path.h"
#include "ruin.h"
#include "temple.h"
#include "playerlist.h"
#include "player.h"
#include "config.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

VectorMap::VectorMap(City * city, PG_Widget* parent, PG_Rect rect)
  :OverviewMap(parent, rect), d_city(city)
{
}

VectorMap::~VectorMap()
{
}

void VectorMap::eventDraw(SDL_Surface* surface, const PG_Rect& rect)
{
    debug("eventDraw()");

    // most of the drawing code (the terrain, cities etc.) is done in OverviewMap
    OverviewMap::eventDraw(surface, rect);

    // Draw the vector
    if (d_city->getVectoring().x != -1)
    {
        debug("Draw vector")
        PG_Point start = d_city->getPos();
        PG_Point end = d_city->getVectoring();
        
        start = mapToSurface(start);
        end = mapToSurface(end);
        
        DrawLine(start.x, start.y, end.x, end.y, PG_Color(0,0,0));
    }

    DrawBorder(PG_Rect(0, 0, my_width, my_height), 1);
}

bool VectorMap::eventMouseButtonDown(const SDL_MouseButtonEvent* event)
{  
    // actualize the mouse position
    PG_Point oldpos = d_pos;
    PG_Point pos;
    d_pos = mapFromScreen(PG_Point(event->x, event->y));

    Stack* st;
    const Armysetlist* al;

    switch (event->button)
    {
        case SDL_BUTTON_LEFT:
            // Here we must check if the army produced by the city can be sent
            // to the vector location, so we create a temporary stack and fill
            // it with an army that is the current production of the city.
            // We calculate if there exists a path that brings it from the city
            // to the vector location. If the path exists, we set the city
            // vectoring, otherwise we unset the vectoring.

            // UL: Do we really want to do all this or shouldn't we rather trust
            // the user's sanity? There areenough cases to break this algorithm
            // (e.g. producing ships vs. producing land units etc.)
            
            // first, create the stack
            pos = d_city->getPos();
            st = new Stack(Playerlist::getActiveplayer(),pos);

            debug("Vectoring from =" << pos.x << "," << pos.y)

            al = Armysetlist::getInstance();
            Uint32 set;
            int index;
            int val;

            if (d_city->getAdvancedProd())
            {
                set = d_city->getPlayer()->getArmyset();
                index = d_city->getArmytype(d_city->getProductionIndex(),true);
            }
            else
            {
                set = al->getStandardId();
                index = d_city->getArmytype(d_city->getProductionIndex(),false);
            }
        
            // The city can have no production so we calculate the path only when index != 0
            if (index != -1) 
            { 
                debug("Index=" << index)

                st->push_back(new Army(*(al->getArmy(set, index)), d_city->getPlayer()));
                val= st->getPath()->calculate(st, d_pos);
         
                debug("Vectoring to   =" << d_pos.x << "," << d_pos.y)
                debug("Path Value=" << val)
            }

            if (val != 0 && index != -1)
            {
                sclickVectMouse.emit(PG_Point(d_pos.x,d_pos.y));
                d_city->setVectoring(d_pos);
            }
            else   
            {
                sclickVectMouse.emit(PG_Point(-1,-1));
                d_city->setVectoring(PG_Point(-1,-1));
            }

            delete st;
            break;

        case SDL_BUTTON_RIGHT:
              
            // We use the right button to unset the city vectoring 
            sclickVectMouse.emit(PG_Point(-1,-1));
            d_city->setVectoring(PG_Point(-1,-1));
            break;

        default:
            break;
    }

    Redraw();
    return true;
}

bool VectorMap::eventMouseMotion(const SDL_MouseMotionEvent* event)
{
    // actualize the mouse position
    PG_Point oldpos = d_pos;
    d_pos = mapFromScreen(PG_Point(event->x, event->y));

    int diff_xtiles = d_pos.x - oldpos.x;
    int diff_ytiles = d_pos.y - oldpos.y;

    if ((diff_xtiles == 0) && (diff_ytiles == 0))
        return true;

    smovVectMouse.emit(PG_Point(d_pos.x,d_pos.y));
    
    return true;
}

void VectorMap::eventMouseLeave()
{
    //set the pos value to a negative value
    d_pos.x = -1;
    d_pos.y = -1;
    smovVectMouse.emit(PG_Point(-1,-1));
}
