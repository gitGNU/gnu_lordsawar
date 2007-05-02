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

#include "QuestCompletedDialog.h"
#include "Quest.h"
#include "hero.h"
#include "player.h"
#include "defs.h"

static const unsigned int WIDTH       = 400;
static const unsigned int HEIGHT      = 340;
static const unsigned int MARGIN_X    = 10;
static const unsigned int LINE_WIDTH  = WIDTH - 2 * MARGIN_X;
static const unsigned int LINE_HEIGHT = 20;

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

QuestCompletedDialog::QuestCompletedDialog(PG_Widget* parent, Quest *quest)
	:PG_Window(parent,
            Rectangle((PG_Application::GetScreenWidth()-WIDTH)/2,
            (PG_Application::GetScreenHeight()-HEIGHT)/2, WIDTH, HEIGHT),
            _("CONGRATULATIONS!"), PG_Window::MODAL)
{

    queue<std::string> msgs;
    quest->getSuccessMsg(msgs);
    unsigned int y = 40;
    char buffer[101];
    buffer[100]='\0';
    std::string s;

    snprintf(buffer, 100, _("Your Hero %s completed the quest."),
             quest->getHero()->getName().c_str());

    PG_RichEdit *msg = new PG_RichEdit(this, Rectangle(MARGIN_X, y, 
                       LINE_WIDTH, LINE_HEIGHT)); 
    msg->SetText(buffer);
    msg->SetTransparency(255);
    msg->SetFontColor(PG_Color(0, 255, 0));
    d_msgs.push(msg);

    y = 70;

    while (msgs.empty() == false)
    {
        s =  msgs.front();
	    msg = new PG_RichEdit(this, Rectangle(MARGIN_X, y, 
                           LINE_WIDTH, 2*LINE_HEIGHT)); 
        msg->SetText(s.c_str());
        msg->SetTransparency(255);
        d_msgs.push(msg);
        msgs.pop();
        y += (LINE_HEIGHT*2 +2);
    }


    Stack *stack;
    Quest::getHeroById(quest->getHeroId(), &stack);
    debug("Stack of the hero: " << stack);

    Player *p = stack->getPlayer();
    debug("Owner of the hero: " << p->getName());

    // now the reward - at the moment a very simple one (just money):
	int gold = rand() % 1000;
    p->giveReward(gold);

    debug("Gold added: " << gold);

	snprintf(buffer, 100, ngettext("You have been rewarded with %i gold piece!",
                             "You have been rewarded, with %i gold pieces!", gold),
            gold);
	msg = new PG_RichEdit(this, Rectangle(MARGIN_X, y, LINE_WIDTH, LINE_HEIGHT*2));
    msg->SetText(buffer);
	msg->SetTransparency(255);
    d_msgs.push(msg);

	d_b_close = new PG_Button(this,Rectangle(MARGIN_X, my_height - 40, 
                              LINE_WIDTH, 30), 
                              _("Close"),1);
	d_b_close->sigClick.connect(slot(*this, &QuestCompletedDialog::b_closeClicked));
}

QuestCompletedDialog::~QuestCompletedDialog()
{
    delete d_b_close;
    while (d_msgs.empty() == false)
    {
        delete d_msgs.front();
        d_msgs.pop();
    }
}

bool QuestCompletedDialog::b_closeClicked(PG_Button* btn)
{
    QuitModal();
	return true;
}

bool QuestCompletedDialog::eventKeyDown(const SDL_KeyboardEvent* key)
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
