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

#ifndef __QUEST_PILLAGE_GOLD_H
#define __QUEST_PILLAGE_GOLD_H

#include <sigc++/trackable.h>

#include <list>
#include "Quest.h"
#include "city.h"

/**
 * \brief Quest - Sack and pillage an amount of gold from enemies cities
 *
 */

class QuestPillageGold : public Quest, public sigc::trackable
{
    public:
        /** \brief Constructor */
        QuestPillageGold(QuestsManager& q_mgr, Uint32 hero);

        /** \brief Constructor - create a new quest from
                   saved data */
        QuestPillageGold(QuestsManager& q_mgr, XML_Helper* helper);
     
        
        //! This quest is always feasible
        static bool isFeasible(Uint32 heroId) {return true;}

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


        //! Returns the amount of gold to be pillaged
        Uint32 getGoldToPillage() {return d_to_pillage;}
         
    private:
         /** slot that receives the ssackingCity and spillagingCity (signal) */
         void citySackedOrPillaged (City* city, Stack* s, int gold, std::list<Uint32> sacked_types);

        void initDescription();


        /** how many armies we should kill */
        Uint32 d_to_pillage;
        /** how many armies we already have killed */
        Uint32 d_pillaged;

	/** the target player */
	Player *d_victim_player;

};

#endif
