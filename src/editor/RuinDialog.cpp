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

#include "RuinDialog.h"
#include "StackDialog.h"
#include "../defs.h"


E_RuinDialog::E_RuinDialog(PG_Widget* parent, PG_Rect rect, Ruin* r)
    :PG_Window(parent, rect, _("Modify ruin"), PG_Window::MODAL), d_ruin(r)
{
    char buffer[81]; buffer[80] = '\0';
    snprintf(buffer, 80, _("Ruin at (%i,%i)"), r->getPos().x, r->getPos().y);
    SetTitle(buffer);

    new PG_Label(this, PG_Rect(40, 50, 50, 20), _("ID:"));
    snprintf(buffer, 80, "%i", r->getId());
    new PG_Label(this, PG_Rect(100, 50, 50, 20), buffer);
    new PG_Label(this, PG_Rect(40, 80, 50, 20), _("Name:"));

    PG_LineEdit* e_name = new PG_LineEdit(this, PG_Rect(100, 80, 150, 20));
    e_name->SetText(r->getName().c_str());
    e_name->sigEditEnd.connect(slot(*this, &E_RuinDialog::nameChanged));

    PG_Rect arect(50, my_height - 40, 80, 30);
    PG_Button* btn = new PG_Button(this, arect, _("OK"));
    btn->sigClick.connect(slot(*this, &E_RuinDialog::okClicked));

    arect.x = my_width - 100;
    btn = new PG_Button(this, arect, _("Keeper"));
    btn->sigClick.connect(slot(*this, &E_RuinDialog::keeperUpdated));
}

E_RuinDialog::~E_RuinDialog()
{
}

bool E_RuinDialog::okClicked(PG_Button* btn)
{
    QuitModal();
    return true;
}

bool E_RuinDialog::nameChanged(PG_LineEdit* edit)
{
    d_ruin->setName(std::string(edit->GetText()));
    return true;
}

bool E_RuinDialog::keeperUpdated(PG_Button* btn)
{
    // first, get the occupant of the ruin; if it doesn't exist, create a stack
    Stack* s = d_ruin->getOccupant();
    if (s == 0)
        s = new Stack(0, d_ruin->getPos());

    // second, fire up the stack dialog. Since this dialog is rather small and
    // the stack dialog needs rather a lot of space, we take the rect of our
    // parent (bigmap)
    PG_Rect r(*GetParent());
    r.x = 0;
    r.y = 0;

    E_StackDialog as(GetParent(), r, s, false);
    as.Show();
    as.RunModal();
    as.Hide();

    // if the stack has been emptied, delete it
    if (s->empty())
    {
        delete s;
        s = 0;
    }

    // set s as new occupant
    d_ruin->setOccupant(s);
    
    return true;
}
