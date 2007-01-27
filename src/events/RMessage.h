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

#ifndef RMESSAGE_H
#define RMESSAGE_H

#include <string>
#include "Reaction.h"
#include "../defs.h"

/** This reaction just displays a message.
  * 
  * Not very nice, though. Maybe a better message display should be invented
  * here...
  */

class RMessage : public Reaction
{
    public:
        //! Standard constructor; specify the message to be displayed
        RMessage(std::string message);

        //! Loading constructor. See xmlhelper.h for details how this works.
        RMessage(XML_Helper* helper);
        ~RMessage();

        //! Saves the reaction data. See xmlhelper.h again.
        bool save(XML_Helper* helper) const;

        //! Triggers the reaction.
        bool trigger() const;

        //! Returns the message
        std::string getMessage(bool translate = true) const;

        //! Sets the message
        void setMessage(std::string message) {d_message = message;}

    private:
        std::string d_message;
};

#endif //RMESSAGE_H
