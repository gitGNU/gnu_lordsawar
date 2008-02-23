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

#ifndef AI_DUMMY_H
#define AI_DUMMY_H

#include <string>
#include <list>
#include <SDL_types.h>

#include "real_player.h"

class XML_Helper;

/** 
 * Dummy AI player.
 * 
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
        AI_Dummy (std::string name, Uint32 armyset, SDL_Color color, 
		  int width, int height, int player_no = -1);

	//! Copy constructor.
        AI_Dummy(const Player& player);
        //! Loading constructor. See XML_Helper.
        AI_Dummy(XML_Helper* helper);
	//! Destructor.
        ~AI_Dummy();
        
        virtual bool startTurn();
        virtual bool invadeCity(City* c);
        virtual bool recruitHero(Hero* hero, City *city, int cost);
        virtual bool levelArmy(Army* a);
};

#endif // AI_DUMMY_H
