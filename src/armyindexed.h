// Copyright (C) 2009 Ben Asselstine
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

#ifndef ARMY_INDEXED_H
#define ARMY_INDEXED_H

#include <gtkmm.h>

class XML_Helper;

class ArmyIndexed
{
public:

    //! Copy constructor.
    ArmyIndexed(const ArmyIndexed & armyindexed);

    //! Loading constructor.
    ArmyIndexed(XML_Helper* helper);

    //! Create an empty army indexed object.
    ArmyIndexed();

    //! Destructor.
    ~ArmyIndexed();

    //! Returns the armyset id for this army.
    guint32 getArmyset() const {return d_armyset;};

    //! Sets the armyset id for this army.
    void setArmyset(guint32 id) {d_armyset = id;};

    //! Set theindex for this army in the armyset.
    void setTypeId(guint32 type_id) {d_type_id = type_id;};

    //! Get the index for this army in the armyset.
    guint32 getTypeId() const {return d_type_id;}


    bool saveData(XML_Helper* helper) const;
protected:

    //! The index of the Army prototype's index in it's Armyset.
    guint32 d_type_id;

    //! The armyset to which this army prototype belongs.
    guint32 d_armyset;
};
#endif
