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

#include <fstream>
#include <algorithm>
#include <stdlib.h>

#include "network_player.h"
#include "playerlist.h"
#include "armysetlist.h"
#include "stacklist.h"
#include "citylist.h"
#include "portlist.h"
#include "templelist.h"
#include "ruinlist.h"
#include "signpostlist.h"
#include "rewardlist.h"
#include "QuestsManager.h"
#include "path.h"
#include "GameMap.h"
#include "army.h"
#include "hero.h"
#include "action.h"
#include "MoveResult.h"
#include "Configuration.h"
#include "FogMap.h"
#include "xmlhelper.h"
#include "ruinlist.h"
#include "game-parameters.h"
#include "signpost.h"
#include "history.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

NetworkPlayer::NetworkPlayer(string name, Uint32 armyset, SDL_Color color, int width,
		       int height, Player::Type type, int player_no)
    :Player(name, armyset, color, width, height, type, player_no)
{
}

NetworkPlayer::NetworkPlayer(const Player& player)
    :Player(player)
{
    d_type = Player::NETWORKED;
}

NetworkPlayer::NetworkPlayer(XML_Helper* helper)
    :Player(helper)
{
}

NetworkPlayer::~NetworkPlayer()
{
}

bool NetworkPlayer::save(XML_Helper* helper) const
{
    // This may seem a bit dumb, but allows derived players (especially
    // AI's) to save additional data, such as character types or so.
    bool retval = true;
    retval &= helper->openTag("player");
    retval &= Player::save(helper);
    retval &= helper->closeTag();

    return retval;
}

bool NetworkPlayer::initTurn()
{
  //get data from our corresponding real player from the network
    clearActionlist();
    //get our action list,
    //get our history list.
    return true;
}

bool NetworkPlayer::startTurn()
{
    return true;
}

bool NetworkPlayer::invadeCity(City* c)
{
    return true;
}

bool NetworkPlayer::recruitHero(Hero* hero, City *city, int cost)
{
    return true;
}

bool NetworkPlayer::levelArmy(Army* a)
{
    
    return true;
}

bool NetworkPlayer::stackSplit(Stack* s)
{
    debug("NetworkPlayer::stackSplit("<<s->getId()<<")")


    return true;
}

bool NetworkPlayer::stackJoin(Stack* receiver, Stack* joining, bool grouped)
{
    debug("NetworkPlayer::stackJoin("<<receiver->getId()<<","<<joining->getId()<<")")

    return true;
}


bool NetworkPlayer::signpostChange(Signpost *s, std::string message)
{
  return true;
}

bool NetworkPlayer::cityRename(City *c, std::string name)
{
  return true;
}

bool NetworkPlayer::stackDisband(Stack* s)
{
    debug("player::stackDisband(Stack*)")
    return true;
}

bool NetworkPlayer::stackMove(Stack* s)
{
    debug("player::stackMove(Stack*)")

    return true;
}

MoveResult *NetworkPlayer::stackMove(Stack* s, Vector<int> dest, bool follow)
{
    debug("player::stack_move()");
    return new MoveResult(true);
}

Fight::Result NetworkPlayer::stackRuinFight (Stack **attacker, Stack **defender)
{
    Fight::Result result = Fight::DRAW;
    debug("stackRuinFight: player = " << getName()<<" at position "
          <<(*defender)->getPos().x<<","<<(*defender)->getPos().y);
      return Fight::ATTACKER_WON;
}

bool NetworkPlayer::treachery (Stack *stack, Player *player, Vector <int> pos)
{
  return true;
}

Fight::Result NetworkPlayer::stackFight(Stack** attacker, Stack** defender, bool ruin) 
{
    debug("stackFight: player = " << getName()<<" at position "
          <<(*defender)->getPos().x<<","<<(*defender)->getPos().y);
    if (ruin)
      return NetworkPlayer::stackRuinFight (attacker, defender);

      return Fight::ATTACKER_WON;
}

Reward* NetworkPlayer::stackSearchRuin(Stack* s, Ruin* r)
{
    Reward *retReward = NULL;
    debug("NetworkPlayer::stack_search_ruin")

     return retReward;
}

int NetworkPlayer::stackVisitTemple(Stack* s, Temple* t)
{
  debug("NetworkPlayer::stackVisitTemple")

  return 0;
}

Quest* NetworkPlayer::stackGetQuest(Stack* s, Temple* t, bool except_raze)
{
  debug("NetworkPlayer::stackGetQuest")
  return NULL;
}

