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

#include "bigmap.h"

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


#include <iostream>
using namespace std;
//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

BigMap::BigMap()
    : d_renderer(0), buffer(0)
{
    // load all pictures
    d_ruinpic = File::getMapsetPicture("default", "misc/ruin.png");
    d_signpostpic = File::getMapsetPicture("default", "misc/signpost.png");


    d_itempic = File::getMiscPicture("items.png");
    // not used _yet_
    d_fogpic = File::getMiscPicture("fog.png");

    // note: we are not fully initialized before set_view is called
}

BigMap::~BigMap()
{
    SDL_FreeSurface(d_ruinpic);
    SDL_FreeSurface(d_signpostpic);
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

	view = new_view;
	Vector<int> new_view_pos = get_view_pos_from_view();

	if (view_pos != new_view_pos)
	{
	    view_pos = new_view_pos;
	    draw();
	}
	
	return;
    }

    view = new_view;
    view_pos = get_view_pos_from_view();
    
    if (buffer)
        SDL_FreeSurface(buffer);
    if (d_renderer)
        delete d_renderer;

    // now create a buffer surface which is two maptiles wider and
    // higher than the screen you actually see. That is how smooth scrolling
    // becomes comparatively easy. You just blit from the extended screen to
    // the screen with some offset.
    buffer_view.dim = view.dim + Vector<int>(2, 2);

    SDL_PixelFormat* fmt = SDL_GetVideoSurface()->format;
    buffer = SDL_CreateRGBSurface(SDL_SWSURFACE,
                buffer_view.w * tilesize, buffer_view.h * tilesize,
                fmt->BitsPerPixel,
                fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

    // now set the MapRenderer so that it draws directly on the new surface
    d_renderer = new MapRenderer(buffer);
}

void BigMap::draw(bool redraw_buffer)
{
    // no size and buffer yet, return
    if (!buffer)
        return;

    SDL_Surface* screen = SDL_GetVideoSurface();
    int tilesize = GameMap::getInstance()->getTileSet()->getTileSize();

    // align the buffer view
    Vector<int> new_buffer_view = clip(
	Vector<int>(0, 0),
	view.pos - Vector<int>(1, 1),
	GameMap::get_dim() - buffer_view.dim + Vector<int>(1, 1));
    buffer_view.pos = new_buffer_view;

    // redraw the buffer
    if (redraw_buffer)
	draw_buffer();

    // blit the visible part of buffer to the screen
    Vector<int> p = view_pos - buffer_view.pos * tilesize;
    assert(p.x >= 0 && p.x + screen->w <= buffer_view.w * tilesize &&
	   p.y >= 0 && p.y + screen->h <= buffer_view.h * tilesize);
    SDL_Rect src, dest;
    src.x = p.x;
    src.y = p.y;
    dest.w = src.w = screen->w;
    dest.h = src.h = screen->h;
    dest.x = dest.y = 0;
    SDL_BlitSurface(buffer, &src, screen, &dest);
    SDL_UpdateRects(screen, 1, &dest);
}

void BigMap::center_view(const Vector<int> p)
{
    Rectangle new_view(
	clip(Vector<int>(0,0), p - view.dim / 2, GameMap::get_dim() - view.dim),
	view.dim);
    
    set_view(new_view);
    view_changed.emit(view);
}

void BigMap::screen_size_changed()
{
    SDL_Surface *v = SDL_GetVideoSurface();
    int ts = GameMap::getInstance()->getTileSet()->getTileSize();

    Rectangle new_view = view;
    
    new_view.w = v->w / ts;
    new_view.h = v->h / ts;

    new_view.pos = clip(Vector<int>(0,0), new_view.pos,
			GameMap::get_dim() - new_view.dim);

    if (new_view != view)
    {
	set_view(new_view);
	view_changed.emit(view);
    }
}

