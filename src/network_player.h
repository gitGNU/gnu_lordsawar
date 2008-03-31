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
        virtual void endTurn();
        virtual void invadeCity(City* c);
        virtual void levelArmy(Army* a);

	void decodeAction(const Action *action);
        
    private:
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
	void decodeActionEndTurn(const Action_EndTurn *action);
	void decodeActionConquerCity(const Action_ConquerCity *action);
	void decodeActionRecruitHero(const Action_RecruitHero *action);
};

#endif // NETWORK_PLAYER_H

// End of file
