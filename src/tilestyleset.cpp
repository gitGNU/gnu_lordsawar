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

#include <SDL_image.h>
#include <sigc++/functors/mem_fun.h>

#include "tilestyleset.h"

#include "GraphicsCache.h"
#include "xmlhelper.h"

using namespace std;

#include <iostream>

        
TileStyleSet::TileStyleSet()
{
}

TileStyleSet::TileStyleSet(XML_Helper *helper)
{
  helper->getData(d_name, "name"); 
}

TileStyleSet::~TileStyleSet()
{
  for (unsigned int i=0; i < size(); i++)
    delete (*this)[i];
}

bool TileStyleSet::save(XML_Helper *helper)
{
  bool retval = true;

  retval &= helper->openTag("tilestyleset");
  retval &= helper->saveData("name", d_name);
  for (TileStyleSet::iterator i = begin(); i != end(); ++i)
    retval &= (*i)->save(helper);
  retval &= helper->closeTag();

  return retval;
}
// End of file
