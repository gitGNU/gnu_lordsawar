// Copyright (C) 2001, 2003 Michael Bartl
// Copyright (C) 2004 Ulf Lorenz
// Copyright (C) 2005, 2006 Andrea Paternesi
// Copyright (C) 2007, 2008 Ben Asselstine
// Copyright (C) 2008 Ole Laursen
//
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#ifndef LOCATIONLIST_H
#define LOCATIONLIST_H

#include <list>
#include "ruin.h"
#include "temple.h"
#include "city.h"

/** A list for object instances
  * 
  * This class extends the stl lists by adding the functions getObjectAt()
  * which return the object at position (x,y). Necessary for such things as
  * the city list.
  */

template<class T> class LocationList : public std::list<T>
{
 public:
  
  LocationList(){};  
  ~LocationList() 
    {
      for (typename LocationList<T>::iterator it = this->begin(); it != this->end(); ++it)
	delete *it;
    };

  //! Returns the object at position (x,y).  
  T getObjectAt(int x, int y) 
    {
      for (typename LocationList<T>::iterator it = this->begin(); it != this->end(); ++it)
	{
	  Vector<int> p = (*it)->getPos();
	  int size = (*it)->getSize() - 1;

	  if (p.x >= (x - size) && p.x <= x && p.y >= (y - size) && p.y <= y)
	    {
	      return (*it);
	    }
	}
      return 0;
    }

  //! Returns the object at position pos.  
  T getObjectAt(const Vector<int>& pos) 
    {
      return getObjectAt(pos.x, pos.y);
    }

  T getNearestObjectInDir(const Vector<int> &pos, const Vector<int> dir)
    {
      int diff = -1;
      typename LocationList<T>::iterator diffit;
      for (typename LocationList<T>::iterator it = this->begin(); it != this->end(); ++it)
        {
          Vector<int> p = (*it)->getPos();
          int delta = abs(p.x - pos.x) + abs(p.y - pos.y);
	  //if dir is -1, then the difference between pos.x and p.x should be positive
	  //if dir is +1, then the difference between pos.x and p.x should be negative
	  //if looking west, and the object is to the east
	  if (dir.x < 0 && (pos.x - p.x) <= 0)
	    continue;
	  //if looking east , and the object is to the west
	  if (dir.x > 0 && (pos.x - p.x) >= 0)
	    continue;
	  //if looking north, and the object is to the south
	  if (dir.y < 0 && (pos.y - p.y) <= 0)
	    continue;
	  //if looking south, and the object is to the north
	  if (dir.y > 0 && (pos.y - p.y) >= 0)
	    continue;

          if ((diff > delta) || (diff == -1))
            {
              diff = delta;
              diffit = it;
            }
        }
      if (diff == -1) return 0;
      return (*diffit);
    }

  T getNearestObject (const Vector<int>& pos, std::list<bool (*)(void*)> *filters)
    {
      int diff = -1;
      typename LocationList<T>::iterator diffit;
      for (typename LocationList<T>::iterator it = this->begin(); it != this->end(); ++it)
        {
          Vector<int> p = (*it)->getPos();
          int delta = abs(p.x - pos.x) + abs(p.y - pos.y);
	  if (filters)
	    {
	      std::list<bool (*)(void*)>::iterator fit = filters->begin();
	      bool filtered = false;
	      for (; fit != filters->end(); fit++)
	        {
	          if ((*fit)(*it) == true)
	            {
		      filtered = true;
		      break;
	            }
	            
	        }
	      if (filtered)
	        continue;
	    }

          if ((diff > delta) || (diff == -1))
            {
              diff = delta;
              diffit = it;
            }
        }
      if (diff == -1) return 0;
      return (*diffit);
    }

  T getNearestObject (const Vector<int>& pos)
    {
      return getNearestObject (pos, NULL);
    }

  T getNearestObjectBefore (const Vector<int>& pos, int dist)
    {
      T t = getNearestObject(pos);
      if (!t)
	return NULL;
      if (t->getPos().x <= pos.x + dist && t->getPos().x >= pos.x - dist &&
          t->getPos().y <= pos.y + dist && t->getPos().y >= pos.y - dist)
        return t;
      return NULL;
    }

  T getNearestObjectAfter(const Vector<int>& pos, int dist, 
			   std::list<bool (*)(void*)> *filters)
    {
      int diff = -1;
      typename LocationList<T>::iterator diffit;

      for (typename LocationList<T>::iterator it = this->begin(); it != this->end(); ++it)
        {
	  if (filters)
	    {
	      std::list<bool (*)(void*)>::iterator fit = filters->begin();
	      bool filtered = false;
	      for (; fit != filters->end(); fit++)
	        {
	          if ((*fit)(*it) == true)
	            {
		      filtered = true;
		      break;
	            }
	            
	        }
	      if (filtered)
	        continue;
	    }
          
            Vector<int> p = (*it)->getPos();
            int delta = abs(p.x - pos.x);
            if (delta < abs(p.y - pos.y))
                delta = abs(p.y - pos.y);
          
            if ((diff > delta && delta >= dist) || (diff == -1))
              {
                diff = delta;
                diffit = it;
              }
        }
    
      if (diff == -1) return 0;
      return (*diffit);
    }

  T getById(Uint32 id)
  {
    for (typename LocationList<T>::iterator i = this->begin(); i != this->end(); ++i)
      if ((*i)->getId() == id)
        return (*i);
    return 0;
  }
};

#endif // LOCATIONLIST_H

// End of file
