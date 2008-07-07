// Copyright (C) 2004 John Farrell
// Copyright (C) 2004, 2005, 2006 Ulf Lorenz
//
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#ifndef AI_ALLOCATION_H
#define AI_ALLOCATION_H

#include <string>

#include "MoveResult.h"

class AI_Analysis;
class Player;
class Ruin;
class Citylist;
class Stack;
class Threat;
class Stacklist;
class City;

using namespace std;

/** An AI's allocation of resources to goals identified in the analysis.
  */

class AI_Allocation
{
    public:
        AI_Allocation(AI_Analysis *analysis, Player *owner);
        ~AI_Allocation();

        // make the player's moves - return the number of stacks which moved.
        int move();

        static void deleteStack(Stack* s);

    private:
        /** Assign stacks to defend cities
          * 
          * This function checks if each city is properly defended and assigns
          * additional stacks from the environment as defenders if neccessary.
          *
          * @param allCities    list of cities to be checked
          * @param stacks       list of stacks available for the task. The
          *                     allocated stacks are removed from the list.
          * @return number of stacks moved
          */
        int allocateDefensiveStacks(Citylist *allCities);
        
        /** Allocate stacks to threats
          * 
          * @param stacks       the list of stacks which we can choose stacks from
          *                     stacks we have dealt with are removed from the list.
          * @return the number of stacks which moved
          */
        int allocateStacksToThreats();
        
        // stack return to a safe city
        bool stackReinforce(Stack *s);
        
        // search a ruin
        void searchRuin(Stack *stack, Ruin *ruin);
        
        // move armies within a city to try to make full stacks
        bool shuffleStacksWithinCity(City *city, Stack *stack, 
				     Vector<int> diff);
        
        // tell the stack to move to the point
        MoveResult *moveStack(Stack *stack, Vector<int> pos);
        
        // find the best attacker for the given threat
        Stack *findBestAttackerFor(Threat *threat);
        
        // find the closest stack to the given position, but 0 if none within
        // limitInMoves (estimate of a maximum number of turns for getting there) 
        Stack *findClosestStackToCity(City *city, int limitInMoves);
        
        // find a position in the city that a stack can move to
        Vector<int> getFreeSpotInCity(City *city, int stackSize);
        
        // move stacks that we have no particular use for
        int defaultStackMovements();
        
        static AI_Allocation* s_instance;
        
        Player *d_owner;
        AI_Analysis *d_analysis;
        Stacklist *d_stacks;
};

#endif // AI_ALLOCATION_H

// End of file
