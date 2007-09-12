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

#include <sstream>
#include <map>
#include "Item.h"
#include "Itemlist.h"
#include "File.h"
#include "counter.h"
#include "playerlist.h"
#include "ucompose.hpp"

using namespace std;

Item::Item(XML_Helper* helper)
{
    
    // Loading of items is a bit complicated, so i'd better loose some words.
    // In general, items can be loaded from the items description file or
    // from a savegame. They both differ a bit, more on that when we encounter
    // such a situation. First, let us deal with the common things.

    helper->getData(d_bonus, "bonus");
    
    helper->getData(d_name, "name");
    helper->getData(d_plantable, "plantable");
    if (d_plantable)
      {
        Uint32 ui = MAX_PLAYERS + 2;
        helper->getData(ui, "plantable_owner");
        d_plantable_owner = Playerlist::getInstance()->getPlayer(ui);
        helper->getData(d_planted, "planted");
      }
    else
      {
	d_plantable_owner = NULL;
	d_planted = false;
      }

    // Now come the differences. Items in the game have an id, other items
    // just the dummy id "0"
    helper->getData(d_id, "id");

}

Item::Item(std::string name, bool plantable, Player *plantable_owner)
{
  d_bonus = 0;
  d_name = name;
  d_plantable = plantable;
  d_plantable_owner = plantable_owner;
  d_planted = false;
  d_id = fl_counter->getNextId();
}

Item::Item(const Item& orig)
:d_bonus(orig.d_bonus),
    d_name(orig.d_name), d_plantable(orig.d_plantable),
    d_plantable_owner(orig.d_plantable_owner), d_planted(orig.d_planted)
{
  // Some things we don't copy from the template; we rather get an own ID
  d_id = fl_counter->getNextId();
}

Item::~Item()
{
}

bool Item::save(XML_Helper* helper) const
{
  bool retval = true;

  // A template is never saved, so we assume this class is a real-life item
  retval &= helper->openTag("item");
  retval &= helper->saveData("name", d_name);
  retval &= helper->saveData("plantable", d_plantable);
  if (d_plantable)
    {
      retval &= helper->saveData("plantable_owner", 
				 d_plantable_owner->getId());
      retval &= helper->saveData("planted", d_planted);
    }
  retval &= helper->saveData("id", d_id);

  retval &= helper->saveData("bonus", d_bonus);

  retval &= helper->closeTag();

  return retval;
}

bool Item::getBonus(Item::Bonus bonus) const
{
  return (d_bonus & bonus) == 0 ? false : true;
}

void Item::setBonus(Item::Bonus bonus)
{
  d_bonus |= bonus;
}

std::string Item::getBonusDescription()
{
  // the attributes column
  std::vector<Glib::ustring> s;
  if (getBonus(Item::ADD1STR))
    s.push_back(_("+1 Battle"));
  if (getBonus(Item::ADD2STR))
    s.push_back(_("+2 Battle"));
  if (getBonus(Item::ADD3STR))
    s.push_back(_("+3 Battle"));
  if (getBonus(Item::ADD1STACK))
    s.push_back(_("+1 Command"));
  if (getBonus(Item::ADD2STACK))
    s.push_back(_("+2 Command"));
  if (getBonus(Item::ADD3STACK))
    s.push_back(_("+3 Command"));
  if (getBonus(Item::FLYSTACK))
    s.push_back(_("Allows Flight"));
  if (getBonus(Item::DOUBLEMOVESTACK))
    s.push_back(_("Doubles Movement"));
  if (getBonus(Item::ADD2GOLDPERCITY))
    s.push_back(_("+2 gold per city"));
  if (getBonus(Item::ADD3GOLDPERCITY))
    s.push_back(_("+3 gold per city"));
  if (getBonus(Item::ADD4GOLDPERCITY))
    s.push_back(_("+4 gold per city"));
  if (getBonus(Item::ADD5GOLDPERCITY))
    s.push_back(_("+5 gold per city"));

  Glib::ustring str;
  bool first = true;
  for (std::vector<Glib::ustring>::iterator i = s.begin(), end = s.end();
       i != end; ++i)
    {
      if (first)
	first = false;
      else
	str += "\n";
      str += *i;
    }
  return str;
}
