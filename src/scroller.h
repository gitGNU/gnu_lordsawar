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

#ifndef SCROLLER_H
#define SCROLLER_H

#include <pgtimerobject.h>
#include <pgbutton.h>
#include "smallmap.h"

/** Authomathic Scroller button for the bigmap
  * 
  * This special button is used to scroll the bigmap when the mouse 
  * moves on it.
  * 
  */

class Scroller : public PG_Button,public PG_TimerObject
{ 
    public:
        //! Default constructor and destructor
        Scroller(PG_Widget* parent, PG_Rect rect, int id, SmallMap * smap, int x, int y);
        ~Scroller();

    private:

        void eventMouseEnter();
        void eventMouseLeave();

        //! Begin the scroll
        Uint32 eventTimer (ID id, Uint32 interval);

        Uint32 d_timer;
        SmallMap * d_smap;
        int d_x;
        int d_y;
        int d_time;
        bool d_firsttime;
};

#endif // SCROLLER_H

