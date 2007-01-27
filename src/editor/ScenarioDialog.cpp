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
#include <pgapplication.h>
#include <sstream>

#include "ScenarioDialog.h"
#include "../defs.h"


E_ScenarioDialog::E_ScenarioDialog(PG_Widget* parent, PG_Rect rect, GameScenario* sc)
    :PG_Window(parent, rect, _("Manage Scenario"), PG_Window::MODAL), d_scenario(sc)
{
    PG_Rect r(15, 40, 200, 20);
    new PG_Label(this, r, _("Scenario Name:"));
    r.SetRect(15, 60, 200, 20);
    d_l_name = new PG_LineEdit(this, r,"LineEdit",50);
    d_l_name->SetText(d_scenario->getName(false).c_str());

    r.SetRect(15, 90, 200, 20);
    new PG_Label(this, r, _("Scenario Description:"));

    r.SetRect(15, 110, 200, 20);
    // d_l_comment = new PG_MultiLineEdit(this, r);
    d_l_comment = new PG_LineEdit(this, r,"LineEdit",51);
    d_l_comment->SetText(d_scenario->getComment(false).c_str());

    int x= (my_width-80)/2;
    int y= my_height-30;

    r.SetRect(x, y, 80, 25);
    PG_Button* b_ok = new PG_Button(this, r, _("OK"));

    d_l_name->sigEditEnd.connect(slot(*this, &E_ScenarioDialog::nameChanged));
    d_l_comment->sigEditEnd.connect(slot(*this, &E_ScenarioDialog::commentChanged));
    b_ok->sigClick.connect(slot(*this, &E_ScenarioDialog::okClicked));
}

E_ScenarioDialog::~E_ScenarioDialog()
{
    delete d_l_name;
    delete d_l_comment;
}

bool E_ScenarioDialog::okClicked(PG_Button* btn)
{
    QuitModal();
    return true;
}

bool E_ScenarioDialog::nameChanged(PG_LineEdit* edit)
{
    if (!d_scenario)
        return true;
    
    d_scenario->setName(std::string(edit->GetText()));
            
    return true;
}

bool E_ScenarioDialog::commentChanged(PG_LineEdit* edit)
{
    if (!d_scenario)
        return true;
    
    d_scenario->setComment(std::string(edit->GetText()));
            
    return true;
}

