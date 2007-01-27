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

#ifndef E_BIGMAP_H
#define E_BIGMAP_H

#include <pgwidget.h>
#include "../MapRenderer.h"

/** Bigmap is more or less the same as in the game, i.e. a large area with a
  * detailed view on the map. However, this one misses many functions of the
  * original bigmap and adds some other functionality.
  *
  * One of this is the status. E.g. if the player wants to change 3x3 areas of
  * terrain, the status is set to AREA3x3 and an additional integer for the
  * terrain type is stored. Now whenever the player moves the mouse, an
  * additional grid is drawn where he would change the terrain. This grid moves
  * when he moves the mouse. If he clicks, the terrain at the position is
  * changed, which wouldn't happen e.g. with status NONE.
  */

class E_Bigmap : public PG_Widget
{
    public:
        enum STATUS {NONE, AREA1x1, AREA3x3, ERASE, STACK, CITY, RUIN, TEMPLE,
	             SIGNPOST, STONE, ROAD};
        
        //! Default paragui constructor
        E_Bigmap(PG_Widget* parent, PG_Rect rect);
        ~E_Bigmap();
        
        //! centers the view at position pos
        void centerView(const PG_Point pos);
        
        /** Changes the default behaviour of the bigmap
          * 
          * With this function you can tell bigmap how it should deal with
          * mouse clicks and redrawing. The status is one of Bigmap::status,
          * the actual use of the integer depends on the status. For
          * AREA1x1 and AREA3x3, the integer gives the selected terrain type.
          * 
          * @param status   tells bigmap what it should do with the cursor
          * @param data     additional information; actual meaning depends on status
          */
        void changeStatus(STATUS status, Uint32 data);

        //! emitted whenever the view changes
        SigC::Signal1<void, PG_Point> schangingView;
        //! emitted whenever the map is changed
        SigC::Signal1<bool, bool> schangingMap;
        //! emitted whenever the user moves the mouse to a new tile
        SigC::Signal1<void, PG_Point> smovingMouse;
        
    private:
	//! which stone type are we going to show?
	int getStoneType();

	//! which road type are we going to show?
	int getRoadType();

        //! draws upon the surface e.g. a small grid depending on the status
        void drawStatus();

        //! change e.g. terrain under the mouse cursor
        void changeMap();

        void eventDraw(SDL_Surface* surface, const PG_Rect& rect);

        bool eventMouseButtonDown(const SDL_MouseButtonEvent* ev);
        bool eventMouseButtonUp(const SDL_MouseButtonEvent* ev);
        bool eventMouseMotion(const SDL_MouseMotionEvent* ev);
        void eventMouseLeave();

        // Data
        PG_Rect d_rect;

        MapRenderer* d_renderer;

        // several stuff for the drawing etc.
        STATUS      d_status;
        Uint32      d_data;
        PG_Point    d_pos;
        bool        d_pressed;
        int         d_tilesize;

        // images needed for drawing
        SDL_Surface* d_stackpic;
        SDL_Surface* d_ruinpic;
        SDL_Surface* d_signpostpic;
        SDL_Surface* d_stonepic;
        SDL_Surface* d_templepic;
        SDL_Surface* d_itempic;
};

#endif //E_BIGMAP_H
