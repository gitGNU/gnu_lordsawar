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

#include "RKillPlayer.h"
#include "../playerlist.h"

RKillPlayer::RKillPlayer(Uint32 player)
    :Reaction(KILLPLAYER), d_player(player)
{
}

RKillPlayer::RKillPlayer(XML_Helper* helper)
    :Reaction(helper)
{
    d_type = KILLPLAYER;
    helper->getData(d_player, "player");
}

RKillPlayer::~RKillPlayer()
{
}

bool RKillPlayer::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("reaction");
    retval &= helper->saveData("player", d_player);
    retval &= Reaction::save(helper);
    retval &= helper->closeTag();

    return retval;
}

bool RKillPlayer::trigger() const
{
    if (!check())
        return false;

    //! _never_ kill the neutral player, we almost always need him
    if (Playerlist::getInstance()->getNeutral()->getId() == d_player)
        return false;

    Playerlist::getInstance()->getPlayer(d_player)->kill();
    return true;
}
