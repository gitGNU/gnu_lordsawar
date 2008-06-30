// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2004 Andrea Paternesi
// Copyright (C) 2007, 2008 Ben Asselstine
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
        virtual void invadeCity(City* c);
        virtual void levelArmy(Army* a);
	virtual bool treachery (Stack *stack, Player *player, Vector <int> pos, 
				DiplomaticState state);

    private:
        //! The actual core function of the ai's logic.
        bool computerTurn(); 

	//! buy a scout unit if we need one.
	void maybeBuyScout();

	//! is it safe to vector from the given city?
	bool safeFromAttack(City *c, Uint32 safe_mp, Uint32 min_defenders);

	//! search through our stacklist for a stack we can join
	Stack *findNearOwnStackToJoin(Stack *s, int max_distance);

	//! Go to a temple if we're near enough.
	/**
	 * Helper method to take a stack on a mission to get blessed.
	 * If the method returns false initially, it means that the nearest 
	 * temple is unsuitable.
	 * @note The idea is that this method is called over subsequent turns, 
	 * until the blessed parameter gets filled with a value of true.
	 *
	 * @param s            The stack to visit a temple.
	 * @param dist         The maximum number of tiles that a temple
	 *                     can be away from the stack, and be considered
	 *                     for visiting.
	 * @param mp           The maximum number of movement points that a
	 *                     stack needs to have to reach the temple.
	 * @param percent_can_be_blessed  If the stack has this many army 
	 *                                units that have not been blessed
	 *                                at the temple (expressed as a
	 *                                percent), then the temple will be
	 *                                considered for visiting.
	 * @param blessed      Gets filled with false if the stack didn't get 
	 *                     blessed.  Gets filled with true if the stack 
	 *                     got blessed at the temple.
	 * @param stack_died   Gets filled with true if the stack got killed
	 *                     by an enemy stack on the same square as the
	 *                     temple.
	 *
	 * Returns true if the stack moved, false if it stayed still.
	 */
	bool maybeVisitTempleForBlessing(Stack *s, int dist, int mp, 
					 double percent_can_be_blessed, 
					 bool &blessed, bool &stack_died);
	bool maybePickUpItems (Stack *s, int dist, int mp, bool &picked_up,
			       bool &stack_died);

	bool maybeVector(City *c, Uint32 safe_mp, Uint32 min_defenders,
			 City *target, City **vector_city = NULL);


	void setupVectoring();

	bool maybeDisband(Stack *s, City *city, Uint32 min_defenders, 
			  int safe_mp, bool &stack_died);

	//! Determines whether to join units or move them separately.
        bool d_join;

	//! Maniac mode: kill and raze everything you encounter.
        bool d_maniac;

        AI_Analysis* d_analysis;
        AI_Diplomacy* d_diplomacy;
};

#endif // AI_FAST_H
