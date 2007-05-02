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

#include <sigc++/functors/mem_fun.h>

#include "stonelist.h"

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

Stonelist* Stonelist::s_instance=0;

Stonelist* Stonelist::getInstance()
{
    if (s_instance == 0)
        s_instance = new Stonelist();

    return s_instance;
}

Stonelist* Stonelist::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new Stonelist(helper);
    return s_instance;
}

void Stonelist::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

Stonelist::Stonelist()
{
}

Stonelist::Stonelist(XML_Helper* helper)
{
    helper->registerTag("stone", sigc::mem_fun(this, &Stonelist::load));
}

bool Stonelist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("stonelist");

    for (const_iterator it = begin(); it != end(); it++)
        retval &= (*it).save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

bool Stonelist::load(std::string tag, XML_Helper* helper)
{
    if (tag != "stone")
    //what has happened?
        return false;
    
    Stone s(helper);
    push_back(s);

    return true;
}

