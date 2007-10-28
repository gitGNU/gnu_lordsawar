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

#ifndef PATH_H
#define PATH_H

#include <SDL_types.h>
#include <list>
#include "vector.h"

class Stack;
class XML_Helper;

/** The path class cares for path storing and calculation.
  *
  * The task of the path class is three-fold.
  * - it stores existing movement paths of stacks (each stack has a path
  *   instance)
  * - it checks existing paths if they are blocked (when a stack doesn't reach
  *   its target in one round, you have to validate the path in the next round)
  *   and recalculates the path
  * - it calculates the shortest path between points on the map
  */
class Path : public std::list<Vector<int>*>
{
    public:
        //! Default constructor
        Path();

        //! Loads the existing path from a savegame
        Path(XML_Helper* helper);
        ~Path();


        //! Save the current path
        bool save(XML_Helper* helper) const;

        /** This function erases an item in the path list. We can't use STL
          * functions, because we need to delete the pointer as well.
          * Otherwise, it behaves like a normal erase function from the STL.
          */
        Path::iterator flErase(Path::iterator it);

        /** The same problem. Erases all items in the path list with the help of
          * fl_erase.
          */
        void flClear();


        /* This function is used to verify if the stack can move on the Tile
         * of the given destination point
         */
        bool canMoveThere(const Stack* s, Vector<int> dest);

        /** Validates an existing path.
          * 
          * If the path is blocked for some reason, this function tries to
          * recalculate it.
          *
          * @param s            the Stack whose path we validate
          * @return true if path is valid, false if path is blocked (and could
          * not be recalculated)
          */
        bool checkPath(Stack* s);

        /** Calculates the path from the stack's location to a destination.
          *
          * The calculated path is stored within the instance (remember: each
          * stack has a path instance). During calculation, bonuses and other
          * specialities are taken into account.
          *
          * @param s            the stack whose path we calculate
          * @param dest         the destination of the calculation
          * @return number of movement points needed to destination or 0
          * if no path is possible
          */
        Uint32 calculate(Stack* s, Vector<int> dest);

	void recalculate (Stack* s);

    private:
        /** Checks if a tile is blocked for the stack
          * 
          * This function returns whether a unit can pass over a tile. The value
          * destx and desty is needed because some cases (e.g. enemy army/city
          * blocking the way) don't count as blocking when we engage them at the
          * _end_ of the path (wo de want to attack after all).
          *
          * @param x, y             x/y position of the tile to be checked
          * @param destx, desty     x/y position of the destination
          * @return false if unit may pass, true otherwise
          */
        bool isBlocked(const Stack* s, int x, int y, int destx, int desty) const;
        /** Checks if the way to a given tile is blocked
          * 
          * This function returns whether a unit can pass over a tile from
	  * another tile.  The idea here is that the "sides" of certain tiles
	  * are blocked from entry.  E.g. when trying to go from water to
	  * land, without going through a city.
          *
          * @param x, y             x/y the position of the tile to travel from
          * @param destx, desty     x/y the position of the tile to travel to
          * @return false if unit may pass, true otherwise
          */
        bool isBlockedDir(int x, int y, int destx, int desty) const;

        /** Checks how many movement points are needed to cross the tile
          * 
          * @param x,y              coordinates of the tile to be moved from
          * @param destx,desty      coordinates of the tile to be moved to
	  * @param s                the stack being moved
          * @return costs in movement points, or -1 if movement not possible
          */
        int pointsToMoveTo(const Stack *s, int x, int y, int destx, int desty) const;

        // data
        Uint32 d_bonus;
};

#endif // PATH_H
