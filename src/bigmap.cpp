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
#include <sigc++/hide.h>
#include <sigc++/bind.h>

#include "bigmap.h"

//#include "cityinfo.h"
//#include "ruininfo.h"
//#include "SignpostInfo.h"
//#include "TempleInfo.h"
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
#include "MapRenderer.h"

#include "timing.h"


#include <iostream>
using namespace std;
//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

namespace 
{
    int selection_timeout = 500;	// 1/2 second
}


BigMap::BigMap()
    : d_renderer(0), buffer(0), current_tile(0, 0)
{
    mouse_state = NONE;
    d_enable = true;

    // load all pictures
    d_arrows = File::getMiscPicture("arrows.png");
    d_ruinpic = File::getMapsetPicture("default", "misc/ruin.png");
    d_signpostpic = File::getMapsetPicture("default", "misc/signpost.png");
    d_selector[0] = File::getMiscPicture("select0000.png");
    d_selector[1] = File::getMiscPicture("select0001.png");
    d_itempic = File::getMiscPicture("items.png");
    // not used _yet_
    d_fogpic = File::getMiscPicture("fog.png");

    // note: we are not fully initialized before set_view is called
}

BigMap::~BigMap()
{
    SDL_FreeSurface(d_arrows);
    SDL_FreeSurface(d_ruinpic);
    SDL_FreeSurface(d_signpostpic);
    SDL_FreeSurface(d_selector[0]);
    SDL_FreeSurface(d_selector[1]);
    SDL_FreeSurface(d_itempic);
    SDL_FreeSurface(d_fogpic);

    if (buffer)
        SDL_FreeSurface(buffer);

    delete d_renderer;
}

