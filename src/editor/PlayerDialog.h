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

#ifndef E_PLAYERDIALOG_H
#define E_PLAYERDIALOG_H

#include <pgwindow.h>
#include <pgbutton.h>
#include <pglistboxbaseitem.h>
#include <pgdropdown.h>
#include <pglineedit.h>
#include <pgcheckbutton.h>

class Player;

/** Dialog to modify player data
  * 
  * This dialog should be finally used to change player's name, gold, color, type
  * and armyset as well as to add and remove players.
  *
  * @note: as multiple players can have the same name (temporary situation or
  * by malicious user), we pass the player pointer as client data to the
  * player selection box.
  */


// TODO: 
// - it is a bit ugly to have the strings for the player types written manually.
//   Find a better solution when this dialog is reworked.

class E_PlayerDialog :public PG_Window
{
    public:
        E_PlayerDialog(PG_Widget* parent, PG_Rect rect);
        ~E_PlayerDialog();

        //! Callback when OK is pressed
        bool okClicked(PG_Button* btn);

        //! Callback when player is to be removed
        bool playerRemoved(PG_Button* btn);

        //! Callback when player is added
        bool playerAdded(PG_Button* btn);

        //!Callback when next/prev. player is selected. Set btn to 0 to take the 1st.
        bool playerSelected(PG_Button* btn);

        //! Callback when type (human etc.) is changed
        bool typeChanged(PG_ListBoxBaseItem* item);

        //! Callback when armyset is changed
        bool armysetChanged(PG_ListBoxBaseItem* item);

        //! Callback when one of the values is changed: color, name, gold
        bool valueChanged(PG_LineEdit* edit);

        //! Callback for special selections (neutral player, maniac mode)
        bool miscSelected(PG_RadioButton* btn, bool click);

    private:
        //! Enables/disables some buttons
        void checkButtons();
        
        // Data
        PG_DropDown* d_types;
        PG_DropDown* d_armysets;
        PG_CheckButton* d_neutral;
        PG_CheckButton* d_maniac;
        PG_CheckButton* d_join;
        PG_CheckButton* d_persistent;
        PG_Label* d_l_id;
        PG_Label* d_l_count;
        PG_LineEdit* d_l_name;
        PG_LineEdit* d_l_gold;
        PG_LineEdit* d_l_red, *d_l_green, *d_l_blue;

        PG_Button* d_b_remove;
        PG_Button* d_b_add;
        PG_Button* d_next;
        PG_Button* d_prev;

        Player* d_player;
};


#endif // E_PLAYERDIALOG_H
