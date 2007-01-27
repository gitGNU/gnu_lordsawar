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

#ifndef SIGNPOSTINFO_H
#define SIGNPOSTINFO_H

#include <pgthemewidget.h>
#include "signpost.h"

/** Small popup that shows signpost properties
  *
  * When you right-click on a signpost, this small widget is to be shown. It
  * shows the contents of the sign and is destryoed when the user releases
  * the right mouse button.
  */

class SignpostInfo : public PG_ThemeWidget
{
    public:
        /** Constructor
          * 
          * @param t        the signpost whose properties we show
          * @param xpos     x position of the click event on the screen
          * @param ypos     y position of the click event on the screen
          */
        SignpostInfo(Signpost* t, int xpos, int ypos);
        ~SignpostInfo();

    private:
        //! Closes dialog when the right mouse button is released.
        bool eventMouseButtonUp(const SDL_MouseButtonEvent* ev);
};


#endif //SIGNPOSTINFO_H