bool NetworkPlayer::cityOccupy(City* c)
{
  debug("NetworkPlayer::cityOccupy")

    return true;
}

bool NetworkPlayer::cityPillage(City* c, int& gold, int& pillaged_army_type)
{
  debug("NetworkPlayer::cityPillage")
  return true;
}

bool NetworkPlayer::citySack(City* c, int& gold, std::list<Uint32> *sacked_types)
{
  debug("NetworkPlayer::citySack")
  return true;
}

bool NetworkPlayer::cityRaze(City* c)
{
  debug("NetworkPlayer::cityRaze")
  return true;
}

bool NetworkPlayer::cityBuyProduction(City* c, int slot, int type)
{
  return true;
}

bool NetworkPlayer::cityChangeProduction(City* c, int slot)
{
  return true;
}

bool NetworkPlayer::giveReward(Stack *s, Reward *reward)
{
  debug("NetworkPlayer::give_reward")

  return true;
}

bool NetworkPlayer::stackMoveOneStep(Stack* s)
{
  return true;
}

bool NetworkPlayer::changeVectorDestination(City *c, Vector<int> dest)
{
  return true;
}

bool NetworkPlayer::vectorFromCity(City * c, Vector<int> dest)
{
  return true;
}

void NetworkPlayer::setFightOrder(std::list<Uint32> order) 
{
}

void NetworkPlayer::resign() 
{
}

bool NetworkPlayer::heroPlantStandard(Stack* s)
{
  debug("player::heroPlantStandard(Stack*)")
  return true;
}

bool NetworkPlayer::heroDropAllItems(Hero *h, Vector<int> pos)
{
  return true;
}

bool NetworkPlayer::heroDropItem(Hero *h, Item *i, Vector<int> pos)
{
  return true;
}

bool NetworkPlayer::heroPickupItem(Hero *h, Item *i, Vector<int> pos)
{
  return true;
}

bool NetworkPlayer::heroCompletesQuest(Hero *h)
{
  return true;
}

Uint32 NetworkPlayer::getScore()
{
  return 0;
}

void NetworkPlayer::tallyTriumph(Player *p, TriumphType type)
{
}

float NetworkPlayer::stackFightAdvise(Stack* s, Vector<int> tile, 
				   bool intense_combat)
{
  float percent = 0.0;
        
  return percent;
}

void NetworkPlayer::decodeAction(const Action *a)
{
  switch(a->getType())
    {
    case Action::STACK_MOVE:
      return decodeActionMove(dynamic_cast<const Action_Move*>(a));
    case Action::STACK_SPLIT:
      return decodeActionSplit(dynamic_cast<const Action_Split*>(a));
    case Action::STACK_FIGHT:
      return decodeActionFight(dynamic_cast<const Action_Fight*>(a));
    case Action::STACK_JOIN:
      return decodeActionJoin(dynamic_cast<const Action_Join*>(a));
    case Action::RUIN_SEARCH:
      return decodeActionRuin(dynamic_cast<const Action_Ruin*>(a));
    case Action::TEMPLE_SEARCH:
      return decodeActionTemple(dynamic_cast<const Action_Temple*>(a));
    case Action::CITY_OCCUPY:
      return decodeActionOccupy(dynamic_cast<const Action_Occupy*>(a));
    case Action::CITY_PILLAGE:
      return decodeActionPillage
	(dynamic_cast<const Action_Pillage*>(a));
    case Action::CITY_SACK:
      return decodeActionSack(dynamic_cast<const Action_Sack*>(a));
    case Action::CITY_RAZE:
      return decodeActionRaze(dynamic_cast<const Action_Raze*>(a));
    case Action::CITY_UPGRADE:
      return decodeActionUpgrade (dynamic_cast<const Action_Upgrade*>(a));
    case Action::CITY_BUY:
      return decodeActionBuy(dynamic_cast<const Action_Buy*>(a));
    case Action::CITY_PROD:
      return decodeActionProduction 
	(dynamic_cast<const Action_Production*>(a));
    case Action::REWARD: 
      return decodeActionReward(dynamic_cast<const Action_Reward*>(a));
    case Action::QUEST:
      return decodeActionQuest(dynamic_cast<const Action_Quest*>(a));
    case Action::HERO_EQUIP:
      return decodeActionEquip(dynamic_cast<const Action_Equip*>(a));
    case Action::UNIT_ADVANCE:
      return decodeActionLevel(dynamic_cast<const Action_Level*>(a));
    case Action::STACK_DISBAND:
      return decodeActionDisband (dynamic_cast<const Action_Disband*>(a));
    case Action::MODIFY_SIGNPOST:
      return decodeActionModifySignpost
	(dynamic_cast<const Action_ModifySignpost*>(a));
    case Action::CITY_RENAME:
      return decodeActionRenameCity 
	(dynamic_cast<const Action_RenameCity*>(a));
    case Action::CITY_VECTOR:
      return decodeActionVector (dynamic_cast<const Action_Vector*>(a));
    case Action::FIGHT_ORDER:
      return decodeActionFightOrder 
	(dynamic_cast<const Action_FightOrder*>(a));
    case Action::RESIGN:
      return decodeActionResign(dynamic_cast<const Action_Resign*>(a));
    case Action::ITEM_PLANT:
      return decodeActionPlant(dynamic_cast<const Action_Plant*>(a));
    case Action::PRODUCE_UNIT:
      return decodeActionProduce (dynamic_cast<const Action_Produce*>(a));
    case Action::PRODUCE_VECTORED_UNIT:
      return decodeActionProduceVectored
	(dynamic_cast<const Action_ProduceVectored*>(a));
    case Action::DIPLOMATIC_STATE:
      return decodeActionDiplomacyState 
	(dynamic_cast<const Action_DiplomacyState*>(a));
    case Action::DIPLOMATIC_PROPOSAL:
      return decodeActionDiplomacyProposal
	(dynamic_cast<const Action_DiplomacyProposal*>(a));
    case Action::DIPLOMATIC_SCORE:
      return decodeActionDiplomacyScore
	(dynamic_cast<const Action_DiplomacyScore*>(a));
    }

  return;
}

