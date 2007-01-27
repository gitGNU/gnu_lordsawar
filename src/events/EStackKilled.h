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

#ifndef ESTACKKILLED_H
#define ESTACKKILLED_H

#include "Event.h"

class Stack;

/** Event that is raised when a stack dies.
  * 
  * This event is quite similar to EArmyKilled, but checks if a certain _stack_
  * has been killed. Be careful with applying this event, though. Advanced 
  * computer players can join stacks, resulting in one of the stacks vanishing
  * or refill stacks in cities. So make sure that the owning player is "dumb",
  * so that the destruction of this stack has some "value".
  */

class EStackKilled : public Event
{
    public:
        //! Initialize event with a given stack id
        EStackKilled(Uint32 stack);

        //! Loading constructor
        EStackKilled(XML_Helper* helper);
        ~EStackKilled();

        //! Saves event data
        bool save(XML_Helper* helper) const;

        //! Initialises event, especially connects signals
        void init();

        
        //! Returns the stack that the event watches
        Stack* getStack() const;

        //! Returns the id of the stack only
        Uint32 getStackId() const {return d_stack;}

        //! Sets the stack that the event watches
        void setStack(Uint32 stack) {d_stack = stack;}

    private:
        Uint32 d_stack;

        //! callback that triggers the event
        void trigger(Stack* s);
};

#endif //ESTACKKILLED_H
