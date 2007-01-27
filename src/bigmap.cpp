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

#include "bigmap.h"
#include <SDL_image.h>
#include <assert.h>
#include <pgdraw.h>
#include <pgapplication.h>
#include <stdlib.h>

#include "cityinfo.h"
#include "ruininfo.h"
#include "SignpostInfo.h"
#include "TempleInfo.h"
#include "army.h"
#include "path.h"
#include "stacklist.h"
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
#include "maptile.h"
#include "w_edit.h"
#include "Configuration.h"
#include "GraphicsCache.h"
#include "config.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

BigMap::BigMap(PG_Widget* parent, PG_Rect rect)
    :PG_Widget(parent, rect, true), d_renderer(0), d_viewrect(0), d_xscreen(0),
    d_scrollstat(0), d_selupdate(false), d_timerID(0)
{
    d_enable = true;
    d_oldrect.x = d_oldrect.y = d_oldrect.w = d_oldrect.h = 0;

    // load all pictures
    d_arrows = File::getMiscPicture("arrows.png");
    d_ruinpic = File::getMapsetPicture("default", "misc/ruin.png");
    d_signpostpic = File::getMapsetPicture("default", "misc/signpost.png");
    d_selector[0] = File::getMiscPicture("select0000.png");
    d_selector[1] = File::getMiscPicture("select0001.png");
    d_itempic = File::getMiscPicture("items.png");
    // not used _yet_
    d_fogpic = File::getMiscPicture("fog.png");
}

BigMap::~BigMap()
{
    //Sleep after interrupting the timer. This is to fix a race condition where
    //the map would be redrawn due to a timer call while the object is being
    //deleted
    interruptTimer();
    SDL_Delay(1);

    SDL_FreeSurface(d_arrows);
    SDL_FreeSurface(d_ruinpic);
    SDL_FreeSurface(d_signpostpic);
    SDL_FreeSurface(d_selector[0]);
    SDL_FreeSurface(d_selector[1]);
    SDL_FreeSurface(d_itempic);
    SDL_FreeSurface(d_fogpic);

    if (d_xscreen)
        SDL_FreeSurface(d_xscreen);

    delete d_renderer;
}

