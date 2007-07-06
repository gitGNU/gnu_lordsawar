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

#ifndef PORT_H
#define PORT_H

#include "Location.h"

/** A port is just a thing on the map to differentiate space
  */

class Port: public Location
{
    public:
        /** Default constructor
          * 
          * @param pos          the location of the port
          */
        Port(Vector<int> pos, std::string name = "Port");

        //! Loading constructor. See XML_Helper
        Port(XML_Helper* helper);
        Port(const Port&);
        ~Port();

        //! Save the port data.
        bool save(XML_Helper* helper) const;

};

#endif // PORT_H
