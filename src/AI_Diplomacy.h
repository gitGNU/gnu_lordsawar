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

#ifndef AI_DIPLOMACY_H
#define AI_DIPLOMACY_H

#include <string>
#include <list>

class Player;

using namespace std;

class AI_Diplomacy
{
    public:
        // Initializes the object 
        AI_Diplomacy(Player *owner);
	void makeProposals();
	void needNewEnemy(Player *player);
        ~AI_Diplomacy();

    private:
        // the analysis currently in use
        static AI_Diplomacy *instance;

        void makeFriendsAndEnemies();
        void makeRequiredEnemies();
        void neutralsDwindlingNeedFirstEnemy();
        void gangUpOnTheBully();
       
        // DATA
        Player *d_owner;
	std::list<Player *> new_enemies;
};

#endif // AI_DIPLOMACY_H

// End of file
