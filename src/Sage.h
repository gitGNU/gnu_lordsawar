// Copyright (C) 2009, 2014 Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
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

#ifndef SAGE_H
#define SAGE_H

#include <gtkmm.h>
#include <list>
#include "reward.h"

//! A sage is a provider of rewards.
/** 
 * 
 */

class Sage: public std::list<Reward*>
{
    public:

	//! Creates a new Sage from scratch.
        Sage();

        //! Destructor.
        virtual ~Sage();

        Reward *getSelectedReward() const {return reward;};
        void selectReward(Reward *r) {reward = r;};

    private:

        Reward *reward;


};

#endif //SAGE_H
