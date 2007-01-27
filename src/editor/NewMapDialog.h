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

#ifndef E_NEWMAPDIALOG_H
#define E_NEWMAPDIALOG_H

#include <pgwindow.h>
#include <pgdropdown.h>
#include <pgbutton.h>
#include "../GameScenario.h"

/** This dialog queries the user for several parameters for a new map. It also
  * deals with creating a new map and sets it up (e.g. supplies a neutral
  * player) as far as neccessary.
  */

class E_NewMapDialog : public PG_Window
{
    public:
        E_NewMapDialog(PG_Widget* parent, PG_Rect rect);
        ~E_NewMapDialog();

        GameScenario* getScenario() {return d_scenario;}

        // Callbacks

        //! different tileset was selected
        bool tilesetChanged(PG_ListBoxBaseItem* item);
        //! OK button clicked
        bool okClicked(PG_Button* button);

    private:
        GameScenario* d_scenario;

        PG_DropDown* d_tilesets;
        PG_DropDown* d_types;
        PG_LineEdit* d_width;
        PG_LineEdit* d_height;
};

#endif //E_NEWMAPDIALOG_H