void BigMap::setViewrect(PG_Rect* viewrect)
{
    debug("BigMap::setViewrect")

    interruptTimer();

    d_viewrect = viewrect;

    if (d_xscreen)
        SDL_FreeSurface(d_xscreen);
    if (d_renderer)
        delete d_renderer;

    //now create an extended screen surface which is two maptiles wider and
    //higher than the screen you actually see. That is how smooth scrolling
    //becomes comparatively easy. You just blit from the extended screen to
    //the screen with some offset.
    d_xrect.w = d_viewrect->w + 2;
    d_xrect.h = d_viewrect->h + 2;
    alignXRect();

    SDL_PixelFormat* fmt = PG_Application::GetScreen()->format;
    d_xscreen = SDL_CreateRGBSurface(SDL_HWSURFACE,
                d_xrect.w * GameMap::getInstance()->getTileSet()->getTileSize(),
                d_xrect.h * GameMap::getInstance()->getTileSet()->getTileSize(),
                fmt->BitsPerPixel,
                fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

    //now set the MapRenderer so that it draws directly on the new surface
    d_renderer = new MapRenderer(d_xscreen);

    restartTimer();
}

void BigMap::eventDraw(SDL_Surface* surface, const PG_Rect& rect)
{
    debug("eventDraw()");

    //first, if we haven't been set a viewrect yet, refuse to draw anything
    if (d_xscreen == 0)
        return;

    SDL_Rect dest;
    dest.x = dest.y = 0;
    dest.w = surface->w;
    dest.h = surface->h;
    int tilesize = GameMap::getInstance()->getTileSet()->getTileSize();

    //SDL_mutexP(d_lock);

    //If d_selupdate is true, we know that there is just a request to reblit
    //the xscreen to the real surface.
    if (d_selupdate)
    {
        //First, set d_selupdate to false, so that this feature is activated
        //only once in a period.
        d_selupdate = false;

        PG_Point p;
        p.x = d_viewrect->x;
        p.y = d_viewrect->y;
        p = getRelativeXPos(p);

        PG_Rect r(p.x, p.y, dest.w, dest.h);
        SDL_BlitSurface(d_xscreen, &r, surface, &dest);
        //SDL_BlitSurface(d_xscreen, PG_Rect(p.x, p.y, dest.w, dest.h).SDLRect(),surface, &dest);

        //DrawBorder(rect, 1);

        return;
    }

    //If smooth scrolling is currently up, simply blit from the extended screen
    //to BigMap's surface.
    if (d_scrollstat != 0)
    {
        SDL_Rect source;

        PG_Point p;
        p.x = d_oldrect.x;
        p.y = d_oldrect.y;
        p = getRelativeXPos(p);
        source.x = p.x;
        source.y = p.y;

        //determine which part of xscreen we want to blit at all
        if (d_oldrect.x < d_viewrect->x)
            source.x += d_scrollstat;
        if (d_oldrect.x > d_viewrect->x)
            source.x -= d_scrollstat;

        if (d_oldrect.y < d_viewrect->y)
            source.y += d_scrollstat;
        if (d_oldrect.y > d_viewrect->y)
            source.y -= d_scrollstat;

        source.w = surface->w;
        source.h = surface->h;

        SDL_BlitSurface(d_xscreen, &source, surface, &dest);
        //DrawBorder(rect, 1);

        return;
    }

    //Align the extended rect and redraw xscreen
    alignXRect();

    d_renderer->render(0, 0, d_xrect.x, d_xrect.y, d_xrect.w, d_xrect.h);

    for (Ruinlist::iterator it = Ruinlist::getInstance()->begin();
        it != Ruinlist::getInstance()->end(); ++it)
    {
        PG_Point p = (*it).getPos();
        if (d_xrect.IsInside(p))
        {
            p = getRelativeXPos(p);
            PG_Rect r(p.x, p.y, tilesize, tilesize);
            SDL_BlitSurface(d_ruinpic, 0, d_xscreen, &r);
        }
    }

    for (Signpostlist::iterator it = Signpostlist::getInstance()->begin();
        it != Signpostlist::getInstance()->end(); ++it)
    {
        PG_Point p = (*it).getPos();
        if (d_xrect.IsInside(p))
        {
            p = getRelativeXPos(p);
            PG_Rect r(p.x, p.y, tilesize, tilesize);
            SDL_BlitSurface(d_signpostpic, 0, d_xscreen, &r);
        }
    }

    for (Templelist::iterator it = Templelist::getInstance()->begin();
        it != Templelist::getInstance()->end(); it++)
    {
        PG_Point p = (*it).getPos();
        if (d_xrect.IsInside(p))
        {
            p = getRelativeXPos(p);

            SDL_Surface* templepic = GraphicsCache::getInstance()->getTemplePic((*it).getType());
            PG_Rect r(p.x, p.y, tilesize, tilesize);
            SDL_BlitSurface(templepic, 0, d_xscreen, &r);
        }
    }

    for (Roadlist::iterator it = Roadlist::getInstance()->begin();
        it != Roadlist::getInstance()->end(); it++)
    {
        PG_Point p = (*it).getPos();
        if (d_xrect.IsInside(p))
        {
            p = getRelativeXPos(p);

            SDL_Surface* stonepic = GraphicsCache::getInstance()->getRoadPic((*it).getType());
            PG_Rect r(p.x, p.y, tilesize, tilesize);
            SDL_BlitSurface(stonepic, 0, d_xscreen, &r);
        }
    }

    for (Stonelist::iterator it = Stonelist::getInstance()->begin();
        it != Stonelist::getInstance()->end(); it++)
    {
        PG_Point p = (*it).getPos();
        if (d_xrect.IsInside(p))
        {
            p = getRelativeXPos(p);

            SDL_Surface* stonepic = GraphicsCache::getInstance()->getStonePic((*it).getType());
            PG_Rect r(p.x, p.y, tilesize, tilesize);
            SDL_BlitSurface(stonepic, 0, d_xscreen, &r);
        }
    }

    // Draw all cities
    for (Citylist::iterator it = Citylist::getInstance()->begin();
        it != Citylist::getInstance()->end(); ++it)
    {
        PG_Point p = (*it).getPos();

        if (d_xrect.IsInside(p))
        {
            p = getRelativeXPos(p);

            SDL_Surface* citypic = GraphicsCache::getInstance()->getCityPic(&(*it));
            PG_Rect r(p.x, p.y, 2*tilesize, 2*tilesize);
            SDL_BlitSurface(citypic, 0, d_xscreen, &r);
        }
    }

    // If there are any items lying around, blit the itempic as well
    for (int x = d_xrect.x; x < d_xrect.x + d_xrect.w; x++)
        for (int y = d_xrect.y; y < d_xrect.y + d_xrect.h; y++)
        if (!GameMap::getInstance()->getTile(x,y)->getItems().empty())
        {
            PG_Point p;
            p.x =x;
            p.y = y;
            p = getRelativeXPos(p);
            PG_Rect r(p.x+(tilesize-15), p.y+(tilesize-15), 10, 10);
            SDL_BlitSurface(d_itempic, 0, d_xscreen,&r);
        }

    // Draw stacks
    for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
        pit != Playerlist::getInstance()->end(); pit++)
    {
        Stacklist* mylist = (*pit)->getStacklist();
        for (Stacklist::iterator it= mylist->begin(); it != mylist->end(); it++)
        {
            PG_Point p = (*it)->getPos();

            // check if the object lies in the viewed part of the map
            // otherwise we shouldn't draw it
            if (d_xrect.IsInside(p) && !(*it)->getDeleting())
            {
                p = getRelativeXPos(p);

                // draw stack
                PG_Rect r(p.x + 6, p.y + 6, 54, 54);
                SDL_BlitSurface((*it)->getStrongestArmy()->getPixmap(), 0,d_xscreen, &r);

                // draw flag
                r.SetRect(p.x, p.y, tilesize, tilesize);
                SDL_BlitSurface(GraphicsCache::getInstance()->getFlagPic(*it), 0, d_xscreen, &r);
            }
        }
    }

    Stack* stack = Playerlist::getActiveplayer()->getActivestack();

    // Draw Path
    if (stack && stack->getPath()->size())
    {
        PG_Point pos;
        PG_Point nextpos;
        SDL_Color c;
        c.r = c.g = c.b = 0;

        // draw all waypoints except the last
        for (list<PG_Point*>::iterator it = stack->getPath()->begin();
                it != --(stack->getPath()->end());)
        {
            // peak at the next waypoint to draw the correct arrow
            pos = getRelativeXPos(**it);
            nextpos = getRelativeXPos(**(++it));
            PG_Rect r1, r2(pos.x, pos.y, tilesize, tilesize);

            if (nextpos.x == pos.x && nextpos.y < pos.y)
                r1.SetRect(0, 0, tilesize, tilesize);
            else if (nextpos.x > pos.x && nextpos.y < pos.y)
                r1.SetRect(tilesize, 0, tilesize, tilesize);
            else if (nextpos.x > pos.x && nextpos.y == pos.y)
                r1.SetRect(2*tilesize, 0, tilesize, tilesize);
            else if (nextpos.x > pos.x && nextpos.y > pos.y)
                r1.SetRect(3*tilesize, 0, tilesize, tilesize);
            else if (nextpos.x == pos.x && nextpos.y > pos.y)
                r1.SetRect(4*tilesize, 0, tilesize, tilesize);
            else if (nextpos.x < pos.x && nextpos.y > pos.y)
                r1.SetRect(5*tilesize, 0, tilesize, tilesize);
            else if (nextpos.x < pos.x && nextpos.y == pos.y)
                r1.SetRect(6*tilesize, 0, tilesize, tilesize);
            else if (nextpos.x < pos.x && nextpos.y < pos.y)
                r1.SetRect(7*tilesize, 0, tilesize, tilesize);

            SDL_BlitSurface(d_arrows, &r1, d_xscreen, &r2);

        }

        pos = getRelativeXPos(*stack->getPath()->back());
        PG_Rect r1(8*tilesize, 0, tilesize, tilesize), r2(pos.x, pos.y, tilesize, tilesize);
        SDL_BlitSurface(d_arrows, &r1, d_xscreen, &r2);
    }

    if (stack && (d_scrollstat == 0))
        drawSelector(false);

    //now finally blit the part of xscreen which is visible to the surface
    PG_Point p;
    p.x = d_viewrect->x;
    p.y = d_viewrect->y;
    p = getRelativeXPos(p);

    PG_Rect r(p.x,p.y,surface->w,surface->h);
    SDL_BlitSurface(d_xscreen, &r,surface, &dest);

    //DrawBorder(rect, 1);
   //   SDL_mutexV(d_lock);
}

