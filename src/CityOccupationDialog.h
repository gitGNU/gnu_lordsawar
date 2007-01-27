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

#ifndef CITYOCCUPATIONDIALOG_H
#define CITYOCCUPATIONDIALOG_H

#include <pgwindow.h>
#include <pgbutton.h>

class Stack;
class City;
class PG_Button;
class PG_Label;

/** \brief Dialog to determine the fate of a city
  * 
  * This dialog pops up whenever a human player conquers a city. Choices are
  * invading the city, razing it (city becomes a ruined city) and pillaging
  * it (the highest production is sold to gain some money)
  */

class CityOccupationDialog : public PG_Window
{ 
    public:
        /** \brief Constructor
          * 
          * @param city     the conquered city
          */
        CityOccupationDialog(City* city);
        ~CityOccupationDialog();

        // CALLBACKS
        //! This function is called if the user chooses to occupy the city.
	bool b_occupyClicked(PG_Button* btn);
        //! This function is called if the user chooses to pillage the city.
	bool b_pillageClicked(PG_Button* btn);
        //! This function is called if the user chooses to raze the city.
	bool b_razeClicked(PG_Button* btn);

    private:
        // WIDGETS
        PG_Button* d_b_occupy;
        PG_Button* d_b_pillage;
        PG_Button* d_b_raze;
        PG_Label* d_l_msg;
        SDL_Surface* d_background;

        // DATA
        City* d_city;
};

#endif

// End of file
