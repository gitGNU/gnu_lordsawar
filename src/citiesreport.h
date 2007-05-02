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

#ifndef CITIES_REPORT_H
#define CITIES_REPORT_H

#include <sigc++/sigc++.h>
#include <pgwindow.h>
#include <pgbutton.h>
#include <pglabel.h>
#include "citylist.h"
#include "playerlist.h"

/** Informational dialog about city status
  * 
  * This dialog is to be found under the menu point reports/cities. It displays
  * information about 4 cities at once and has an OK, an up and a down button.
  *
  * It contains subwidgets (CityItem instances) which care for the actual
  * display, the class itself just cares for filling the city items with the
  * correct city data.
  */

//TODO: maybe using a scrollbar would make the dialog more interesting

class PG_Button;
class PG_Lable;
class CityItem;    //internal data item, further info in citiesreport.cpp,

class CitiesReport : public PG_Window
{
    public:
        //! paragui constructor; showall determines if enemy cities are also detailed
        CitiesReport(PG_Widget* parent, Rectangle rect, bool showall=false);
        ~CitiesReport();

        //! Callback when ok button was clicked
        bool b_okClicked(PG_Button* btn);

        //! Callback when down button was clicked (scroll forward)
        bool b_upClicked(PG_Button* btn);

        //! Callback when up button was clicked (scroll backward)
        bool b_downClicked(PG_Button* btn);

        //! Callback when one item was selected, mainly raises sselectingCity
        void citySelected(City* c);

        sigc::signal<void, Vector<int>> sselectingCity;

    private:
        //! Fills the data items beginning with the current index
        void fillCityItems();

        bool eventKeyDown(const SDL_KeyboardEvent* key);

        PG_Button* d_b_ok;
        PG_Button* d_b_up;
        PG_Button* d_b_down;
        PG_Label* d_l_number;
        CityItem* d_items[4];
        int d_index;
        bool d_showall;
        std::vector<City*> d_citylist;
};

#endif /* CITIES_REPORT_H */
