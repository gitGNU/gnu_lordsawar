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

#ifndef RWINGAME_H
#define RWINGAME_H

#include <sigc++/signal.h>
#include <SDL.h>
#include "Reaction.h"

/** This reaction determines that the game has been "won" (whatever that means).
  * It also shows a win game dialog.
  *
  * To make it a bit more complex, you can supply a number with this reaction.
  * This number can be used later to signal _how_ the game was won (for
  * non-linear campaigns). However, currently this is only a vague idea.
  */

class RWinGame : public Reaction
{
    public:
        RWinGame(Uint32 status);
        RWinGame(XML_Helper* helper);
        ~RWinGame();

        //! Saves the reaction data
        bool save(XML_Helper* helper) const;

        //! Triggers the reaction
        bool trigger() const;

        
        //! Returns the status for this reaction
        Uint32 getStatus() const {return d_status;}

        //! Sets the status for this reaction
        void setStatus(Uint32 status) {d_status = status;}
        

        //! signal to show the win game dialog. For technical reasons we cannot
        //! dur this here.
        static sigc::signal<void> swinDialog;
        
        //! signal is connected outside of the scope of this class
        static sigc::signal<void, Uint32> swinning;

    private:
        Uint32 d_status;
};

#endif //RWINGAME_H
