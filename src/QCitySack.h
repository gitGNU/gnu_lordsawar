#ifndef __QUEST_CITY_SACK_H
#define __QUEST_CITY_SACK_H

#include <sigc++/trackable.h>

#include <list>
#include "Quest.h"


class City;
class XML_Helper;


/** Class describing a city sack quest
  * 
  * A hero that receives this quest has to sack a specific city to fulfill
  * it.
  */

class QuestCitySack : public Quest, public sigc::trackable
{
    public:
        /** \brief Constructor - create a new quest */
        QuestCitySack(QuestsManager& q_mgr, Uint32 hero);

        /** \brief Constructor - create a new quest from
                   saved data */
        QuestCitySack(QuestsManager& q_mgr, XML_Helper* helper);

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


         //! Returns the id of the city to be sacked
         Uint32 getCityId() const {return d_city;}

         //! Returns the city to be sacked
         City* getCity() const;

    private:
         /** slot that would receive the scitySacked (signal) */
         void citySacked (City* city, Stack* s, int gold);

         /** \brief Make quest description from the city we'll sack */
         void initDescription();
         
         /** \brief Select a victim city */
         static City* chooseToSack(Player *p);

         /** city id to be sacked by the hero */
         Uint32 d_city;
};

#endif
