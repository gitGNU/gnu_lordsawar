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

#ifndef STACKINFO_H
#define STACKINFO_H

#include <vector> 
#include <pgthemewidget.h> 

class  ArmyInfo;

/** Display information about armies in a stack.
  *
  * The stackinfo can be seen at the bottom of the game screen. When the player
  * selects a stack, the stackinfo is filled with the stack's data. It simply
  * contains of 8 ArmyInfo objects and coordinates their data supply.
  */

class Stackinfo : public PG_ThemeWidget
{
    public:
        //! Contructor
        Stackinfo(PG_Widget* parent, PG_Rect rect);
        ~Stackinfo();

        /** Update display
          * 
          * This function gets the currently selected stack, fills the armyinfo
          * objects with the correct data and hides the ones which are not
          * needed.
          */
        void readData();
    
    private:
        std::vector<ArmyInfo*> d_armyinfovector;
};

#endif // STACKINFO_H
