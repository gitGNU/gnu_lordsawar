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

#ifndef __QUEST_KILL_HERO_H
#define __QUEST_KILL_HERO_H

#include <sigc++/trackable.h>

#include <list>
#include "Quest.h"
#include "hero.h"
#include "playerlist.h"


/** Kill hero quest
  * 
  * This specific quest demands a certain hero to be killed.
  */

class QuestKillHero : public Quest, public sigc::trackable
{
    public:

        QuestKillHero(QuestsManager& q_mgr, Uint32 hero);

        /** \brief Constructor - create a new quest from
                   saved data */
        QuestKillHero(QuestsManager& q_mgr, XML_Helper* helper);

        /**
         * \brief Return whether quest is possible at all
         */
        static bool isFeasible(Uint32 heroId);

        //! Saves the data
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
                  the quest expiration.
         */
         void getExpiredMsg(std::queue<std::string>& msgs) const;

         //! Returns the hunted hero
         Uint32 getVictim() const {return d_victim;}

	void armyDied(Army *a, bool heroIsCulprit);

	void cityAction(City *c, CityDefeatedAction action, 
			bool heroIsCulprit, int gold);
    private:

        //! Initializes the description string
        void initDescription();

        /** \brief Choose a hero to be killed */
        static Hero* chooseToKill();
        
        /** hero id (the target) to be searched by the hero */
        Uint32 d_victim;
};

#endif