bool BigMap::eventMouseButtonDown(const SDL_MouseButtonEvent* event)
{
    debug("eventMouseButtonDown()")

    //pretend to handle event, but don't do it (right now we're
    //waiting for the AI or network guy)
    if (!d_enable)
    {
        return true;
    }

    int tilesize = GameMap::getInstance()->getTileSet()->getTileSize();
    int pos_X = (event->x - my_xpos) / tilesize + d_viewrect->my_xpos;
    int pos_Y = (event->y - my_ypos) / tilesize + d_viewrect->my_ypos;

    if (event->button == SDL_BUTTON_LEFT)
    {
        Stack* stack = Playerlist::getActiveplayer()->getActivestack();

        if (stack)
        {
            // clicked on an already active stack
            if (stack->getPos().x == pos_X && stack->getPos().y == pos_Y) return true;

            PG_Point p;
            p.x = pos_X; p.y = pos_Y;

            Playerlist::getActiveplayer()->stackSplit(stack); // split if ungrouped
            
            PG_Point* dest = 0;
            if (!stack->getPath()->empty())
                dest = *stack->getPath()->rbegin();
            
            if (dest && dest->x == pos_X && dest->y == pos_Y)
            {
                Playerlist::getActiveplayer()->stackMove(stack);
                if (Playerlist::getActiveplayer()->getActivestack() == 0)
                {
                    Stacklist *sl = Playerlist::getActiveplayer()->getStacklist();
                    sl->setActivestack(sl->getObjectAt(pos_X, pos_Y));
                    stackSelected();
                }
            }
            else
                stack->getPath()->calculate(stack, p);
            
            PG_Application::ClearOldMousePosition();
            Redraw();
            PG_Application::DrawCursor();
        }
        // Stack hasn't been active yet
        else
        {
            stack = Stacklist::getObjectAt(pos_X, pos_Y);
            if ((stack) && (stack->isFriend(Playerlist::getActiveplayer())))
            {
                Playerlist::getActiveplayer()->getStacklist()->setActivestack(stack);
                stackSelected();
            }

        }
        // this is to reset the w_edit buttons
        ((W_Edit*)GetParent())->initButtons();

        return true;
    }

    // right mousebutton to click at city
    if (event->button == SDL_BUTTON_RIGHT)
    {
        City* city = Citylist::getInstance()->getObjectAt(pos_X, pos_Y);

        if ((city) && (!city->isBurnt()) && (city->isFriend(Playerlist::getInstance()->getActiveplayer())))
        {
            CityInfo cityinfo(city);
            cityinfo.Show();
            cityinfo.RunModal();
            cityinfo.Hide();

            //some visible city properties (defense level) may have changed
            Redraw();

            return true;
        }
	else if ((city) && (city->isFriend(Playerlist::getInstance()->getActiveplayer()) == false))
        {
            CityInfoSmall cityinfo(city, event->x, event->y);
            cityinfo.Show();
            cityinfo.RunModal();
            cityinfo.Hide();
            return true;
        }

        Ruin* ruin = Ruinlist::getInstance()->getObjectAt(pos_X, pos_Y);
        if (ruin)
        {
            RuinInfo ruininfo(ruin, event->x, event->y);
            ruininfo.Show();
            ruininfo.RunModal();
            ruininfo.Hide();
            return true;
        }

        Signpost* signpost = Signpostlist::getInstance()->getObjectAt(pos_X, pos_Y);
        if (signpost)
        {
            SignpostInfo signpostinfo(signpost, event->x, event->y);
            signpostinfo.Show();
            signpostinfo.RunModal();
            signpostinfo.Hide();
            return true;
        }

        Temple* t = Templelist::getInstance()->getObjectAt(pos_X, pos_Y);
        if (t)
        {
            TempleInfo templeinfo(t, event->x, event->y);
            templeinfo.Show();
            templeinfo.RunModal();
            templeinfo.Hide();
            return true;
        }
        
        Stack* stack = Playerlist::getActiveplayer()->getActivestack();
        if (stack) {
            stackDeselected();
        }
        return true;
    }

    return false;
}

