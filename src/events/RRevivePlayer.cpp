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

#include "RRevivePlayer.h"
#include "../playerlist.h"

RRevivePlayer::RRevivePlayer(Uint32 player)
    :Reaction(REVIVEPLAYER), d_player(player)
{
}

RRevivePlayer::RRevivePlayer(XML_Helper* helper)
    :Reaction(helper)
{
    d_type = REVIVEPLAYER;
    helper->getData(d_player, "player");
}

RRevivePlayer::~RRevivePlayer()
{
}

bool RRevivePlayer::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("reaction");
    retval &= helper->saveData("player", d_player);
    retval &= Reaction::save(helper);
    retval &= helper->closeTag();

    return retval;
}

bool RRevivePlayer::trigger() const
{
    if (!check())
        return false;

    Playerlist::getInstance()->getPlayer(d_player)->revive();

    return true;
}
