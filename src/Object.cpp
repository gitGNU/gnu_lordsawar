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

#include "Object.h"
#include "counter.h"

Object::Object(PG_Point pos, Uint32 size)
    :d_pos(pos), d_size(size)
{
    d_id = fl_counter->getNextId();
}

Object::Object(const Object& obj)
    :d_pos(obj.d_pos), d_id(obj.d_id), d_size(obj.d_size)
{
}

Object::Object(XML_Helper* helper, Uint32 size)
    :d_size(size)
{
    int i;
    helper->getData(d_id, "id");
    helper->getData(i, "x");
    d_pos.x = i;
    helper->getData(i, "y");
    d_pos.y = i;
}

Object::~Object()
{
}

bool Object::contains(PG_Point pos) const
{
    return (pos.x >= d_pos.x) && (pos.x < d_pos.x + (int) d_size) && (pos.y >= d_pos.y) && (pos.y < d_pos.y + (int) d_size);
}

