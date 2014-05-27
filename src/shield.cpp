//  Copyright (C) 2008, 2009, 2010, 2011, 2014 Ben Asselstine
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

//#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "shield.h"
#include "GraphicsCache.h"
#include "xmlhelper.h"
#include "ucompose.hpp"
#include "shieldset.h"
#include "tarhelper.h"

std::string Shield::d_tag = "shield";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Shield::Shield(XML_Helper* helper)
{
  helper->getData(d_owner, "owner");
  helper->getData(d_color, "color");
}

Shield::Shield(const Shield& s)
: d_owner(s.d_owner), d_color(s.d_color)
{
  for (const_iterator it = s.begin(); it != s.end(); it++)
    push_back(new ShieldStyle(*(*it)));
}

Shield::Shield(Shield::Colour owner, Gdk::RGBA color)
{
  d_owner = guint32(owner);
  d_color = color;
}

Shield::~Shield()
{
  for (iterator it = begin(); it != end(); it++)
      delete *it;
}

Gdk::RGBA Shield::get_default_color_for_no(int player_no)
{
  Gdk::RGBA c;
  switch (player_no % MAX_PLAYERS)
    {
    case Shield::WHITE: c.set_rgba(252.0/255.0,252.0/255.0,252.0/255.0); break;
    case Shield::GREEN: c.set_rgba(80.0/255.0, 195.0/255.0, 28.0/255.0); break;
    case Shield::YELLOW: c.set_rgba(252.0/255.0,236.0/255.0,32.0/255.0); break;
    //case Shield::DARK_BLUE: c.set_rgba(0,252.0/255.0,252.0/255.0); break;
    case Shield::DARK_BLUE: c.set_rgba(22.0/255.0,92.0/255.0, 252.0/255.0); break;
    case Shield::ORANGE: c.set_rgba(252.0/255.0,160.0/255.0,0);break;
    case Shield::LIGHT_BLUE: 
		      c.set_rgba(44.0/255.0,184.0/255.0,252.0/255.0); break;
    case Shield::RED: c.set_rgba(196.0/255.0, 28.0/255.0, 0); break;
    case Shield::BLACK: c.set_rgba(0,0,0); break;
    }
    
    return c;
}

Gdk::RGBA Shield::get_default_color_for_neutral()
{
  Gdk::RGBA color;
  color.set_rgba(204.0/255.0,204.0/255.0,204.0/255.0);
  return color;
}

std::string Shield::colourToString(const Shield::Colour c)
{
  switch (c)
    {
    case Shield::WHITE:
      return "Shield::WHITE"; break;
    case Shield::GREEN:
      return "Shield::GREEN"; break;
    case Shield::YELLOW:
      return "Shield::YELLOW"; break;
    case Shield::LIGHT_BLUE:
      return "Shield::LIGHT_BLUE"; break;
    case Shield::RED:
      return "Shield::RED"; break;
    case Shield::DARK_BLUE:
      return "Shield::DARK_BLUE"; break;
    case Shield::ORANGE:
      return "Shield::ORANGE"; break;
    case Shield::BLACK:
      return "Shield::BLACK"; break;
    case Shield::NEUTRAL:
      return "Shield::NEUTRAL"; break;
    }
  return "Shield::NEUTRAL";
}

bool Shield::save(XML_Helper *helper) const
{
  bool retval = true;

  retval &= helper->openTag(d_tag);
  retval &= helper->saveData("owner", d_owner);
  retval &= helper->saveData("color", d_color);
  for (const_iterator it = begin(); it != end(); it++)
    (*it)->save(helper);
  retval &= helper->closeTag();
  return retval;
}
	
void Shield::instantiateImages(Shieldset *s, bool &broken)
{
  broken = false;
  Tar_Helper t(s->getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return;
  int count = 0;
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getImageName().empty() == false)
        {
          std::string pngfile = t.getFile((*it)->getImageName() + ".png", 
                                          broken);
          if (broken == false)
            {
              (*it)->instantiateImages(pngfile, s, broken);
              File::erase(pngfile);
            }
          else
            return;
        }
      count++;
    }
  t.Close();
}

void Shield::uninstantiateImages()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->uninstantiateImages();
}

ShieldStyle *Shield::getFirstShieldstyle(ShieldStyle::Type type)
{
  for (iterator i = begin(); i != end(); i++)
    {
      if (ShieldStyle::Type((*i)->getType()) == type)
	return *i;
    }
  return NULL;
}
    
guint32 Shield::get_next_shield(guint32 colour)
{
  if (colour == Shield::NEUTRAL)
    {
      colour = Shield::WHITE;
      return colour;
    }
  colour++;
  return colour;
}
