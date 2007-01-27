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

#ifndef ARMYDIALOG_H
#define ARMYDIALOG_H

#include <pgwindow.h>
#include <pgbutton.h>
#include <pgcheckbutton.h>
#include <pglabel.h>
#include "army.h"

class PG_Label;
class PG_Button;
class PG_RadioButton;
class Army;

/** brief Dialog for level advancement
  * 
  * This dialog is displayed whenever a unit gains enough experience points
  * to advance a level. The user can choose which stat he wants to raise.
  */

class ArmyLevelDialog : public PG_Window
{
    public:
        /** \brief The constructor which sets up the whole thing
          * 
          * @param army     the army which advances
          * @param parent   the parent widget (derived from paragui)
          * @param rect     the rect of this widget (derived from paragui)
          */
        ArmyLevelDialog(Army* army, PG_Widget* parent, PG_Rect rect);
        
        ~ArmyLevelDialog();

        //! Initialises all the labels etc.
        void initStats();
        
        //CALLBACKS and event functions
        //! This function is called when the user selects a stat.
        bool stat_chosen(PG_RadioButton* btn, bool state);

        //! This function is called when the user pushes the OK button.
        bool b_ok_clicked(PG_Button* btn);

        Army::Stat getResult() const {return d_result;}

    private:
        bool eventKeyDown(const SDL_KeyboardEvent* key);

        Army* d_army;
        PG_Button* d_b_ok;
        PG_Button* d_b_pic;
        PG_Label* d_l_strength;
        PG_Label* d_l_ranged;
        PG_Label* d_l_defense;
        PG_Label* d_l_vitality;
        PG_Label* d_l_sight;
        PG_Label* d_l_hp;
        PG_Label* d_l_moves;

        Army::Stat d_result;
};

#endif
