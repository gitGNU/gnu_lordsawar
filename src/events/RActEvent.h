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

#ifndef RACTEVENT_H
#define RACTEVENT_H

#include <sigc++/signal.h>
#include "Reaction.h"

class Event;

/** Reaction that activates or deactivates other events.
  * 
  * The idea behind this reaction is that some events should become available
  * only at certain parts of the game. Since creating new events is way too
  * complicated, you just disable them in the beginning and activate them after
  * certain key actions have happened.
  *
  * Another use for this class are persistent events. If one of the reactions of
  * an event is of this class and enables the event again, it will "live"
  * forever. May also be useful.
  */

class RActEvent : public Reaction
{
    public:
        /** Standard constructor
          *
          * @param event    the id of the event we deal with
          * @param activate whether the event should be enabled or disabled
          */
        RActEvent(Uint32 event, bool activate);

        //! Loading constructor
        RActEvent(XML_Helper* helper);
        ~RActEvent();

        //! Saves the reaction data
        bool save(XML_Helper* helper) const;

        //! Triggers the reaction
        bool trigger() const;


        //! Returns the event to be modified
        Uint32 getEvent() const {return d_event;}

        //! Sets the event to be modified
        void setEvent(Uint32 event) {d_event = event;}

        //! Returns whether the event is activated or deactivated
        bool getActivate() const {return d_activate;}

        //! Sets whether the event is activated or deactivated
        void setActivate(bool activate) {d_activate = activate;}


        //! Signal can be used to get the list of all events (connected in W_Edit)
        static sigc::signal<std::list<Event*> > sgettingEvents;

    private:
        Uint32 d_event;
        bool d_activate;
};

#endif //RACTEVENT_H
