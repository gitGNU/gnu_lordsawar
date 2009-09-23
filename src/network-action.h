// Copyright (C) 2008 Ben Asselstine
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

#ifndef NETWORK_ACTION_H
#define NETWORK_ACTION_H

#include "action.h"
#include "Ownable.h"

class NetworkAction: public Ownable
{
public:
	
    //! The xml tag of this object in a network stream.
    static std::string d_tag; 

     //! Default constructor.
     NetworkAction(Action *action, Player *owner);

     //! Loading constructor.
     NetworkAction(XML_Helper* helper);

     //! Destructor.
     ~NetworkAction();

     //! Returns debug information.
     std::string toString() const;

     //! copying the deep contents of the action
     static NetworkAction* copy(Action *action, Player *owner);

     //!Saving the network action to an xml stream.
     bool save(XML_Helper* helper) const;

     Action * getAction() const {return d_action;};
     void setAction (Action *action) {d_action = action;};
 private:
     Action *d_action;
};
#endif
