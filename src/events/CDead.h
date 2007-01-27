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

#ifndef CDEAD_H
#define CDEAD_H

#include "Condition.h"

/** This condition checks if a player has already died.
  */

class CDead : public Condition
{
    public:
        //! Initialises condition with player id
        CDead(Uint32 player);

        //! Loading constructor
        CDead(XML_Helper* helper);
        ~CDead();

        //! Returns true if the player has already died
        bool check();

        //! Saves game data
        bool save(XML_Helper* helper) const;

        //! Returns player id
        Uint32 getPlayer() const {return d_player;}

        //! Sets the player id
        void setPlayer(Uint32 player) {d_player = player;}

    private:
        Uint32 d_player;
};

#endif //CDEAD_H
