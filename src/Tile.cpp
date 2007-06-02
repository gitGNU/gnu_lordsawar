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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include "Tile.h"
#include "defs.h"
#include <iostream>

using namespace std;

Tile::Tile(XML_Helper* helper)
{
    int i;
    
    for (i = 0; i < 8*4; i++)
        d_surface[i] = 0;

    helper->getData(d_name, "name");
    helper->getData(d_moves, "moves");
    helper->getData(i, "type");
    d_type = static_cast<Tile::Type>(i);

    helper->getData(i, "red");      d_color.r = i;
    helper->getData(i, "green");    d_color.g = i;
    helper->getData(i, "blue");     d_color.b = i;

    helper->getData(i, "pattern");
    d_pattern = static_cast<Tile::Pattern>(i);
    
    if (d_pattern != Tile::SOLID)
      {
        helper->getData(i, "2nd_red");      d_second_color.r = i;
        helper->getData(i, "2nd_green");    d_second_color.g = i;
        helper->getData(i, "2nd_blue");     d_second_color.b = i;
        if (d_pattern != Tile::STIPPLED && d_pattern != Tile::SUNKEN)
          {
            helper->getData(i, "3rd_red");      d_third_color.r = i;
            helper->getData(i, "3rd_green");    d_third_color.g = i;
            helper->getData(i, "3rd_blue");     d_third_color.b = i;
          }
      }
}
    
Tile::~Tile()
{
    for (int i = 0; i < 8*4; i++)
    {
        SDL_FreeSurface(d_surface[i]);
    }
}

void Tile::printDebugInfo() const
{
    cout << "name = " << d_name << endl;
    cout << "moves = " << d_moves << endl;
}

// End of file
