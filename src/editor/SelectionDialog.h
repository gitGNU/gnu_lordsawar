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

#ifndef E_SELECTIONDIALOG_H
#define E_SELECTIONDIALOG_H

#include <list>
#include <string>
#include <pgwindow.h>
#include <pgbutton.h>


/** Dialog for choosing from multiple strings
  * 
  * A simple problem that occurs in the event and reaction dialogs is that,
  * when adding a reaction/event, the user has to be queried which type of
  * reaction/event is to be added. A dropdown, as used in other cases, is a
  * bit problematic here (much additional code). So we add a dialog that is
  * supplied with a list of strings , presents the user a choice and returns
  * the selection. Simple, but has to be implemented...
  *
  * The return value is the index of the item in the list or -1 if cancel
  * was pressed.
  *
  * @note The dialog only works properly if you calculate the size of the widget
  * accordingly. The height should be something like 60 + 25 x number_of_items.
  */

class E_SelectionDialog : public PG_Window
{
    public:
        /** Constructor
          * 
          * @param parent       parent widget (paragui internals)
          * @param rect         rectangle (paragui internals)
          * @param strings      list of strings for choosing
          */
        E_SelectionDialog(PG_Widget* parent, PG_Rect rect,
                          const std::list<std::string> strings);
        ~E_SelectionDialog();

        //! Callback when one of the buttons is pressed
        bool buttonClicked(PG_Button* btn);

        //! Returns the index of the selection
        int getSelection() {return d_index;}

    private:
        int d_index;
};

#endif //E_SELECTIONDIALOG_H
