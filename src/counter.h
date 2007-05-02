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

#ifndef FL_COUNTER_H
#define FL_COUNTER_H

#include <string>
#include <sigc++/trackable.h>
#include <SDL.h>

class XML_Helper;

/** The purpose of this class is very simple. Each object (player etc.) has a
  * unique id by which it may be accessed (this isn't important for now, but
  * becomes crucial as soon as you play e.g. over the network). Therefore, each 
  * important game object queries this class for an id and gets a unique
  * identifier. The current counter position is saved together with a game.
  *
  * The implementation with the global variable could be changed in favour of
  * static functions...
  */

class FL_Counter : public sigc::trackable
{
    public:
        //! Initialise the counter with a start value
        FL_Counter(Uint32 start = 1);

        //! Load the counter. See XML_Helper for details.
        FL_Counter(XML_Helper* helper);
        ~FL_Counter();

        //! Returns a unique id
        Uint32 getNextId();

        //! Saves the current counter position
        bool save(XML_Helper* helper);

    private:
        Uint32 d_curID;
};

extern FL_Counter* fl_counter;
#endif
