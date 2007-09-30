#ifndef __QUEST_CITY_OCCUPY_H
#define __QUEST_CITY_OCCUPY_H

#include <sigc++/trackable.h>

#include <list>
#include "Quest.h"


class City;
class XML_Helper;


/** Class describing a city occupy quest
  * 
  * A hero that receives this quest has to occupy a specific city to fulfill
  * it.
  */

class QuestCityOccupy : public Quest, public sigc::trackable
{
    public:
        /** \brief Constructor - create a new quest */
        QuestCityOccupy(QuestsManager& q_mgr, Uint32 hero);

        /** \brief Constructor - create a new quest from
                   saved data */
        QuestCityOccupy(QuestsManager& q_mgr, XML_Helper* helper);

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


         //! Returns the id of the city to be occupied
         Uint32 getCityId() const {return d_city;}

         //! Returns the city to be occupied
         City* getCity() const;

	 void armyDied(Army *a, bool heroIsCulprit);
	 void cityAction(City *c, CityDefeatedAction action, 
			 bool heroIsCulprit, int gold);
    private:

         /** \brief Make quest description from the city we'll occupy */
         void initDescription();
         
         /** \brief Select a victim city */
         static City* chooseToOccupy(Player *p);

         /** city id to be occupied by the hero */
         Uint32 d_city;
};

#endif
