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

#include "hero_offer.h"
#include <stdlib.h>
#include <iostream>
#include <pgapplication.h>
#include <pglineedit.h>
#include <pglabel.h>
#include <pgbutton.h>

#include "hero.h"
#include "playerlist.h"
#include "citylist.h"
#include "city.h"
#include "player.h"
#include "File.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

Hero_offer::Hero_offer(PG_Widget* parent, Hero* hero, int gold)
    :PG_Window(parent, PG_Rect(0, 0, 210, 300), _("Hero offer"), PG_Window::MODAL),
    d_gold(gold)
{
    debug("Hero_offer::Hero_offer")

    MoveWidget((PG_Application::GetScreenWidth() - my_width) / 2,
            (PG_Application::GetScreenHeight() - my_height) / 2 - 50);

    if (hero->getGender() == Army::MALE)    //male hero
        d_recruitpic = File::getMiscPicture("recruit_male.png");
    else
        d_recruitpic = File::getMiscPicture("recruit_female.png");

    d_l_pic = new PG_Label(this, PG_Rect(5, 30, 200, 200), "");
    d_l_pic->SetIcon(d_recruitpic);

    d_b_accept = new PG_Button(this, PG_Rect(10, my_height - 30, 70, 20), _("Accept"),1);
    d_b_accept->sigClick.connect(slot(*this, &Hero_offer::b_acceptClicked));

    d_b_reject = new PG_Button(this, PG_Rect(my_width - 80, my_height - 30, 70, 20), _("Reject"),2);
    d_b_reject->sigClick.connect(slot(*this, &Hero_offer::b_rejectClicked));

    d_e_name = new PG_LineEdit(this, PG_Rect(40, 240, 120, 20));
    d_e_name->SetText(hero->getName().c_str());

    char text[80], text2[80];

    if (gold)
    {
        strcpy(text, _("A hero wants to join you"));
        sprintf(text2, _("for %i gold"), gold);
    }
    else
    {
        strcpy(text, _("A hero wants to join for free!"));
        strcpy(text2, "");
    }

    d_l_text = new PG_Label(this, PG_Rect(10, 195, 180, 20), text);
    d_l_text2 = new PG_Label(this, PG_Rect(10, 215, 180, 20), text2);
}

Hero_offer::~Hero_offer()
{
    delete d_b_accept;
    delete d_b_reject;
    delete d_e_name;
    delete d_l_pic;
    delete d_l_text;
    delete d_l_text2;
    SDL_FreeSurface(d_recruitpic);
}

bool Hero_offer::b_acceptClicked(PG_Button* btn)
{
    debug("b_accept_clicked()");
    d_retval = true;

    QuitModal();
    return true;
}

bool Hero_offer::b_rejectClicked(PG_Button* btn)
{
    d_retval = false;

    QuitModal();
    return true;
}
