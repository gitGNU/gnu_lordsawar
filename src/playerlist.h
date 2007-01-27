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

#ifndef PLAYERLIST_H
#define PLAYERLIST_H

#include <list>
#include <string>
#include <sigc++/object_slot.h>

#include "player.h"

/** List of all players of a scenario
  * 
  * The playerlist is also implemented as a singleton function. The currently
  * active player is designated, you can access players by name or id and the
  * playerlist can check if there are more than one player remaining alive.
  */

class Playerlist : public std::list<Player*>, public SigC::Object
{
    public:

        //! Gets the singleton instance or creates a new one
        static Playerlist* getInstance();

        //! Loads the playerlist (including all players)
        static Playerlist* getInstance(XML_Helper* helper);

        //! Explicitely deletes the singleton instance
        static void deleteInstance();

        //! Returns the active player of the singleton instance
        static Player* getActiveplayer() {return d_activeplayer;}
        
        //! Returns whether the game has been finished. I didn't know a better place...
        static bool isFinished() {return s_finish;}

        //! set the game to a finished state
        static void finish() {s_finish = true;}
        
        //! Sets the active player to the next player in the order
        void nextPlayer();

        /** Checks if a player has died (==doesn't own a city). If so, it
          * marks the player as killed, so he is ignored the next time.
          */
        void checkPlayers();

        /** Sets the neutral player. Though the neutral player is located in the
          * list of existing players, it is handled in a special way. His colors
          * are e.g. used for stacks not owned by a player (such as monsters
          * infesting ruins), which is why we keep track of the neutral player
          * with a special pointer.
          *
          * @param neutral          the new neutral player
          */
        void setNeutral(Player* neutral) {d_neutral = neutral;}

        //! Returns the neutral player
        Player* getNeutral() const {return d_neutral;}

        //! Returns player named "name" or none if not existent
        Player* getPlayer(std::string name) const;

        //! Returns player with the specified id or none if not existent
        Player* getPlayer(Uint32 id) const;

        //! Returns the number of living players (neutral player excluded)
        int getNoOfPlayers() const;

        /** Returns the first player alive (needed to determine the begin of
          * a new round).
          */
        Player* getFirstLiving() const;
        
        
        //! Saves the playerlist. See XML_Helper for further docu.
        bool save(XML_Helper* helper) const;

        /** Erase an item. This needs to be implemented, because we have to free
          * the pointer. Else it behaves like the STL erase member functions.
          */
        iterator flErase(iterator it);

        //! This signal is emitted when a player has died.
        SigC::Signal1<void, Player*> splayerDead;
    
    protected:
        // CREATORS
        Playerlist();
        Playerlist(XML_Helper* helper);
        ~Playerlist();
        
    private:
        //! Callback for loading the playerlist.
        bool load(std::string, XML_Helper* helper);

        //static pointers for the singleton
        static Playerlist* s_instance;
        
        // DATA
        static Player* d_activeplayer;
        static bool s_finish;
        Player* d_neutral;
};

#endif // PLAYERLIST_H
