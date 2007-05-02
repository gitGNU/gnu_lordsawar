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

#ifndef MAPCONFDIALOG_H
#define MAPCONFDIALOG_H

#include <string>
#include <sigc++/sigc++.h>
#include <pgwindow.h>
#include "GameScenario.h"

class PG_Slider;
class PG_ScrollBar;
class PG_Button;
class PG_Label;

/** This class is simply a PG_Slider with a label. You find them in the
  * GamePreferences dialog where you can adjust the amount of water, grass...
  */

class TerrainConfig : public sigc::Object
{
    public:
        /** The constructor
          * 
          * @param parent       the parent widget (inherited from PG_Widget)
          * @param rect         the rectangle for this widget (inherited)
          * @param name         the label for the slider
          * @param min          the minimum value
          * @param max          the maximum value
          * @param pos          the initial position of the slider
          */
        TerrainConfig(PG_Widget* parent, Rectangle rect, char* name, int min, int max, int pos);
        ~TerrainConfig();

        // MANIPULATORS
        void hide();
        void show();

        void setValue(int value);

        // ACCESSORS
        int getValue() {return d_value;}
        
        // CALLBACKS
        bool slide(PG_ScrollBar* slider, long pos);

    private:
        PG_Label* d_l_name;
        PG_Label* d_l_value;
        PG_Slider* d_slider;
        PG_Button* d_button;
        int d_value;
};

#endif

// End of file
