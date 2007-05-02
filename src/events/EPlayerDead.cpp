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

#include <sigc++/functors/mem_fun.h>

#include "EPlayerDead.h"
#include "../playerlist.h"

EPlayerDead::EPlayerDead(Uint32 p)
    :Event(PLAYERDEAD), d_player(p)
{
}

EPlayerDead::EPlayerDead(XML_Helper* helper)
    :Event(helper)
{
    helper->getData(d_player, "player");
    d_type = PLAYERDEAD;
}

EPlayerDead::~EPlayerDead()
{
}

bool EPlayerDead::save(XML_Helper* helper) const
{
    bool retval = true;
    
    retval &= helper->openTag("event");
    retval &= helper->saveData("player", d_player);

    retval &= Event::save(helper);
    
    retval &= helper->closeTag();
    
    return retval;
}

void EPlayerDead::init()
{
    Playerlist::getInstance()->splayerDead.connect(
                               sigc::mem_fun(*this, &EPlayerDead::trigger));
}

void EPlayerDead::trigger(Player* dead)
{
    if (dead->getId() != d_player)
        return;

    raise();
}
