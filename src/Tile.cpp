// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2002, 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2007, 2008 Ben Asselstine
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

#include "Tile.h"
#include "defs.h"
#include <iostream>

using namespace std;

Tile::Tile(XML_Helper* helper)
{
    int i;
    
    helper->getData(d_name, "name");
    helper->getData(d_moves, "moves");
    helper->getData(i, "type");
    d_type = static_cast<Tile::Type>(i);

}

bool Tile::save(XML_Helper *helper)
{
  bool retval = true;

  retval &= helper->openTag("tile");
  retval &= helper->saveData("name", d_name);
  retval &= helper->saveData("moves", d_moves);
  retval &= helper->saveData("type", d_type);
  retval &= helper->openTag("smallmap");
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
    case RANDOMIZED: case TABLECLOTH:
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
  for (Tile::iterator i = begin(); i != end(); ++i)
    retval &= (*i)->save(helper);
  retval &= helper->closeTag();

  return retval;
}
    
Tile::~Tile()
{
    for (unsigned int i=0; i < size(); i++)
        delete (*this)[i];
}

void Tile::instantiatePixmaps(std::string tileset, Uint32 tilesize)
{
  for (unsigned int i=0; i < size(); i++)
    (*this)[i]->instantiatePixmaps(tileset, tilesize);
}
// End of file
