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

#include <config.h>

#include <SDL_image.h>
#include <assert.h>
#include <stdlib.h>

#include "gamebigmap.h"

#include "army.h"
#include "path.h"
#include "stacklist.h"
#include "stack.h"
#include "citylist.h"
#include "ruinlist.h"
#include "signpostlist.h"
#include "templelist.h"
#include "stonelist.h"
#include "roadlist.h"
#include "ruin.h"
#include "signpost.h"
#include "temple.h"
#include "stone.h"
#include "road.h"
#include "playerlist.h"
#include "defs.h"
#include "File.h"
#include "GameMap.h"
#include "Configuration.h"
#include "GraphicsCache.h"

#include "timing.h"


#include <iostream>
using namespace std;
//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

namespace 
{
    int selection_timeout = 150;	// controls speed of selector rotation
}


GameBigMap::GameBigMap()
{
    current_tile.x = current_tile.y = 0;
    mouse_state = NONE;
    input_locked = false;
    
    d_arrows = File::getMiscPicture("arrows.png");
    prev_mouse_pos = Vector<int>(0, 0);
    
    // setup timeout
    selection_timeout_handler = 
	Timing::instance().register_timer(
	    sigc::mem_fun(*this, &GameBigMap::on_selection_timeout),
		selection_timeout);
}

GameBigMap::~GameBigMap()
{
    SDL_FreeSurface(d_arrows);
}

void GameBigMap::select_active_stack()
{
    Stack* stack = Playerlist::getActiveplayer()->getActivestack();
    if (!stack)
        return;

    if (!stack->getPath()->checkPath(stack))
    {
        //error handling is required here, up to now we only barf on cerr
        cerr << _("original path of stack was blocked\n");
    }

    center_view(stack->getPos());

    stack_selected.emit(stack);
}

void GameBigMap::unselect_active_stack()
{
    draw();
    stack_selected.emit(0);
}

bool GameBigMap::on_selection_timeout()
{
    // redraw to update the selection
    if (Playerlist::getActiveplayer()->getActivestack())
	draw();

    return Timing::CONTINUE;
}

void GameBigMap::mouse_button_event(MouseButtonEvent e)
{
    if (input_locked)
	return;
    
    Vector<int> tile = mouse_pos_to_tile(e.pos);
    
    if (e.button == MouseButtonEvent::LEFT_BUTTON
	&& e.state == MouseButtonEvent::PRESSED)
    {
        Stack* stack = Playerlist::getActiveplayer()->getActivestack();

        if (stack)
        {
            Vector<int> p;
            p.x = tile.x; p.y = tile.y;

            // clicked on an already active stack
            if (stack->getPos().x == tile.x && stack->getPos().y == tile.y)
	    {
		// clear the path
                stack->getPath()->flClear();
		path_set.emit();
		draw();
		return;
	    }

	    // split if ungrouped
            Playerlist::getActiveplayer()->stackSplit(stack); 
            
            Vector<int>* dest = 0;
            if (!stack->getPath()->empty())
                dest = *stack->getPath()->rbegin();
            
            if (dest && dest->x == tile.x && dest->y == tile.y)
            {
                Playerlist::getActiveplayer()->stackMove(stack);
                if (!Playerlist::getActiveplayer()->getActivestack())
                  return;
            }
            else
                stack->getPath()->calculate(stack, p);

	    path_set.emit();
	    
            draw();
        }
        // Stack hasn't been active yet
        else
        {
            stack = Stacklist::getObjectAt(tile.x, tile.y);
            if (stack && stack->isFriend(Playerlist::getActiveplayer()))
            {
                Playerlist::getActiveplayer()->getStacklist()->setActivestack(stack);
                select_active_stack();
            }
            else
            {
              if (City* c = Citylist::getInstance()->getObjectAt(tile.x, tile.y))
              {
	          if (c->getPlayer() == Playerlist::getActiveplayer() && !c->isBurnt())
                    city_selected(c, false);
              }
            }
        }
    }

	    
    // right mousebutton to get information about things on the map and to
    // unselect the active stack
    else if (e.button == MouseButtonEvent::RIGHT_BUTTON)
    {
	if (e.state == MouseButtonEvent::PRESSED)
	{
	    if (City* c = Citylist::getInstance()->getObjectAt(tile.x, tile.y))
	    {
		city_selected(c, true);
	        mouse_state = SHOWING_CITY;
	    }
	    else if (Ruin* r = Ruinlist::getInstance()->getObjectAt(tile.x, tile.y))
	    {
                if ((r->isHidden() == true && 
                      r->getOwner() == Playerlist::getActiveplayer()) ||
                     r->isHidden() == false)
                  {
		    ruin_selected(r);
		    mouse_state = SHOWING_RUIN;
                  }
	    }
	    else if (Signpost* s = Signpostlist::getInstance()->getObjectAt(tile.x, tile.y))
	    {
		signpost_selected(s);
		mouse_state = SHOWING_SIGNPOST;
	    }
	    else if (Temple* t = Templelist::getInstance()->getObjectAt(tile.x, tile.y))
	    {
		temple_selected.emit(t);
		mouse_state = SHOWING_TEMPLE;
	    }
	}
	else // button released
	{
	    switch(mouse_state)
	    {
	    case DRAGGING:
		break;

	    case SHOWING_CITY:
		city_selected.emit(0, true);
		break;

	    case SHOWING_RUIN:
		ruin_selected.emit(0);
		break;

	    case SHOWING_TEMPLE:
		temple_selected.emit(0);
		break;

	    case SHOWING_SIGNPOST:
		signpost_selected.emit(0);
		break;
		
	    case NONE:
		Stack* stack = Playerlist::getActiveplayer()->getActivestack();
		if (stack)
		{
		    Playerlist::getActiveplayer()->getStacklist()->setActivestack(0);
		    unselect_active_stack();
		}
		break;
	    }

	    // in any case reset mouse state
	    mouse_state = NONE;
	}
    }
}

