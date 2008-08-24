//  Copyright (C) 2007, 2008 Ben Asselstine
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

#include "armyset.h"
#include "File.h"
#include "defs.h"
#include "GraphicsCache.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

#define DEFAULT_ARMY_TILE_SIZE 40
Armyset::Armyset(Uint32 id, std::string name)
	: d_id(id), d_name(name), d_dir(""), d_tilesize(DEFAULT_ARMY_TILE_SIZE)
{
  d_ship = NULL;
  d_shipmask = NULL;
  d_standard = NULL;
  d_standard_mask = NULL;
}

Armyset::Armyset(XML_Helper *helper)
    : d_id(0), d_name(""), d_dir(""), d_tilesize(DEFAULT_ARMY_TILE_SIZE)
{
  d_ship = NULL;
  d_shipmask = NULL;
  d_standard = NULL;
  d_standard_mask = NULL;
  helper->getData(d_id, "id");
  helper->getData(d_name, "name");
  helper->getData(d_tilesize, "tilesize");
  helper->registerTag("army", sigc::mem_fun((*this), 
					      &Armyset::loadArmyTemplate));
}

Armyset::~Armyset()
{
  for (iterator it = begin(); it != end(); it++)
      delete *it;
}

bool Armyset::loadArmyTemplate(string tag, XML_Helper* helper)
{
    if (tag == "army")
      {
	std::string s;
	// First step: Load the army data
	Army* a = new Army(helper, Army::TYPE);
	a->setArmyset(d_id, size());
	push_back(a);
      }
    return true;
}

bool Armyset::save(XML_Helper* helper)
{
    bool retval = true;

    retval &= helper->openTag("armyset");

    retval &= helper->saveData("id", d_id);
    retval &= helper->saveData("name", d_name);
    retval &= helper->saveData("tilesize", d_tilesize);

    for (const_iterator it = begin(); it != end(); it++)
        (*it)->save(helper, Army::TYPE);
    
    retval &= helper->closeTag();

    return retval;
}

Army * Armyset::lookupArmyByType(Uint32 army_type)
{
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getType() == army_type)
	return *it;
    }
  return NULL;
}
