// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
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

#ifndef NEXT_TURN_NETWORKED_H
#define NEXT_TURN_NETWORKED_H

#include "NextTurn.h"

/**
   \brief The class to pass turns around the players during a networked game.
 
 */

class NextTurnNetworked: public NextTurn
{
    public:
        /**
           \brief constructor

           @param   turnmode    setting for the private variable d_turnmode
           @param   random_turns change the order every round

         */
        NextTurnNetworked(bool turnmode, bool random_turns);

        /**
           \brief destructor
         */
        ~NextTurnNetworked();
        
        /**
           \brief start a new game
          
           This function starts with the currently active player, or the first
           if there is none active. For starting a game. This should be the
           lowest of all scenario-related functions in the stack.
         */
        void start();

        /**
           \brief go on to the next player
           
           This function starts the next to the active player's turn. Used when
           a human player has pushed the next_turn button.
         */
        void endTurn();

	/**
           \brief signal for telling when the game stops due to player absence.
	 */
	sigc::signal<void, Player*> snextPlayerUnavailable;

	//! clients get this called at the start of a round.
	void start_round(int round);
    private:
        /**
           \brief The function for all actions which are taken at the beginning
           of a player's turn
          */
        void startTurn();
        
        /**
           \brief The function for all actions which are taken at the end of a
           player's turn
         */
        void finishTurn();

        /**
           \brief The function for all actions which are taken at the end of a
           Game Turn.
         */
        void finishRound();

};

#endif //NEXT_TURN_NETWORKED_H
