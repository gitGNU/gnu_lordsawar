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

#ifndef E_ARMYDIALOG_H
#define E_ARMYDIALOG_H

#include <pgwindow.h>
#include <pgbutton.h>
#include <pglistboxitem.h>
#include <string>
#include <vector>

#include "../army.h"

/** This dialog lets you select an army. You can specify one or different
  * armysets to select the army from. It is used in multiple ways, from 
  * selecting the production of a city to choosing the content of a stack.
  */

class E_ArmyDialog : public PG_Window
{
    public:
        /** Constructor, mainly stuff inherited from PG_Window
          * 
          * @param parent   the parent widget (paragui internal)
          * @param rect     the location of the widget (paragui stuff)
          * @param armyset  if set to 0, add a combobox to select the armyset,
          *                 else make only armies of this set available
          */
        E_ArmyDialog(PG_Widget* parent, PG_Rect rect, Uint32 armyset = 0);
        ~E_ArmyDialog();

        //! returns whether the action was cancelled
        bool getSuccess() {return d_success;}
        
        //! returns the army type we have selected
        Army* getSelection();
        
        //! sometimes more direct: get the index of the armyset
        int getIndex() {return d_index;}
        
        //! Callback when an item of the combobox is selected
        bool armysetSelected(PG_ListBoxBaseItem* item);

        //! Callback when an army is selected
        bool armySelected(PG_Button* btn);
        
        //! Callback when the ok button is pushed
        bool okClicked(PG_Button* btn);

        //! Callback when cancel button is pushed
        bool cancelClicked(PG_Button* btn);

    private:
        //! sets up the graphics with some armyset
        void fillData(Uint32 armyset);

        //! update the stats when a specific army type is selected
        void updateStats(int index);

        // Data, most of it is supplied to the callback functions anyway and
        // therefore not listed here.
        std::vector<PG_Button*> d_armies;

        PG_Label* d_l_name;
        PG_Label* d_l_production;
        PG_Label* d_l_cost;
        PG_Label* d_l_upkeep;
        PG_Label* d_l_strength;
        PG_Label* d_l_ranged;
        PG_Label* d_l_shots;
        PG_Label* d_l_defense;
        PG_Label* d_l_hp;
        PG_Label* d_l_moves;
        PG_Label* d_l_vitality;
        PG_Label* d_l_sight;
        
        int d_index;
        Uint32 d_set;
        bool d_success;
        
};

#endif //E_ARMYDIALOG_H
