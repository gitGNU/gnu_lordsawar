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

#include "RAddGold.h"
#include "../playerlist.h"

RAddGold::RAddGold(Uint32 player, Sint32 gold)
    :Reaction(ADDGOLD), d_player(player), d_gold(gold)
{
}

RAddGold::RAddGold(XML_Helper* helper)
    :Reaction(helper)
{
    d_type = ADDGOLD;
    helper->getData(d_player, "player");
    helper->getData(d_gold, "gold");
}

RAddGold::~RAddGold()
{
}

bool RAddGold::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("reaction");
    retval &= helper->saveData("player", d_player);
    retval &= helper->saveData("gold", d_gold);
    retval &= Reaction::save(helper);
    retval &= helper->closeTag();
    
    return retval;
}

bool RAddGold::trigger() const
{
    Player* target = Playerlist::getInstance()->getPlayer(d_player);

    if (!target || !check())
        return false;

    target->addGold(d_gold);
    return true;
}
