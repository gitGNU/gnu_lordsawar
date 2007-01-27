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

#include <pgapplication.h>
#include <pgmessagebox.h>
#include "RMessage.h"
#include "../playerlist.h"

RMessage::RMessage(std::string message)
    :Reaction(MESSAGE), d_message(message)
{
}

RMessage::RMessage(XML_Helper* helper)
    :Reaction(helper)
{
    d_type = MESSAGE;
    helper->getData(d_message, "message");
}

RMessage::~RMessage()
{
}

bool RMessage::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("reaction");
    retval &= helper->saveData("message", d_message);
    retval &= Reaction::save(helper);
    retval &= helper->closeTag();
        
    return retval;
}

bool RMessage::trigger() const
{
    if (!check())
        return false;
    
    //Display a message box for the beginning in the middle of the screen
    int height = 200;
    int width = 400;
    int x = (PG_Application::GetScreenWidth() - width)/2;
    int y = (PG_Application::GetScreenHeight() - height)/2;

    PG_MessageBox mb(0, PG_Rect(x,y,width,height), "", _(d_message.c_str()),
                     PG_Rect(150, 170, 100, 20), _("OK"));
    mb.SetTransparency(150, true);
    mb.Show();
    mb.RunModal();
    mb.Hide();
    
    return true;
}

std::string RMessage::getMessage(bool translate) const
{
    if (translate)
        return __(d_message);

    return d_message;
}
