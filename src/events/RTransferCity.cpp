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

#include "RTransferCity.h"
#include "../citylist.h"
#include "../playerlist.h"

RTransferCity::RTransferCity(Uint32 city, Uint32 player)
    :Reaction(TRANSFERCITY), d_city(city), d_player(player)
{
}

RTransferCity::RTransferCity(XML_Helper* helper)
    :Reaction(helper)
{
    d_type = TRANSFERCITY;
    helper->getData(d_city, "city");
    helper->getData(d_player, "player");
}

RTransferCity::~RTransferCity()
{
}

bool RTransferCity::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("reaction");
    retval &= helper->saveData("city", d_city);
    retval &= helper->saveData("player", d_player);
    retval &= Reaction::save(helper);
    retval &= helper->closeTag();

    return retval;
}

bool RTransferCity::trigger() const
{
    if (!check())
        return false;

    Citylist* cl = Citylist::getInstance();
    for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
        if ((*it).getId() == d_city)
        {
            // if the city has been razed, this reaction is obsolete
            if ((*it).isBurnt())
                return false;

            // change ownership of the city
            (*it).conquer(Playerlist::getInstance()->getPlayer(d_player));
            return true;
        }

    // didn't find city
    return false;
}
