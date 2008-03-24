// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2004 John Farrell
// Copyright (C) 2007, 2008 Ben Asselstine
// Copyright (C) 2008 Ole Laursen
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

#ifndef PATH_H
#define PATH_H

#include <SDL_types.h>
#include <list>
#include "vector.h"

class Stack;
class XML_Helper;

//! A list of waypoint coordinates (Vector<int>) on the GameMap.
/** 
 * The path class is used to store movement paths, determine new movement 
 * paths, and to query existing movement paths.
 *
 */
class Path : public std::list<Vector<int>*>
{
    public:
        //! Default constructor.
        Path();
        //! Make a new path by loading it from an opened saved-game file.
        Path(XML_Helper* helper);
	//! Destructor.
        ~Path();

        //! Save the path to an opened saved-game file.
        bool save(XML_Helper* helper) const;

        /** 
	 * This function erases an item in the path list. We can't use STL
         * functions, because we need to delete the pointer as well.
         * Otherwise, it behaves like a normal erase function from the STL.
         */
        /** 
	 * Erase a waypoint from the Path, and free the waypoint too.
	 *
	 * @param it   The place in the Path to erase.
	 *
	 * @return The place in the path that was erased.
         */
	//! Erase a waypoint from the list.
        Path::iterator flErase(Path::iterator it);

        //! Erase the path, deleting the waypoints too.
        void flClear();

        /**
	 * The purpose of this method is to verify if the Stack can move 
	 * onto the Tile of the given destination point.
	 *
	 * @note This method does not calculate a path and it does not 
	 * consider the amount of movement points that the given Stack has. 
	 *
	 * This method uses two shortcuts to check if it is impossible for 
	 * the given stack to travel to the given destination on the GameMap.  
	 * Firstly it checks to see if the destination terrain Tile::Type is 
	 * of a kind that the Stack can't travel on at all (e.g. Mountains, 
	 * and the Stack can't fly).  Secondly it checks to see if the tile 
	 * is both adjacent and blocked from that direction.
	 *
	 * This method is primarily used to assist in mouse cursor display.
	 *
	 * @param stack   The stack to move.
	 * @param dest    The position on the map to see if the Stack can 
	 *                move to.
	 *
	 * @return True if the Stack can move to the given position on the 
	 *         GameMap.  Otherwise the return value is false.
         */
	//! Calculates if a Stack move to a position on the GameMap.
        bool canMoveThere(const Stack* stack, Vector<int> dest);

        /** 
         * Check if the path is blocked for some reason, and recalculate it
	 * if necessary.
         *
         * @param stack        The Stack whose path we validate.
	 *
         * @return True if path is valid, False if path is blocked and could
         *         not be recalculated, True if the path was invalid but was
	 *         recalculated succesfully.
         */
	//! Validate an existing path.
        bool checkPath(Stack* stack);

        /** 
	 * Calculates the path from the stack's position to a destination.
         *
         * The calculated path is stored within the instance (remember: each
         * stack has a path instance). During calculation, bonuses and other
         * specialities are taken into account.
         *
         * @param stack        The stack whose path we calculate.
         * @param dest         The destination point to calculate for.
	 * @param zigzag       Whether we're using the normal way to
	 *                     calculate paths or not.  False means we never 
	 *                     go diagonally.  True means we do.
	 *
         * @return The number of movement points needed to destination or 0
         *         if no path is possible.
         */
	//! Calculate a Stack object's Path to a destination on the GameMap.
        Uint32 calculate(Stack* stack, Vector<int> dest, bool zigzag = true);

	//! Recalculate a Stack object's Path.
	void recalculate (Stack* s);

	//! Return the number of points the stack can move along it's path.
	Uint32 getMovesExhaustedAtPoint() {return d_moves_exhausted_at_point;}

	/**
	 * Set the point at which the stack can't move along it's path.
	 * If the first point in the stack's path cannot be moved to,
	 * this method should return 0.  If the second point can't be moved 
	 * to, then this method should return 1, etc.
	 * 
	 * The purpose of this method is to assist in drawing the waypoint
	 * graphics.
	 *
	 * @param index   The index of the point in the stack's path that
	 *                cannot be moved to.
	 */
	//! Set the number of points the stack can move along it's path.
	void setMovesExhaustedAtPoint(Uint32 index) 
	  {d_moves_exhausted_at_point = index;}

