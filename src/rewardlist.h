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

#ifndef REWARDLIST_H
#define REWARDLIST_H

#include <list>
#include <vector>
#include <sigc++/trackable.h>
#include <SDL_types.h>

class Reward;
class XML_Helper;

/** List of rewards in the game
  *
  */

class Rewardlist : public std::list<Reward*>, public sigc::trackable
{
    public:
        Rewardlist();
        Rewardlist(Rewardlist *rewardlist);
        Rewardlist(XML_Helper* helper);
        ~Rewardlist();

        //! Searches through the player's lists and deletes the reward
        void deleteReward(Reward* s);

        //! Save the data. See XML_Helper for details
        bool save(XML_Helper* helper) const;

        //! Behaves like std::list::clear(), but frees pointers as well
        void flClear();

        //! Behaves like std::list::erase(), but frees pointers as well
        iterator flErase(iterator object);

        //! Behaves like std::list::remove(), but frees pointers as well
        bool flRemove(Reward* object);

    private:
        //! Callback function for loading
        bool load(std::string tag, XML_Helper* helper);

};

#endif // REWARDLIST_H

// End of file
