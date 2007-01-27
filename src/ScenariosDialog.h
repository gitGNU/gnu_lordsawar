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

#ifndef ScenariosDialog_H
#define ScenariosDialog_H

#include <pgwindow.h>
#include <pgcheckbutton.h>
#include <pglabel.h>
#include <pgmultilineedit.h>
#include <pglistbox.h>
#include <pgbutton.h>
#include <pgapplication.h>

/* Dialog for game options
 *
 */

class ScenariosDialog : public PG_Window
{
    public:
        // CREATORS
        ScenariosDialog(PG_Widget* parent, PG_Rect rect,std::string * fname);
        ~ScenariosDialog();

        // CALLBACK FUNCTIONS
        bool okClicked(PG_Button* btn);
        bool cancelClicked(PG_Button* btn);

        //! Callback when another map is selected from the listbox
        bool mapSelected(PG_ListBoxBaseItem* item);

        //! Callback to get data about scenario when another map is selected from the listbox
        bool scan(std::string tag, XML_Helper* helper);

    private:
        // WIDGETS
        PG_Button* d_b_ok;
	PG_Button* d_b_canc;
        PG_ListBox* d_maps;
        PG_Label* l_name;
        PG_Label* l_desc;
        PG_Label* l_scname;
        PG_MultiLineEdit* l_scdesc;
        SDL_Surface* d_background;
	PG_Label *d_back;
	std::string *filename;
	std::string d_name;
	std::string d_comment;
};

#endif

// End of file
