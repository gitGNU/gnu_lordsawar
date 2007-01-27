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

#ifndef THREATLIST_H
#define THREATLIST_H

#include <list>
#include "Threat.h"

class Stack;
class Ruin;
class AICityInfo;

using namespace std;

/** List of threats + some neccessary AddOns.
  */
class Threatlist : public std::list<Threat*>
{
    public:
        Threatlist();
        ~Threatlist();

        // add a ruin as a threat
        void addRuin(Ruin *ruin);
        
        // Adds a stack as a threat. If other threats posed by the owner of
        // the stack are close by, they are merged to a single threat.
        void addStack(Stack *stack);

        //! Searches through the threat list and deletes the stack
        void deleteStack(Stack* s);


        // how much danger does this set of threats pose to the given city?
        void findThreats(AICityInfo *info) const;

        //! sort into a list of most dangerous first
        void sortByValue();

        //! sort into list by closest first
        void sortByDistance(PG_Point pos);

        //! return some debugging information
        string toString() const;
        
        
        //! Behaves like std::list::clear(), but frees pointers as well
        void flClear();

        //! Behaves like std::list::erase(), but frees pointers as well
        iterator flErase(iterator object);

        //! Behaves like std::list::remove(), but frees pointers as well
        bool flRemove(Threat* object);
};

#endif // THREATLIST_H

// End of file
