// Copyright (C) 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2004 Andrea Paternesi
// Copyright (C) 2007, 2008 Ben Asselstine
// Copyright (C) 2008 Ole Laursen
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

#ifndef QUEST_KILL_HERO_H
#define QUEST_KILL_HERO_H

#include <sigc++/trackable.h>

#include <list>
#include "Quest.h"
#include "hero.h"
#include "playerlist.h"


//! A Quest to kill another Player's Hero.
/**
 * A hero that receives this quest has to kill a particular Hero.  The Quest 
 * is completed when this happens, or the quest is expired if enemy Hero dies.
 */
class QuestKillHero : public Quest, public sigc::trackable
{
    public:

	//! Default constructor.
	/**
	 * Make a new kill-hero quest.
	 *
	 * @param q_mgr  The quests manager to associate this quest with.
	 * @param hero   The Id of the Hero who is responsible for the quest.
	 */
        QuestKillHero(QuestsManager& q_mgr, guint32 hero);

	//! Loading constructor.
	/**
	 * @param q_mgr   The quests manager to associate this quest with.
	 * @param helper  The opened saved-game file to load this quest from.
	 */
        QuestKillHero(QuestsManager& q_mgr, XML_Helper* helper);

        // Construct from remote action.
        QuestKillHero(QuestsManager& q_mgr, guint32 hero, guint32 victim);
        
	//! Returns whether or not this quest is impossible.
        /**
	 * Checks to see if any Players have a Hero to target.
	 *
	 * @param heroId  The Id of the Hero responsible for the kill-hero
	 *                quest.
	 *
	 * @return Whether or not the quest is possible.
         */
        static bool isFeasible(guint32 heroId);

        //! Saves the kill-hero quest data to an opened saved-game file.
        bool save(XML_Helper* helper) const;
        
	//! Return a description of how well the quest to kill a hero is going.
        std::string getProgress() const;

	//! Return a queue of strings to show when the quest is compeleted.
         void getSuccessMsg(std::queue<std::string>& msgs) const;

	//! Return a queue of strings to show when the quest has expired.
        void getExpiredMsg(std::queue<std::string>& msgs) const;

        //! Returns the Id of the hunted hero object.
        guint32 getVictim() const {return d_victim;}

	//! Callback for when an Army object is killed.
	/**
	 * This method is used to check when the Hero responsible for the 
	 * quest kills the Hero that is the target of this quest.
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

	//! Choose a hero to be killed.
	/**
	 * @return A pointer to the Hero object to be the target for this
	 *         quest.
	 */
        static Hero* chooseToKill();
        
        //! The Id of the Hero object to be hunted and killed.
        guint32 d_victim;
};

#endif
