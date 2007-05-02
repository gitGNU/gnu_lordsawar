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

#ifndef THREAT_H
#define THREAT_H

#include <string>
#include "vector.h"

class City;
class Stacklist;
class Stack;
class Player;
class Ruin;

/** Class which describes a threat to a player.
  *
  * The smart AI player tries to assess all enemy stacks and cities and all
  * ruins and store the results in threats. A threat has three characteristic
  * figures:
  *
  * - a strength which determines how strong the stack (or the stacks defending
  *   the city) are
  * - a danger which determines how close the enemy object is (if an enemy stack
  *   endangers two cities at once, the danger value doubles). The danger depends
  *   on the distance to the AI's cities as well as the strength of the stack.
  * - a value, which gives an assessment of how valuable it is to destroy the
  *   threat. As an example, taking over a dangerous enemy city is more valuable
  *   than destroying a stack because the AI gets an additional city. The value
  *   is the sum of the danger of the threat and some additional bonus.
  *
  * Furthermore, the threat class has some additional functions. They are e.g.
  * neccessary because the AI bundles several stacks which are close together
  * or a stack which is in an enemy city to one single threat.
  *
  * For more information about the smart AI, see ai_smart.h
  */

class Threat
{
    public:
        // CREATORS

        //! Our threat is an enemy city.
        Threat(City *c);

        //! The threat is an enemy stack.
        Threat(Stack *s);

        //! The "threat" is a ruin (the danger value is 0)
        Threat(Ruin *r);
        ~Threat();

        
        //! Can be used for some general debug output
        std::string toString() const;

        /** Checks if a threat is close to a certain position and "belongs"
          * (i.e. is caused by) a certain player.
          *
          * The background is that threats close to each other are merged into
          * a single threat.
          *
          * @param pos  the position the threat has to be close to
          * @param p    the player who has to cause the threat
          * @return true if both the position and the player satisfy the
          * conditions, else return false
          */
        bool Near(Vector<int> pos, Player *p) const;

        //! add a stack to this threat
        void addStack(Stack *stack);

        //! how strong is this threat?
        float strength() const;

        //! how valuable is to us to destroy this threat?
        float value() const;

        /** Returns the closest point of a threat to a certain location
          * (remember that a threat can consist of several single threats
          * or a city which covers more than one tile)
          *
          * If there are no dangers left, it returns the point (-1, -1).
          */
        Vector<int> getClosestPoint(Vector<int> location) const;

        //! Removes the stack s from the threat
        void deleteStack(Stack* s);

        //! return the danger posed by this threat to the current player
        float getDanger() const { return d_danger; }

        //! Does this threat contain a city?
        bool isCity() const { return d_city != 0; }

        //! Is this threat a ruin?
        bool isRuin() const { return d_ruin != 0; }

        //! Increase the danger of this threat 
        void addDanger(float danger) { d_danger += danger; }

        //! return the player that causes this threat
        Player *getPlayer() const { return d_player; }
        
    private:
        City *d_city;
        Ruin *d_ruin;
        Player *d_player;
        Stacklist *d_stacks;
        float d_danger;
};

#endif // THREAT_H

// End of file
