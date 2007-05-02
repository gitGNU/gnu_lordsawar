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

#include <string>
#include "RLoseGame.h"
#include <stdlib.h>
#include "../sound.h"

sigc::signal<void, Uint32> RLoseGame::slosing;

RLoseGame::RLoseGame(Uint32 status)
    :Reaction(LOSEGAME), d_status(status)
{
}

RLoseGame::RLoseGame(XML_Helper* helper)
    :Reaction(helper)
{
    d_type = LOSEGAME;
    helper->getData(d_status, "status");
}

RLoseGame::~RLoseGame()
{
}

bool RLoseGame::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("reaction");
    retval &= helper->saveData("status", d_status);

    retval &= Reaction::save(helper);
    
    retval &= helper->closeTag();
    
    return retval;
}

bool RLoseGame::trigger() const
{
    if (!check())
        return false;

    // crude hack: play one of the two music tunes
    if (rand() % 2)
        Sound::getInstance()->playMusic("defeat", 1);
    else
        Sound::getInstance()->playMusic("defeat2", 1);
    slosing.emit(d_status);
    Sound::getInstance()->haltMusic();
    return true;
}
