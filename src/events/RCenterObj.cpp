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

#include "RCenterObj.h"
#include "../templelist.h"
#include "../ruinlist.h"
#include "../citylist.h"
#include "../playerlist.h"
#include "../stacklist.h"
#include "../army.h"

sigc::signal<void, Vector<int> > RCenterObj::scentering;


RCenterObj::RCenterObj(Uint32 id)
    :Reaction(CENTEROBJ), d_object(id)
{
}

RCenterObj::RCenterObj(XML_Helper* helper)
    :Reaction(helper)
{
    d_type = CENTEROBJ;
    helper->getData(d_object, "object");
}

RCenterObj::~RCenterObj()
{
}

bool RCenterObj::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("reaction");
    retval &= helper->saveData("object", d_object);
    retval &= Reaction::save(helper);
    retval &= helper->closeTag();
            
    return retval;
}

bool RCenterObj::trigger() const
{
    if (!check())
        return false;
    
    // First, we need to find the object. As we do not know which type the object
    // has, we need to search all lists:

    // the templelist ...
    Templelist* tl = Templelist::getInstance();
    for (Templelist::iterator it = tl->begin(); it != tl->end(); it++)
        if ((*it).getId() == d_object)
        {
            scentering.emit((*it).getPos());
            return true;
        }

    // ... the ruinlist ...
    Ruinlist* rl = Ruinlist::getInstance();
    for (Ruinlist::iterator it = rl->begin(); it != rl->end(); it++)
        if ((*it).getId() == d_object)
        {
            scentering.emit((*it).getPos());
            return true;
        }

    // ... the citylist ...
    Citylist* cl = Citylist::getInstance();
    for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
        if ((*it).getId() == d_object)
        {
            scentering.emit((*it).getPos());
            return true;
        }

    // ... and all the stacklists.
    Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::iterator pit = pl->begin(); pit != pl->end(); pit++)
    {
        Stacklist* sl = (*pit)->getStacklist();
        for (Stacklist::iterator it = sl->begin(); it != sl->end(); it++)
        {
            if ((*it)->getId() == d_object)
            {
                scentering.emit((*it)->getPos());
                return true;
            }

            // now also check for armies (stacks may change)
            for (Stack::iterator sit = (*it)->begin(); sit != (*it)->end(); sit++)
                if ((*sit)->getId() == d_object)
                {
                    scentering.emit((*it)->getPos());
                    return true;
                }
        }
        
    }

    // We did not find the object; maybe it was destroyed (e.g. stacks) or so.
    // Ignore the command, then.
    return false;
}
