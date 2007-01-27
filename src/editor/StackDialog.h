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

#ifndef E_STACKDIALOG_H
#define E_STACKDIALOG_H

#include <pgwindow.h>
#include <pgbutton.h>
#include <pgdropdown.h>

#include "../stack.h"

/** Dialog to modify stacks
  *
  * Using this dialog, one can modify a stack's armies and their properties.
  * You can add/remove armies and change their strength, hitpoints and such.
  *
  * The dialog should have a size of at least 400x350 or so.
  */

// TODO: layout is ugly, as usual

// NOTE: There may be an error if an army is removed. Under some circumstances,
// the image which an army button uses for displaying is removed and the
// button is hidden. This can be removed if paragui accepts PG_Button::SetIcon
// without parameters (i.e. all set to 0).

class E_StackDialog : public PG_Window
{
    public:
        /** Constructor
          * 
          * @param parent       paragui inheritance
          * @param rect         the same
          * @param s            the stack we deal with
          * @param player       whether we may change the stack's owner or not
          */
        E_StackDialog(PG_Widget* parent, PG_Rect rect, Stack* s,
                        bool player = true);
        ~E_StackDialog();

        //! Callback when clicking on the OK button
        bool okClicked(PG_Button* btn);

        //! Callback to add an army to the stack
        bool armyAdded(PG_Button* btn);

        //! Callback to remove an army from the stack
        bool armyRemoved(PG_Button* btn);

        //! Callback when selecting another army
        bool armySelected(PG_Button* btn);
        
        //! Callback to change the various stats
        bool statChanged(PG_Button* btn);

        //! Callback to change the player
        bool playerChanged(PG_ListBoxBaseItem* item);

    private:
        //! enumeration of all stats that can be changed
        enum STAT {STRENGTH, RANGED, SHOTS, DEFENSE, HP, VITALITY,
                   SIGHT, MOVES};
        
        //! updates the army pictures
        void updatePics();

        //! updates the values
        void updateStats();
        
        //! enables/disables some buttons
        void checkButtons();

        
        // data
        Stack* d_stack;
         
        PG_Button* d_b_army[8];
        unsigned int d_selected;

        std::list<SDL_Surface*> d_cache;

        PG_Button* d_b_add;
        PG_Button* d_b_remove;
        PG_Button* d_b_ok;
        
        PG_Label* d_l_name;
        PG_Label* d_l_strength;
        PG_Label* d_l_ranged;
        PG_Label* d_l_shots;
        PG_Label* d_l_defense;
        PG_Label* d_l_hp;
        PG_Label* d_l_vitality;
        PG_Label* d_l_sight;
        PG_Label* d_l_moves;
        PG_Label* d_l_id;
};


#endif //E_STACKDIALOG_H
