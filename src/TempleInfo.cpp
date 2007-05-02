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

#include <pglabel.h>

#include "TempleInfo.h"
#include "defs.h"


#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)


TempleInfo::TempleInfo(Temple* t, int xpos, int ypos)
    :PG_ThemeWidget(0, Rectangle(xpos, ypos, 210, 70))
{
    char buf[101]; buf[100] = '\0';

    new PG_Label(this, Rectangle(15, 15, 180, 15), t->getName().c_str());

    snprintf(buf, 100, "(%i,%i)", t->getPos().x, t->getPos().y);
    new PG_Label(this, Rectangle(15, 40, 80, 15), _("Position:"));
    new PG_Label(this, Rectangle(95, 40, 70, 15), buf);

    // set a capture so we are informed when user release button
    SetCapture();
}

TempleInfo::~TempleInfo()
{
}

bool TempleInfo::eventMouseButtonUp(const SDL_MouseButtonEvent* ev)
{
    if (ev->button == SDL_BUTTON_RIGHT)
        QuitModal();
    return true;
}
