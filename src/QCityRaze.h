#ifndef __QUEST_CITY_RAZE_H
#define __QUEST_CITY_RAZE_H

#include <sigc++/trackable.h>

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

    private:
         /** slot that would receive the scityRazed (signal) */
         void cityRazed (City* city, Stack* s);

         /** \brief Make quest description from the city we'll raze */
         void initDescription();
         
         /** \brief Select a victim city */
         static City* chooseToRaze(Player *p);

         /** city id to be razed by the hero */
         Uint32 d_city;
};

#endif
