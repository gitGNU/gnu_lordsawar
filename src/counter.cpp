// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2005 Ulf Lorenz
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

#include "counter.h"

#include "xmlhelper.h"

FL_Counter* fl_counter;

FL_Counter::FL_Counter(Uint32 start)
    :d_curID(start)
{
}

FL_Counter::FL_Counter(XML_Helper* helper)
{
    helper->getData(d_curID, "curID");
}

FL_Counter::~FL_Counter()
{
}

Uint32 FL_Counter::getNextId()
{
  Uint32 ret = d_curID;
  d_curID++;
  return ret;
}

bool FL_Counter::save(XML_Helper* helper)
{
    bool retval =true;

    retval &= helper->openTag("counter");
    retval &= helper->saveData("curID", d_curID);
    retval &= helper->closeTag();

    return retval;
}

