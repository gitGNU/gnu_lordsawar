// Copyright (C) 2008 Ben Asselstine
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

#include <sstream>
#include "SightMap.h"

std::string SightMap::d_tag = "sightmap";
using namespace std;

SightMap::SightMap(XML_Helper* helper)
	:Renamable(helper)
{
    helper->getData(x, "x");
    helper->getData(x, "y");
    helper->getData(w, "width");
    helper->getData(h, "height");
}

SightMap::SightMap(std::string name, Vector<int> pos, Uint32 height, Uint32 width)
:Rectangle(pos, Vector<int>(width, height)), Renamable(name)
{
}

SightMap::SightMap(const SightMap& orig)
:Rectangle(orig), Renamable(orig)
{
}

SightMap::~SightMap()
{
}

bool SightMap::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag(SightMap::d_tag);
  retval &= helper->saveData("name", getName());
  retval &= helper->saveData("x", x);
  retval &= helper->saveData("y", y);
  retval &= helper->saveData("width", w);
  retval &= helper->saveData("height", h);
  retval &= helper->closeTag();

  return retval;
}
