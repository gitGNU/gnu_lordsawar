//  Copyright (C) 2008, Ben Asselstine
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

std::string Shield::d_tag = "shield";

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

Shield::Shield(XML_Helper* helper)
{
  helper->getData(d_owner, "owner");
  std::string s;
  helper->getData(s, "color");
  int i;
  std::istringstream scolor(s);
  scolor >> i; d_color.r = i;
  scolor >> i; d_color.g = i;
  scolor >> i; d_color.b = i;
}

Shield::~Shield()
{
  for (iterator it = begin(); it != end(); it++)
      delete *it;
}

SDL_Color Shield::get_default_color_for_no(int player_no)
{
    SDL_Color color;
    color.r = color.b = color.g = color.unused = 0;
    switch (player_no % MAX_PLAYERS)
    {
    case Shield::WHITE: color.r = 252; color.b = 252; color.g = 252; break;
    //case 1: color.r = 80; color.b = 28; color.g = 172; break;
    case Shield::GREEN: color.r = 80; color.b = 28; color.g = 193; break;
    case Shield::YELLOW: color.r = 252; color.b = 32; color.g = 236; break;
    case Shield::LIGHT_BLUE: color.r = 0; color.b = 252; color.g = 252; break;
    case Shield::RED: color.r = 252; color.b = 0; color.g = 160;break;
    case Shield::DARK_BLUE: color.r = 44; color.b = 252; color.g = 184; break;
    case Shield::ORANGE: color.r = 196; color.b = 0; color.g = 28; break;
    case Shield::BLACK: color.r = color.g = color.b = 0; break;
    }
    
    return color;
}

SDL_Color Shield::get_default_color_for_neutral()
{
    SDL_Color color;
    color.r = color.g = color.b = 204; color.unused = 0;
    return color;
}

SDL_Color Shield::getMaskColor() const
{
    // This is a bit tricky. The color values we return here encode additional
    // shifts that are performed when getting the color. I.e. a color value for
    // red of 8 means that the red color is completely ignored.

    // For each color component, find the n where 2^n best describes the color.
    // The mask value then is (8-n).
    SDL_Color c;
    c.r = c.g = c.b = 0;

    for (int i = 8, diff = 257; i > 0; i--)
    {
        int color = 1<<i;
        int tmp_diff = abs(d_color.r - color);
        c.r = 8 - (i+1);

        if (diff < tmp_diff)
            break;
        else
            diff = tmp_diff;
    }
        
    for (int i = 8, diff = 257; i > 0; i--)
    {
        int color = 1<<i;
        int tmp_diff = abs(d_color.g - color);
        c.g = 8 - (i+1);

        if (diff < tmp_diff)
            break;
        else
            diff = tmp_diff;
    }
        
    for (int i = 8, diff = 257; i > 0; i--)
    {
        int color = 1<<i;
        int tmp_diff = abs(d_color.b - color);
        c.b = 8 - (i+1);

        if (diff < tmp_diff)
            break;
        else
            diff = tmp_diff;
    }
        
    return c;
}
