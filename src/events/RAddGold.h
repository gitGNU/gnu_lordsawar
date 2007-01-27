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

#ifndef RADDGOLD_H
#define RADDGOLD_H

#include "Reaction.h"

class Player;

/** This reaction adds gold to a player's treasure (it is also possible to
  * withdraw gold by specifying a negative amount of gold).
  *
  * See Reaction.h for some more description of Reactions.
  */

class RAddGold : public Reaction
{
    public:
        /** Default constructor
          * 
          * @param player   the player who gets the gold
          * @param gold     the amount of gold added
          */
        RAddGold(Uint32 player, Sint32 gold);

        //! Loads the data from the savegame given by the helper
        RAddGold(XML_Helper* helper);
        ~RAddGold();

        //! Saves the data
        bool save(XML_Helper* helper) const;

        //! Triggers the reaction
        bool trigger() const;

        
        //! Returns the player who gets the gold
        Uint32 getPlayer() {return d_player;}

        //! Sets the player that gets the gold
        void setPlayer(Uint32 player) {d_player = player;}


        //! Returns the amount of gold to be added
        Sint32 getGold() {return d_gold;}

        //! Sets the amount of gold to be added
        void setGold(Sint32 gold) {d_gold = gold;}

    private:
        Uint32 d_player;
        Sint32 d_gold;
    
};

#endif //RADDGOLD_H
