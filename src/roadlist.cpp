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

#include <sigc++/functors/mem_fun.h>

#include "roadlist.h"
#include "GameMap.h"
#include "xmlhelper.h"

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

Roadlist* Roadlist::s_instance=0;

Roadlist* Roadlist::getInstance()
{
    if (s_instance == 0)
        s_instance = new Roadlist();

    return s_instance;
}

Roadlist* Roadlist::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new Roadlist(helper);
    return s_instance;
}

void Roadlist::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

Roadlist::Roadlist()
{
}

Road* Roadlist::getNearestRoad(const Vector<int>& pos)
{
    int diff = -1;
    iterator diffit;
    
    for (iterator it = begin(); it != end(); ++it)
    {
       Vector<int> p = (*it).getPos();
       int delta = abs(p.x - pos.x);
       if (delta < abs(p.y - pos.y))
           delta = abs(p.y - pos.y);
       if ((diff > delta) || (diff == -1))
       {
           diff = delta;
           diffit = it;
       }
    }

    if (diff == -1) return 0;
    return &(*diffit);
}
Road* Roadlist::getNearestRoad(const Vector<int>& pos, int dist)
{
  Road *r = getNearestRoad(pos);
  if (!r)
    return r;
  if (r->getPos().x <= pos.x + dist && r->getPos().x >= pos.x - dist &&
      r->getPos().y <= pos.y + dist && r->getPos().y >= pos.y - dist)
    return r;
  return NULL;
}


Roadlist::Roadlist(XML_Helper* helper)
{
    helper->registerTag("road", sigc::mem_fun(this, &Roadlist::load));
}

bool Roadlist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("roadlist");

    for (const_iterator it = begin(); it != end(); it++)
        retval &= (*it).save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

bool Roadlist::load(std::string tag, XML_Helper* helper)
{
    if (tag != "road")
    //what has happened?
        return false;
    
    Road s(helper);
    push_back(s);

    return true;
}

int Roadlist::calculateType (Vector<int> t)
{
    // examine neighbour tiles to discover whether there's a road on them
    bool u = false; //up
    bool b = false; //bottom
    bool l = false; //left
    bool r = false; //right

    if (t.y > 0)
      u = getObjectAt(t + Vector<int>(0, -1));
    if (t.y < GameMap::getHeight() - 1)
      b = getObjectAt(t + Vector<int>(0, 1));
    if (t.x > 0)
      l = getObjectAt(t + Vector<int>(-1, 0));
    if (t.x < GameMap::getWidth() - 1)
    r = getObjectAt(t + Vector<int>(1, 0));

    // then translate this to the type
    int type = 2; 
    //show road type 2 when no other road tiles are around
    if (!u && !b && !l && !r)
	type = 2;
    else if (u && b && l && r)
	type = 2;
    else if (!u && b && l && r)
	type = 9;
    else if (u && !b && l && r)
	type = 8;
    else if (u && b && !l && r)
	type = 7;
    else if (u && b && l && !r)
	type = 10;
    else if (u && b && !l && !r)
	type = 1;
    else if (!u && !b && l && r)
	type = 0;
    else if (u && !b && l && !r)
	type = 3;
    else if (u && !b && !l && r)
	type = 4;
    else if (!u && b && l && !r)
	type = 6;
    else if (!u && b && !l && r)
	type = 5;
    else if (u && !b && !l && !r)
	type = 1;
    else if (!u && b && !l && !r)
	type = 1;
    else if (!u && !b && l && !r)
	type = 0;
    else if (!u && !b && !l && r)
	type = 0;
    return type;
}