void BigMap::centerView(const PG_Point p)
{
    debug("centerView()");

    //set the viewrect to the actual position
    d_viewrect->my_xpos = p.x - (int)(d_viewrect->my_width / 2);
    d_viewrect->my_ypos = p.y - (int)(d_viewrect->my_height / 2);

    if (d_viewrect->my_xpos < 0) d_viewrect->my_xpos = 0;
    if (d_viewrect->my_ypos < 0) d_viewrect->my_ypos = 0;
    if (d_viewrect->my_xpos > (GameMap::getWidth() - d_viewrect->my_width))
        d_viewrect->my_xpos = GameMap::getWidth() - d_viewrect->my_width;
    if (d_viewrect->my_ypos > (GameMap::getHeight() - d_viewrect->my_height))
        d_viewrect->my_ypos = GameMap::getHeight() - d_viewrect->my_height;

   schangingViewrect.emit(true);

    //If we do not intend to scroll in a smooth fashion, we simply redraw
    //the screen. We scroll smoothly if
    //a) it is activated
    //b) the viewrect has moved for only one tile
    if (Configuration::s_smoothScrolling
        && (d_viewrect->x - d_oldrect.x <2) && (d_viewrect->x - d_oldrect.x >-2)
        && (d_viewrect->y - d_oldrect.y <2) && (d_viewrect->y -d_oldrect.y >-2)
        && ((d_viewrect->x != d_oldrect.x) || (d_viewrect->y != d_oldrect.y)))
    {
        //The logic behind drawing smoothly is: As long as we scroll, we do
        //nothing else. Either it is our turn, then we wait the short moment,
        //or it is the enemy's turn, then we look at it anyway. So we sleep
        //during the intervals. This solution is applied because else program
        //execution would go on which gives the developer a headache if a unit
        //moves e.g. many steps, then we either have to wait for the next step
        //(bad solution) or we need some extended communication to tell bigmap
        //what is happening.

        interruptTimer();

        for (d_scrollstat = 5; d_scrollstat < 50; d_scrollstat += 5)
        {
            PG_Application::ClearOldMousePosition();
            
            Redraw();
            SDL_Delay(TIMER_BIGMAP_SCROLLING);

            PG_Application::DrawCursor();
        }

        restartTimer();
        d_scrollstat = 0;
    }

    Redraw();
    d_oldrect = *d_viewrect;
}

