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

#ifndef QUEST_ENEMY_ARMIES_H
#define QUEST_ENEMY_ARMIES_H

#include <sigc++/trackable.h>

#include <list>
#include "Quest.h"
#include "army.h"
#include "player.h"


//! A Quest to kill a certain number of another Player's Army objects.
/**
 * A hero that receives this quest has to kill a number of armies.  The Quest 
 * is completed when this happens, or the quest is expired if enemy Player 
 * dies.
 */
class QuestEnemyArmies : public Quest, public sigc::trackable
{
    public:
	//! Default constructor.
	/**
	 * Make a new kill-armies quest.
	 *
	 * @param q_mgr  The quests manager to associate this quest with.
	 * @param hero   The Id of the Hero who is responsible for the quest.
	 */
        QuestEnemyArmies(QuestsManager& q_mgr, Uint32 hero);

        // Construct from remote action.
        QuestEnemyArmies(QuestsManager& q_mgr, Uint32 hero,
                         Uint32 armies_to_kill, Uint32 victim_player);
        
	//! Loading constructor.
	/**
	 * @param q_mgr   The quests manager to associate this quest with.
	 * @param helper  The opened saved-game file to load this quest from.
	 */
        QuestEnemyArmies(QuestsManager& q_mgr, XML_Helper* helper);
     
	//! Returns whether or not this quest is impossible.
        /**
	 * Scans all Player objects in the Playerlist to see if there is one 
	 * that is alive that isn't the neutral player.
	 *
	 * @note This method is static because it is executed before the
	 *       Quest is instantiated.  It is also called from within the
	 *       instantiated Quest.
	 *
	 * @param heroId  The Id of the Hero responsible for the kill-armies 
	 *                quest.
	 *
	 * @return Whether or not the quest is possible.
         */
        static bool isFeasible(Uint32 heroId);

        //! Saves the kill-armies quest data to an opened saved-game file.
        bool save(XML_Helper* helper) const;

	//! Return a description of how many armies have been killed so far.
        std::string getProgress() const;

	//! Return a queue of strings to show when the quest is compeleted.
        void getSuccessMsg(std::queue<std::string>& msgs) const;

	//! Return a queue of strings to show when the quest has expired.
        void getExpiredMsg(std::queue<std::string>& msgs) const;

        //! Returns the number of Army objects to be killed in this Quest.
        Uint32 getArmiesToKill() {return d_to_kill;}
         
	//! Returns the enemy player whose Army objects are to be killed.
	Uint32 getVictimPlayerId() {return d_victim_player->getId();}

	//! Callback for when an Army object is killed.
	/**
	 * This method is used to account for the number armies killed by the
	 * Hero.
	 *
	 * @param army           A pointer to the Army object that has been
	 *                       killed.
	 * @param heroIsCulprit  Whether or not the Hero object responsible for
	 *                       this Quest was involved with the killing of
	 *                       the given Army object.
	 */
	void armyDied(Army *army, bool heroIsCulprit);

	//! Callback for when a City is defeated.
	/**
	 * @note This method is not used.
	 */
	void cityAction(City *city, CityDefeatedAction action, 
			bool heroIsCulprit, int gold);
    private:

	//! Generate a description of the Quest.
        void initDescription();

	//! Recalculate the positions on the map of the target Army objects.
        void update_targets();

	//! The number of Army objects the Hero must kill to succeed.
        Uint32 d_to_kill;

	//! The number of Army objects the Hero has already killed.
        Uint32 d_killed;

	//! The victim player who the Hero is targeting Army objects of.
	Player *d_victim_player;
};

#endif
