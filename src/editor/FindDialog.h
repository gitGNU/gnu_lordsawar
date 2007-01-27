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

#ifndef E_FINDDIALOG_H
#define E_FINDDIALOG_H

#include <pgwindow.h>
#include <pglineedit.h>
#include <pgbutton.h>
#include <pglabel.h>

#include "Bigmap.h"

/** This dialog has the function that you enter the id of an object, and the
  * map is automatically centered on the object.
  */

class E_FindDialog : public PG_Window
{
    public:
        E_FindDialog(PG_Widget* parent, PG_Rect rect, E_Bigmap* map);
        ~E_FindDialog();

        //! callback when entering the id
        bool idChanged(PG_LineEdit* edit);

        //! Callback when pressing the ok button
        bool okClicked(PG_Button* btn);

    private:
        E_Bigmap* d_map;
        PG_Label* d_result;
        
};

#endif //E_FINDDIALOG_H
