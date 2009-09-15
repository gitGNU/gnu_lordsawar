// Copyright (C) 2002, 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2007, 2008, 2009 Ben Asselstine
// Copyright (C) 2007, 2008 Ole Laursen
//
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#ifndef AI_DUMMY_H
#define AI_DUMMY_H

#include <string>
#include <list>
#include <gtkmm.h>

#include "real_player.h"

class XML_Helper;

//! A simple artificial intelligence Player suitable to be the neutral Player.
/** 
 * This class is a dummy AI used for the neutral player. It just does, well,
 * nothing.
 */

class AI_Dummy : public RealPlayer
{
    public:
        /** 
	 * Make a new AI_Dummy player.
         * 
         * @param name         The name of the player.
         * @param armyset      The Id of the player's Armyset.
         * @param color        The player's colour.
	 * @param width        The width of the player's FogMap.
	 * @param height       The height of the player's FogMap.
	 * @param player_no    The Id of the player.  If this value is -1,
	 *                     the next free Id it used.
         */
	//! Default constructor.
        AI_Dummy (std::string name, guint32 armyset, Gdk::Color color, 
		  int width, int height, int player_no = -1);

	//! Copy constructor.
        AI_Dummy(const Player& player);
        //! Loading constructor. See XML_Helper.
        AI_Dummy(XML_Helper* helper);
	//! Destructor.
        ~AI_Dummy();
        
        virtual void abortTurn();
        virtual bool startTurn();
        virtual void invadeCity(City* c);
        virtual void levelArmy(Army* a);

	void setDefensiveProduction(City *city);
	void examineCities();

    private:
	//DATA
	bool d_abort_requested;
};

#endif // AI_DUMMY_H
