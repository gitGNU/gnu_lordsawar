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
  ~LocationList() {};

  //! Returns the object at position (x,y).  
  T* getObjectAt(int x, int y) 
    {
      for (typename LocationList<T>::iterator it = this->begin(); it != this->end(); ++it)
	{
	  Vector<int> p = (*it).getPos();
	  int size = (*it).getSize() - 1;

	  if (p.x >= (x - size) && p.x <= x && p.y >= (y - size) && p.y <= y)
	    {
	      return &(*it);
	    }
	}
      return 0;
    }

  //! Returns the object at position pos.  
  T* getObjectAt(const Vector<int>& pos) 
    {
      return getObjectAt(pos.x, pos.y);
    }

  T* getNearestObject (const Vector<int>& pos, std::list<bool (*)(void*)> *filters)
    {
      int diff = -1;
      typename LocationList<T>::iterator diffit;
      for (typename LocationList<T>::iterator it = this->begin(); it != this->end(); ++it)
        {
          Vector<int> p = (*it).getPos();
          int delta = abs(p.x - pos.x) + abs(p.y - pos.y);
	  if (filters)
	    {
	      std::list<bool (*)(void*)>::iterator fit = filters->begin();
	      bool filtered = false;
	      for (; fit != filters->end(); fit++)
	        {
	          if ((*fit)(&*it) == true)
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
      return &(*diffit);
    }

  T* getNearestObject (const Vector<int>& pos)
    {
      return getNearestObject (pos, NULL);
    }

  T* getNearestObjectBefore (const Vector<int>& pos, int dist)
    {
      T *t = getNearestObject(pos);
      if (!t)
	return NULL;
      if (t->getPos().x <= pos.x + dist && t->getPos().x >= pos.x - dist &&
          t->getPos().y <= pos.y + dist && t->getPos().y >= pos.y - dist)
        return t;
      return NULL;
    }
};

#endif // LOCATIONLIST_H

// End of file
