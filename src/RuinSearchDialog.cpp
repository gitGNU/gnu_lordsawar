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

#include "RuinSearchDialog.h"
#include "FightDialog.h"
#include "File.h"
#include "defs.h"
#include <SDL_image.h>
#include <stdlib.h>
#include <pgapplication.h>
#include <pgbutton.h>
#include <pglabel.h>

RuinSearchDialog::RuinSearchDialog(PG_Widget* parent, int gold)
    :PG_Window(parent,
            Rectangle((PG_Application::GetScreenWidth()-330)/2,
            (PG_Application::GetScreenHeight()-370)/2, 330, 370),
            _("Entering the ruin..."),  PG_Window::MODAL)
{
    d_background = File::getMiscPicture("ruin_1.jpg", false);

    char buffer[100];
    d_l_msg1 = new PG_Label(this, Rectangle(10, 280, 280, 20), _("You defeated the monsters"));
    d_l_msg1->SetAlignment(PG_Label::CENTER);
    sprintf(buffer,
            ngettext("and found %i gold piece!", "and found %i gold pieces!", gold),
            gold);
    d_l_msg2 = new PG_Label(this, Rectangle(10, 300, 280, 20), buffer);
    d_l_msg2->SetAlignment(PG_Label::CENTER);

    //Ugly, hardcoded sizes...
    d_l_pic = new PG_Label(this, Rectangle(5, 30, 320, 240), "");
    d_l_pic->SetIcon(d_background);

    d_b_close = new PG_Button(this, Rectangle(20, my_height - 40, 280, 30), _("Close"),3);
    d_b_close->sigClick.connect(slot(*this, &RuinSearchDialog::b_closeClicked));
}

RuinSearchDialog::~RuinSearchDialog()
{
    delete d_l_msg1;
    delete d_l_msg2;
    SDL_FreeSurface(d_background);
}

bool RuinSearchDialog::b_closeClicked(PG_Button* btn)
{
    QuitModal();
    return true;
}

bool RuinSearchDialog::eventKeyDown(const SDL_KeyboardEvent* key)
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
