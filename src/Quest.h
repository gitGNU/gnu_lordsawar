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

#ifndef __QUEST_H
#define __QUEST_H

#include <string>
#include <list>
#include <queue>
#include "xmlhelper.h"
#include "stack.h"

class QuestsManager;
class Hero;

/** Base class for quests
  *
  * This class adds some basic functionality for quests, mainly code concerning
  * hero association and such.
  * 
  * \note Alas the quest cannot disconnect itself when (during processing
  *       of the signal) it detects that it has been marked as 'not pending'
  *       anymore.
  */
class Quest 
{
    public:

        enum Type {KILLHERO = 1, KILLARMIES = 2,
	           CITYSACK = 3, CITYRAZE = 4, CITYOCCUPY = 5,
	           KILLARMYTYPE = 6, PILLAGEGOLD = 7};

        //! Standard constructor
        Quest(QuestsManager& q_mgr, Uint32 hero, Type type);
        
        //! Loading constructor
        Quest(QuestsManager& q_mgr, XML_Helper* helper);
        virtual ~Quest() {};
        
        /** \brief Notification that the quest is to be deleted.
         */
        void deactivate() {d_pending = false;}

        /** \brief Auxiliary function
         *
         * @param hero      the id of the hero
         * @param stack     if != 0, set to the hero's stack
         * @return pointer to hero or 0 otherwise
         */
         static Hero* getHeroById(Uint32 id, Stack** stack = NULL);

        /** \brief Return the description of the quest 
         *
         *  This is the 'static' part of the quest description,
         *  set once the quest has been initialized. Another,
         *  dynamic part consists of the quest's status info,
         *  which is obtained by the getProgress method.
         */
         std::string getDescription() const { return d_description; }

        /**
         * \brief Get progress information 
         *
         * \param s here we append the progress information
         */
        virtual std::string getProgress() const = 0;

        /**
         * \brief Provide the lines of the message describing
                  the quest completion.
         */
        virtual void getSuccessMsg(std::queue<std::string>& msgs) const = 0;

        /**
         * \brief Provide the lines of the message describing
                  the quest completion.
         */
        virtual void getExpiredMsg(std::queue<std::string>& msgs) const = 0;

        /** Saves the quest data
          * 
          * @note This function is called by the actual quests and only saves
          * the common data. It does NOT open/close tags etc. This has to be
          * done by the derived classes.
          */
        virtual bool save(XML_Helper* helper) const;

	/** provides a list of positions that the hero is seeking.
	 * This method is called by the questmap object to show the
	 * quest on a map.
	 */
	std::list< Vector<int> > getTargets() {return d_targets;}

        /** \brief Get the id of the hero owning this quest */
        Uint32 getHeroId() const { return d_hero; }

        //! Shortcut: Directly returns a pointer to the quest's hero.
        Hero* getHero() const { return getHeroById(d_hero); }

        //! Get the type of the quest
        Uint32 getType() const { return d_type; }

        /** \brief Checks and returns if quest is still valid (hero living etc.) */
        bool isActive();

    protected:
        /** \brief Reference to the QuestManager. */
        QuestsManager& d_q_mgr;

        /** \brief Text description of the quest 
         *
         * This is to be filled by the children (specific quests).
         */
        std::string d_description;

        /** \brief id of the hero owning this quest */
        Uint32 d_hero;

        //! Type of the quest
        Uint32 d_type;

        //! If set to false, this quest is deactivated and not to be processed.
        bool d_pending;

	//! list of targets to display on a questmap
	std::list< Vector<int> > d_targets;
};

#endif
