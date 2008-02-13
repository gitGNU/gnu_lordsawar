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

#ifndef SIGNPOST_H
#define SIGNPOST_H

#include <string>
#include "Location.h"

/** A signpost is the place where a human player can read a relevant message.
  */

class Signpost: public Location
{
    public:
        /** Default constructor
          * 
          * @param pos          the location of the signpost
          * @param name         the contents of the sign
          */
        Signpost(Vector<int> pos, std::string name = "nowhere");

        //! Loading constructor. See XML_Helper
        Signpost(XML_Helper* helper);
        Signpost(const Signpost&);
        ~Signpost();

        //! Save the signpost data.
        bool save(XML_Helper* helper) const;

};

#endif // SIGNPOST_H
