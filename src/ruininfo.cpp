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

#include <sstream>
#include <pgmessagebox.h>
#include "ruininfo.h"
#include "ruin.h"
#include "defs.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

static unsigned int width  = 200;
static unsigned int height = 90;

RuinInfo::RuinInfo(Ruin* ruin, int screen_x, int screen_y)
    :PG_ThemeWidget(0, Rectangle(screen_x, screen_y, width, height)), d_ruin(ruin)
{
    char buf[100+1]; buf[100] = '\0';

    unsigned int y = 20;
    const unsigned int col1 = 15, col2 = 115, h = 15;
    const unsigned int step_y = h + 4;

    new PG_Label(this, Rectangle(col1, y, 150, h), d_ruin->getName().c_str()); 

    y += step_y;
    new PG_Label(this, Rectangle(col1, y, 80, h), _("Position:")); 
    snprintf(buf, 100, "(%i,%i)", ruin->getPos().x, ruin->getPos().y);
    new PG_Label(this, Rectangle(col2, y, 100, h), buf); 

    y += step_y;
    new PG_Label(this, Rectangle(col1, y, 90, h), _("Searched:")); 
    new PG_Label(this, Rectangle(col2, y, 100, h), 
                               ruin->isSearched() ? _("Yes") : _("No"));

    // SetCapture, so the ruininfo object can detect
    // the releasing of the
    // right mouse button outside the RuinInfo window
    SetCapture();
}

RuinInfo::~RuinInfo()
{
}

bool RuinInfo::eventMouseButtonUp(const SDL_MouseButtonEvent* event)
{
    if(event->button == SDL_BUTTON_RIGHT)
        QuitModal();
    return true;
}
