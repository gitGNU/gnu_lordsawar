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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

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
    d_curID++;
    return d_curID;
}

bool FL_Counter::save(XML_Helper* helper)
{
    bool retval =true;

    retval &= helper->openTag("counter");
    retval &= helper->saveData("curID", d_curID);
    retval &= helper->closeTag();

    return retval;
}

