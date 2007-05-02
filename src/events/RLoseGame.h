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

#ifndef RLOSEGAME_H
#define RLOSEGAME_H

#include <sigc++/sigc++.h>
#include "Reaction.h"

/** This reaction determines that the game has been "lost" (whatever that means).
  * Usually (with some more patching), it will show the loose dialog.
  *
  * To make it a bit more complex, you can supply a number with this reaction.
  * This number can be used later to signal _how_ the game was lost (for
  * non-linear campaigns). However, currently this is only a vague idea.
  */

class RLoseGame : public Reaction
{
    public:
        RLoseGame(Uint32 status);
        RLoseGame(XML_Helper* helper);
        ~RLoseGame();

        //! Saves the reaction data
        bool save(XML_Helper* helper) const;

        //! Triggers the reaction
        bool trigger() const;


        //! Returns the status of this reaction
        Uint32 getStatus() const {return d_status;}

        //! Sets the status of this reaction
        void setStatus(Uint32 status) {d_status = status;}
        

        //! the signal is connected out of scope of this class
        static sigc::signal<void, Uint32> slosing;

    private:
        Uint32 d_status;
};

#endif //RLOSEGAME_H
