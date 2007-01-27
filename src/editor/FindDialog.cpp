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

#include "FindDialog.h"
#include "../defs.h"
#include "../citylist.h"
#include "../ruinlist.h"
#include "../templelist.h"
#include "../playerlist.h"
#include "../stacklist.h"

E_FindDialog::E_FindDialog(PG_Widget* parent, PG_Rect rect, E_Bigmap* map)
    :PG_Window(parent, rect, _("Find object"), PG_Window::MODAL), d_map(map)
{
    SetTitle(_("Find object"));
    
    PG_Rect r(20, 40, 50, 20);
    new PG_Label(this, r, _("ID:"));

    r.x += 60;
    PG_LineEdit* edit = new PG_LineEdit(this, r);
    edit->SetText("");
    edit->SetValidKeys("0123456789");
    edit->sigEditEnd.connect(slot(*this, &E_FindDialog::idChanged));

    r.x += 60; r.w = 60;
    d_result = new PG_Label(this, r, "");
    
    r.SetRect(my_width/2 - 30, my_height - 50, 60, 30);
    PG_Button* btn = new PG_Button(this, r, _("OK"));
    btn->sigClick.connect(slot(*this, &E_FindDialog::okClicked));
}

E_FindDialog::~E_FindDialog()
{
}

bool E_FindDialog::idChanged(PG_LineEdit* edit)
{
    // create an integer from the input
    Uint32 id = static_cast<Uint32>(atoi(edit->GetText()));
    
    bool found = false;
    PG_Point pos;
    
    // First, search for cities
    Citylist* cl = Citylist::getInstance();
    for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
        if ((*it).getId() == id)
        {
            found = true;
            pos = (*it).getPos();
            d_result->SetText(_("City"));
            break;
        }

    // next, search for temples
    if (!found)
    {
        Templelist* tl = Templelist::getInstance();
        for (Templelist::iterator it = tl->begin(); it != tl->end(); it++)
            if ((*it).getId() == id)
            {
                found = true;
                pos = (*it).getPos();
                d_result->SetText(_("Temple"));
                break;
            }
    }

    // next, search for ruins
    if (!found)
    {
        Ruinlist* rl = Ruinlist::getInstance();
        for (Ruinlist::iterator it = rl->begin(); it != rl->end(); it++)
            if ((*it).getId() == id)
            {
                found = true;
                pos = (*it).getPos();
                d_result->SetText(_("Ruin"));
                break;
            }
    }

    // finally, look if we have a stack
    if (!found)
    {
        Playerlist* pl = Playerlist::getInstance();
        for (Playerlist::iterator pit = pl->begin(); pit != pl->end(); pit++)
        {
            Stacklist* sl = (*pit)->getStacklist();
            for (Stacklist::iterator it = sl->begin(); it != sl->end(); it++)
                if ((*it)->getId() == id)
                {
                    found = true;
                    pos = (*it)->getPos();
                    d_result->SetText(_("Stack"));
                    break;
                }
            if (found)
                break;
        }
    }

    if (!found)
    {
        d_result->SetText(_("n.a."));
        return true;
    }

    d_map->centerView(pos);
    return true;
}

bool E_FindDialog::okClicked(PG_Button* btn)
{
    QuitModal();
    return true;
}
