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

#include <sigc++/functors/mem_fun.h>

#include "Itemlist.h"

#include "File.h"
#include "defs.h"

Itemlist* Itemlist::d_instance = 0;

Itemlist* Itemlist::getInstance()
{
    return d_instance;
}

void Itemlist::createInstance()
{
    deleteInstance();

    XML_Helper helper(File::getItemDescription(), std::ios::in, false);
    d_instance = new Itemlist(&helper);

    if (!helper.parse())
    {
        std::cerr <<_("Could not parse items description file. Exiting!\n");
        exit(-1);
    }
    helper.close();
}

void Itemlist::deleteInstance()
{
    if (d_instance != 0)
        delete d_instance;

    d_instance = 0;
}


Itemlist::Itemlist(XML_Helper* helper)
{
    helper->registerTag("item", sigc::mem_fun(*this, &Itemlist::loadItem));
}

Itemlist::~Itemlist()
{
    fl_clear();
}

bool Itemlist::loadItem(std::string tag, XML_Helper* helper)
{
    if (tag != "item")
        return false;

    Item* i = new Item(helper);
    (*this)[i->getIndex()] = i;

    return true;
}

void Itemlist::fl_erase(iterator it)
{
    delete (*it).second;
    erase(it);
}

void Itemlist::fl_clear()
{
    while (!empty())
        fl_erase(begin());
}
