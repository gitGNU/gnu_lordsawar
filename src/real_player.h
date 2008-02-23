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

/** 
 * This class implements the abstract Player class in a reasonable manner
 * for local players. It is suitable for local human players, and AI players
 * can derive from this class and overwrite the start_turn and other 
 * callback methods for their own purposes.  For complete descriptions of
 * the callback functions see the Player class.
 */

class RealPlayer : public Player
{
    public:
        // CREATORS
        RealPlayer(std::string name, Uint32 armyset, SDL_Color color, 
		   int width, int height, Player::Type type = Player::HUMAN, 
		   int player_no = -1);
        RealPlayer(const Player&);
        RealPlayer(XML_Helper* helper);
        ~RealPlayer();

        virtual Uint32 getScore();

	virtual void setFightOrder(std::list<Uint32> order);

        virtual bool save(XML_Helper* helper) const;

        virtual bool startTurn();
        virtual bool initTurn();
        virtual bool invadeCity(City* c);
        virtual bool recruitHero(Hero* hero, City *city, int cost);
        virtual bool levelArmy(Army* a);
        virtual bool stackSplit(Stack* s);
        virtual bool stackJoin(Stack* receiver, Stack* joining, bool grouped);
        virtual bool stackMove(Stack* s);
        virtual MoveResult *stackMove(Stack* s, Vector<int> dest, bool follow);
        virtual Fight::Result stackFight(Stack** attacker, Stack** defender,
					 bool ruin=false);
        virtual Fight::Result stackRuinFight (Stack **attacker, 
					      Stack **defender);
	virtual bool treachery (Stack *stack, Player *player, Vector <int> pos);
        virtual Reward* stackSearchRuin(Stack* s, Ruin* r);
        virtual int stackVisitTemple(Stack* s, Temple* t);
        virtual Quest* stackGetQuest(Stack* s, Temple*t, bool except_raze);
	virtual float stackFightAdvise(Stack* s, Vector<int> tile, 
				       bool intense_combat);
        virtual bool cityOccupy(City* c);
        virtual bool cityPillage(City* c, int& gold, int& pillaged_army_type);
        virtual bool citySack(City* c, int& gold, 
			      std::list<Uint32> *sacked_types);
        virtual bool cityRaze(City* c);
        virtual bool cityBuyProduction(City* c, int slot, int type);
        virtual bool cityChangeProduction(City* c, int slot);
        virtual bool giveReward (Stack *s, Reward *r);
        virtual bool stackDisband(Stack* s);
        virtual bool heroDropItem(Hero *h, Item *item, Vector<int> pos);
        virtual bool heroDropAllItems(Hero *h, Vector<int> pos);
        virtual bool heroPickupItem(Hero *h, Item *item, Vector<int> pos);
        virtual bool heroCompletesQuest(Hero *h);
        virtual void resign();
        virtual bool signpostChange(Signpost *s, std::string message);
        virtual bool cityRename(City *c, std::string name);
        virtual bool vectorFromCity(City* c, Vector<int> dest);
	virtual bool changeVectorDestination(City *c, Vector<int> dest);
        virtual bool heroPlantStandard(Stack *s);

    protected:
        /**
	 * Returns all heroes in the given list of stacks.
         *
         * @param stacks           the list of stacks which is searched.
         * @param heroes           Return a list of id's of the heroes found.
         */
	//! Get heroes.
        void getHeroes(const std::list<Stack*> stacks, 
		       std::vector<Uint32>& heroes);

        /** 
	 * Goes through a list of stacks and removes all armies with less
         * than 1 hitpoint.  It also removes empty stacks. 
         * This function also heals regenerating units at the end of combat. 
         *
         * @param stacks           The list searched for dead armies.
         * @param culprits         The list of heroes responsible for killing
	 *                         the armies.  This is needed for tracking
	 *                         the progress of a Quest.
         * @return The sum of the XP of the killed armies.
         */
	//! Remove dead Armies from a list of stacks after a fight.
        double removeDeadArmies(std::list<Stack*>& stacks,
                              std::vector<Uint32>& culprits);
        
        /** 
	 * Increases the number of experience points of a stack
         * the number of battles and checks if an army can get a medal
         *
         * This functions takes a number of experience points and distributes
         * them equally over all armies in the stack list. Therefore, the less
         * armies fight, the more experience the single armies get. It emits a
         * signal when a unit gains a level.
         *
         * @param stacks           A list of all stacks gaining experience.
         * @param xp_sum           The number of XP to distribute.
         */
	//! update Army state after a Fight.
        void updateArmyValues(std::list<Stack*>& stacks, double xp_sum);

	//! Move stack forward one step forward on it's Path.
        bool stackMoveOneStep(Stack* s);
    private:
        bool cityOccupy(City* c, bool emit);
	void tallyTriumph(Player *p, TriumphType type);
};

#endif // REAL_PLAYER_H

// End of file
