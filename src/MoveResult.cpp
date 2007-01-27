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

#include <iostream>
#include "MoveResult.h"
#include "fight.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
//#define debug(x)

MoveResult::MoveResult(bool result)
    :d_result(result), d_fight(false), d_join(false),
    d_stepCount(0), d_fightResult(Fight::DRAW)
{
}

MoveResult::~MoveResult()
{
}

void MoveResult::setFightResult(Fight::Result fightResult)
{
    d_fight = true;
    d_fightResult = fightResult;
}
// End of file
