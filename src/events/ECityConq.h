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

#ifndef ECITYCONQ_H
#define ECITYCONQ_H

#include "Event.h"
#include "../stack.h"

class City;

/** This event is raised if a specified city is conquered by someone.
  *
  * See Event.h for more information about the event structures.
  */


class ECityConq : public Event
{
    public:
        //Default constructor
        ECityConq(Uint32 city);

        //! Loading constructor
        ECityConq(XML_Helper* helper);
        virtual ~ECityConq();

        //! Saves the event data
        bool save(XML_Helper* helper) const;

        //! Initialises the event (Links trigger function to all signals)
        void init();

        
        //! Returns the city which we observe for occupation.
        Uint32 getCity() const {return d_city;}

        //! Sets the city to be observed
        void setCity(Uint32 city) {d_city = city;}

    private:
        Uint32 d_city;
        
        //! Callback which triggers the event
        void trigger(City* city, Stack *stack);
};
        

#endif //ECITYCONQ_H
