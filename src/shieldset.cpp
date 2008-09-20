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

#include <iostream>
#include <expat.h>
#include <SDL.h>
#include "rectangle.h"
#include <sigc++/functors/mem_fun.h>

#include "shieldset.h"
#include "shieldstyle.h"
#include "File.h"

using namespace std;

std::string Shieldset::d_tag = "shieldset";

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

Shieldset::Shieldset(XML_Helper *helper)
    : d_dir("")
{
  helper->getData(d_name, "name");
  helper->getData(d_small_width, "small_width");
  helper->getData(d_small_height, "small_height");
  helper->getData(d_medium_width, "medium_width");
  helper->getData(d_medium_height, "medium_height");
  helper->getData(d_large_width, "large_width");
  helper->getData(d_large_height, "large_height");
  helper->registerTag(Shield::d_tag, 
		      sigc::mem_fun((*this), &Shieldset::loadShield));
  helper->registerTag(ShieldStyle::d_tag, sigc::mem_fun((*this), 
						   &Shieldset::loadShield));
}

Shieldset::~Shieldset()
{
  for (iterator it = begin(); it != end(); it++)
      delete *it;
}

bool Shieldset::loadShield(string tag, XML_Helper* helper)
{
    if (tag == Shield::d_tag)
      {
	Shield* sh = new Shield(helper);
	push_back(sh);
	return true;
      }
    if (tag == ShieldStyle::d_tag)
      {
	ShieldStyle *sh = new ShieldStyle(helper);
	(*back()).push_back(sh);
	return true;
      }
    return false;
}

ShieldStyle * Shieldset::lookupShieldByTypeAndColour(Uint32 type, Uint32 owner)
{
  for (iterator it = begin(); it != end(); it++)
    {
      for (Shield::iterator i = (*it)->begin(); i != (*it)->end(); i++)
	{
	  if ((*i)->getType() == type && (*it)->getOwner() == owner)
	    return *i;
	}
    }
  return NULL;
}

SDL_Color Shieldset::getColor(Uint32 owner)
{
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getOwner() == owner)
	return (*it)->getColor();
    }
  SDL_Color def;
  memset (&def, 0, sizeof (def));
  return def;
}

SDL_Color Shieldset::getMaskColor(Uint32 owner)
{
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getOwner() == owner)
	return (*it)->getMaskColor();
    }
  SDL_Color def;
  memset (&def, 0, sizeof (def));
  return def;
}