void BigMap::stackMoved(Stack* s)
{
    debug("stackMoved()");

    // s = 0 means center on the active stack
    if (!s)
        s = Playerlist::getActiveplayer()->getActivestack();

    if (!s)
        Redraw();
    else
        // center the view around the new stack position
        centerView(s->getPos());
}

void BigMap::stackSelected()
{
    debug("stackSelected()");

    Stack* stack = Playerlist::getActiveplayer()->getActivestack();
    if (!stack->getPath()->checkPath(stack))
    {
        //error handling is required here, up to now we only barf on cerr
        cerr << _("original path of stack was blocked\n");
    }

    centerView(stack->getPos());

    sselectingStack.emit(stack);
}

void BigMap::stackDeselected()
{
    debug("stackDeselected()");
    sdeselectingStack.emit();
}

// calculate the relative pos from the absolute pos by subtracting
// the viewrect-pos and multiplying with the tilesize

const PG_Point BigMap::getRelativePos(const PG_Point& absolutePos)
{
    PG_Point relativePos;
    int ts = GameMap::getInstance()->getTileSet()->getTileSize();
    
    relativePos.x = (absolutePos.x - d_viewrect->my_xpos) * ts;
    relativePos.y = (absolutePos.y - d_viewrect->my_ypos) * ts;

    return relativePos;
}

