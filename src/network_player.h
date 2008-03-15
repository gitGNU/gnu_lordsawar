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

#ifndef NETWORK_PLAYER_H
#define NETWORK_PLAYER_H

#include <string>
#include <list>
#include <SDL_types.h>

#include "player.h"
#include "action.h"

class MoveResult;
class XML_Helper;

/** This class implements the network player.
  */

class NetworkPlayer : public Player
{
    public:
        // CREATORS
        NetworkPlayer(std::string name, Uint32 armyset, SDL_Color color, int width, int height,
                   Player::Type type = Player::HUMAN, int player_no = -1);
        NetworkPlayer(const Player&);
        NetworkPlayer(XML_Helper* helper);
        ~NetworkPlayer();

        //! Saves the data
        virtual bool save(XML_Helper* helper) const;
        
        //! Actions, see player.h for explanation
        virtual bool startTurn();
        virtual bool initTurn();
        virtual bool invadeCity(City* c);
        virtual bool recruitHero(Hero* hero, City *city, int cost);
        virtual bool levelArmy(Army* a);

        bool stackSplit(Stack* s);
        bool stackJoin(Stack* receiver, Stack* joining, bool grouped);
        bool stackDisband(Stack* s);
        bool signpostChange(Signpost *s, std::string message);
        bool cityRename(City *c, std::string name);
        bool vectorFromCity(City* c, Vector<int> dest);
	bool changeVectorDestination(City *c, Vector<int> dest);
	void setFightOrder(std::list<Uint32> order);
        void resign();
        bool heroPlantStandard(Stack *s);
        bool heroDropItem(Hero *h, Item *item, Vector<int> pos);
        bool heroDropAllItems(Hero *h, Vector<int> pos);
        bool heroPickupItem(Hero *h, Item *item, Vector<int> pos);
        bool heroCompletesQuest(Hero *h);


        bool stackMove(Stack* s);
        MoveResult *stackMove(Stack* s, Vector<int> dest, bool follow);
        Reward* stackSearchRuin(Stack* s, Ruin* r);
        int stackVisitTemple(Stack* s, Temple* t);
        Quest* stackGetQuest(Stack* s, Temple*t, bool except_raze);

	bool treachery (Stack *stack, Player *player, Vector <int> pos);

        Fight::Result stackFight(Stack** attacker, Stack** defender,
                                 bool ruin=false);

        Fight::Result stackRuinFight (Stack **attacker, Stack **defender);
	float stackFightAdvise(Stack* s, Vector<int> tile, bool intense_combat);
        bool cityOccupy(City* c);
        bool cityPillage(City* c, int& gold, int& pillaged_army_type);
        bool citySack(City* c, int& gold, std::list<Uint32> *sacked_types);
        bool cityRaze(City* c);
        bool cityBuyProduction(City* c, int slot, int type);
        bool cityChangeProduction(City* c, int slot);
        Uint32 getScore();

    protected:

        //if reward != 0, give this reward, else randomize
        bool giveReward (Stack *s, Reward *r);
        bool stackMoveOneStep(Stack* s);
    private:
        bool cityOccupy(City* c, bool emit);
	void tallyTriumph(Player *p, TriumphType type);
	void decodeAction(const Action *action);
	void decodeActionMove(const Action_Move *action);
	void decodeActionSplit(const Action_Split *action);
	void decodeActionFight(const Action_Fight *action);
	void decodeActionJoin(const Action_Join *action);
	void decodeActionRuin(const Action_Ruin *action);
	void decodeActionTemple(const Action_Temple *action);
	void decodeActionOccupy(const Action_Occupy *action);
	void decodeActionPillage(const Action_Pillage *action);
	void decodeActionSack(const Action_Sack *action);
	void decodeActionRaze(const Action_Raze *action);
	void decodeActionUpgrade(const Action_Upgrade *action);
	void decodeActionBuy(const Action_Buy *action);
	void decodeActionProduction(const Action_Production *action);
	void decodeActionReward(const Action_Reward *action);
	void decodeActionQuest(const Action_Quest *action);
	void decodeActionEquip(const Action_Equip *action);
	void decodeActionLevel(const Action_Level *action);
	void decodeActionDisband(const Action_Disband *action);
	void decodeActionModifySignpost(const Action_ModifySignpost *action);
	void decodeActionRenameCity(const Action_RenameCity *action);
	void decodeActionVector(const Action_Vector *action);
	void decodeActionFightOrder(const Action_FightOrder *action);
	void decodeActionResign(const Action_Resign *action);
	void decodeActionPlant(const Action_Plant *action);
	void decodeActionProduce(const Action_Produce *action);
	void decodeActionProduceVectored(const Action_ProduceVectored *action);
	void decodeActionDiplomacyState(const Action_DiplomacyState *action);
	void decodeActionDiplomacyProposal(const Action_DiplomacyProposal *action);
	void decodeActionDiplomacyScore(const Action_DiplomacyScore *action);
};

#endif // NETWORK_PLAYER_H

// End of file
