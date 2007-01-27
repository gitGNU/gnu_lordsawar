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

#ifndef E_EVENTDIALOG_H
#define E_EVENTDIALOG_H

#include <pgdropdown.h>

#include "ReactionDialog.h"
#include "../GameScenario.h"

/** Dialog to manage events
  * 
  * To deal with the event system, this dialog is used. In detail, it consists
  * of:
  * 
  * - a list of existing events
  * - buttons to add/remove events and quit the dialog
  * - buttons to manage conditions and reactions
  * - a label to show the id; a label+edit to edit the event data
  * - a multilineedit to comment events
  * 
  * Since order does not matter here, new events are appended at the end of the
  * event list.
  *
  * some more notes:
  * - the event list items have the event pointers as user data
  * - size should be 500x400
  */

// TODO: PG_MultiLineEdit seems to segfault, I suppose this is caused by unicode
// usage somehow. Fix it!

class E_EventDialog : public PG_Window
{
    public:
        E_EventDialog(PG_Widget* parent, PG_Rect rect, GameScenario* sc);
        ~E_EventDialog();

        //! Callback when item is selected
        bool eventSelected(PG_ListBoxBaseItem* item);

        //! Callback when user decides if event is active/deactivated
        bool activationChanged(PG_ListBoxBaseItem* item);
        
        //! Callback when value is changed
        bool valueChanged(PG_LineEdit* edit);

        //! Callback when comment is changed
        bool commentChanged(PG_LineEdit* edit);

        //! Callback when reactions are dealt with
        bool reactionsClicked(PG_Button* btn);

        //! Callback when conditions are dealt with
        bool conditionsClicked(PG_Button* btn);

        //! Callback when event is removed
        bool removeClicked(PG_Button* btn);

        //! Callback when event is added
        bool addClicked(PG_Button* btn);

        //! Callback when OK is clicked
        bool okClicked(PG_Button* btn);

    private:
        //! Refills the listbox
        void fillEvents();

        //! Fills in the data, checks buttons etc.
        void fillData();
        
        
        GameScenario* d_scenario;
        Event* d_active;

        PG_ListBox* d_events;
        PG_Label* d_l_id;
        PG_DropDown* d_activate;
        PG_Label* d_l_val1, *d_l_val2, *d_l_comment;
        PG_LineEdit* d_val1, *d_val2;
//        PG_MultiLineEdit* d_comment;
        PG_LineEdit* d_comment;
        PG_Button* d_b_reaction;
        PG_Button* d_b_condition;
        PG_Button* d_b_remove;
};

#endif //E_EVENTDIALOG_H
