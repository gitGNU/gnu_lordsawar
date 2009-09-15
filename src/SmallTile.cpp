// Copyright (C) 2008, 2009 Ben Asselstine
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
  d_color.set_rgb_p(80.0/255.0,172.0/255.0,28.0/255.0);
  d_second_color.set_rgb_p(0,0,0);
  d_third_color.set_rgb_p(0,0,0);
}

SmallTile::SmallTile(XML_Helper* helper)
{
  guint32 r, g, b;
  helper->getData(r, "red");
  helper->getData(g, "green");
  helper->getData(b, "blue");
  d_color.set_rgb_p((float)r/255.0,(float)g/255.0, (float)b/255.0);

  guint32 i;
  helper->getData(i, "pattern");
  SmallTile::Pattern pattern = static_cast<SmallTile::Pattern>(i);
  setPattern(pattern);

  if (pattern != SOLID)
    {
      helper->getData(r, "2nd_red");
      helper->getData(g, "2nd_green");
      helper->getData(b, "2nd_blue");
      d_second_color.set_rgb_p((float)r/255.0,(float)g/255.0, (float)b/255.0);
      if (pattern != STIPPLED && pattern != SUNKEN)
	{
	  helper->getData(r, "3rd_red");
	  helper->getData(g, "3rd_green");
	  helper->getData(b, "3rd_blue");
	  d_third_color.set_rgb_p((float)r/255.0,(float)g/255.0, 
				  (float)b/255.0);
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
      retval &= helper->saveData("red", int(d_color.get_red_p() *255));
      retval &= helper->saveData("green", int(d_color.get_green_p()*255));
      retval &= helper->saveData("blue", int(d_color.get_blue_p()*255));
      break;
      //patterns with two colours
    case STIPPLED: case SUNKEN:
      retval &= helper->saveData("red", int(d_color.get_red_p() *255));
      retval &= helper->saveData("green", int(d_color.get_green_p()*255));
      retval &= helper->saveData("blue", int(d_color.get_blue_p()*255));
      retval &= helper->saveData("2nd_red", 
				 int(d_second_color.get_red_p() *255));
      retval &= helper->saveData("2nd_green", 
				 int(d_second_color.get_green_p()*255));
      retval &= helper->saveData("2nd_blue", 
				 int(d_second_color.get_blue_p()*255));
      break;
      //patterns with three colours
    case RANDOMIZED: case TABLECLOTH: case DIAGONAL: case CROSSHATCH:
    case SUNKEN_STRIPED:
      retval &= helper->saveData("red", int(d_color.get_red_p() *255));
      retval &= helper->saveData("green", int(d_color.get_green_p()*255));
      retval &= helper->saveData("blue", int(d_color.get_blue_p()*255));
      retval &= helper->saveData("2nd_red", 
				 int(d_second_color.get_red_p() *255));
      retval &= helper->saveData("2nd_green", 
				 int(d_second_color.get_green_p()*255));
      retval &= helper->saveData("2nd_blue", 
				 int(d_second_color.get_blue_p()*255));
      retval &= helper->saveData("3rd_red", 
				 int(d_third_color.get_red_p() *255));
      retval &= helper->saveData("3rd_green", 
				 int(d_third_color.get_green_p()*255));
      retval &= helper->saveData("3rd_blue", 
				 int(d_third_color.get_blue_p()*255));
      break;
    }
  retval &= helper->closeTag();

  return retval;
}

SmallTile::~SmallTile()
{
}

// End of file
