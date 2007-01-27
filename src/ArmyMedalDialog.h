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

#ifndef ARMYMEDALDIALOG_H
#define ARMYMEDALDIALOG_H

#include <pgwindow.h>
#include "army.h"

/** Brief Dialog for medal award
  * 
  *
  *
  *
  */

class ArmyMedalDialog : public PG_Window
{ 
    public:
        /** Default constructor
          *
          * The dialog is displayed when an army gets a medal
          * 
          * @param army         the army that gets a medal
        */
        ArmyMedalDialog(Army* army, PG_Widget* parent, PG_Rect rect);
        ~ArmyMedalDialog();

	//! This function is called when the user pushes the OK button.
	bool b_okClicked(PG_Button* btn);

    private:
        bool eventKeyDown(const SDL_KeyboardEvent* key);
  
        Army* d_army;
        PG_Button* d_b_ok;
        PG_Button* d_b_pic;
        PG_Label* d_l_info;
        PG_Label* d_l_name;

        PG_Label* d_l_xp;
        PG_Label* d_l_level1;
        PG_Label* d_l_level2;
};

#endif // ARMYMEDALDIALOG_H

