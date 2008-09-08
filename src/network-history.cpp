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

#include <sstream>
#include "network-history.h"
#include "player.h"

#include "xmlhelper.h"


NetworkHistory::NetworkHistory(History *history, Player *owner)
     : Ownable(owner)
{
  d_history = History::copy(history);
}

NetworkHistory::NetworkHistory(XML_Helper* helper)
     : Ownable(helper)
{
}

NetworkHistory::~NetworkHistory()
{
  delete d_history;
}

bool NetworkHistory::save(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->openTag("networkhistory");
  retval &= helper->saveData("owner", d_owner->getId());
  d_history->save(helper);
  retval &= helper->closeTag();
  return retval;
}

std::string NetworkHistory::toString() const
{
  std::stringstream s;
  std::string history= d_history->dump();
  s <<"Player \""<< getOwner()->getName() << "\"--> ";
  s <<history;
    
  return s.str();
}
