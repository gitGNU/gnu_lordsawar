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

#include "Popup.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

Popup::Popup(PG_Widget* parent, Rectangle rect)
    :PG_Label(parent, rect)
{
}

Popup::~Popup()
{
}

bool Popup::eventMouseButtonUp(const SDL_MouseButtonEvent* ev)
{
    QuitModal();
    Hide();
    return true;
}

bool Popup::AcceptEvent(const SDL_Event* ev)
{
    if (ev->type == SDL_MOUSEBUTTONUP)
        return true;

    return PG_Widget::AcceptEvent(ev);
}

// End of file
