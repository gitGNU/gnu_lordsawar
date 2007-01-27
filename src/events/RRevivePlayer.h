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

#ifndef RREVIVEPLAYER_H
#define RREVIVEPLAYER_H

#include "Reaction.h"

/** Brings a player back to the living.
  * 
  * This reaction revives a player. Be careful! If the player doesn't own any
  * cities after having been revived and if he is not persistent (to be
  * implemented), the player may die again next round or (other way) not die at
  * all.
  */

class RRevivePlayer : public Reaction
{
    public:
        //! Standard constructor with player id
        RRevivePlayer(Uint32 player);

        //! Loading constructor
        RRevivePlayer(XML_Helper* helper);
        ~RRevivePlayer();

        //! Saves the reaction data
        bool save(XML_Helper* helper) const;

        //! Triggers the reaction
        bool trigger() const;


        //! Returns the player id
        Uint32 getPlayer() const {return d_player;}

        //! Sets the player id
        void setPlayer(Uint32 player) {d_player = player;}

    private:
        Uint32 d_player;
};

#endif //RREVIVEPLAYER_H
