// Copyright (C) 2009, 2014 Ben Asselstine
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

#ifndef SET_H
#define SET_H
#include <string>
#include "File.h"

class Set
{
public:
    enum Origin { SYSTEM, PERSONAL, SCENARIO, NONE};
    Set();
    ~Set();
    Set(const Set &s);

    Set::Origin getOrigin() {return origin;};
    void setOrigin(Set::Origin origination) {origin = origination;};

    std::string getDirectory() const {return dir;};
    void setDirectory(std::string d) {dir = File::add_slash_if_necessary(d);};

    std::string getFile(std::string file) const;
private:

    Origin origin;
    std::string dir;
};

#endif