        void eraseFirstPoint();
        
    private:
        /** 
         * This method returns whether or not a Stack can pass over a tile.  
	 * The value destx and desty is needed because some cases (e.g. enemy 
	 * army/city blocking the way) don't count as blocking when we engage 
	 * them at the _end_ of the path (wo de want to attack after all).
	 *
	 * The Stack is passed so that a movement bonus can be calculated.
         *
	 * @param stack  The stack doing the potential movement.  This is
	 *               passed to calculate a movement bonus.
	 * @param x      The number of tiles down in the vertical axis from the
	 *               topmost edge of the map.  This is half of the origin
	 *               point that the given Stack is moving from.
	 * @param y      The number of tiles right in the horizontal axis from
	 *               the leftmost edge of the map.  This is the other half
	 *               of the origin point that the Stack is moving from.
	 * @param destx  The number of tiles down in the vertical axis from the
	 *               topmost edge of the map.  This is half of the 
	 *               destination point that we're checking if the Stack
	 *               can move to.
	 * @param desty  The number of tiles right in the horizontal axis from
	 *               the leftmost edge of the map.  This is the other half
	 *               of the destination point that we're checking if the 
	 *               Stack can move to.
	 *
         * @return False if the Stack may pass, true otherwise.
         */
	//! Check if a position is blocked to the given Stack.
        bool isBlocked(const Stack* stack, int x, int y, 
		       int destx, int desty) const;

        /** 
	 * Checks if the way to a given tile is blocked
         * 
         * This function returns whether a unit can pass over a tile from
	 * another tile.  The idea here is that the "sides" of certain tiles
	 * are blocked from entry.  E.g. when trying to go from water to
	 * land, without going through a city.
         *
	 * @param x      The number of tiles down in the vertical axis from the
	 *               topmost edge of the map.  This is half of the origin
	 *               point that the given Stack is moving from.
	 * @param y      The number of tiles right in the horizontal axis from
	 *               the leftmost edge of the map.  This is the other half
	 *               of the origin point that the Stack is moving from.
	 * @param destx  The number of tiles down in the vertical axis from the
	 *               topmost edge of the map.  This is half of the 
	 *               destination point that we're checking if the Stack
	 *               can move to.
	 * @param desty  The number of tiles right in the horizontal axis from
	 *               the leftmost edge of the map.  This is the other half
	 *               of the destination point that we're checking if the 
	 *               Stack can move to.
	 *
	 * @note The movement capabilities of a Stack are not taken into 
	 *       account in this method.  This method should only be called 
	 *       for Stack objects that are not flying.
	 *
         * @return False if a non-flying stack may pass, true otherwise.  
	 *         False if the two points are not adjacent.
         */
	//! Checks if the way between adjacent tiles is blocked.
        bool isBlockedDir(int x, int y, int destx, int desty) const;

        /** 
	 * Checks how many movement points are needed to cross a tile from
	 * an adjacent tile.
         * 
	 * @param stack  The stack doing the potential movement.  This is
	 *               passed to calculate the proper movement bonus.
	 * @param x      The number of tiles down in the vertical axis from the
	 *               topmost edge of the map.  This is half of the origin
	 *               point that the given Stack is moving from.
	 * @param y      The number of tiles right in the horizontal axis from
	 *               the leftmost edge of the map.  This is the other half
	 *               of the origin point that the Stack is moving from.
	 * @param destx  The number of tiles down in the vertical axis from the
	 *               topmost edge of the map.  This is half of the 
	 *               destination point that we're checking if the Stack
	 *               can move to.
	 * @param desty  The number of tiles right in the horizontal axis from
	 *               the leftmost edge of the map.  This is the other half
	 *               of the destination point that we're checking if the 
	 *               Stack can move to.
         * @return The number of movement points required to traverse the
	 *         destination tile, or -1 if movement not possible.
         */
	//! Calculates movement points to traverse an adjacent tile.
        int pointsToMoveTo(const Stack *s, int x, int y, int destx, int desty) const;

        // Data

	//! A cached copy of a Stack object's movement bonus.
        Uint32 d_bonus;

	//! The point in the path that can't be reached.
	Uint32 d_moves_exhausted_at_point;
};

#endif // PATH_H
