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

#include "signpostlist.h"

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

Signpostlist* Signpostlist::s_instance=0;

Signpostlist* Signpostlist::getInstance()
{
    if (s_instance == 0)
        s_instance = new Signpostlist();

    return s_instance;
}

Signpostlist* Signpostlist::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new Signpostlist(helper);
    return s_instance;
}

void Signpostlist::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

Signpostlist::Signpostlist()
{
}

Signpostlist::Signpostlist(XML_Helper* helper)
{
    helper->registerTag("signpost", sigc::mem_fun(this, &Signpostlist::load));
}

bool Signpostlist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("signpostlist");

    for (const_iterator it = begin(); it != end(); it++)
        retval &= (*it).save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

bool Signpostlist::load(std::string tag, XML_Helper* helper)
{
    if (tag != "signpost")
    //what has happened?
        return false;
    
    Signpost s(helper);
    push_back(s);

    return true;
}

