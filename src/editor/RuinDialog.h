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

#ifndef E_RUINDIALOG_H
#define E_RUINDIALOG_H

#include <pgwindow.h>
#include <pgbutton.h>
#include <pglineedit.h>

#include "../ruin.h"

/** Using this class, you can modify a ruin's properties.
  *
  * Namely these are name and content (the defending stack).
  */

class E_RuinDialog : public PG_Window
{
    public:
        E_RuinDialog(PG_Widget* parent, PG_Rect rect, Ruin* r);
        ~E_RuinDialog();

        //! Callback when OK was pressed
        bool okClicked(PG_Button* btn);

        //! Callback when name was changed
        bool nameChanged(PG_LineEdit* edit);

        //! Callback when stack is managed
        bool keeperUpdated(PG_Button* btn);

    private:
        Ruin* d_ruin;
};

#endif //E_RUINDIALOG_H
