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

#include "RRaiseEvent.h"
#include "EDummy.h"

sigc::signal<std::list<Event*> > RRaiseEvent::sgettingEvents;

RRaiseEvent::RRaiseEvent(Uint32 event)
    :Reaction(RAISEEVENT), d_event(event)
{
}

RRaiseEvent::RRaiseEvent(XML_Helper* helper)
    :Reaction(helper)
{
    d_type = RAISEEVENT;
    helper->getData(d_event, "event");
}

RRaiseEvent::~RRaiseEvent()
{
}

bool RRaiseEvent::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("reaction");
    retval &= helper->saveData("event", d_event);
    retval &= Reaction::save(helper);
    retval &= helper->closeTag();

    return retval;
}

bool RRaiseEvent::trigger() const
{
    if (!check())
        return false;

    std::list<Event*> evlist = sgettingEvents.emit();

    for (std::list<Event*>::iterator it = evlist.begin(); it != evlist.end(); it++)
        if ((*it)->getId() == d_event && (*it)->getType() == Event::DUMMY)
        {
            (dynamic_cast<EDummy*>(*it))->trigger();
            return true;
        }

    return false;
}
