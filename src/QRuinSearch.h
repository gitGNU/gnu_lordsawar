#ifndef __QUEST_RUIN_SEARCH_H
#define __QUEST_RUIN_SEARCH_H

#include <sigc++/trackable.h>

#include <list>
#include "Quest.h"


class Ruin;
class XML_Helper;


/** Class describing a ruin quest
  * 
  * A hero that receives this quest has to search a specific ruin to fulfill
  * it.
  */

class QuestRuinSearch : public Quest, public sigc::trackable
{
    public:
        /** \brief Constructor - create a new quest */
        QuestRuinSearch(QuestsManager& q_mgr, Uint32 hero);

        /** \brief Constructor - create a new quest from
                   saved data */
        QuestRuinSearch(QuestsManager& q_mgr, XML_Helper* helper);

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


         //! Returns the id of the ruin to be searched
         Uint32 getRuinId() const {return d_ruin;}

         //! Returns the ruin to be searched
         Ruin* getRuin() const;

    private:
         /** slot that would receive the sruinSearched (signal) */
         void ruinSearched(Ruin* ruin, Stack* s);

         /** \brief Make quest description from the ruin we'll search */
         void initDescription();
         
         /** \brief Count the unsearched ruins */
         static Ruin* chooseToSearch();

         /** ruin id to be searched by the hero */
         Uint32 d_ruin;
};

#endif
