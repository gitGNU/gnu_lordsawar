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

#include "CPlayer.h"
#include "../playerlist.h"

CPlayer::CPlayer(Uint32 player)
    :Condition(PLAYER), d_player(player)
{
}

CPlayer::CPlayer(XML_Helper* helper)
    :Condition(PLAYER)
{
    helper->getData(d_player, "player");
}

CPlayer::~CPlayer()
{
}

bool CPlayer::check()
{
    Player* p = Playerlist::getActiveplayer();

    if (p && (p->getId() == d_player))
        return true;

    return false;
}

bool CPlayer::save(XML_Helper* helper) const
{
    bool retval = true;
    
    retval &= helper->openTag("condition");
    retval &= helper->saveData("type", d_type);
    retval &= helper->saveData("player", d_player);
    retval &= helper->closeTag();

    return retval;
}
