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

#include "goldreport.h"
#include "defs.h"
#include <pglabel.h>
#include <pgbutton.h>
#include <sstream>

GoldReport::GoldReport(PG_Widget* parent, PG_Rect rect)
    :PG_Window(parent, rect, _("Gold report"), PG_Window::MODAL)
{
    PG_Button* b_ok = new PG_Button(this, PG_Rect((Width()/2 - 40),
                    (Height() - 35), 80, 25), _("OK"),1);
    b_ok->sigClick.connect(slot(*this, &GoldReport::b_okClicked));

    PG_Label* label = new PG_Label(this, PG_Rect(50, 50, 200, 20),
                    _("Player's gold ressources:"));

    int height = 80;
    Playerlist* plist = Playerlist::getInstance();
    for (Playerlist::iterator it = plist->begin(); it != plist->end(); it++)
    {
        if (((*it) == plist->getNeutral()) || ((*it)->isDead()))
            continue;

        label = new PG_Label(this, PG_Rect(70, height, 100, 15),
                    (*it)->getName().c_str());
        label->SetFontColor((*it)->getColor());

        std::stringstream sgold;
        sgold <<(*it)->getGold();
        label = new PG_Label(this, PG_Rect(200, height, 50, 15), sgold.str().c_str());
        label->SetFontColor((*it)->getColor());

        height += 20;
    }
}

GoldReport::~GoldReport()
{
}

bool GoldReport::eventKeyDown(const SDL_KeyboardEvent* key)
{	
    switch (key->keysym.sym)
    {		
	case SDLK_RETURN:
	  b_okClicked(0);
	  break;
        default:
          break;
    }
	
    return true;
}

bool GoldReport::b_okClicked(PG_Button* btn)
{
    QuitModal();
    return true;
}


//End of file
