// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2007, 2008 Ben Asselstine
// Copyright (C) 2007, 2008 Ole Laursen
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

#ifndef ARMY_PRODBASE_H
#define ARMY_PRODBASE_H

#include <string>

#include "defs.h"

#include "armyprotobase.h"
class ArmyProto;

class XML_Helper;

class ArmyProdBase: public ArmyProtoBase
{
    public:

	//! Copy constructor.
        ArmyProdBase(const ArmyProdBase& prodbase);

	//! Copy constructor.
        ArmyProdBase(const ArmyProto& prodbase);

	//! Loading constructor.
        ArmyProdBase(XML_Helper* helper);
        
	//! Destructor.
        ~ArmyProdBase();

        //! Saves the Army prototype to an opened armyset file.
        virtual bool save(XML_Helper* helper) const;
        
    private:

};

#endif // ARMY_PRODBASE_H