void GameBigMap::mouse_motion_event(MouseMotionEvent e)
{
    if (input_locked)
	return;
    
    // drag with right mouse button
    if (e.pressed[MouseMotionEvent::RIGHT_BUTTON]
	&& (mouse_state == NONE || mouse_state == DRAGGING))
    {
	Vector<int> delta = -(e.pos - prev_mouse_pos);

	// ignore very small drags to ensure that a shaking mouse does not
	// prevent the user from making right clicks
	if (mouse_state == NONE && length(delta) <= 2)
	    return;
	
	// FIXME: show a drag cursor
	
	int ts = GameMap::getInstance()->getTileSet()->getTileSize();
	SDL_Surface *screen = SDL_GetVideoSurface();
	Vector<int> screen_dim(screen->w, screen->h);
	view_pos = clip(Vector<int>(0, 0),
			view_pos + delta,
			GameMap::get_dim() * ts - screen_dim);

	// calculate new view position in tiles, rounding up
	Vector<int> new_view = (view_pos + Vector<int>(ts - 1, ts - 1)) / ts;

	bool redraw_buffer = false;
	
	if (new_view != view.pos)
	{
	    view.x = new_view.x;
	    view.y = new_view.y;
	    view_changed.emit(view);
	    redraw_buffer = true;
	}

	draw(redraw_buffer);
	mouse_state = DRAGGING;
    }

    prev_mouse_pos = e.pos;
}

void GameBigMap::after_draw()
{
    int tilesize = GameMap::getInstance()->getTileSet()->getTileSize();
    
    Stack* stack = Playerlist::getActiveplayer()->getActivestack();

    // Draw Path
    if (stack && stack->getPath()->size())
    {
        Vector<int> pos;
        Vector<int> nextpos;
        SDL_Color c;
        c.r = c.g = c.b = 0;

        // draw all waypoints except the last
        for (list<Vector<int>*>::iterator it = stack->getPath()->begin();
	     it != --(stack->getPath()->end());)
        {
            // peak at the next waypoint to draw the correct arrow
            pos = tile_to_buffer_pos(**it);
            nextpos = tile_to_buffer_pos(**(++it));
	    SDL_Rect r1, r2;
	    r1.y = 0;
	    r1.w = r1.h = tilesize;
	    r2.x = pos.x;
	    r2.y = pos.y;
	    r2.w = r2.h = tilesize;

            if (nextpos.x == pos.x && nextpos.y < pos.y)
                r1.x = 0;
            else if (nextpos.x > pos.x && nextpos.y < pos.y)
                r1.x = tilesize;
            else if (nextpos.x > pos.x && nextpos.y == pos.y)
                r1.x = 2 * tilesize;
            else if (nextpos.x > pos.x && nextpos.y > pos.y)
                r1.x = 3 * tilesize;
            else if (nextpos.x == pos.x && nextpos.y > pos.y)
                r1.x = 4 * tilesize;
            else if (nextpos.x < pos.x && nextpos.y > pos.y)
                r1.x = 5 * tilesize;
            else if (nextpos.x < pos.x && nextpos.y == pos.y)
                r1.x = 6 * tilesize;
            else if (nextpos.x < pos.x && nextpos.y < pos.y)
                r1.x = 7 * tilesize;

            SDL_BlitSurface(d_arrows, &r1, buffer, &r2);

        }

        pos = tile_to_buffer_pos(*stack->getPath()->back());
	SDL_Rect r1, r2;
	r1.x = 8 * tilesize;
	r1.y = 0;
	r1.w = r1.h = tilesize;
	r2.x = pos.x;
	r2.y = pos.y;
	r2.w = r2.h = tilesize;
        SDL_BlitSurface(d_arrows, &r1, buffer, &r2);
    }

    if (stack)
    {
	// draw the selection
	Vector<int> p = stack->getPos();
	if (is_inside(buffer_view, Vector<int>(p.x, p.y)))
	{
	    static int bigframe = -1;
	    static int smallframe = -1;
        
	    bigframe++;
	    if (bigframe > 5)
		bigframe = 0;

	    smallframe++;
	    if (smallframe > 3)
		smallframe = 0;
	    
	    p = tile_to_buffer_pos(p);
	    SDL_Rect r;
	    r.x = p.x;
	    r.y = p.y;
	    r.w = r.h = tilesize;
	    GraphicsCache *gc = GraphicsCache::getInstance();
	    Player *p = Playerlist::getActiveplayer();
	    SDL_Surface *tmp;
            int num_selected = 0;
            for (Stack::iterator it = stack->begin(); it != stack->end(); it++)
	    {
                if ((*it)->isGrouped())
		    num_selected++;
	    }
	    if (num_selected > 1)
		tmp = gc->getSelectorPic(0, bigframe, p);
	    else
		tmp = gc->getSelectorPic(1, smallframe, p);
            SDL_BlitSurface(tmp, 0, buffer, &r);
	}
    }
}
