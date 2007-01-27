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

#ifndef ARMYINFO_H
#define ARMYINFO_H

#include <pgthemewidget.h>

class Army;
class PG_Label;

/** \brief ArmyInfo is a togglebutton with a drawing of an army,
  * hitpoint bar and moves.
  *
  * This class is the main part behind the stackinfo which is displayed
  * at the bottom of the game screen. It is a small widget which
  * displays the picture of a given army, its hitpoints/left hitpoints
  * and the number of moves left.
  */

class ArmyInfo : public PG_ThemeWidget
{
	public:
        /** \brief The constructor
          * 
          * @param  army    the army which the ArmyInfo deals with
          * @param  pressable   if the army button may be deselected
          *                     (false if there is only one army in the stack)
          * @param  parent  the parent widget
          * @param  rect    the rect which we may fill out (parent's coordinates)
          */
	ArmyInfo(Army* army, bool pressable, PG_Widget* parent, PG_Rect rect);

        /** \brief The destructor, nothing special here */
	~ArmyInfo();

        /** \brief make the ArmyInfo pressable or not
          * 
          * If the ArmyInfo is made pressable, clicking on it marks the
          * unit as selected or not selected for further movement. This
          * may be prohibited if a stack has only one army left.
          *
          * @param  pressable   if the ArmyInfo is rendered pressable
          */
	static void setPressable(ArmyInfo* armyinfo, bool pressable = true)
        {armyinfo->d_pressable = pressable;}

	private:
		// EVENT HANDLER
        /** \brief The event handler for the draw event
          * 
          * This handler draws the army pixmap and two colored bars. The
          * green one indicates the relative amount of hitpoints left,
          * the red one fills the rest of the way.
          */
		void eventDraw(SDL_Surface* surface, const PG_Rect& rect);

        /** \brief The event handler for the mouse button event
          * 
          * This handler looks if the ArmyInfo can be toggled. If so,
          * it switches between selected/deselected and stores this
          * information in Army::d_grouped.
          */
	bool eventMouseButtonDown(const SDL_MouseButtonEvent* event);

	// DATA
	Army* d_army;
	bool d_pressed;
	bool d_pressable;
	
	// WIDGETS
	PG_Label* d_l_moves;
};

#endif // ARMYINFO_H
