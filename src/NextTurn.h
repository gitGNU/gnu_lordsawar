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

#ifndef NEXT_TURN_H
#define NEXT_TURN_H

#include <sigc++/object_slot.h>
#include "playerlist.h"

/**
   \brief The class to pass turns around the players
 
   \note    This class takes care of calling the correct players in the correct
            order. The problem is, no other class really has the scope of doing
            this whole stuff. The playerlist and the W_Edit object aren't the
            right candidates (they are busy with other things) and the former
            algorithm of each player calling the next one produces a huge
            stackload if two ai players fight each other. Plus, you want to do
            several actions at the end or the beginning of each player's turn
            or each round and therefore want a central place for this code.
 */

class NextTurn: public SigC::Object
{
    public:
        /**
           \brief constructor

           @param   turnmode    setting for the private variable d_turnmode
         */
        NextTurn(bool turnmode);

        /**
           \brief destructor
         */
        ~NextTurn();
        
        /**
           \brief start a new game
          
           This function starts with the currently active player, or the first
           if there is none active. For starting a game. This should be the
           lowest of all scenario-related functions in the stack.
         */
        void start();

        //! Interrupts the game on the next possible occasion
        void stop() {d_stop = true;}
        
        /**
           \brief go on to the next player
           
           This function starts the next to the active player's turn. Used when
           a human player has pushed the next_turn button.
         */
        void endTurn();

        /**
           \brief signals for announcing events

           If the splayerStart signal returns false, the main loop will quit,
           which is useful if a human player's turn starts (the control is then 
           handed over to teh SDL/ParaGUI message queue). Don't use this signal
           to hook up your own events or so, use the next, snextTurn.
         */
        SigC::Signal1<bool, Player*> splayerStart;

        /** This signal is a bit problematic and needs some more comments. It is
            emitted whenever a new player's turn starts. However, it is sometimes
            (in case of an AI round) started by the NextTurn class. Since it was
            introduced for events and propable reactions will be showing a message
            and such, and since in case a human player starts his turn, W_Edit will
            set a capture and therefore break any possible dialog boxes, this signal
            is also emitted in W_Edit after the introductory message (Player xxx's
            turn) has been shown.
          */
        SigC::Signal1<void, Player*> snextTurn;
        
        //! Signal which is emitted whenever a new round starts
        SigC::Signal0<void> snextRound;

        //! Signal as a workaround for a display bug; updates the screen
        SigC::Signal1<bool,bool> supdating;

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

        /** \brief determines whether armies are healed/produced at the
          * beginning of a round or at the beginning of each player's turn.
          *
          * If the value is set to true, the production/healing of armies takes
          * place at the beginning of each player's turn (which is fairer); else
          * all armies of all playes are healed when a new game round starts.
          * The latter setting is a bit unfair, because the last player knows
          * that his armies are healed immediately when he has finished his turn
          * while the other player's armies may have to survive some attacks,
          * but it may be useful in some circumstances.
          */
        bool d_turnmode;

        //! If set to true, the game is interrupted at the next occasion
        bool d_stop;
};

#endif //NEXT_TURN_H
