// Copyright (C) 2008, 2014 Ben Asselstine
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

#ifndef HERO_PROTO_H
#define HERO_PROTO_H

class XML_Helper;

#include "armyproto.h"
#include "hero.h"
#include "OwnerId.h"

//! A prototype of a Hero object.
class HeroProto : public ArmyProto, public OwnerId
{
    public:

	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

	//! Copy constructor.
        HeroProto(const HeroProto& heroproto);

	//! Copy constructor.
        HeroProto(const ArmyProto& heroproto);

	//! Create an empty hero prototype.
	HeroProto();

	//! Loading constructor.
        HeroProto(XML_Helper* helper);

	//! Destructor.
        ~HeroProto() {};

        //! Set the gender of the hero.
        void setGender(Hero::Gender gender){d_gender = gender;}

        //! Return the gender of the hero.
        guint32 getGender() const {return d_gender;}
        
        //! Saves the hero prototype to an action
        virtual bool save(XML_Helper* helper) const;
    private:

	//! Gender of the hero
	Hero::Gender d_gender;
};

#endif // HERO_PROTO_H
