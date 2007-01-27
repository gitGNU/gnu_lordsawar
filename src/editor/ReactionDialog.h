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

#ifndef E_REACTIONDIALOG_H
#define E_REACTIONDIALOG_H

#include <pgmultilineedit.h>

#include "ConditionDialog.h"

/** Dialog to modify reactions etc.
  * 
  * This dialog is to add reactions to an event and modify them. The ugly thing
  * here is that many different reactions have many different properties that
  * you can change. So don't be surprised if there are huge switch statements in
  * the code. :)
  *
  * For changing the reaction, we need:
  * - three edit fields with labels (maximum of two things to edit)
  * - an edit for a message (for RMessage)
  * - a button to change the stack (for RAddStack)
  * - a button to deal with conditions
  * - buttons for adding, removing reactions and for exiting
  *
  * For simplicity, it is not allowed to change a reaction type. Instead, one
  * can remove the reaction and add a new one at any place by selecting the item
  * before which the new one is to be inserted.
  *
  * Some error checking is done in the fillData, e.g. if you enter ids and such,
  * you are not able to enter negative values etc.
  *
  * The size of this dialog is the same as the event dialog, i.e. 500x400
  */

// FIXME: Again, the PG_MultiLineEdit class makes problems. Fix them! (to be
// done within paragui)

class E_ReactionDialog : public PG_Window
{
    public:
        E_ReactionDialog(PG_Widget* parent, PG_Rect rect, Event* ev);
        ~E_ReactionDialog();

        //! Callback when different reaction was selected
        bool itemSelected(PG_ListBoxBaseItem* item);

        //! Callback when edit field was edited
        bool valueChanged(PG_LineEdit* edit);

        //! Callback when stack is to be modified
        bool stackClicked(PG_Button* btn);

        //! Callback when conditions are to be managed
        bool conditionsClicked(PG_Button* btn);

        //! Callback when reaction is added
        bool addClicked(PG_Button* btn);

        //! Callback when reaction is removed
        bool removeClicked(PG_Button* btn);

        //! Callback when "OK" is pressed
        bool okClicked(PG_Button* btn);

    private:
        //! refills the list of action items
        void fillReactions();

        //! updates the stuff to change the reaction data, enables/disables data
        void fillData();
        
        Event* d_event;
        int d_index;
        PG_ListBox* d_reactions;
        PG_Button* d_b_remove;
        PG_Button* d_b_stack;
        PG_Button* d_b_condition;
        PG_Label* d_l_val1, *d_l_val2, *d_l_val3;
        PG_LineEdit* d_val1, *d_val2, *d_val3;
//        PG_MultiLineEdit* d_text;
PG_LineEdit* d_text;
};

#endif //E_REACTIONDIALOG_H
