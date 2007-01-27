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

#ifndef RTRANSFERCITY_H
#define RTRANSFERCITY_H

#include "Reaction.h"

/** Changes the ownership of a city
  * 
  * Use this reaction especially if you want to revive a player. Players without
  * city are usually killed very soon (maybe persistent players will be
  * introduced lateron, but that doesn't change the situation). So if you revive
  * a player, don't forget to give him a city using this function. For fairness,
  * be sure that using this reaction doesn't screw up the gameplay.
  */

class RTransferCity : public Reaction
{
    public:
        /** Standard constructor
          * 
          * @param city     the id of the city to be transferred
          * @param player   the id of the lucky player
          */
        RTransferCity(Uint32 city, Uint32 player);

        //! Loading constructor
        RTransferCity(XML_Helper* helper);
        ~RTransferCity();

        //! Saves the reaction data
        bool save(XML_Helper* helper) const;

        //! Triggers the reaction
        bool trigger() const;


        //! Returns the city id
        Uint32 getCity() const {return d_city;}

        //! Sets the city id
        void setCity(Uint32 city) {d_city = city;}

        //! Returns the player's id that gets the city
        Uint32 getPlayer() const {return d_player;}

        //! Sets that player's id
        void setPlayer(Uint32 player) {d_player = player;}

    private:
        Uint32 d_city;
        Uint32 d_player;
};

#endif //RTRANSFERCITY_H
