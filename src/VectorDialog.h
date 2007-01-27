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

#ifndef VectorDialog_H
#define VectorDialog_H

#include <pgwindow.h>
#include <pgcheckbutton.h>
#include <pglabel.h>
#include <pglistbox.h>
#include <pgbutton.h>
#include <pgfilearchive.h>
#include <pgapplication.h>
#include "city.h"

class VectorMap;

/* Dialog for game options
 *
 */

class VectorDialog : public PG_Window
{
    public:
        // CREATORS
        VectorDialog(City* city,PG_Widget* parent, PG_Rect rect);
        ~VectorDialog();


        PG_Point getVectorPos() {return d_vpos;}
        // CALLBACK FUNCTIONS
        bool okClicked(PG_Button* btn);

        //! callback when the user moves the mouse over the vectormap 
        void movingMouse(PG_Point pos);
        //! callback when the user clicks the mouse over the vectormap
        void clickedMouse(PG_Point pos);
        //! Callback when another city is selected from the listbox
        bool citySelected(PG_ListBoxBaseItem* item);

    private:
        // WIDGETS
        PG_Button* d_b_ok;
        PG_Label* d_l_tilepos;
        PG_Label* d_l_vectpos;
        PG_ListBox* d_cities;
        PG_Point d_vpos;

        City* d_city;
        
        VectorMap* d_vectormap;
};

#endif

// End of file
