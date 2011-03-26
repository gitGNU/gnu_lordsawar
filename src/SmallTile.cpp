// Copyright (C) 2008, 2009, 2010, 2011 Ben Asselstine
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

#include "SmallTile.h"
#include <iostream>

std::string SmallTile::d_tag = "smallmap";

using namespace std;

SmallTile::SmallTile()
{
  d_pattern = SOLID;
  d_color.set_rgb_p(80.0/255.0,172.0/255.0,28.0/255.0);
  d_second_color.set_rgb_p(0,0,0);
  d_third_color.set_rgb_p(0,0,0);
}

SmallTile::SmallTile(const SmallTile &orig)
       : d_pattern(orig.d_pattern), d_color(orig.d_color), 
       d_second_color(orig.d_second_color), d_third_color(orig.d_third_color)
{
}

SmallTile::SmallTile(XML_Helper* helper)
{
  helper->getData(d_color, "color");

  guint32 i;
  helper->getData(i, "pattern");
  SmallTile::Pattern pattern = static_cast<SmallTile::Pattern>(i);
  setPattern(pattern);

  if (pattern != SOLID)
    {
      helper->getData(d_second_color, "2nd_color");
      if (pattern != STIPPLED && pattern != SUNKEN)
	helper->getData(d_third_color, "3rd_color");
    }
}

bool SmallTile::save(XML_Helper *helper) const
{
  bool retval = true;

  retval &= helper->openTag(d_tag);
  retval &= helper->saveData("pattern", d_pattern);
  switch (d_pattern)
    {
      //patterns with a single colour
    case SOLID:
      retval &= helper->saveData("color", d_color);
      break;
      //patterns with two colours
    case STIPPLED: case SUNKEN:
      retval &= helper->saveData("color", d_color);
      retval &= helper->saveData("2nd_color", d_second_color);
      break;
      //patterns with three colours
    case RANDOMIZED: case TABLECLOTH: case DIAGONAL: case CROSSHATCH:
    case SUNKEN_STRIPED: case SUNKEN_RADIAL:
      retval &= helper->saveData("color", d_color);
      retval &= helper->saveData("2nd_color", d_second_color);
      retval &= helper->saveData("3rd_color", d_third_color);
      break;
    }
  retval &= helper->closeTag();

  return retval;
}

SmallTile::~SmallTile()
{
}

// End of file
