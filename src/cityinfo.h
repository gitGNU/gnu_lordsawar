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

#ifndef CITYINFO_H
#define CITYINFO_H

#include <pgwindow.h>
#include <pgthemewidget.h>
#include "d_buy_production.h"

class City;
class PG_Label;
class PG_Button;

/** Dialog for user interaction with city parameters
  * 
  * This is the user interface for the city status. The user can change the
  * city production, upgrade the city or choose to buy new production.
  */

class CityInfo : public PG_Window
{ 
    public:
        //! Initialise the dialog with city as the object to modify
        CityInfo(City* city);
        ~CityInfo();

        // CALLBACKS
        bool b_upgradeClicked(PG_Button* btn);
        bool b_vectoringClicked(PG_Button* btn);
        bool b_buyBasicClicked(PG_Button* btn);
        bool b_productionClicked(PG_Button* btn);
        bool b_closeClicked(PG_Button* btn);

    private:
        //! Enables or disables some buttons
        void checkButtons();

        //! Updates the production slots an some labels
        void updateProductionStats();

        //! Returns the gold needed for upgrading
        int neededGold();

        //! Return the first empty production slot of the city or -1
        int getEmptySlot();

        
        // WIDGETS
        PG_Label* d_l_production;
        PG_Label* d_l_duration;
        PG_Label* d_l_strength;
        PG_Label* d_l_ranged;
        PG_Label* d_l_defense;
        PG_Label* d_l_moves;
        PG_Label* d_l_upkeep;
        PG_Label* d_l_basic;
        PG_Button* d_b_no_production;
        PG_Button* d_b_basic[4];
        PG_Button* d_b_upgrade;
        PG_Button* d_b_vectoring;
        PG_Button* d_b_buy_basic;
        PG_Button* d_b_close;

        // DATA
        City* d_city;
};


/** Simple information popup for a city not owned by the player
  * 
  * If you right-click on a city that you don't own in the game window, this 
  * dialog is shown, which just tells the properties of the city (name, 
  * defense level, income, and location). It captures all SDL events and 
  * finished as soon as the right mouse button is released.
  */

class CityInfoSmall : public PG_ThemeWidget
{ 
    public:
        /** Default constructor
          *
          * The dialog is displayed with the top right at the position of the
          * start of the right-click
          * 
          * @param city         the city to be displayed
          * @param screen_x     the x_coordinate of the original click
          * @param screen_y     the y coordinate of the original click
          */
        CityInfoSmall(City* city, int screen_x, int screen_y);
        ~CityInfoSmall();

    private:
        //! Closes dialog when the right mouse button is released
        bool eventMouseButtonUp(const SDL_MouseButtonEvent* event);

        // DATA
        City* d_city;
};
#endif // CITYINFO_H

