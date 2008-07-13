//  Copyright (C) 2008, Ben Asselstine
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

#ifndef OWNABLE_H
#define OWNABLE_H

#include "defs.h"

class Player;
class XML_Helper;

//! A game object that has an owner.
/** 
 * An Ownable is a map object that can be owned by a Player.
 */

class Ownable
{
 public:
     //! Default constructor.
     Ownable(Player *owner);
     //! Copy constructor.
     Ownable(const Ownable&);
     //! Loading constructor.
     Ownable(XML_Helper* helper);
     //! Destructor.
    ~Ownable();
    
    static Ownable load(XML_Helper *helper);

    //! Return a pointer to the Player who owns an object.
    Player *getOwner() const {return d_owner;}

    //! Set the Player who owns an object.
    void setOwner(Player *player){d_owner = player;}

    //! Return true if the player parameter matches the owner.
    bool isFriend (Player *player);

 protected:
    Player *d_owner;
};

#endif
