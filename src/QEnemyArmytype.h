//  Copyright (C) 2007, 2008 Ben Asselstine
//
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#ifndef QUEST_ENEMY_ARMYTYPES_H
#define QUEST_ENEMY_ARMYTYPES_H

#include <sigc++/trackable.h>

#include <list>
#include "Quest.h"
#include "army.h"

//! A Quest to kill one army of another Player's Army objects.
/**
 * A hero that receives this quest has to kill a single instance of a 
 * particular king of Army object (e.g. Ghosts).  The Quest is completed when 
 * this happens, and does not expire.
 * This quest presumes that all players have the same Armyset.
 */
class QuestEnemyArmytype : public Quest, public sigc::trackable
{
    public:
	//! Default constructor.
	/**
	 * Make a new kill-armytype quest.
	 *
	 * @param q_mgr  The quests manager to associate this quest with.
	 * @param hero   The Id of the Hero who is responsible for the quest.
	 */
        QuestEnemyArmytype(QuestsManager& q_mgr, Uint32 hero);

	//! Loading constructor.
	/**
	 * @param q_mgr   The quests manager to associate this quest with.
	 * @param helper  The opened saved-game file to load this quest from.
	 */
        QuestEnemyArmytype(QuestsManager& q_mgr, XML_Helper* helper);
     
	//! Returns whether or not this quest is impossible.
        /**
	 * Scans all of the Stack objects for each Player in the Playerlist 
	 * for Army objects that are awardable.  Pick a random one.
	 *
	 * @param heroId  The Id of the Hero responsible for the kill-armytype
	 *                quest.
	 *
	 * @return Whether or not the quest is possible.
         */
        static bool isFeasible(Uint32 heroId);

        //! Saves the kill-armytype quest data to an opened saved-game file.
        bool save(XML_Helper* helper) const;

	//! Return a description of how the quest is going.
        std::string getProgress() const;

	//! Return a queue of strings to show when the quest is compeleted.
        void getSuccessMsg(std::queue<std::string>& msgs) const;

	//! Return a queue of strings to show when the quest has expired.
        void getExpiredMsg(std::queue<std::string>& msgs) const;

        //! Returns the target army type the Hero must kill.
	/**
	 * @return The index of the Army protoype in the Armyset belonging to
	 *         the Player who owns the Hero responsible for this Quest.
	 */
        Uint32 getArmytypeToKill() {return d_type_to_kill;}
         
	//!Callback when an Army object is killed.
	/**
	 * This method is used to check when the Hero kills the correct army
	 * type.
	 *
	 * @param army           A pointer to the Army object that has been
	 *                       killed.
	 * @param heroIsCulprit  Whether or not the Hero object responsible for
	 *                       this Quest was involved with the killing of
	 *                       the given Army object.
	 */
	void armyDied(Army *a, bool heroIsCulprit);

	//! Callback for when a City is defeated.
	/**
	 * @note This method is not used.
	 */
	void cityAction(City *c, CityDefeatedAction action, 
			bool heroIsCulprit, int gold);
    private:

	//! Generate a description of the Quest.
        void initDescription();

	//! The kind of Army object the Hero must kill to succeed.
	/**
	 * The index of the Army protoype in the Armyset belonging to the 
	 * Player who owns the Hero responsible for this Quest.
	 */
        Uint32 d_type_to_kill;
};

#endif
