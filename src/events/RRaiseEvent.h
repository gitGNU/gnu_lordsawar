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

#ifndef RRAISEEVENT_H
#define RRAISEEVENT_H

#include <sigc++/signal.h>
#include "Reaction.h"

class Event;

/** Reaction that raises a specific event
  *
  * This reaction can be quite handy to trigger a cascade of other events
  * For implementation purposes, however, this reaction can only trigger
  * dummy events, so put stuff that can be triggered by multiple actions in
  * reaction assigned to a dummy event.
  */

// TODO: Maybe extend this class to trigger non-dummy events as well?

class RRaiseEvent : public Reaction
{
    public:
        //! Initialises reaction with id of the event to be triggered
        RRaiseEvent(Uint32 event);

        //! Loading constructor
        RRaiseEvent(XML_Helper* helper);
        ~RRaiseEvent();

        //! Saves the reaction data
        bool save(XML_Helper* helper) const;

        //! Triggers the reaction
        bool trigger() const;


        //! Returns the event to be triggered
        Uint32 getEvent() const {return d_event;}

        //! Sets the event to be triggered
        void setEvent(Uint32 event) {d_event = event;}

        
        //! The signal is connected out of scope of this class with a function
        //! that returns the list of all events.
        static sigc::signal<std::list<Event*> > sgettingEvents;

    private:
        Uint32 d_event;
};

#endif  //RRAISEEVENT_H
