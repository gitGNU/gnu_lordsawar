// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2004, 2005 Ulf Lorenz
// Copyright (C) 2005 Andrea Paternesi
// Copyright (C) 2007, 2008 Ben Asselstine
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

#include "UniquelyIdentified.h"
#include "counter.h"
#include "xmlhelper.h"

UniquelyIdentified::UniquelyIdentified()
{
    d_id = fl_counter->getNextId();
}

UniquelyIdentified::UniquelyIdentified(const UniquelyIdentified& obj)
    :d_id(obj.d_id)
{
}

UniquelyIdentified::UniquelyIdentified(XML_Helper* helper)
{
    helper->getData(d_id, "id");
}

UniquelyIdentified::~UniquelyIdentified()
{
}

