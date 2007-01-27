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

#include <stdlib.h>
#include <pgapplication.h>

#include "QuestExpiredDialog.h"
#include "Quest.h"
#include "hero.h"
#include "player.h"
#include "defs.h"

static const unsigned int WIDTH       = 400;
static const unsigned int HEIGHT      = 180;
static const unsigned int MARGIN_X    = 10;
static const unsigned int LINE_WIDTH  = WIDTH - 2 * MARGIN_X;
static const unsigned int LINE_HEIGHT = 20;

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

QuestExpiredDialog::QuestExpiredDialog(PG_Widget* parent, Quest *quest)
	:PG_Window(parent,
            PG_Rect((PG_Application::GetScreenWidth()-WIDTH)/2,
            (PG_Application::GetScreenHeight()-HEIGHT)/2, WIDTH, HEIGHT),
            _("SORRY FOR YOU!"),  PG_Window::MODAL)
{

    queue<std::string> msgs;
    quest->getExpiredMsg(msgs);
    unsigned int y = 40;
    std::string s;
    char buffer[101];
    buffer[100] = '\0';

    snprintf(buffer, 100, _("Your Hero %s did not complete the quest."),
            quest->getHero()->getName().c_str());

    PG_Label *msg = new PG_Label(this, PG_Rect(MARGIN_X, y, 
                       LINE_WIDTH, LINE_HEIGHT), 
                       buffer);
    msg->SetAlignment(PG_Label::CENTER);
    msg->SetFontColor(PG_Color(0, 255, 0));
    d_msgs.push(msg);

    y = 70;

    while (msgs.empty() == false)
    {
        s =  msgs.front();
	    msg = new PG_Label(this, PG_Rect(MARGIN_X, y, 
                           LINE_WIDTH, LINE_HEIGHT), 
                           s.c_str());
	    msg->SetAlignment(PG_Label::CENTER);
        d_msgs.push(msg);
        msgs.pop();
        y += (LINE_HEIGHT +2);
    }

    d_b_close = new PG_Button(this, PG_Rect(MARGIN_X, my_height - 40,LINE_WIDTH, 30),_("Close"),1);
    d_b_close->sigClick.connect(slot(*this, &QuestExpiredDialog::b_closeClicked));
}

QuestExpiredDialog::~QuestExpiredDialog()
{
    delete d_b_close;
    while (d_msgs.empty() == false)
    {
        delete d_msgs.front();
        d_msgs.pop();
    }
}

bool QuestExpiredDialog::b_closeClicked(PG_Button * btn)
{
        QuitModal();
	return true;
}

bool QuestExpiredDialog::eventKeyDown(const SDL_KeyboardEvent* key)
{
    switch(key->keysym.sym)
    {
        case SDLK_RETURN:
            b_closeClicked(0);
            break;
        default:
            break;
    }
    return true;
}

// End of file
