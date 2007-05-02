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

#include "RDelUnit.h"
#include "../playerlist.h"
#include "../stacklist.h"
#include "../army.h"
#include "../stack.h"

RDelUnit::RDelUnit(Uint32 army)
    :Reaction(DELUNIT), d_army(army)
{
}

RDelUnit::RDelUnit(XML_Helper* helper)
    :Reaction(helper)
{
    d_type = DELUNIT;
    helper->getData(d_army, "army");
}

RDelUnit::~RDelUnit()
{
}

bool RDelUnit::save(XML_Helper* helper) const
{
    bool retval = true;
    
    retval &= helper->openTag("reaction");
    retval &= helper->saveData("army", d_army);
    retval &= Reaction::save(helper);
    retval &= helper->closeTag();
    
    return retval ;
}

bool RDelUnit::trigger() const
{
    if (!check())
        return false;
    
    // Unfortunately, we have to catch the case of the army being the only army
    // in the stack and so forth, so we have to duplicate the code from getArmy()
    Playerlist* plist = Playerlist::getInstance();
    
    for (Playerlist::iterator pit = plist->begin(); pit != plist->end(); pit++)
    {
        Stacklist* slist = (*pit)->getStacklist();
        for (Stacklist::iterator sit = slist->begin(); sit != slist->end(); sit++)
        {
            Stack* s = (*sit);
            for (Stack::iterator it = s->begin(); it != s->end(); it++)
                if ((*it)->getId() == d_army)
                {
                    s->flErase(it);
                    if (s->size() == 0)
                        (*pit)->deleteStack(s);
                    return true;
                }
        }
    }

    return false;
}

Army* RDelUnit::getArmy()
{
    Playerlist* plist = Playerlist::getInstance();
    
    for (Playerlist::iterator pit = plist->begin(); pit != plist->end(); pit++)
    {
        Stacklist* slist = (*pit)->getStacklist();
        for (Stacklist::iterator sit = slist->begin(); sit != slist->end(); sit++)
        {
            Stack* s = (*sit);
            for (Stack::iterator it = s->begin(); it != s->end(); it++)
                if ((*it)->getId() == d_army)
                    return (*it);
        }
    }

    return 0;
}
