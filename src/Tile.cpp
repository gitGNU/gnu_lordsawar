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
