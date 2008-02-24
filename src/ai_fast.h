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

#ifndef AI_FAST_H
#define AI_FAST_H

#include <string>
#include <list>

#include "real_player.h"
#include "AI_Analysis.h"
#include "AI_Diplomacy.h"

class XML_Helper;


//! A simple artificial intelligence Player.
/** 
 * This AI has two modes. In normal modes it basically assembles stacks of
 * 8 units each and sends them to the next city, reinforcing them in own cities
 * if neccessary. In maniac mode, however (meant for wandering monsters etc.),
 * this AI will attack everything that is close up or take the nearest city if
 * no enemies are close. When it takes over an enemy city, it razes it.
 * 
 */

class AI_Fast : public RealPlayer
{
    public:
        /** 
	 * Make a new AI_Fast player.
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
        AI_Fast(std::string name, Uint32 armyset, SDL_Color color, 
		int width, int height, int player_no = -1);

        //! Copy constructor.
        AI_Fast(const Player&);

        //! Loading constructor. See XML_Helper for an explanation.
        AI_Fast(XML_Helper* helper);

	//! Destructor.
        ~AI_Fast();
        
        //! Saves data, the method is for saving additional data.
        bool save(XML_Helper* helper) const;

        //! Sets whether the ai joins close armies to make them stronger
        void setJoin(bool join) {d_join = join;}

        //! Returns the current behaviour regarding joining armies
        bool getJoin() const {return d_join;}

        //! Set maniac/normal mode
        void setManiac(bool maniac) {d_maniac = maniac;}

        //! Returns the current behaviour
        bool getManiac() const {return d_maniac;}

        
        virtual bool startTurn();
        virtual bool invadeCity(City* c);
        virtual bool recruitHero(Hero* hero, City *city, int cost);
        virtual bool levelArmy(Army* a);
	virtual bool treachery (Stack *stack, Player *player, Vector <int> pos, 
				DiplomaticState state);

    private:
        //! The actual core function of the ai's logic.
        void computerTurn(); 

	//! Determines whether to join units or move them separately.
        bool d_join;

	//! Maniac mode: kill and raze everything you encounter.
        bool d_maniac;

        AI_Analysis* d_analysis;
        AI_Diplomacy* d_diplomacy;
};

#endif // AI_FAST_H
