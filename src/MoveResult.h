// Copyright (C) 2004 John Farrell
// Copyright (C) 2005, 2007 Ulf Lorenz
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

#ifndef MOVE_RESULT_H
#define MOVE_RESULT_H

#include "fight.h"

using namespace std;

/** The result of a move by a stack.
  * This is needed by the AI so it can tell when a stack dies.
  */
class MoveResult
{
    public:
        MoveResult(bool result);
        ~MoveResult();

        //! set the result of any fight that happened
        void setFightResult(Fight::Result d_fightResult);

        //! set how many steps were taken in this move
        void setStepCount(int stepCount) { d_stepCount = stepCount; }

        //! set whether the move ended in a join to another stack
        void setJoin(bool join) { d_join = join; }
        

        //! did the move succeed?
        bool moveSucceeded() const { return d_result; }

        //! did the stack go away as a result of this move?
        bool stackVanished() const { return (d_fightResult == Fight::DEFENDER_WON) || d_join; }

        //! return the result of the fight, if there was one
        Fight::Result getFightResult() const { return d_fightResult; }

        //! did anything actually happen in this move?
        bool didSomething() const { return (d_fight || (d_stepCount > 0) || d_join); }

    private:
        bool d_result;
        bool d_fight;
        bool d_join;
        int d_stepCount;
        Fight::Result d_fightResult;
};

#endif // MOVE_RESULT_H

// End of file
