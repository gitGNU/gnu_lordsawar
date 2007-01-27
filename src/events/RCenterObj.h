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

#ifndef RCENTEROBJ_H
#define RCENTEROBJ_H

#include <pgpoint.h>

#include "Reaction.h"

/** Centers on an object
  * 
  * This class is almost identical to RCenter, but instead of centering on a
  * map position it centers on an object. The object can be anything, from a
  * city or temple to a stack. May be useful even for static objects if you move
  * them around or so.
  */

class RCenterObj : public Reaction
{
    public:
        //! Creates reaction with object id
        RCenterObj(Uint32 id);

        //! Loading constructor
        RCenterObj(XML_Helper* helper);
        ~RCenterObj();

        //! Saves the reaction data
        bool save(XML_Helper* helper) const;

        //! Triggers the reaction
        bool trigger() const;


        //! Returns the object id
        Uint32 getObject() const {return d_object;}

        //! Sets the object id
        void setObject(Uint32 object) {d_object = object;}
        

        //! The signal will be connected with the map out of scope here
        static SigC::Signal1<void, PG_Point> scentering;

    private:
        Uint32 d_object;
};

#endif //RCENTEROBJ_H