void BigMap::set_view(Rectangle new_view)
{
    int tilesize = GameMap::getInstance()->getTileSet()->getTileSize();
    
    if (view.dim == new_view.dim && buffer)
    {
	// someone wants us to move the view, not resize it, no need to
	// construct new surfaces and all that stuff

	if (view.pos == new_view.pos)
	    return;		// short-circuit
	    
	view = new_view;
	view_pos = view.pos * tilesize;
	draw();
	return;
    }

    view = new_view;
    
    if (buffer)
        SDL_FreeSurface(buffer);
    if (d_renderer)
        delete d_renderer;

    // now create an extended screen surface which is two maptiles wider and
    // higher than the screen you actually see. That is how smooth scrolling
    // becomes comparatively easy. You just blit from the extended screen to
    // the screen with some offset.
    buffer_view.w = view.w + 2;
    buffer_view.h = view.h + 2;

    SDL_PixelFormat* fmt = SDL_GetVideoSurface()->format;
    buffer = SDL_CreateRGBSurface(SDL_SWSURFACE, // FIXME: correct?
                buffer_view.w * tilesize, buffer_view.h * tilesize,
                fmt->BitsPerPixel,
                fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

    // now set the MapRenderer so that it draws directly on the new surface
    d_renderer = new MapRenderer(buffer);

    // setup timeout
    if (!selection_timeout_handler)
	selection_timeout_handler = 
	    Timing::instance().register_timer(
		sigc::mem_fun(*this, &BigMap::on_selection_timeout),
		selection_timeout);
}

void BigMap::draw(bool redraw_buffer)
{
    // no size and buffer yet, return
    if (buffer == 0)
        return;

    SDL_Surface* surface = SDL_GetVideoSurface();

    SDL_Rect dest;
    dest.x = dest.y = 0;
    dest.w = surface->w;
    dest.h = surface->h;
    int tilesize = GameMap::getInstance()->getTileSet()->getTileSize();

    // align the buffer view
    Vector<int> new_buffer_view = clip(
	Vector<int>(0, 0),
	view.pos - Vector<int>(1, 1),
	GameMap::get_dim() - buffer_view.dim + Vector<int>(1, 1));
    buffer_view.x = new_buffer_view.x;
    buffer_view.y = new_buffer_view.y;

    // redraw the buffer
    if (redraw_buffer)
	draw_buffer();

    // blit the visible part of buffer to the screen
    Vector<int> p = view_pos - buffer_view.pos * tilesize;
    assert(p.x >= 0 && p.x + surface->w <= buffer_view.w * tilesize &&
	   p.y >= 0 && p.y + surface->h <= buffer_view.h * tilesize);
    SDL_Rect r;
    r.x = p.x;
    r.y = p.y;
    r.w = surface->w;
    r.h = surface->h;
    SDL_BlitSurface(buffer, &r, surface, &dest);
    SDL_UpdateRects(surface, 1, &dest);
}

void BigMap::centerView(const Vector<int> p)
{
    // FIXME: check whether this is a new position
    
    // set the viewrect to the actual position
    view.x = p.x - view.w / 2;
    view.y = p.y - view.h / 2;

    // clip it to fit
    if (view.x < 0)
	view.x = 0;
    if (view.y < 0)
	view.y = 0;
    if (view.x > (GameMap::getWidth() - view.w))
        view.x = GameMap::getWidth() - view.w;
    if (view.y > (GameMap::getHeight() - view.h))
        view.y = GameMap::getHeight() - view.h;

    view_pos
	= view.pos * GameMap::getInstance()->getTileSet()->getTileSize();

    view_changed.emit(view);

    draw();
    old_view = view;
}

void BigMap::stackMoved(Stack* s)
{
    debug("stackMoved()");

    // s = 0 means center on the active stack
    if (!s)
        s = Playerlist::getActiveplayer()->getActivestack();

    if (!s)
        draw();
    else
        // center the view around the new stack position
        centerView(s->getPos());
}

void BigMap::select_active_stack()
{
    Stack* stack = Playerlist::getActiveplayer()->getActivestack();
    if (!stack)
        return;

    if (!stack->getPath()->checkPath(stack))
    {
        //error handling is required here, up to now we only barf on cerr
        cerr << _("original path of stack was blocked\n");
    }

    centerView(stack->getPos());

    stack_selected.emit(stack);
}

void BigMap::unselect_active_stack()
{
    draw();
    stack_selected.emit(0);
}

const Vector<int> BigMap::getRelativeXPos(const Vector<int>& absolutePos)
{
    Vector<int> relativePos;
    int ts = GameMap::getInstance()->getTileSet()->getTileSize();

    relativePos.x = (absolutePos.x - buffer_view.x) * ts;
    relativePos.y = (absolutePos.y - buffer_view.y) * ts;

    return relativePos;
}

bool BigMap::on_selection_timeout()
{
    if (!Playerlist::getActiveplayer()->getActivestack())
        return Timing::CONTINUE;

    draw();

    return Timing::CONTINUE;
}

void BigMap::mouse_button_event(MouseButtonEvent e)
{
    //ignore event, right now we're waiting for the AI or network guy
    if (!d_enable)
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
                if (Playerlist::getActiveplayer()->getActivestack() == 0)
                {
                    Stacklist *sl = Playerlist::getActiveplayer()->getStacklist();
                    sl->setActivestack(sl->getObjectAt(tile.x, tile.y));
                    select_active_stack();
                }
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
                    city_selected(c, map_tip_position(c->get_area()));
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
	        if (c->getPlayer() != Playerlist::getActiveplayer() || c->isBurnt())
                {
		    city_selected(c, map_tip_position(c->get_area()));
		    mouse_state = SHOWING_CITY;
                }
	    }
	    else if (Ruin* r = Ruinlist::getInstance()->getObjectAt(tile.x, tile.y))
	    {
		ruin_selected(r, map_tip_position(r->get_area()));
		mouse_state = SHOWING_RUIN;
	    }
	    else if (Signpost* s = Signpostlist::getInstance()->getObjectAt(tile.x, tile.y))
	    {
		signpost_selected(s, map_tip_position(s->get_area()));
		mouse_state = SHOWING_SIGNPOST;
	    }
	    else if (Temple* t = Templelist::getInstance()->getObjectAt(tile.x, tile.y))
	    {
		temple_selected.emit(t, map_tip_position(t->get_area()));
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
		city_selected.emit(0, MapTipPosition());
		break;

	    case SHOWING_RUIN:
		ruin_selected.emit(0, MapTipPosition());
		break;

	    case SHOWING_TEMPLE:
		temple_selected.emit(0, MapTipPosition());
		break;

	    case SHOWING_SIGNPOST:
		signpost_selected.emit(0, MapTipPosition());
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

void BigMap::mouse_motion_event(MouseMotionEvent e)
{
    static Vector<int> prev_mouse_pos = Vector<int>(0, 0);
    
    Vector<int> new_tile = mouse_pos_to_tile(e.pos);

    if (new_tile != current_tile)
    {
	current_tile = new_tile;
	mouse_on_tile.emit(current_tile);
    }

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
	view_pos = clip(Vector<int>(0, 0),
			view_pos + delta,
			(GameMap::get_dim() - view.dim) * ts);

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

Vector<int> BigMap::mouse_pos_to_tile(Vector<int> pos)
{
    int ts = GameMap::getInstance()->getTileSet()->getTileSize();

    return (view_pos + pos) / ts;
}

MapTipPosition BigMap::map_tip_position(Rectangle tile_area)
{
    // convert area to pixels on the SDL screen
    int tilesize = GameMap::getInstance()->getTileSet()->getTileSize();

    Rectangle area(tile_area.pos * tilesize - view_pos,
		   tile_area.dim * tilesize);

    // calculate screen edge distances
    SDL_Surface *screen = SDL_GetVideoSurface();
    int left, right, top, bottom;
    
    left = area.x;
    right = screen->w - (area.x + area.w);
    top = area.y;
    bottom = screen->h - (area.y + area.h);

    int const MARGIN = 2;
    
    // then set the position
    MapTipPosition m;
    if (right >= left && right >= top && right >= bottom)
    {
	m.pos.x = area.x + area.w + MARGIN;
	m.pos.y = area.y;
	m.justification = MapTipPosition::LEFT;
    }
    else if (left >= top && left >= bottom)
    {
	m.pos.x = area.x - MARGIN;
	m.pos.y = area.y;
	m.justification = MapTipPosition::RIGHT;
    }
    else if (bottom >= top)
    {
	m.pos.x = area.x;
	m.pos.y = area.y + area.h + MARGIN;
	m.justification = MapTipPosition::TOP;
    }
    else
    {
	m.pos.x = area.x;
	m.pos.y = area.y - MARGIN;
	m.justification = MapTipPosition::BOTTOM;
    }
    
    return m;
}

void BigMap::blit_if_inside_buffer(const Object &obj, SDL_Surface *image)
{
    if (is_overlapping(buffer_view, obj.get_area()))
    {
	Vector<int> p = getRelativeXPos(obj.getPos());
	    
	SDL_Rect rect;
	rect.x = p.x;
	rect.y = p.y;
	rect.w = obj.getSize();
	rect.h = obj.getSize();
	SDL_BlitSurface(image, 0, buffer, &rect);
    }
}

void BigMap::draw_buffer()
{
    int tilesize = GameMap::getInstance()->getTileSet()->getTileSize();
    d_renderer->render(0, 0, buffer_view.x, buffer_view.y,
		       buffer_view.w, buffer_view.h);

    for (Ruinlist::iterator i = Ruinlist::getInstance()->begin();
	 i != Ruinlist::getInstance()->end(); ++i)
	blit_if_inside_buffer(*i, d_ruinpic);

    for (Signpostlist::iterator i = Signpostlist::getInstance()->begin();
	 i != Signpostlist::getInstance()->end(); ++i)
	blit_if_inside_buffer(*i, d_signpostpic);

    for (Templelist::iterator i = Templelist::getInstance()->begin();
	 i != Templelist::getInstance()->end(); ++i)
	blit_if_inside_buffer(
	    *i, GraphicsCache::getInstance()->getTemplePic(i->getType()));

    for (Roadlist::iterator i = Roadlist::getInstance()->begin();
	 i != Roadlist::getInstance()->end(); ++i)
	blit_if_inside_buffer(
	    *i, GraphicsCache::getInstance()->getRoadPic(i->getType()));

    for (Stonelist::iterator i = Stonelist::getInstance()->begin();
	 i != Stonelist::getInstance()->end(); ++i)
	blit_if_inside_buffer(
	    *i, GraphicsCache::getInstance()->getStonePic(i->getType()));

    for (Citylist::iterator i = Citylist::getInstance()->begin();
	 i != Citylist::getInstance()->end(); ++i)
	blit_if_inside_buffer(*i, GraphicsCache::getInstance()->getCityPic(&*i));

    // If there are any items lying around, blit the itempic as well
    for (int x = buffer_view.x; x < buffer_view.x + buffer_view.w; x++)
        for (int y = buffer_view.y; y < buffer_view.y + buffer_view.h; y++)
	    if (x < GameMap::getWidth() && y < GameMap::getHeight()
		&& !GameMap::getInstance()->getTile(x,y)->getItems().empty())
	    {
		Vector<int> p;
		p.x =x;
		p.y = y;
		p = getRelativeXPos(p);
		SDL_Rect r;
		r.x = p.x+(tilesize-15);
		r.y = p.y+(tilesize-15);
		r.w = r.h = 10;
		SDL_BlitSurface(d_itempic, 0, buffer,&r);
	    }

    // Draw stacks
    for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
	 pit != Playerlist::getInstance()->end(); pit++)
    {
        Stacklist* mylist = (*pit)->getStacklist();
        for (Stacklist::iterator it= mylist->begin(); it != mylist->end(); it++)
        {
            Vector<int> p = (*it)->getPos();

            // check if the object lies in the viewed part of the map
            // otherwise we shouldn't draw it
            if (is_inside(buffer_view, p) && !(*it)->getDeleting())
            {
                p = getRelativeXPos(p);

                // draw stack
		SDL_Rect r;
		r.x = p.x + 6;
		r.y = p.y + 6;
		r.w = r.h = 54;
                SDL_BlitSurface((*it)->getStrongestArmy()->getPixmap(), 0,
				buffer, &r);

                // draw flag
		r.x = p.x;
		r.y = p.y;
		r.w = r.h = tilesize;
                SDL_BlitSurface(GraphicsCache::getInstance()->getFlagPic(*it),
				0, buffer, &r);
            }
        }
    }

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
            pos = getRelativeXPos(**it);
            nextpos = getRelativeXPos(**(++it));
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

        pos = getRelativeXPos(*stack->getPath()->back());
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
	    // FIXME: a better idea is to use an animation class
	    static int frame = 0;
        
	    if (frame == 0)
		frame = 1;
	    else
		frame = 0;
	    
	    p = getRelativeXPos(p);
	    SDL_Rect r;
	    r.x = p.x;
	    r.y = p.y;
	    r.w = r.h = tilesize;
	    SDL_BlitSurface(d_selector[frame], 0, buffer, &r);
	}
    }
}

#if 0
void BigMap::eventMouseLeave()
{
    //set the pos value to a negative value, so the status is not drawn
    d_pos.x = -51;
    smovingMouse.emit(Vector<int>(-1,-1));
}
#endif
