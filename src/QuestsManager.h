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

#ifndef __QUEST_MANAGER_H
#define __QUEST_MANAGER_H

#include <queue>
#include "Quest.h"
#include "army.h"
#include "player.h"
#include "xmlhelper.h"
#include "hero.h"

class Quest;

/**
 * \brief The manager of the quests in the Freelords game.
 *
 * Responsibilities:
 *
 * - creation of new quests and assigning them to heroes,
 * - separating the Quest objects from the rest of the system,
 * - keeping track of pending quests and reacting to heroes' deaths
 *   by invalidating the quests that belonged to the dead,
 * - ability to return the information of recently completed quests
 *   (possibly at the end of the players' turn).
 *
 * \note We encountered some problems connected with the signals emitted
 *       and received by using the libsigc++ library. If the QM detects
 *       that some quest should be cancelled as a result of processing
 *       the sdyingArmy signal, it cannot delete this Quest, nor can
 *       it disconnect this quest, if this quest has connected
 *       itself to the same signal. As a result when we want to
 *       delete a quest, we move it from the hash of active quests
 *       to a vector of 'quests being marked to delete'. Then we
 *       wait for someone to emit the signal sendingTurn and
 *       delete all marked quests when processing this signal.
 *
 * \todo Implement more that one quest for a single hero
 *
 */
class QuestsManager : public SigC::Object
{
    public:
        /** \brief Singleton creator */
        static QuestsManager* getInstance();

        /** \brief Singleton creator, but builds an object
                   from XML-parsed data (savegame) */
        static QuestsManager* getInstance(XML_Helper* helper);

        /** \brief Singleton destruction method */
        static void deleteInstance();

        /**
         * \brief Create a random quest and assign it to the hero.
         */
        Quest* createNewQuest(Uint32 heroId);

        /** \brief Quest completion - called by the particular Quest.
         *
         *  Here we deactivate the quest and possibly save the notification
         *  which will be presented to the player.
         */
        void questCompleted(Uint32 heroId);

        /** \brief Quest expiration - called by the particular Quest.
         *
         *  Here we deactivate the quest and possibly save the notification
         *  which will be presented to the player.
         */
        void questExpired(Uint32 heroId);

        /**
         * \brief Get quests for a specific player
         */
        void getPlayerQuests(Player *player, std::vector<Quest*>& dst,
                            std::vector<Hero*>& heroes);

        /**
         * \brief Save all quests 
         */
        bool save(XML_Helper* helper) const;

	protected:
        /** \brief Constructor */
        QuestsManager();

        /** \brief Constructor - use XML data */
        QuestsManager(XML_Helper* helper);

        /** \brief Destructor */
		~QuestsManager();

    private:
        typedef bool (*QFeasibilityType)(void);

        /**
         * \brief Load one quest from the savegame. 
         */
        bool load(std::string tag, XML_Helper* helper);

        //! Does some setup that has to be done on loading as well as creation.
        void _sharedInit();

        /** \brief our slot to the sdyingArmy signal. 
         *
         * Here we detect when a hero dies and deactivate his quests
         * accordingly.
         */
        void _dyingArmy(Army *army, std::vector<Uint32> culprits);

        //! this method deactivates a quest, i.e. marks it as 'to-delete'
        void _deactivateQuest(Uint32 heroId);

        //! this method performs cleanup of the marked quests
        void _cleanup(Player::Type type = Player::HUMAN);


        // Data
        
        /** hash of active quests */
        std::map<Uint32,Quest*> d_quests;

        /** quests that have been marked as 'to-delete' */
        std::queue<Quest*> d_inactive_quests;

        /** \brief Vector of pointers to code (class static members)
         *         checking if a particular question makes sense
         *         in the current game state.
         */
        std::vector<QFeasibilityType> d_questsFeasible;

        /** instance variable pointer */
        static QuestsManager * s_instance;
};

#endif