Vector<int> BigMap::get_view_pos_from_view()
{
    SDL_Surface *screen = SDL_GetVideoSurface();
    Vector<int> screen_dim(screen->w, screen->h);
    int ts = GameMap::getInstance()->getTileSet()->getTileSize();

    // clip to make sure we don't see a black border at the bottom and right
    return clip(Vector<int>(0, 0), view.pos * ts,
		GameMap::get_dim() * ts - screen_dim);
}

Vector<int> BigMap::tile_to_buffer_pos(Vector<int> tile)
{
    int ts = GameMap::getInstance()->getTileSet()->getTileSize();
    return (tile - buffer_view.pos) * ts;
}

Vector<int> BigMap::mouse_pos_to_tile(Vector<int> pos)
{
    int ts = GameMap::getInstance()->getTileSet()->getTileSize();

    return (view_pos + pos) / ts;
}

Vector<int> BigMap::mouse_pos_to_tile_offset(Vector<int> pos)
{
    int ts = GameMap::getInstance()->getTileSet()->getTileSize();

    return (view_pos + pos) % ts;
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
	Vector<int> p = tile_to_buffer_pos(obj.getPos());
	    
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
    GraphicsCache *gc = GraphicsCache::getInstance();
    int tilesize = GameMap::getInstance()->getTileSet()->getTileSize();
    d_renderer->render(0, 0, buffer_view.x, buffer_view.y,
		       buffer_view.w, buffer_view.h);

    for (Ruinlist::iterator i = Ruinlist::getInstance()->begin();
	 i != Ruinlist::getInstance()->end(); ++i)
      {
        if (((*i).isHidden() == true && 
               (*i).getOwner() == Playerlist::getActiveplayer()) ||
             (*i).isHidden() == false)
	  blit_if_inside_buffer(*i, d_ruinpic);
      }

    for (Signpostlist::iterator i = Signpostlist::getInstance()->begin();
	 i != Signpostlist::getInstance()->end(); ++i)
	blit_if_inside_buffer(*i, d_signpostpic);

    for (Templelist::iterator i = Templelist::getInstance()->begin();
	 i != Templelist::getInstance()->end(); ++i)
	blit_if_inside_buffer( *i, gc->getTemplePic(i->getType()));

    for (Roadlist::iterator i = Roadlist::getInstance()->begin();
	 i != Roadlist::getInstance()->end(); ++i)
	blit_if_inside_buffer( *i, gc->getRoadPic(i->getType()));

    for (Stonelist::iterator i = Stonelist::getInstance()->begin();
	 i != Stonelist::getInstance()->end(); ++i)
	blit_if_inside_buffer( *i, gc->getStonePic(i->getType()));

    for (Citylist::iterator i = Citylist::getInstance()->begin();
	 i != Citylist::getInstance()->end(); ++i)
	blit_if_inside_buffer(*i, gc->getCityPic(&*i));

    // If there are any items lying around, blit the itempic as well
    for (int x = buffer_view.x; x < buffer_view.x + buffer_view.w; x++)
        for (int y = buffer_view.y; y < buffer_view.y + buffer_view.h; y++)
	    if (x < GameMap::getWidth() && y < GameMap::getHeight()
		&& !GameMap::getInstance()->getTile(x,y)->getItems().empty())
	    {
		Vector<int> p = tile_to_buffer_pos(Vector<int>(x, y));
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
		if ((*it)->empty())
		{
		    std::cerr << "WARNING: empty stack found" << std::endl;
		    continue;
		}

                p = tile_to_buffer_pos(p);

                // draw stack
		SDL_Rect r;
		r.x = p.x + 6;
		r.y = p.y + 6;
		r.w = r.h = 54;
                if ((*it)->hasShip())
                  SDL_BlitSurface(gc->getShipPic(), 0, buffer, &r);
                else
                  SDL_BlitSurface((*it)->getStrongestArmy()->getPixmap(), 0,
				  buffer, &r);

                // draw flag
		r.x = p.x;
		r.y = p.y;
		r.w = r.h = tilesize;
                SDL_BlitSurface(gc->getFlagPic(*it), 0, buffer, &r);
            }
        }
    }

    after_draw();
}
