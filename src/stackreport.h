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

#ifndef STACK_REPORT_H
#define STACK_REPORT_H

#include <sigc++/sigc++.h>
#include <pgwindow.h>
#include <pgbutton.h>
#include <pglabel.h>
#include "stacklist.h"
#include "playerlist.h"

class PG_Label;
class PG_Button;
class StackItem;    //further information in stackreport.cpp, internal data

/** Report details of the stacks on the map
  * 
  * This class implements a stack report, details of 4 stacks are shown on the
  * screen, the user has the possibility to scroll up or down. First come the
  * user's stacks, than the other player's, finally the neutral ones.
  *
  * The dialog has an internal appropriately sorted list of stacks and the index
  * of the upper stack. Clicking on the up or down button increases or decreases
  * the index and redraws the dialog.
  */

//TODO: why does bigmap overwrite the stackreport in LordsAWar, but not in the editor
// (to see what I mean, disable Update() in StackReport::stackSelected)
//
// TODO: maybe one could implement this report more elegant by using a scrollbar

class StackReport : public PG_Window
{
    public:
        StackReport(PG_Widget* parent, PG_Rect rect);
        ~StackReport();

        //! Callback for quitting the dialog
        bool b_okClicked(PG_Button* btn);

        //! Callback for moving up
        bool b_upClicked(PG_Button* btn);
        //! Callback for moving down
        bool b_downClicked(PG_Button* btn);

        //! Callback when a stackitem has been selected
        void stackSelected(Stack* s);
        
        // gives the position of the selected item
        SigC::Signal1<void, PG_Point> sselectingStack;

    private:
        //! Redraw the dialog showing the selected stacks
        void fillStackItems();

        bool eventKeyDown(const SDL_KeyboardEvent* key);

        
        PG_Button* d_b_ok;
        PG_Button* d_b_up;
        PG_Button* d_b_down;
        PG_Label* d_l_number;
        StackItem* d_items[4];
        int d_index;
        std::vector<Stack*> d_stacklist;
};

#endif /* ARMIES_REPORT_H */
