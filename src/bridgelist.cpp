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

#include <sigc++/functors/mem_fun.h>

#include "bridgelist.h"
#include "xmlhelper.h"

std::string Bridgelist::d_tag = "bridgelist";
//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

Bridgelist* Bridgelist::s_instance=0;

Bridgelist* Bridgelist::getInstance()
{
    if (s_instance == 0)
        s_instance = new Bridgelist();

    return s_instance;
}

Bridgelist* Bridgelist::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new Bridgelist(helper);
    return s_instance;
}

void Bridgelist::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

Bridgelist::Bridgelist()
{
}

Bridgelist::Bridgelist(XML_Helper* helper)
{
    helper->registerTag(Bridge::d_tag, sigc::mem_fun(this, &Bridgelist::load));
}

bool Bridgelist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(Bridgelist::d_tag);

    for (const_iterator it = begin(); it != end(); it++)
        retval &= (*it).save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

bool Bridgelist::load(std::string tag, XML_Helper* helper)
{
    if (tag != Bridge::d_tag)
    //what has happened?
        return false;
    
    Bridge s(helper);
    push_back(s);

    return true;
}

int Bridgelist::calculateType(Vector<int> t)
{
    // examine neighbour tiles to discover whether there's a bridge on them
    bool u = Bridgelist::getInstance()->getObjectAt(t + Vector<int>(0, -1));
    bool b = Bridgelist::getInstance()->getObjectAt(t + Vector<int>(0, 1));
    bool l = Bridgelist::getInstance()->getObjectAt(t + Vector<int>(-1, 0));
    bool r = Bridgelist::getInstance()->getObjectAt(t + Vector<int>(1, 0));

    if (u)
      return Bridge::CONNECTS_TO_NORTH;
    if (b)
      return Bridge::CONNECTS_TO_SOUTH;
    if (r)
      return Bridge::CONNECTS_TO_EAST;
    if (l)
      return Bridge::CONNECTS_TO_WEST;
    return Bridge::CONNECTS_TO_NORTH;
}

