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

#include "EKillAll.h"
#include "../playerlist.h"


EKillAll::EKillAll()
    :Event(KILLALL)
{
}

EKillAll::EKillAll(XML_Helper* helper)
    :Event(helper)
{
    d_type = KILLALL;
}

EKillAll::~EKillAll()
{
}

bool EKillAll::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("event");
    retval &= Event::save(helper);
    retval &= helper->closeTag();

    return retval;
}

void EKillAll::init()
{
    Playerlist::getInstance()->splayerDead.connect(
                               SigC::slot((*this), &EKillAll::trigger));
}

void EKillAll::trigger(Player* p)
{
    if (Playerlist::getInstance()->getNoOfPlayers() != 1)
        return;

    raise();
}
