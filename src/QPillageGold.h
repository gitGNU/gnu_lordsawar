//  Copyright (C) 2007, 2008 Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
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

#ifndef QUEST_PILLAGE_GOLD_H
#define QUEST_PILLAGE_GOLD_H

#include <sigc++/trackable.h>

#include <list>
#include "Quest.h"
#include "city.h"

//! A Quest to accrue an amount of gold pieces from another Player.
/**
 * The Hero is required to conquer cities and obtain a given amount of gold
 * pieces from a victim Player.  Sacking and pillaging means to cash-in Army 
 * production bases from conquered cities.
 *
 * The quest succeeds when the Player successfully accrues the given amount
 * of gold pieces.  This quest never expires.
 */
class QuestPillageGold : public Quest, public sigc::trackable
{
    public:
	//! Default constructor.
	/**
	 * Make a new sack and pillage quest.
	 *
	 * @param q_mgr  The quests manager to associate this quest with.
	 * @param hero   The Id of the Hero who is responsible for the quest.
	 */
        QuestPillageGold(QuestsManager& q_mgr, guint32 hero);

	//! Loading constructor.
	/**
	 * @param q_mgr   The quests manager to associate this quest with.
	 * @param helper  The opened saved-game file to load this quest from.
	 */
        QuestPillageGold(QuestsManager& q_mgr, XML_Helper* helper);
     
        // Construct from remote action.
        QuestPillageGold(QuestsManager& q_mgr, guint32 hero, guint32 gold);
        
	//! Returns that this quest is feasible.
        static bool isFeasible(guint32 heroId) {return true;}

        //! Saves the sack and pillage quest data to an opened saved-game file.
        bool save(XML_Helper* helper) const;

	//! Return a description of how many gold pieces have been accrued.
        std::string getProgress() const;

	//! Return a queue of strings to show when the quest is compeleted.
        void getSuccessMsg(std::queue<std::string>& msgs) const;

	//! Return a queue of strings to show when the quest has expired.
        void getExpiredMsg(std::queue<std::string>& msgs) const;

        //! Returns the amount of gold to be pillaged.
        guint32 getGoldToPillage() {return d_to_pillage;}
         
	//! Callback for when an Army object is killed.
	/**
	 * @note This method is not used.
	 */
	void armyDied(Army *a, bool heroIsCulprit);

	//! Callback for when a City object is defeated.
	/**
	 * This method notifies the Quest that a City has fallen, and what the 
	 * conquering action (pillage/sack/raze/occupy) was.  It also notifies
	 * whether or not the hero responsible for this quest was involved in 
	 * the conquering, and how much gold was taken as a result.
	 *
	 * The amount of gold is added to Quest_Pillage::d_pillaged.
	 *
	 * @param city           The City object that has been conquered.
	 * @param action         What action was taken by the Player.  See
	 *                       CityDefeatedAction for more information.
	 * @param heroIsCulprit  Whether or not the Hero object associated with
	 *                       this Quest object is responsible for 
	 *                       conquering the given City object.
	 * @param gold           How many gold pieces were taken as a result
	 *                       of the action.
	 */
	void cityAction(City *city, CityDefeatedAction action, 
			bool heroIsCulprit, int gold);

    private:

	//! Generate a description of the Quest.
        void initDescription();

        //! The amount of gold pieces to sack and pillage to succeed.
        guint32 d_to_pillage;

        //! The number of gold pieces already sacked and pillaged.
        guint32 d_pillaged;

	//! The player whose cities this quest is targetting.
	Player *d_victim_player;

};

#endif
