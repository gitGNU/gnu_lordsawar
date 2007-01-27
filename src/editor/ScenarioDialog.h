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

#ifndef E_SCENARIODIALOG_H
#define E_SCENARIODIALOG_H

#include "../GameScenario.h"
#include <pgwindow.h>
#include <pgbutton.h>
#include <pglineedit.h>

class Player;

/** Dialog to modify Scenario data
  * 
  * This dialog should be finally used to change scenarios's name, and all future
  * customizable options
  *
  */

class E_ScenarioDialog :public PG_Window
{
    public:
        E_ScenarioDialog(PG_Widget* parent, PG_Rect rect, GameScenario* sc);
        ~E_ScenarioDialog();

        //! Callback when OK is pressed
        bool okClicked(PG_Button* btn);

        //! Callback when scenario name is changed
        bool nameChanged(PG_LineEdit* edit);
        bool commentChanged(PG_LineEdit* edit);

    private:
        // Data
        PG_LineEdit* d_l_name;
        PG_LineEdit* d_l_comment;

        GameScenario* d_scenario;
};


#endif // E_SCENARIODIALOG_H
