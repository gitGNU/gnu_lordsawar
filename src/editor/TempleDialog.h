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

#ifndef E_TEMPLEDIALOG_H
#define E_TEMPLEDIALOG_H

#include <pgwindow.h>
#include <pglineedit.h>
#include <pgbutton.h>

#include "../temple.h"

/** Dialog to modify temple attributes
  * 
  * This dialog pops up upon selecting a temple. It includes items to modify the
  * temple's behaviour, currently these are only the temple name.
  */

class E_TempleDialog : public PG_Window
{
    public:
        /** Default constructor
          * 
          * @param parent   parent widget (used internally by paragui)
          * @param rect     the rectangle of this widget (paragui stuff)
          * @param temple   pointer to the temple to be modified
          */
        E_TempleDialog(PG_Widget* parent, PG_Rect rect, Temple* t);
        ~E_TempleDialog();

        //! Callback when clicking on the OK button
        bool okClicked(PG_Button* btn);

        //! Callback when changing the temple name
        bool nameChanged(PG_LineEdit* edit);

        //! Callback when changing the temple type
        bool typeChanged(PG_Button* btn);

    private:
        Temple* d_temple;

        PG_Label* d_l_type;

        PG_Button* d_b_up;
        PG_Button* d_b_down;
};

#endif //E_TEMPLEDIALOG_H
