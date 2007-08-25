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

#ifndef REAL_PLAYER_H
#define REAL_PLAYER_H

#include <string>
#include <list>
#include <SDL_types.h>

#include "player.h"

class MoveResult;
class XML_Helper;

/** This class implements the action function from player.h in a "reasonable"
  * manner for local players. It is suitable for local human players, AI players
  * can derive from this class and overwrite the start_turn and "callback"
  * functions for their own purposes. I will not describe the single functions
  * here apart from newly introduced ones, see the detailed comments in player.h.
  */

class RealPlayer : public Player
{
    public:
        // CREATORS
        RealPlayer(std::string name, Uint32 armyset, SDL_Color color,
                   Player::Type type = Player::HUMAN, int player_no = -1);
        RealPlayer(const Player&);
        RealPlayer(XML_Helper* helper);
        ~RealPlayer();

        //! Saves the data
        virtual bool save(XML_Helper* helper) const;
        
        //! Actions, see player.h for explanation
        virtual bool startTurn();
        virtual bool invadeCity(City* c);
        virtual bool recruitHero(Hero* hero, City *city, int cost);
        virtual bool levelArmy(Army* a);

        bool stackSplit(Stack* s);
        bool stackJoin(Stack* receiver, Stack* joining, bool grouped);
        bool stackDisband(Stack* s);
        bool signpostChange(Signpost *s, std::string message);

        bool stackMove(Stack* s);
        MoveResult *stackMove(Stack* s, Vector<int> dest, bool follow);
        Reward* stackSearchRuin(Stack* s, Ruin* r);
        int stackVisitTemple(Stack* s, Temple* t);
        Quest* stackGetQuest(Stack* s, Temple*t);

        Fight::Result stackFight(Stack** attacker, Stack** defender,
                                 bool ruin=false);

        Fight::Result stackRuinFight (Stack **attacker, Stack **defender);
        bool cityOccupy(City* c);
        bool cityPillage(City* c, int& gold, int& pillaged_army_type);
        bool citySack(City* c, int& gold, std::list<Uint32> *sacked_types);
        bool cityRaze(City* c);
        bool cityBuyProduction(City* c, int slot, int type);
        bool cityChangeProduction(City* c, int slot);

    protected:
        /**
          * Returns all heroes in the stack list
          *
          * @param stacks           the list which is searched
          * @param dst              a list of id's of the heroes found
          */
        void getHeroes(const std::list<Stack*> stacks, std::vector<Uint32>& dst);

        /** Goes through a list of stacks and removes all armies with less
          * than 1 hitpoint. It also removes empty stacks. 
          * TODO: This function also heals regenerating units at the end of
          * combat. Since this is like the last place to search for it, perhaps
          * find a better solution?
          *
          * @param stacks           the list searched for dead armies
          * @param culprits         the list of heroes responsible for killing
          *                         (needed for quest handling)
          * @return the sum of the XP of the killed armies
          */
        double removeDeadArmies(std::list<Stack*>& stacks,
                              std::vector<Uint32>& culprits);
        
        /** Increases the number of experience points of a stack
          * the number of battles and checks if an army can get a medal
          *
          * This functions takes a number of experience points and distributes
          * them equally over all armies in the stack list. Therefore, the less
          * armies fight, the more experience the single armies get. It emits a
          * signal when a unit gains a level.
          *
          * @param stacks           a list of all stacks gaining experience
          * @param xp_sum           the number of XP to distribute
          */
        void updateArmyValues(std::list<Stack*>& stacks, double xp_sum);

        //if reward != 0, give this reward, else randomize
        bool giveReward (Stack *s, Reward *r);
        bool stackMoveOneStep(Stack* s);
    private:
        bool cityOccupy(City* c, bool emit);
};

#endif // REAL_PLAYER_H

// End of file
