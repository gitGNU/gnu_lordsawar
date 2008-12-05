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

#include "SmallTile.h"
#include <iostream>

std::string SmallTile::d_tag = "smallmap";

using namespace std;

SmallTile::SmallTile()
{
  d_pattern = SOLID;
  d_color.r = 80;
  d_color.g = 172;
  d_color.b = 28;
}

SmallTile::SmallTile(XML_Helper* helper)
{
  Uint32 i;
  SDL_Color color;
  color.unused = 0;
  helper->getData(i, "red");      color.r = i;
  helper->getData(i, "green");    color.g = i;
  helper->getData(i, "blue");     color.b = i;
  setColor(color);

  helper->getData(i, "pattern");
  SmallTile::Pattern pattern = static_cast<SmallTile::Pattern>(i);
  setPattern(pattern);

  if (pattern != SOLID)
    {
      helper->getData(i, "2nd_red");      color.r = i;
      helper->getData(i, "2nd_green");    color.g = i;
      helper->getData(i, "2nd_blue");     color.b = i;
      setSecondColor(color);
      if (pattern != STIPPLED && pattern != SUNKEN)
	{
	  helper->getData(i, "3rd_red");      color.r = i;
	  helper->getData(i, "3rd_green");    color.g = i;
	  helper->getData(i, "3rd_blue");     color.b = i;
	  setThirdColor(color);
	}
    }
}

bool SmallTile::save(XML_Helper *helper)
{
  bool retval = true;

  retval &= helper->openTag(d_tag);
  retval &= helper->saveData("pattern", d_pattern);
  switch (d_pattern)
    {
      //patterns with a single colour
    case SOLID:
      retval &= helper->saveData("red", d_color.r);
      retval &= helper->saveData("green", d_color.g);
      retval &= helper->saveData("blue", d_color.b);
      break;
      //patterns with two colours
    case STIPPLED: case SUNKEN:
      retval &= helper->saveData("red", d_color.r);
      retval &= helper->saveData("green", d_color.g);
      retval &= helper->saveData("blue", d_color.b);
      retval &= helper->saveData("2nd_red", d_second_color.r);
      retval &= helper->saveData("2nd_green", d_second_color.g);
      retval &= helper->saveData("2nd_blue", d_second_color.b);
      break;
      //patterns with three colours
    case RANDOMIZED: case TABLECLOTH: case DIAGONAL: case CROSSHATCH:
    case SUNKEN_STRIPED:
      retval &= helper->saveData("red", d_color.r);
      retval &= helper->saveData("green", d_color.g);
      retval &= helper->saveData("blue", d_color.b);
      retval &= helper->saveData("2nd_red", d_second_color.r);
      retval &= helper->saveData("2nd_green", d_second_color.g);
      retval &= helper->saveData("2nd_blue", d_second_color.b);
      retval &= helper->saveData("3rd_red", d_third_color.r);
      retval &= helper->saveData("3rd_green", d_third_color.g);
      retval &= helper->saveData("3rd_blue", d_third_color.b);
      break;
    }
  retval &= helper->closeTag();

  return retval;
}

SmallTile::~SmallTile()
{
}

// End of file
