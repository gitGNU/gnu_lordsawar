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

#include "armyindexed.h"
#include "xmlhelper.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

ArmyIndexed::ArmyIndexed(const ArmyIndexed& a)
  :d_type_id(a.d_type_id), d_armyset(a.d_armyset)
{
}

ArmyIndexed::ArmyIndexed()
  :d_type_id(0), d_armyset(0)
{
}

ArmyIndexed::ArmyIndexed(XML_Helper* helper)
  :d_type_id(0), d_armyset(0)
{

  helper->getData(d_type_id, "type_id");
  helper->getData(d_armyset, "armyset");
}

ArmyIndexed::~ArmyIndexed()
{
}

bool ArmyIndexed::saveData(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->saveData("type_id", d_type_id);
  retval &= helper->saveData("armyset", d_armyset);
  return retval;
}
