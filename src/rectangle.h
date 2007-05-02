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

#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "vector.h"

struct Rectangle
{
    Rectangle() : x(pos.x), y(pos.y), w(dim.x), h(dim.y) {}

    Rectangle(int x_, int y_, int w_, int h_)
	: pos(x_, y_), dim(w_, h_), x(pos.x), y(pos.y), w(dim.x), h(dim.y) {}

    Rectangle(Vector<int> pos_, Vector<int> dim_)
	: pos(pos_), dim(dim_), x(pos.x), y(pos.y), w(dim.x), h(dim.y) {}
    
    Rectangle(const Rectangle &other)
	: pos(other.pos), dim(other.dim), x(pos.x), y(pos.y), w(dim.x), h(dim.y) {}

    const Rectangle &operator=(const Rectangle &other)
    {
	pos = other.pos;
	dim = other.dim;
	return *this;
    }

    Vector<int> pos, dim; // position and dimensions

    // accessors - sometimes it's easier with .x instead of .pos.x
    int &x, &y, &w, &h;
};

inline bool operator==(const Rectangle &lhs, const Rectangle &rhs)
{
    return lhs.pos == rhs.pos && lhs.dim == rhs.dim;
}

inline bool operator!=(const Rectangle &lhs, const Rectangle &rhs)
{
    return !(lhs == rhs);
}

inline bool is_inside(const Rectangle &r, Vector<int> v)
{
    return r.x <= v.x && v.x < r.x + r.w
	&& r.y <= v.y && v.y < r.y + r.h;
}

inline bool is_overlapping(const Rectangle &r1, const Rectangle &r2)
{
    // find the leftmost rectangle
    Rectangle const *l, *r;
    if (r1.x <= r2.x)
    {
	l = &r1;
	r = &r2;
    }
    else
    {
	l = &r2;
	r = &r1;
    }
    
    // leftmost is too far to the left
    if (l->x + l->w <= r->x)
	return false;

    // find the upper rectangle
    Rectangle const *u, *d;
    if (r1.y <= r2.y)
    {
	u = &r1;
	d = &r2;
    }
    else
    {
	u = &r2;
	d = &r1;
    }

    // upper is too high up
    if (u->y + u->h <= d->y)
	return false;

    return true;
}

#endif
