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

#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <pgthemewidget.h>
#include <pgtimerobject.h>
#include <pglabel.h>

/** Simple information tooltip
  * 
  * At the beginning of a game, the appropriate master class (e.g. W_Edit)
  * creates a tooltip. This tooltip is then positioned and given some text
  * whenever the user hovers the mouse pointer over an interesting button.
  */

class ToolTip : public PG_ThemeWidget,public PG_TimerObject
{ 
    public:
        //! Default ParaGUI constructor
        ToolTip(PG_Widget* parent, PG_Rect rect);
        ~ToolTip();

        /** Initializes the tooltip
          * 
          * @param text     the text to be displayed
          * @param screen_x the preferred x position of the tooltip (unless it
          *                 is too wide)
          * @param screen_y the preferred y position of the tooltip
          * @param max_x    the maximum x extension for the tooltip
          * @param max_y    the maximum y extension of the tooltip
          */
        void setData(const char* text, int screen_x, int screen_y, int max_x, int max_y);

        // Shortcut to cut down the parameters a bit
        void setData(const char* text, PG_Rect r)
        {
            setData(text, r.x, r.y, r.w, r.h);
        }

    private:
        //! Closes dialog when mouse button is pressed or mouse is moved
        bool eventMouseMotion(const SDL_MouseMotionEvent* event);
        bool eventMouseButtonDown(const SDL_MouseButtonEvent* event);
        //! Shows the tooltip after TOOLTIP_WAIT milliseconds
        Uint32 eventTimer (ID id, Uint32 interval);

        Uint32 d_timer;
        int d_movecounter;
        PG_Label* d_lab;
};

#endif // TOOLTIP_H

