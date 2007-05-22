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

#ifndef __QUEST_ENEMY_ARMIES_H
#define __QUEST_ENEMY_ARMIES_H

#include <sigc++/trackable.h>

#include "Quest.h"
#include "army.h"

/**
 * \brief Quest - kill an amount of enemy armies
 *
 */

class QuestEnemyArmies : public Quest, public sigc::trackable
{
    public:
        /** \brief Constructor */
        QuestEnemyArmies(QuestsManager& q_mgr, Uint32 hero);

        /** \brief Constructor - create a new quest from
                   saved data */
        QuestEnemyArmies(QuestsManager& q_mgr, XML_Helper* helper);
     
        
        //! We need an enemy player to produce this quest
        static bool isFeasible(Uint32 heroId);

        //! Save the quest data
        bool save(XML_Helper* helper) const;


        /**
         * \brief Get progress information 
         *
         * \param s here we append the progress information
         */
        std::string getProgress() const;

        /**
         * \brief Provide the lines of the message describing
                  the quest completion.
         */
        void getSuccessMsg(std::queue<std::string>& msgs) const;

        /**
         * \brief Provide the lines of the message describing
                  the quest completion.
         */
        void getExpiredMsg(std::queue<std::string>& msgs) const;


        //! Returns the number of armies to be killed
        Uint32 getArmiesToKill() {return d_to_kill;}
         
    private:
        /** slot that would receive the sdyingArmy (signal) */
        void dyingArmy(Army *stack, std::vector<Uint32> culprits);

        void initDescription();


        /** how many armies we should kill */
        Uint32 d_to_kill;
        /** how many armies we already have killed */
        Uint32 d_killed;

	/** the target player */
	Player *d_victim_player;


};

#endif
