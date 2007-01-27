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

#ifndef RUINSEARCHDIALOG_H
#define RUINSEARCHDIALOG_H

#include <pgwindow.h>
#include <pgbutton.h>

#include "ruin.h"

class Stack;
class PG_Label;
class PG_Button;

/** This dialog pops up when you have searched a ruin. it consists of an
  * image, a text telling you what you have found (currently only gold) and
  * a button to close the dialog.
  */

class RuinSearchDialog : public PG_Window
{ 
    public:
        /** Create the dialog
          * 
          * @param parent       the parent widget
          * @param gold         how much gold you have found
          */
        RuinSearchDialog(PG_Widget* parent, int gold);
        ~RuinSearchDialog();

        // CALLBACKS
        bool b_closeClicked(PG_Button* btn);

    private:
        bool eventKeyDown(const SDL_KeyboardEvent* key);

        // WIDGETS
        SDL_Surface* d_background;
        PG_Button* d_b_close;
        PG_Label* d_l_msg1;
        PG_Label* d_l_msg2;
        PG_Label* d_l_pic;
};

#endif

// End of file
