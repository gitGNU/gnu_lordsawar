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

#ifndef D_BUY_PRODUCTION_H
#define D_BUY_PRODUCTION_H

#include <pgwindow.h>
#include <pglabel.h>
#include <pgbutton.h>

class City;
class Armyset;
class PG_Label;
class PG_Button;

/** Dialog for buying new production for a city
  * 
  * If you select "Buy Production" in a city dialog, this dialog here pops up.
  * It displays all armies in the current player's armyset. You may then click
  * on arbitrary armies to view the stats and buy a new one.
  */

class D_Buy_Production : public PG_Window
{ 
    public:
        /** Default constructor
          * 
          * @param city         the city you buy production for
          * @param parent       the parent widget
          * @param rect         the rectangle which the dialog occupies
          */
        D_Buy_Production(City* city, PG_Widget* parent, PG_Rect rect);
        ~D_Buy_Production();

        // CALLBACKS
        bool b_buyClicked(PG_Button* btn);
        bool b_cancelClicked(PG_Button* btn);
        bool b_productionClicked(PG_Button* btn);

        //! Returns the index of the bought army
        int getChosenArmy() const {return d_chosenArmy;}

    private:
        // DATA
        City* d_city;
        unsigned int d_armyset;
        int d_chosenArmy;
        int d_gold;
        
        // WIDGETS
        PG_Label* d_l_moves;
        PG_Label* d_l_upkeep;
        PG_Label* d_l_production;
        PG_Label* d_l_duration;
        PG_Label* d_l_strength;
        PG_Label* d_l_ranged;
        PG_Label* d_l_shots;
        PG_Label* d_l_defense;
        PG_Label* d_l_hp;
        PG_Label* d_l_productionCost;
        PG_Button* d_b_production[20];
        PG_Button* d_b_buy;
        PG_Button* d_b_cancel;
};

#endif // D_BUY_PRODUCTION_H
