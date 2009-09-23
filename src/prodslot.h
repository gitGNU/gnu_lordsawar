//  Copyright (C) 2008, Ben Asselstine
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

#ifndef PRODSLOT_H
#define PRODSLOT_H

#include "vector.h"
#include <string>

class XML_Helper;
class ArmyProdBase;

//! A placeholder for an army production object.
/** 
 */

class ProdSlot
{
 public:
     //! The xml tag of this object in a saved-game file.
     static std::string d_tag;

     //! Default constructor.
     ProdSlot();

     //! Copy constructor.
     ProdSlot(const ProdSlot&);

     //! Loading constructor.
     ProdSlot(XML_Helper* helper);

     //! Destructor.
    ~ProdSlot();

    //! Save the production slot to an opened saved-game file.
    bool save(XML_Helper *helper) const;

    ArmyProdBase *getArmyProdBase() const {return d_armyprodbase;};

    void setArmyProdBase(ArmyProdBase *prod) {d_armyprodbase = prod;};

    void clear();
 private:

    bool load(std::string tag, XML_Helper *helper);

    ArmyProdBase *d_armyprodbase;
};

#endif
