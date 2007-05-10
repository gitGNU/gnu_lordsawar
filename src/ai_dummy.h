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

#ifndef AI_DUMMY_H
#define AI_DUMMY_H

#include <string>
#include <list>
#include <SDL_types.h>

#include "real_player.h"

class XML_Helper;

/** Dummy AI player.
  * 
  * This class is a dummy AI used for the neutral player. It just does, well,
  * nothing. See the player class for the derivation scheme.
  */

class AI_Dummy : public RealPlayer
{
    public:
        /** The recommended constructor
          * 
          * @param name         the name of the player
          * @param armyset      the armyset of the player
          * @param color        the color of the player
          */
        AI_Dummy (std::string name, Uint32 armyset, SDL_Color color);

        /** Copy constructor
          * 
          * @param player       player whose data is copied
          */
        AI_Dummy(const Player& player);

        //! Loading constructor. See XML_Helper.
        AI_Dummy(XML_Helper* helper);
        ~AI_Dummy();
        
        //! This function is called whenever the player's turn starts.
        bool startTurn();

        //! A kind of callback when a city has been conquered.
        bool invadeCity(City* c);

        //! A kind of callback when a hero offers his services.
        bool recruitHero(Hero* hero, City *city, int cost);

        //! Callback to raise the army by a level
        bool levelArmy(Army* a);
};

#endif // AI_DUMMY_H
