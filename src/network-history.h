// Copyright (C) 2008 Ben Asselstine
//
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#ifndef NETWORK_HISTORY_H
#define NETWORK_HISTORY_H

#include "history.h"
#include "Ownable.h"

class NetworkHistory: public Ownable
{
public:
     //! Default constructor.
     NetworkHistory(History *history, Player *owner);

     //! Loading constructor.
     NetworkHistory(XML_Helper* helper);

     //! Destructor.
     ~NetworkHistory();

     //! Returns debug information.
     std::string toString() const;

     //! copying the deep contents of the history
     static NetworkHistory* copy(History *history, Player *owner);

     //!Saving the network history to an xml stream.
     bool save(XML_Helper* helper) const;

     History * getHistory() const {return d_history;};
     void setHistory (History *history) {d_history = history;};
 private:
     History *d_history;
};
#endif
