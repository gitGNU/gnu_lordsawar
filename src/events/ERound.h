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

#ifndef EROUND_H
#define EROUND_H

#include "Event.h"

class Player;

/** This event is raised when a specific round starts.
  *
  * For more info about events, see Event.h
  */

class ERound : public Event
{
    public:
        //! Default constructor with round which we look for
        ERound(Uint32 round);

        //! Loading constructor
        ERound(XML_Helper* helper);
        ~ERound();

        //! Saves the data to savegame given by helper
        bool save(XML_Helper* helper) const;
        
        //! Callback for triggering the event
        void trigger(Player* p);

        
        //! Returns the round which the event awaits
        Uint32 getRound() const {return d_round;}

        //! Sets the round what we await
        void setRound(Uint32 round) {d_round = round;}


        /** This signal is connected elsewhere and is used by the event
          * to get the current round number.
          */
        SigC::Signal0<Uint32> sgettingRound;

    private:
        Uint32 d_round;
};

#endif //EROUND_H
