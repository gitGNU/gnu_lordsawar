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

#ifndef RKILLPLAYER_H
#define RKILLPLAYER_H

#include "Reaction.h"

/** Reaction that kills a specified player instantly.
  * 
  * What can be said here... be careful :)
  *
  * _Never_ kill the neutral player, that may cause problems. For safety
  * reasons, this case is caught and ignored.
  */

class RKillPlayer : public Reaction
{
    public:
        //! Standard constructor with player id
        RKillPlayer(Uint32 player);

        //! Loading constructor
        RKillPlayer(XML_Helper* helper);
        ~RKillPlayer();

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

#endif //RKILLPLAYER_H
