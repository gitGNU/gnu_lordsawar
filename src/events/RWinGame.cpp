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
#include <stdlib.h>
#include "RWinGame.h"
#include "../sound.h"

sigc::signal<void, Uint32> RWinGame::swinning;
sigc::signal<void> RWinGame::swinDialog;

RWinGame::RWinGame(Uint32 status)
    :Reaction(WINGAME), d_status(status)
{
}

RWinGame::RWinGame(XML_Helper* helper)
    :Reaction(helper)
{
    d_type = WINGAME;
    helper->getData(d_status, "status");
}

RWinGame::~RWinGame()
{
}

bool RWinGame::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("reaction");
    retval &= helper->saveData("status", d_status);

    retval &= Reaction::save(helper);
    
    retval &= helper->closeTag();
    
    return retval;
}

bool RWinGame::trigger() const
{
    if (!check())
        return false;
    
    // start playing the win game music; do some randomizing (this is crude, don't copy it!)
    if (rand() % 2)
        Sound::getInstance()->playMusic("victory", 1);
    else
        Sound::getInstance()->playMusic("victory2",1);
    swinDialog.emit();
    swinning.emit(d_status);
    Sound::getInstance()->haltMusic();

    return true;
}
