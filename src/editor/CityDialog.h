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

#ifndef E_CITYDIALOG_H
#define E_CITYDIALOG_H

#include <pglineedit.h>
#include <pgcheckbutton.h>

#include "ArmyDialog.h"
#include "../city.h"

/** This dialog is to pop up when a city is selected. With this, dialog, one
  * should be able to modify all the important city data, namely the name,
  * gold, owner, level, and production
  *
  * @note The cache is used because a user could excessively use the army
  * dialog, thus throwing out the necessary images from the GraphicsCache.
  * This is unlikely (the cache size is usually too large), but possible.
  */

// TODO:
// - some better layout

class E_CityDialog : public PG_Window
{
    public:
        //! Constructor; pass the city to be modified as parameter
        E_CityDialog(PG_Widget* parent, PG_Rect rect, City* c);
        ~E_CityDialog();

        
        //! Callback when name has been changed
        bool nameChanged(PG_LineEdit* edit);

        //! Callback when gold has been changed
        bool goldChanged(PG_LineEdit* edit);

        //! Callback when level of the city has been changed
        bool levelChanged(PG_Button* btn);

        //! Callback when the player has been changed
        bool playerChanged(PG_ListBoxBaseItem* item);

        //! Callback to set the ruined status of a city
        bool statusChanged(PG_ListBoxBaseItem* item);
        
        //! Callback to set the capitalness of a city
        bool capitalToggled();
        
        //! Callback when setting a production
        bool productionSet(PG_Button* btn);

        //! Callback for buying a basic production
        bool buyBasic(PG_Button* btn);

        //! Callback for buying an advanced production
        bool buyAdvanced(PG_Button* btn);

        //! Callback when clearing a production
        bool removeClicked(PG_Button* btn);
        
        //! Callback when OK button is clicked
        bool okClicked(PG_Button* btn);

    private:
        //! enables/disables some buttons, places pictures etc.
        void checkButtons();

        //! updates the labels showing stats about the current production
        void updateStats();

        //! updates the images for the production buttons
        void updatePics();
   
        
        // Data
        City* d_city;

        std::vector<PG_Button*> d_basic;
        std::vector<PG_Button*> d_advanced;
        
        // we don't display all the army data here, just some stats
        PG_Label* d_l_name;
        PG_Label* d_l_strength;
        PG_Label* d_l_ranged;
        PG_Label* d_l_defense;
        PG_Label* d_l_production;
        PG_Label* d_l_upkeep;

        PG_Label* d_l_level;

        PG_Button* d_b_up;
        PG_Button* d_b_down;
        PG_CheckButton* d_cb_capital;

        // the cache is solely filled by updatePics, so look there
        std::list<SDL_Surface*> d_cache;
        SDL_Surface* d_blanksurf;
};

#endif //E_CITYDIALOG_H
