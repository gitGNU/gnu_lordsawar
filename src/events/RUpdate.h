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

#ifndef RUPDATE_H
#define RUPDATE_H

#include <sigc++/signal.h>

#include "Reaction.h"

/** This class simply raises a signal which updates the screen.
  * 
  * The background is that occasionally you want to have a chain of reactions
  * which would normally look bad. Imagine an event being triggered, which leads
  * to a unit being added, some text and another unit being added (in this order),
  * then the first unit will not be displayed until the processing of all
  * reactions has been done. So here we have an update reaction which can be called
  * after the first unit being created.
  */

class RUpdate : public Reaction
{
    public:
        RUpdate();

        //! Loading constructor
        RUpdate(XML_Helper* helper);
        ~RUpdate();

        //! Saves the data. See xmlhelper.h for details.
        bool save(XML_Helper* helper) const;

        bool trigger() const;
        
        static sigc::signal<void> supdating;
};

#endif //RUPDATE_H
