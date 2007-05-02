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

#ifndef POPUP_H
#define POPUP_H

#include <pglabel.h>


/** A widget that vanishes on mouse clicks.
  * 
  * Paragui _almost_ implements this already, but only almost, some details
  * still have to be done.
  */

class Popup : public PG_Label
{
    public:
        //! Constructor; the parameters are inherited from paragui
        Popup(PG_Widget* parent, Rectangle rect);
        ~Popup();

        //! Event handler, hides the widget
        bool eventMouseButtonUp(const SDL_MouseButtonEvent *ev);

        /** We need this handler since we even want to capture events not
          * meant for us.
          */
        bool AcceptEvent(const SDL_Event* ev);
};

#endif

// End of file
