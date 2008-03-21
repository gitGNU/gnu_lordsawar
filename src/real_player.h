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

#ifndef REAL_PLAYER_H
#define REAL_PLAYER_H

#include <string>
#include <list>
#include <SDL_types.h>

#include "player.h"

class MoveResult;
class XML_Helper;

//! A local human Player.
/** 
 * This class implements the abstract Player class in a reasonable manner
 * for local players. It is suitable for local human players, and AI players
 * can derive from this class and overwrite the start_turn and other 
 * callback methods for their own purposes.  For complete descriptions of
 * the callback functions see the Player class.
 */

class RealPlayer : public Player
{
    public:
        // CREATORS
        RealPlayer(std::string name, Uint32 armyset, SDL_Color color, 
		   int width, int height, Player::Type type = Player::HUMAN, 
		   int player_no = -1);
        RealPlayer(const Player&);
        RealPlayer(XML_Helper* helper);
        ~RealPlayer();

        virtual bool save(XML_Helper* helper) const;

        virtual bool startTurn();
        virtual void endTurn();
        virtual void invadeCity(City* c);
        virtual void levelArmy(Army* a);

    protected:
        void maybeRecruitHero();
};

#endif // REAL_PLAYER_H

// End of file
