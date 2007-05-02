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

#ifndef RCENTER_H
#define RCENTER_H

#include <sigc++/signal.h>

#include "../vector.h"
#include "Reaction.h"

/** Centers the game screen on a given point
  * 
  * This reaction does almost the same as update, but additionally centers the
  * game screen on a given point. May be useful sometimes.
  *
  * @note This reaction just releases a signal; the signal has to be connected
  * elsewhere.
  */

class RCenter : public Reaction
{
    public:
        //! Default constructor with centering point
        RCenter(Vector<int> pos);

        //! Loading constructor
        RCenter(XML_Helper* helper);
        ~RCenter();

        //! Saves the reaction data
        bool save(XML_Helper* helper) const;

        //! Triggers the reaction
        bool trigger() const;


        //! Returns the point of centering
        Vector<int> getPos() const {return d_pos;}

        //! Sets the point of centering
        void setPos(Vector<int> pos) {d_pos = pos;}


        //! static signal to center the game screen
        static sigc::signal<void, Vector<int> > scentering;

    private:
        Vector<int> d_pos;
};

#endif //RCENTER_H
