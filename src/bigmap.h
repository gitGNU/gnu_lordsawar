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

#ifndef BIGMAP_H
#define BIGMAP_H

#include <pgwidget.h>
#include <pgtimerobject.h>
#include "MapRenderer.h"
#include <sigc++/sigc++.h>
#include "stack.h"

/** The large game map
  * 
  * When you have started a game, you see two maps, one for the overview
  * (smallmap) and one detailed one. The latter is the bigmap. The class cares
  * for everything which takes place in the context of the large map, i.e. mouse
  * clicks, army movement etc.
  *
  * One note concerning smooth scrolling. When smooth scrolling is activated,
  * the bigmap doesn't jump from one position to another one, but blits some
  * steps in between ( you can try it out if you don't understand what I mean).
  * To keep this simple, Bigmap has two surfaces. One is inherited from paragui
  * and used to draw to, the other one is d_xscreen. This surface is exactly one
  * tile larger than the inherited surface and all rendering operations take
  * place with this surface. When using smooth scrolling, the d_xscreen is then
  * simply blitted to the widget surface with some offsets.
  */

class BigMap : public PG_Widget, public PG_TimerObject
               
{
    public:
        /** Constructor, form inherited from paragui
          * 
          * @param parent       the parent widget
          * @param rect         the rectangle for this widget
          */
        BigMap(PG_Widget* parent, PG_Rect rect);
        ~BigMap();

        /** The timer callback
          * 
          * This function is called whenever the bigmap timer is raised. It does
          * nothing more than advance the frame around the active stack and
          * redraw the surface.
          */
        Uint32 eventTimer(ID id, Uint32 interval);

        /** Assign a viewrect to bigmap
          * 
          * The viewrect determines which portion of the map is viewed and how
          * large it is.
          * @note The viewrect is shared between bigmap and smallmap (so if one
          * of the two changes the viewrect, the other one sees the change
          * immediately), so be careful with deleting it. Perhaps this should be
          * changed and made cleaner...
          *
          * @param viewrect         the portion of the map displayed in bigmap
          */
        void setViewrect(PG_Rect* viewrect);

        /** Callback if a stack is selected.
          * 
          * This sets an internal pointer to the currently active stack, centers
          * the bigmap on the stack and starts to draw a frame around it. With a
          * selected active stack the behaviour for mouse clicks changes from
          * selecting a stack to selecting a target for a move.
          */
        void stackSelected();

        //! Callback if a stack has been deselected
        void stackDeselected();

        /** Callback if a stack has moved
          * 
          * This centers the bigmap around the stack.
          */
        void stackMoved(Stack* s);

        //! Center the bigmap around point p
        void centerView(const PG_Point p);

        //! Enable/Disable the processing of mouse clicks
        void setEnable (bool enable){d_enable=enable;}

        //! Draw the frame around the currently active unit.
        void drawSelector(bool changeFrame);

        /** Function to kill the internal timer
          * 
          * On some occasions, timers which lead to redraws are ugly, e.g. if a
          * dialog is displayed, it flickers. So each class which uses timers
          * also provides functions to kill and restart the timer during
          * critical parts of the code.
          */
        void interruptTimer();

        //! Function to restart the internal timer
        void restartTimer();
        
        //! Signal which is emitted when the viewrect changes and redraws smallmap
        SigC::Signal1<bool, bool> schangingViewrect;

        //! Signal which is emitted when a stack is selected
        SigC::Signal1<void, Stack*> sselectingStack;

        //! Signal which is emitted when a stack is selected
        SigC::Signal0<void> sdeselectingStack;

        //! emitted whenever the user moves the mouse to a new tile
        SigC::Signal1<void, PG_Point> smovingMouse;

    private:
        //! Redraws the portion defined by rect on the internal surface
        void eventDraw(SDL_Surface* surface, const PG_Rect& rect);

        //! Event function for mouse clicks
        bool eventMouseButtonDown(const SDL_MouseButtonEvent* event);

        bool eventMouseMotion(const SDL_MouseMotionEvent* ev);
        void eventMouseLeave();

        //! Converts tile to surface coordinates of the internal surface
        const PG_Point getRelativePos(const PG_Point& absolutePos); 

        //! Converts tile to surface coordinates of d_xscreen
        const PG_Point getRelativeXPos(const PG_Point& absolutePos);

        /** Aligns the d_xscreen so that it encloses the internal surface in an
          * optimal fashion
          */
        void alignXRect();
        
        // DATA
        MapRenderer* d_renderer;
        PG_Rect* d_viewrect;
        
        PG_Rect d_oldrect;
        PG_Rect d_xrect;
        SDL_Surface* d_xscreen;
        int d_scrollstat;
        bool d_selupdate;
        
        SDL_Surface* d_arrows;
        SDL_Surface* d_ruinpic;
        SDL_Surface* d_signpostpic;
        SDL_Surface* d_selector[2];
        SDL_Surface* d_itempic;
        SDL_Surface* d_fogpic;
        
        Uint32 d_timerID;
        //SDL_mutex* d_lock;
        bool d_enable;

        PG_Rect  d_rect;
        PG_Point d_pos;
};

#endif // BIGMAP_H

// End of file
