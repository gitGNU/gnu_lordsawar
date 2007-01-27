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

#ifndef E_SMALLMAP_H
#define E_SMALLMAP_H

#include <pgwidget.h>

/** As in the game, the smallmap provides an overview of the map. However, it
  * looses some of the gimmicks of the original one. Clicking on it still
  * centers the view.
  */

class E_Smallmap : public PG_Widget
{
    public:
        /** Almost default paragui constructor
          * 
          * @param parent   the parent widget
          * @param rect     the rectangle for this widget
          * @param tilex    the number of horizontal tiles for the viewrect
          * @param tiley    the number of vertical tiles for the viewrect
          */
        E_Smallmap(PG_Widget* parent, PG_Rect rect, int tilex, int tiley);
        ~E_Smallmap();
        
        //! centers the view at the given position
        void centerView(const PG_Point pos);
        

        //! emitted whenever the view is centered on a new position
        SigC::Signal1<void, PG_Point> schangingView;
        
    private:
        /** centers the view
          * 
          * @param x    x coordinate (apsolute to the whole program!)
          * @param y    y coordinate (the same)
          */
        void setView(int x, int y);
        
        void eventDraw(SDL_Surface* surface, const PG_Rect& rect);
        void eventMouseLeave();
        bool eventMouseMotion(const SDL_MouseMotionEvent* ev);
        bool eventMouseButtonDown(const SDL_MouseButtonEvent* ev);
        bool eventMouseButtonUp(const SDL_MouseButtonEvent* ev);


        // Data
        PG_Rect d_rect;
        bool d_pressed;
};

#endif //E_SMALLMAP_H
