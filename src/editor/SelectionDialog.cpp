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

#include "SelectionDialog.h"
#include "../defs.h"

E_SelectionDialog::E_SelectionDialog(PG_Widget* parent, PG_Rect rect,
                                     const std::list<std::string> strings)
    :PG_Window(parent, rect, _("Select an item"), PG_Window::MODAL)
{
    PG_Rect r(30, 40, my_width - 60, 20);
    PG_Button* btn = 0;
    int id = 0;
    
    // For each string, create a button with the string and a raising id plus
    // a button for cancelling the selection
    for (std::list<std::string>::const_iterator it = strings.begin();
         it != strings.end(); it++, id++)
    {
        btn = new PG_Button(this, r, (*it).c_str(), id);
        btn->sigClick.connect(slot(*this, &E_SelectionDialog::buttonClicked));
        r.y += 25;
    }

    btn = new PG_Button(this, r, _("Cancel"), -1);
    btn->sigClick.connect(slot(*this, &E_SelectionDialog::buttonClicked));
}

E_SelectionDialog::~E_SelectionDialog()
{
}

bool E_SelectionDialog::buttonClicked(PG_Button* btn)
{
    d_index = btn->GetID();
    QuitModal();
    return true;
}
