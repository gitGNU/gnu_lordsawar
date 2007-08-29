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

#include <stdlib.h>
#include <sstream>
#include <sigc++/functors/mem_fun.h>

#include "history.h"
#include "hero.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<< x << endl<<flush;}
//#define debug(x)

History::History(Type type)
:d_type(type)
{
}

History::~History()
{
}

History* History::handle_load(XML_Helper* helper)
{
  Uint32 t;
  helper->getData(t, "type");

  switch (t)
    {
    case START_TURN:
      return (new History_StartTurn(helper));
    case FOUND_SAGE:
      return (new History_FoundSage(helper));
    }

  return 0;
}

History* History::copy(const History* a)
{
  switch(a->getType())
    {
    case START_TURN:
      return 
	(new History_StartTurn(*dynamic_cast<const History_StartTurn*>(a)));
    case FOUND_SAGE:
      return 
	(new History_FoundSage(*dynamic_cast<const History_FoundSage*>(a)));
    }

  return 0;
}

//-----------------------------------------------------------------------------
//History_StartTurn

History_StartTurn::History_StartTurn()
:History(History::START_TURN)
{
}

History_StartTurn::History_StartTurn(XML_Helper* helper)
:History(History::START_TURN)
{
}

History_StartTurn::~History_StartTurn()
{
}

std::string History_StartTurn::dump() const
{
  std::stringstream s;

  s <<"player starts a turn" << "\n";

  return s.str();
}

bool History_StartTurn::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag("history");
  retval &= helper->saveData("type", d_type);
  retval &= helper->closeTag();

  return retval;
}

bool History_StartTurn::fillData()
{
  return true;
}

//-----------------------------------------------------------------------------
//History_FoundSage

History_FoundSage::History_FoundSage()
:History(History::FOUND_SAGE), d_hero(0)
{
}

History_FoundSage::History_FoundSage(XML_Helper* helper)
:History(History::FOUND_SAGE)
{
  helper->getData(d_hero, "hero");
}

History_FoundSage::~History_FoundSage()
{
}

std::string History_FoundSage::dump() const
{
  std::stringstream s;

  s <<"player found a sage with hero " << d_hero<< "\n";

  return s.str();
}

bool History_FoundSage::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag("history");
  retval &= helper->saveData("type", d_type);
  retval &= helper->saveData("hero", d_hero);
  retval &= helper->closeTag();

  return retval;
}

bool History_FoundSage::fillData(Hero *h)
{
  d_hero = h->getId();
  return true;
}

