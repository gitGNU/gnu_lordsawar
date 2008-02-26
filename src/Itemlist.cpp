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
    if (!d_instance)
        d_instance = new Itemlist();

    return d_instance;
}

Itemlist* Itemlist::getInstance(XML_Helper *helper)
{
    if (!d_instance)
        d_instance = new Itemlist();

    d_instance = new Itemlist(helper);
    return d_instance;
}

void Itemlist::createStandardInstance()
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

Itemlist::Itemlist()
{
}

Itemlist::~Itemlist()
{
    flClear();
}

bool Itemlist::loadItem(std::string tag, XML_Helper* helper)
{
    if (tag != "item")
        return false;

    Item* i = new Item(helper);
    (*this)[(*this).size()] = i;

    return true;
}

void Itemlist::flErase(iterator it)
{
    delete (*it).second;
    erase(it);
}

void Itemlist::flClear()
{
    while (!empty())
        flErase(begin());
}

bool Itemlist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("itemlist");

    for (const_iterator it = begin(); it != end(); it++)
      (*it).second->save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

void Itemlist::remove(Item *item)
{
  Uint32 index = 0;
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it).second == item)
	{
	  erase(index);
	  break;
	}
      index++;
    }
}
