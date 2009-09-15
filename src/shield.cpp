//  Copyright (C) 2008, 2009 Ben Asselstine
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

//#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "shield.h"
#include "GraphicsCache.h"
#include "xmlhelper.h"
#include "ucompose.hpp"
#include "rgb_shift.h"

std::string Shield::d_tag = "shield";

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

Shield::Shield(XML_Helper* helper)
{
  helper->getData(d_owner, "owner");
  std::string s;
  helper->getData(s, "color");
  guint32 r,g,b;
  std::istringstream scolor(s);
  scolor >> r;
  scolor >> g;
  scolor >> b;
  d_color.set_rgb_p(r/255.0,g/255.0,b/255.0);
}

Shield::~Shield()
{
  for (iterator it = begin(); it != end(); it++)
      delete *it;
}

Gdk::Color Shield::get_default_color_for_no(int player_no)
{
  Gdk::Color c;
  switch (player_no % MAX_PLAYERS)
    {
    case Shield::WHITE: c.set_rgb_p(252.0/255.0,252.0/255.0,252.0/255.0); break;
    //case 1: color.r = 80; color.b = 28; color.g = 172; break;
    case Shield::GREEN: c.set_rgb_p(80.0/255.0, 194.0/255.0, 28.0/255.0); break;
    case Shield::YELLOW: c.set_rgb_p(252.0/255.0,236.0/255.0,32.0/255.0); break;
    case Shield::LIGHT_BLUE: c.set_rgb_p(0,252.0/255.0,252.0/255.0); break;
    case Shield::RED: c.set_rgb_p(252.0/255.0,160.0/255.0,0);break;
    case Shield::DARK_BLUE: 
		      c.set_rgb_p(44.0/255.0,184.0/255.0,252.0/255.0); break;
    case Shield::ORANGE: c.set_rgb_p(196.0/255.0, 28.0/255.0, 0); break;
    case Shield::BLACK: c.set_rgb_p(0,0,0); break;
    }
    
    return c;
}

Gdk::Color Shield::get_default_color_for_neutral()
{
  Gdk::Color color;
  color.set_rgb_p(204.0/255.0,204.0/255.0,204.0/255.0);
  return color;
}

struct rgb_shift Shield::getMaskColorShifts()
{
    // This is a bit tricky. The color values we return here encode additional
    // shifts that are performed when getting the color. I.e. a color value for
    // red of 8 means that the red color is completely ignored.

    // For each color component, find the n where 2^n best describes the color.
    // The mask value then is (8-n).
    struct rgb_shift shifts;

    guint32 r,g,b;
    guint8 target_red = guint8(d_color.get_red_p() * 255.0);
    for (int i = 8, diff = 257; i > 0; i--)
    {
        int color = 1<<i;
        int tmp_diff = abs(target_red - color);

	r = 8 - (i + 1);

        if (diff < tmp_diff)
            break;
        else
            diff = tmp_diff;
    }
        
    guint8 target_green = guint8(d_color.get_green_p() * 255.0);
    for (int i = 8, diff = 257; i > 0; i--)
    {
        int color = 1<<i;
        int tmp_diff = abs(target_green - color);
	g = 8 - (i + 1);

        if (diff < tmp_diff)
            break;
        else
            diff = tmp_diff;
    }
        
    guint8 target_blue = guint8(d_color.get_blue_p() * 255.0);
    for (int i = 8, diff = 257; i > 0; i--)
    {
        int color = 1<<i;
        int tmp_diff = abs(target_blue - color);
	b = 8 - (i + 1);

        if (diff < tmp_diff)
            break;
        else
            diff = tmp_diff;
    }
    //c.set_rgb_p(r,g,b);
    shifts.r = r;
    shifts.g = g;
    shifts.b = b;
    return shifts;
}

Gdk::Color Shield::getMaskColor() const
{
    // This is a bit tricky. The color values we return here encode additional
    // shifts that are performed when getting the color. I.e. a color value for
    // red of 8 means that the red color is completely ignored.

    // For each color component, find the n where 2^n best describes the color.
    // The mask value then is (8-n).
    Gdk::Color c;

    guint32 r,g,b;
    for (int i = 8, diff = 257; i > 0; i--)
    {
        int color = 1<<i;
        int tmp_diff = abs(int(d_color.get_red_p() *255.0) - color);

	r = 8 - (i + 1);

        if (diff < tmp_diff)
            break;
        else
            diff = tmp_diff;
    }
        
    for (int i = 8, diff = 257; i > 0; i--)
    {
        int color = 1<<i;
        int tmp_diff = abs(int(d_color.get_green_p() *255.0) - color);
	g = 8 - (i + 1);

        if (diff < tmp_diff)
            break;
        else
            diff = tmp_diff;
    }
        
    for (int i = 8, diff = 257; i > 0; i--)
    {
        int color = 1<<i;
        int tmp_diff = abs(int(d_color.get_blue_p()*255.0) - color);
	b = 8 - (i + 1);

        if (diff < tmp_diff)
            break;
        else
            diff = tmp_diff;
    }
        
    c.set_rgb_p(r/255.0,g/255.0,b/255.0);
    return c;
}
