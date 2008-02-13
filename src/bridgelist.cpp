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
    helper->registerTag("bridge", sigc::mem_fun(this, &Bridgelist::load));
}

bool Bridgelist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("bridgelist");

    for (const_iterator it = begin(); it != end(); it++)
        retval &= (*it).save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

bool Bridgelist::load(std::string tag, XML_Helper* helper)
{
    if (tag != "bridge")
    //what has happened?
        return false;
    
    Bridge s(helper);
    push_back(s);

    return true;
}

