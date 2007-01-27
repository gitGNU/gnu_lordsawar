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

#include "SignpostDialog.h"
#include "../defs.h"

E_SignpostDialog::E_SignpostDialog(PG_Widget* parent, PG_Rect rect, Signpost* s)
    :PG_Window(parent, rect, _("Modify signpost"), PG_Window::MODAL), d_signpost(s)
{
    char buffer[51]; buffer[50]='\0';
    snprintf(buffer, 50, _("Signpost at (%i,%i)"), s->getPos().x, s->getPos().y);
    SetTitle(buffer);

    // the layout is simple: an OK button,
    PG_Button* b_ok = new PG_Button(this, PG_Rect(my_width/2-30, my_height-40, 60, 30),
                                   _("OK"));
    b_ok->sigClick.connect(slot(*this, &E_SignpostDialog::okClicked));

    // a label showing the id,
    snprintf(buffer, 50, _("ID: %i"), s->getId());
    new PG_Label(this, PG_Rect(20, 40, 100, 20), buffer);
    
    // another label telling you where to enter the signpost contents
    new PG_Label(this, PG_Rect(20, 70, 50, 20), _("Name:"));
    
    // and a field to enter the name
    PG_LineEdit* e_name = new PG_LineEdit(this, PG_Rect(90, 70, 150, 20));
    e_name->SetText(s->getName().c_str());

    e_name->sigEditEnd.connect(slot(*this, &E_SignpostDialog::nameChanged));
}

E_SignpostDialog::~E_SignpostDialog()
{
}

bool E_SignpostDialog::okClicked(PG_Button* btn)
{
    QuitModal();
    return true;
}

bool E_SignpostDialog::nameChanged(PG_LineEdit* edit)
{
    d_signpost->setName(std::string(edit->GetText()));
    return true;
}
