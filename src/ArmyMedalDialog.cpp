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
#include <pgbutton.h>
#include "ArmyMedalDialog.h"
#include "army.h"
#include "defs.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

ArmyMedalDialog::ArmyMedalDialog(Army* army, PG_Widget* parent, PG_Rect rect)
    :PG_Window(parent, rect, _("medal awarded"), PG_Window::MODAL), d_army(army)
{
    char buffer[31];
    buffer[30] = '\0';

    //the army pic
    d_b_pic = new PG_Button(this, PG_Rect(my_width/2 - 25, 40, 50, 50), "",2);
    d_b_pic->EnableReceiver(false);
    d_b_pic->SetIcon(d_army->getPixmap(), 0, 0);

   //the name of the army
    d_l_name = new PG_Label(this, PG_Rect(my_width/2 - 100, 100, 200, 20),
                            d_army->getName().c_str());
    d_l_name->SetAlignment(PG_Label::CENTER);

    snprintf(buffer, 30, _("xp: %.2f"), d_army->getXP());
    d_l_xp = new PG_Label(this, PG_Rect(my_width/2 - 40, 130, 95, 20), buffer);
    snprintf(buffer, 30, "%i", d_army->getLevel());
    d_l_level1 = new PG_Label(this, PG_Rect(my_width/2 - 40, 150, 45, 20),_("level: "));
    d_l_level2 = new PG_Label(this, PG_Rect(my_width/2 + 10, 150, 30, 20), buffer);
    d_l_level2->SetFontColor(PG_Color(50, 255, 50));

    d_b_ok = new PG_Button(this, PG_Rect(my_width/2 - 40, 190, 80, 30),_("OK"),1);
    d_b_ok->sigClick.connect(slot(*this, &ArmyMedalDialog::b_okClicked));
}

ArmyMedalDialog::~ArmyMedalDialog() {
    delete d_b_ok;
    delete d_b_pic;
    //delete d_l_info;
    delete d_l_name;

    delete d_l_xp;
    delete d_l_level1;
    delete d_l_level2;
}

bool ArmyMedalDialog::b_okClicked(PG_Button* btn)
{
    d_army->printAllDebugInfo();

    QuitModal();
    return true;
}

bool ArmyMedalDialog::eventKeyDown(const SDL_KeyboardEvent* key)
{
    switch(key->keysym.sym)
    {
        case SDLK_RETURN:
            b_okClicked(0);
            break;
        default:
            break;
    }
    return true;
}
