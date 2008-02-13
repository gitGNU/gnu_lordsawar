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
#ifndef __QUEST_CITY_RAZE_H
#define __QUEST_CITY_RAZE_H

#include <sigc++/trackable.h>

#include <list>
#include "Quest.h"


class City;
class XML_Helper;


/** Class describing a city raze quest
  * 
  * A hero that receives this quest has to raze a specific city to fulfill
  * it.
  */

class QuestCityRaze: public Quest, public sigc::trackable
{
    public:
        /** \brief Constructor - create a new quest */
        QuestCityRaze(QuestsManager& q_mgr, Uint32 hero);

        /** \brief Constructor - create a new quest from
                   saved data */
        QuestCityRaze(QuestsManager& q_mgr, XML_Helper* helper);

         /**
          * \brief Checks if such a quest is possible at all.
          */
         static bool isFeasible(Uint32 heroId);

         //! Saves the quest data.
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


         //! Returns the id of the city to be razed
         Uint32 getCityId() const {return d_city;}

         //! Returns the city to be razed
         City* getCity() const;

	 void armyDied(Army *a, bool heroIsCulprit);
	 void cityAction(City *c, CityDefeatedAction action, 
			 bool heroIsCulprit, int gold);
    private:

         /** \brief Make quest description from the city we'll raze */
         void initDescription();
         
         /** \brief Select a victim city */
         static City* chooseToRaze(Player *p);

         /** city id to be razed by the hero */
         Uint32 d_city;

};

#endif
