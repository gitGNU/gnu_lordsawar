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

#ifndef CLIVING_H
#define CLIVING_H

#include "Condition.h"

/** This condition checks whether a certain player still lives.
  */

class CLiving : public Condition
{
    public:
        //! Initialises condition with player id
        CLiving(Uint32 player);

        //! Loading constructor
        CLiving(XML_Helper* helper);
        ~CLiving();

        //! Returns true if the player still lives
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

#endif //CLIVING_H
