//  Copyright (C) 2008, 2009, 2014 Ben Asselstine
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

#ifndef OWNABLE_H
#define OWNABLE_H


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
    virtual ~Ownable() {};
    
    // Get Methods

    //! Return a pointer to the Player who owns an object.
    Player *getOwner() const {return d_owner;}

    //! Return true if the player parameter matches the owner.
    bool isFriend (Player *player) const;

    // Set Methods

    //! Set the Player who owns an object.
    void setOwner(Player *player){d_owner = player;}

    // Statics

    //! Callback for loading an Ownable object from an opened saved-game file.
    static Ownable load(XML_Helper *helper);

 protected:

    // A pointer to the player owning this object.
    Player *d_owner;
};

#endif
