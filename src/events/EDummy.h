
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

#ifndef EDUMMY_H
#define EDUMMY_H

#include "Event.h"

/** This is a "dummy" event.
  *
  * "Dummy" because it is usually not raised at all. However, especially with
  * a counter condition and an event triggering reaction, it can be put to
  * serious uses to keep track of a player's progress. A scenario to show this:
  *
  * Imagine a scenario where the player has to search 3 out of five ruins to
  * win. How do you keep track how many ruins the player has searched in a
  * simple fashion? Solution: Take a dummy event with a counter condition and
  * assign reactions that trigger this event to all RuinSearch events that you
  * associate with the ruins to be searched. Then, every time the player
  * searches a ruin, the dummy event is triggered; when being triggered the
  * third time, the dummy event ends the game.
  */

class EDummy : public Event
{
    public:
        EDummy();
        EDummy(XML_Helper* helper);
        ~EDummy();

        //! Saves the event data
        bool save(XML_Helper* helper) const;

        //! triggers the event
        void trigger();
};

#endif //EDUMMY_H
