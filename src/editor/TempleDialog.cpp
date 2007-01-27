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

#include <stdio.h>
#include <pglabel.h>

#include "TempleDialog.h"
#include "../defs.h"

E_TempleDialog::E_TempleDialog(PG_Widget* parent, PG_Rect rect, Temple* t)
    :PG_Window(parent, rect, _("Modify temple"), PG_Window::MODAL), d_temple(t)
{
    char buffer[51]; buffer[50]='\0';
    snprintf(buffer, 50, _("Temple at (%i,%i)"), t->getPos().x, t->getPos().y);
    SetTitle(buffer);

    // the layout is simple: an OK button,
    PG_Button* b_ok = new PG_Button(this, PG_Rect(my_width/2-30, my_height-40, 60, 30),
                                   _("OK"));
    b_ok->sigClick.connect(slot(*this, &E_TempleDialog::okClicked));

    // a label showing the id,
    snprintf(buffer, 50, _("ID: %i"), t->getId());
    new PG_Label(this, PG_Rect(20, 40, 100, 20), buffer);
    
    // another label telling you where to enter the temple name
    new PG_Label(this, PG_Rect(20, 70, 50, 20), _("Name:"));
    
    // and a field to enter the name
    PG_LineEdit* e_name = new PG_LineEdit(this, PG_Rect(90, 70, 150, 20));
    e_name->SetText(t->getName().c_str());

    // another label telling you the temple type
    new PG_Label(this, PG_Rect(20, 100, 50, 20), _("Type:"));

    snprintf(buffer, 50, "%i", d_temple->getType());
    d_l_type = new PG_Label(this, PG_Rect(80, 100, 30, 20), buffer);

    // raise_level button
    d_b_up = new PG_Button(this, PG_Rect(100, 100, 10, 8), "+");
    d_b_up->sigClick.connect(slot(*this, &E_TempleDialog::typeChanged));
    d_b_up->SetFontSize(8);

    // lower_level button
    d_b_down = new PG_Button(this, PG_Rect(100, 110, 10, 8), "-");
    d_b_down->sigClick.connect(slot(*this, &E_TempleDialog::typeChanged));
    d_b_down->SetFontSize(8);

    e_name->sigEditEnd.connect(slot(*this, &E_TempleDialog::nameChanged));
}

E_TempleDialog::~E_TempleDialog()
{
}

bool E_TempleDialog::okClicked(PG_Button* btn)
{
    QuitModal();
    return true;
}

bool E_TempleDialog::nameChanged(PG_LineEdit* edit)
{
    d_temple->setName(std::string(edit->GetText()));
    return true;
}

bool E_TempleDialog::typeChanged(PG_Button* btn)
{
    unsigned int type=d_temple->getType(); 

    if (btn == d_b_up)
    {
        if (type<TEMPLE_TYPES-1) type++;
    }
    else
    {
        if (type>0) type--;
    }

    d_temple->setType(type);

    char buffer[2]; buffer[2]='\0';
    snprintf(buffer, 2, "%i", type);
    d_l_type->SetText(buffer);

    return true;
}
