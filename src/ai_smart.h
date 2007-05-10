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

#ifndef AI_SMART_H
#define AI_SMART_H

#include <string>
#include <list>
#include <SDL_types.h>

#include "real_player.h"
#include "citylist.h"

class Threatlist;
class Threat;
class AI_Analysis;
class XML_Helper;

/** More advanced AI
  *
  * After examining if it buys additional production, this AI uses the classical
  * strategy technique of:
  * - analyse the game situation - this involves identifying stacks and cities as
  *   threats, and deciding which are the greatest threats to which cities.
  * - allocate resources to deal with the threats to our best advantage.
  *
  * TODO:  Ruins are also identified as threats, though they are not handled yet.
  * TODO:  The Ai only buys basic productions
  * TODO:  The AI doesn't rally make use of the multifight, and it cannot really
  * handle the results (not checked)
  * TODO:  The Ai should be able to upgrade cities (Increases their income etc.)
  * TODO:  AI is way too defensive (in fact, the fast AI tends to win games)
  *
  * The code is split up in several classes. Be sure to read the comments there
  * before trying to read the code. :)
  *
  * - AI_Analysis includes the code for the assessment of the game situation
  * - AI_Allocation distributes the AI's ressources the engage threats
  * - Threat/Threatlist contains the code for the definition of single threats
  *   and the container class for threats
  * - AICityInfo is used to collect the threats and reinforcements etc. of a
  *   single city of the AI.
  *
  * Also see the Player class for the derivation scheme of players.
  */

class AI_Smart : public RealPlayer
{
    public:
        /** Default constructor
          *
          * @param name         the name of the player
          * @param armyset      the armyset of the player
          * @param color        the player's color
          */
        AI_Smart(std::string name, Uint32 armyset, SDL_Color color);

        //! Copy constructor
        AI_Smart(const Player&);

        //! Loading constructor. See XML_Helper for an explanation.
        AI_Smart(XML_Helper* helper);
        ~AI_Smart();

        /** This function is called whenever the player's turn starts. As soon
          * as it returns, the player's turn ends.
          */
        bool startTurn();

        //! Callback when the player invades a city
        bool invadeCity(City* c);

        //! Callback when a hero offers his services
        bool recruitHero(Hero* hero, City *city, int cost);

        //! Callback when an army of the player advances a level
        bool levelArmy(Army* a);

    private:
        // Choose a new type of army to buy production for.
        int chooseArmyTypeToBuy(City *c);

        // Consider buying new production for this city
        int maybeBuyProduction(City *c);
        
        // Set the city to produce the best armies possible
        int setBestProduction(City *c);
        
        // assign a score to an army type to try to figure out which is best
        int scoreArmyType(const Army *proto);

        // suggest somewhere that a hero stack might like to visit
        Location *getAlternateHeroTarget(Stack *s);
        
        // what is the biggest danger to this city?
        Threat *getBiggestDangerTo(City *city, Threatlist *threats);
        
        // examine cities to see if we need to change production
        void examineCities();

        // DATA
        int d_mustmakemoney;  // used to avoid to buy new production 
                              // and to reinforce cities to earn more money

};

#endif // AI_SMART_H
