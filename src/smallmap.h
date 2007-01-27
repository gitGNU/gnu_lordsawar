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

#ifndef SMALLMAP_H
#define SMALLMAP_H

#include <pgtimerobject.h>
#include <sigc++/sigc++.h>
#include <SDL_mutex.h>
#include "overviewmap.h"

/** Display of the whole game map.
  * 
  * SmallMap is the widget at the top right of the game screen which provides an
  * overview of the whole game map. It handles mouse clicks, moving of the
  * currently visible portion and changes of the underlying map.
  *
  * @note: This class shares the viewrect (=which part of the map is currently
  * visible in the main screen) with BigMap. Furthermore, it draws a rectangle
  * with changing colors at the position of the viewrect, which is why it has to
  * keep track of it.
  */

class SmallMap : public OverviewMap, public PG_TimerObject
{
    public:
        /** Constructor
          * 
          * @param parent       the parent widget
          * @param rect         the rectangle for the smallmap
          * @param viewsize     a rectangle whose width and height give
          *                     the size of the viewrect 
          *
          * The pixel size per terrain tile is automatically calculated.
          */
        SmallMap(PG_Widget* parent, PG_Rect rect, PG_Rect viewsize);
        ~SmallMap();

        
        /** called if resolution changes; resets the viewrect
          * @note: No timers may be running simultanously!!!
          */
        void changeResolution(PG_Rect viewsize);
        
        //! Interrupt the internal timer (for redrawing the viewrect)
        void interruptTimer();

        //! Restart the internal timer
        void restartTimer();

        //! Returns the viewrect
        PG_Rect* getViewrect(){return d_viewrect;}

        //! Timer callback for redrawing the viewrect with a different color
        Uint32 eventTimer(ID id, Uint32 interval);

        //! Draws the viewrect over the (internal) display of the map
        void drawViewrect();
        
        //! Paragui callback if the viewrect changes
        bool changedViewrect(PG_Widget *widget);

        // 2 helping things for the viewrect
        void inputFunction(int arrowx, int arrowy);

        // will be connected to BigMap::Redraw()
        SigC::Signal1<bool, bool> schangingViewrect;
        
    private:
        void setViewrect(int x, int y);

        // EVENT HANDLERS
        void eventDraw(SDL_Surface* surface, const PG_Rect& rect);
        bool eventMouseButtonDown(const SDL_MouseButtonEvent* event);
        bool eventMouseMotion(const SDL_MouseMotionEvent* event);
        
        // DATA
        PG_Rect* d_viewrect;
        
        bool d_pressed;
        int d_r;
        int d_g;
        int d_b;
        bool d_add;
        
        // keycheck , Value changes to 1 : SDL_KeyboardEvent , Value changes to 0 : SDL_MouseButtonEvent
        int keycheck;
        // helping variables for the KeyboardEvent from Thomas Plonka
        int arrowx ,arrowy; 

        Uint32 d_timerID;
        SDL_mutex* d_lock;
};

#endif // SMALLMAP_H
