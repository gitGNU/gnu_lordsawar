// Copyright (C) 2008 Ben Asselstine
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

#ifndef HERO_PROTO_H
#define HERO_PROTO_H

#include <string>

#include "defs.h"

class XML_Helper;

#include "armyproto.h"
#include "hero.h"

class ArmyProto;

class HeroProto : public ArmyProto
{
    public:

	//! Copy constructor.
        HeroProto(const HeroProto& heroproto);

	//! Copy constructor.
        HeroProto(const ArmyProto& heroproto);

	//! Create an empty hero prototype.
	HeroProto();

	//! Loading constructor.
        HeroProto(XML_Helper* helper);

	//! Destructor.
        ~HeroProto();

        //! Set the gender of the hero.
        void setGender(Hero::Gender gender){d_gender = gender;}

        //! Return the gender of the hero.
        Uint32 getGender() const {return d_gender;}
        
        //! Saves the hero prototype to an action
        virtual bool save(XML_Helper* helper) const;
    private:

	//! Gender of the hero
	Hero::Gender d_gender;
};

#endif // HERO_PROTO_H
