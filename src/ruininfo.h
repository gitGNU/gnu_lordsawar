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

#ifndef RUININFO_H
#define RUININFO_H

#include <pgthemewidget.h>

class Ruin;

/** Simple information popup for a ruin
  * 
  * If you right-click on a ruin in the game window, this dialog is shown, which
  * just tells the properties of the ruin (name, location, search status). It
  * captures all SDL events and finished as soon as the right mouse button is
  * released.
  */

class RuinInfo : public PG_ThemeWidget
{ 
    public:
        /** Default constructor
          *
          * The dialog is displayed with the top right at the position of the
          * start of the right-click
          * 
          * @param ruin         the ruin to be displayed
          * @param screen_x     the x_coordinate of the original click
          * @param screen_y     the y coordinate of the original click
          */
        RuinInfo(Ruin* ruin, int screen_x, int screen_y);
        ~RuinInfo();

    private:
        //! Closes dialog when the right mouse button is released
        bool eventMouseButtonUp(const SDL_MouseButtonEvent* event);

        // DATA
        Ruin* d_ruin;
};

#endif // RUININFO_H

