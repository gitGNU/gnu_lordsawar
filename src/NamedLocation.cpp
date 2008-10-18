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

#include "NamedLocation.h"

#include "xmlhelper.h"

NamedLocation::NamedLocation(Vector<int> pos, Uint32 size, std::string name,
			     std::string desc)
  :Location(pos, size), Namable(name), d_description(desc)
{
}

NamedLocation::NamedLocation(const NamedLocation& object)
  :Location(object), Namable(object), d_description(object.d_description)
{
}

NamedLocation::NamedLocation(XML_Helper* helper, Uint32 size)
  :Location(helper, size), Namable(helper)
{
  helper->getData(d_description, "description");
}

NamedLocation::~NamedLocation()
{
}