//This function is almost the same as getRelativePos, but with xrect as rect
const PG_Point BigMap::getRelativeXPos(const PG_Point& absolutePos)
{
    PG_Point relativePos;
    int ts = GameMap::getInstance()->getTileSet()->getTileSize();

    relativePos.x = (absolutePos.x - d_xrect.my_xpos) * ts;
    relativePos.y = (absolutePos.y - d_xrect.my_ypos) * ts;

    return relativePos;
}


Uint32 BigMap::eventTimer(ID id, Uint32 interval)
{
    if (!Playerlist::getActiveplayer()->getActivestack())
        return interval;

    PG_Application::ClearOldMousePosition();
    
    drawSelector(true);

    d_selupdate = true;
    Redraw();

    PG_Application::DrawCursor();

    return interval;
}

// Draw Selector
void BigMap::drawSelector(bool changeFrame)
{
    Stack* stack = Playerlist::getActiveplayer()->getActivestack();
    if (!stack)
        return;

    PG_Point p = stack->getPos();
    if (d_viewrect->IsInside(p))
    {
        static int frame = 0;
        int ts = GameMap::getInstance()->getTileSet()->getTileSize();
        
        if (frame == 0) frame = 1;
        else frame = 0;
        p = getRelativeXPos(p);
        PG_Rect r(p.x, p.y, ts, ts);
        SDL_BlitSurface(d_selector[frame], 0, d_xscreen, &r);
    }
}

void BigMap::interruptTimer()
{
    if (d_timerID != 0)
        RemoveTimer(d_timerID);

    d_timerID = 0;
}

void BigMap::restartTimer()
{
    if (d_timerID != 0)
        return;     //we already have a timer

#ifndef FL_NO_TIMERS
    d_timerID = AddTimer(TIMER_BIGMAP_SELECTOR);  //slow timer for selector
#endif
}

void BigMap::alignXRect()
{
    //set xrect so that in includes viewrect as much as possible
    d_xrect.x = d_viewrect->x - 1;
    d_xrect.y = d_viewrect->y - 1;
    if (d_xrect.x < 0)
        d_xrect.x = 0;
    if (d_xrect.y < 0)
        d_xrect.y = 0;
    if (d_xrect.x + d_xrect.w > GameMap::getWidth())
        d_xrect.x = GameMap::getWidth() - d_xrect.w;
    if (d_xrect.y + d_xrect.h > GameMap::getHeight())
        d_xrect.y = GameMap::getHeight() - d_xrect.h;
}

bool BigMap::eventMouseMotion(const SDL_MouseMotionEvent* ev)
{
    // actualize the mouse position
    PG_Point oldpos = d_pos;
    int ts = GameMap::getInstance()->getTileSet()->getTileSize();
    
    d_pos.x = ev->x - my_xpos;
    d_pos.y = ev->y - my_ypos;

    int diff_xtiles = d_pos.x/ts - oldpos.x/ts;
    int diff_ytiles = d_pos.y/ts - oldpos.y/ts;

    if ((diff_xtiles == 0) && (diff_ytiles == 0))
        return true;

    smovingMouse.emit(PG_Point(d_pos.x/ts+d_rect.x, d_pos.y/ts+d_rect.y));
    
    return true;
}

void BigMap::eventMouseLeave()
{
    //set the pos value to a negative value, so the status is not drawn
    d_pos.x = -51;
    smovingMouse.emit(PG_Point(-1,-1));
}

// End of file