void NetworkPlayer::decodeActionMove(const Action_Move *action)
{
}

void NetworkPlayer::decodeActionSplit(const Action_Split *action)
{
}

void NetworkPlayer::decodeActionFight(const Action_Fight *action)
{
}

void NetworkPlayer::decodeActionJoin(const Action_Join *action)
{
}

void NetworkPlayer::decodeActionRuin(const Action_Ruin *action)
{
}

void NetworkPlayer::decodeActionTemple(const Action_Temple *action)
{
}

void NetworkPlayer::decodeActionOccupy(const Action_Occupy *action)
{
}

void NetworkPlayer::decodeActionPillage(const Action_Pillage *action)
{
}

void NetworkPlayer::decodeActionSack(const Action_Sack *action)
{
}

void NetworkPlayer::decodeActionRaze(const Action_Raze *action)
{
}

void NetworkPlayer::decodeActionUpgrade(const Action_Upgrade *action)
{
}

void NetworkPlayer::decodeActionBuy(const Action_Buy *action)
{
}

void NetworkPlayer::decodeActionProduction(const Action_Production *action)
{
}

void NetworkPlayer::decodeActionReward(const Action_Reward *action)
{
}

void NetworkPlayer::decodeActionQuest(const Action_Quest *action)
{
}

void NetworkPlayer::decodeActionEquip(const Action_Equip *action)
{
}

void NetworkPlayer::decodeActionLevel(const Action_Level *action)
{
}

void NetworkPlayer::decodeActionDisband(const Action_Disband *action)
{
}

void NetworkPlayer::decodeActionModifySignpost(const Action_ModifySignpost *action)
{
}

void NetworkPlayer::decodeActionRenameCity(const Action_RenameCity *action)
{
}

void NetworkPlayer::decodeActionVector(const Action_Vector *action)
{
}

void NetworkPlayer::decodeActionFightOrder(const Action_FightOrder *action)
{
}

void NetworkPlayer::decodeActionResign(const Action_Resign *action)
{
}

void NetworkPlayer::decodeActionPlant(const Action_Plant *action)
{
}

void NetworkPlayer::decodeActionProduce(const Action_Produce *action)
{
}

void NetworkPlayer::decodeActionProduceVectored(const Action_ProduceVectored *action)
{
}

void NetworkPlayer::decodeActionDiplomacyState(const Action_DiplomacyState *action)
{
}

void NetworkPlayer::decodeActionDiplomacyProposal(const Action_DiplomacyProposal *action)
{
}

void NetworkPlayer::decodeActionDiplomacyScore(const Action_DiplomacyScore *action)
{
}
// End of file
