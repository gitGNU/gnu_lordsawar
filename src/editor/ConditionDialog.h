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

#ifndef E_CONDITIONDIALOG_H
#define E_CONDITIONDIALOG_H

#include <pgwindow.h>
#include <pglistbox.h>
#include <pgdropdown.h>
#include <pgbutton.h>
#include <pglineedit.h>

#include "../events/Event.h"
#include "../events/Reaction.h"
#include "../events/Condition.h"

/** Dialog to manage conditions
  * 
  * Using this dialog, you should be able to manage the conditions related to an
  * event or reaction. You can add, remove and edit the conditions. Most of the
  * code, however, deals with changing the content of the actual condition, as
  * there are so many of them...
  *
  * To each condition, there is just one piece of actual information (player id
  * and such), so it is still not as complicated as the reaction dialog.
  *
  * Some notes which I'll leave out in the code itself:
  * - the listbox has the index as userdata
  * - the type dropdown box has the type (as given by Condition::TYPE) as
  *   user data
  * - the typical size of this dialog  should be 400x300
  *
  * @note The dialog does not check if the entered data makes sense. If not, the
  * condition (or the event/reaction which it is associated with) dill simply
  * never be raised.
  */

class E_ConditionDialog : public PG_Window
{
    public:
        //! Initialize dialog with an event
        E_ConditionDialog(PG_Widget* parent, PG_Rect rect, Event* ev);

        //! Initializes the dialog with a reaction
        E_ConditionDialog(PG_Widget* parent, PG_Rect rect, Reaction* r);
        ~E_ConditionDialog();

        //! Callback when another item was selected
        bool itemSelected(PG_ListBoxBaseItem* item);

        //! Callback when the type of the condition has been changed
        bool typeChanged(PG_ListBoxBaseItem* item);
        
        //! Callback when the edit field was changed
        bool valueChanged(PG_LineEdit* edit);

        //! Callback when condition has been added
        bool addClicked(PG_Button* btn);

        //! Callback when condition has been removed
        bool removeClicked(PG_Button* btn);

        //! Callback when dialog should be quit
        bool okClicked(PG_Button* btn);
        
    private:
        //! Initializes the dialog
        void init();

        //! fills the data with the one of a new condition
        void fillData();

        //! Shortcut to refill the list of all conditions
        void refillList();

        //! checks if several of the elements of the dialog should be disabled
        void checkButtons();

        //! Shortcut to get the currently active condition or 0
        Condition* getCondition();

        // data
        Event* d_event;
        Reaction* d_reaction;
        int d_index;                // index of the currently selected condition
        PG_ListBox* d_condlist;
        PG_DropDown* d_type;
        PG_Label* d_l_value;
        PG_LineEdit* d_value;
        PG_Button* d_b_remove;
};


#endif //E_CONDITIONDIALOG_H
