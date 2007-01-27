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

#ifndef E_SIGNPOSTDIALOG_H
#define E_SIGNPOSTDIALOG_H

#include <pgwindow.h>
#include <pglineedit.h>
#include <pgbutton.h>

#include "../signpost.h"

/** Dialog to modify signpost attributes
  * 
  * This dialog pops up upon selecting a signpost. It includes items to modify 
  * the signpost's behaviour, currently these are only the signpost contents.
  */

class E_SignpostDialog : public PG_Window
{
    public:
        /** Default constructor
          * 
          * @param parent   parent widget (used internally by paragui)
          * @param rect     the rectangle of this widget (paragui stuff)
          * @param temple   pointer to the temple to be modified
          */
        E_SignpostDialog(PG_Widget* parent, PG_Rect rect, Signpost* s);
        ~E_SignpostDialog();

        //! Callback when clicking on the OK button
        bool okClicked(PG_Button* btn);

        //! Callback when changing the temple name
        bool nameChanged(PG_LineEdit* edit);

    private:
        Signpost* d_signpost;
};

#endif //E_SIGNPOSTDIALOG_H
