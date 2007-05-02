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

#ifndef AI_FAST_H
#define AI_FAST_H

#include <string>
#include <list>

#include "real_player.h"
#include "AI_Analysis.h"

class XML_Helper;


/** Simple AI
  *
  * This AI has two modes. In normal modes it basically assembles stacks of
  * 8 units each and sends them to the next city, reinforcing them in own cities
  * if neccessary. In maniac mode, however (meant for wandering monsters etc.),
  * this AI will attack everything that is close up or take the nearest city if
  * no enemies are close. When it takes over an enemy city, it razes it.
  * 
  * See the Player class for the derivation scheme
  */

class AI_Fast : public RealPlayer
{
    public:
        /** Default constructor
          * 
          * @param name         the name of the player
          * @param armyset      the armyset of the player
          * @param color        the player's color
          */
        AI_Fast(std::string name, Uint32 armyset, SDL_Color color);

        //! Copy constructor
        AI_Fast(const Player&);

        //! Loading constructor. See XML_Helper for an explanation.
        AI_Fast(XML_Helper* helper);
        ~AI_Fast();
        
        //! Saves data, the function is for saving additional data
        bool save(XML_Helper* helper) const;
        

        //! Sets whether the ai joins close armies to make them stronger
        void setJoin(bool join) {d_join = join;}

        //! Returns the current behaviour regarding joining armies
        bool getJoin() const {return d_join;}

        //! Set maniac/normal mode
        void setManiac(bool maniac) {d_maniac = maniac;}

        //! Returns the current behaviour
        bool getManiac() const {return d_maniac;}

        
        /** This function is called whenever the player's turn starts. As soon
          * as it returns, the player's turn ends.
          */
        bool startTurn();

        //! Callback when the player invades a city
        bool invadeCity(City* c);

        //! Callback when a hero offers his services
        bool recruitHero(Hero* hero, int cost);

        //! Callback when an army of the player advances a level
        bool levelArmy(Army* a);

    private:
        //! The actual core function of the ai's logic.
        void computerTurn(); 


        // Data
        bool d_join;        //!< determines whether to join units or move them separately
        bool d_maniac;      //!< maniac mode: kill and raze everything you encounter
        AI_Analysis* d_analysis;
};

#endif // AI_FAST_H
