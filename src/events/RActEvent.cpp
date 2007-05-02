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

#include "RActEvent.h"
#include "Event.h"

sigc::signal<std::list<Event*> > RActEvent::sgettingEvents;


RActEvent::RActEvent(Uint32 event, bool activate)
    :Reaction(ACTEVENT), d_event(event), d_activate(activate)
{
}

RActEvent::RActEvent(XML_Helper* helper)
    :Reaction(helper)
{
    d_type = ACTEVENT;
    helper->getData(d_event, "event");
    helper->getData(d_activate, "activate");
}

RActEvent::~RActEvent()
{
}

bool RActEvent::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("reaction");
    retval &= helper->saveData("event", d_event);
    retval &= helper->saveData("activate", d_activate);
    retval &= Reaction::save(helper);
    retval &= helper->closeTag();

    return retval;
}

bool RActEvent::trigger() const
{
    if (!check())
        return false;

    std::list<Event*> evlist = sgettingEvents.emit();
    for (std::list<Event*>::iterator it = evlist.begin(); it != evlist.end(); it++)
        if ((*it)->getId() == d_event)
        {
            (*it)->setActive(d_activate);
            return true;
        }

    return false;
}
