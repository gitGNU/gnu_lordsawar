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

#ifndef TEMPLEINFO_H
#define TEMPLEINFO_H

#include <pgthemewidget.h>
#include "temple.h"

/** Small popup that shows temple properties
  *
  * When you right-click on a temple, this small widget is to be shown. It
  * describes temple name and position and is destryoed when the user releases
  * the right mouse button.
  */

class TempleInfo : public PG_ThemeWidget
{
    public:
        /** Constructor
          * 
          * @param t        the temple whose properties we show
          * @param xpos     x position of the click event on the screen
          * @param ypos     y position of the click event on the screen
          */
        TempleInfo(Temple* t, int xpos, int ypos);
        ~TempleInfo();

    private:
        //! Closes dialog when the right mouse button is released.
        bool eventMouseButtonUp(const SDL_MouseButtonEvent* ev);
};


#endif //TEMPLEINFO_H
