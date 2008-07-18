// Copyright (C) 2008 Ben Asselstine
// Copyright (C) 2008 Ole Laursen
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

#include <fstream>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>

#include "network_player.h"
#include "playerlist.h"
#include "armysetlist.h"
#include "stacklist.h"
#include "citylist.h"
#include "portlist.h"
#include "templelist.h"
#include "ruinlist.h"
#include "signpostlist.h"
#include "Itemlist.h"
#include "rewardlist.h"
#include "QuestsManager.h"
#include "Quest.h"
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
    :Player(name, armyset, color, width, height, type, player_no),
    d_connected(false)
{
}

NetworkPlayer::NetworkPlayer(const Player& player)
    :Player(player), d_connected(false)
{
    d_type = Player::NETWORKED;
}

NetworkPlayer::NetworkPlayer(XML_Helper* helper)
    :Player(helper), d_connected(false)
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

bool NetworkPlayer::startTurn()
{
  //d_stacklist->setActivestack(0);

  return false;
}

void NetworkPlayer::endTurn()
{
}

void NetworkPlayer::invadeCity(City* c)
{
  assert(false);
}

void NetworkPlayer::levelArmy(Army* a)
{
  assert(false);
}

void NetworkPlayer::decodeActions(std::list<Action *> actions)
{
  std::list<Action*>::iterator it = actions.begin();
  pruneActionlist();
  for (; it != actions.end(); it++)
    {
     decodeAction(*it);
    }
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
    case Action::END_TURN:
      return decodeActionEndTurn
        (dynamic_cast<const Action_EndTurn*>(a));
    case Action::CITY_CONQUER:
      return decodeActionConquerCity
        (dynamic_cast<const Action_ConquerCity*>(a));
    case Action::RECRUIT_HERO:
      return decodeActionRecruitHero
        (dynamic_cast<const Action_RecruitHero*>(a));
    case Action::PLAYER_RENAME:
      return decodeActionRenamePlayer
        (dynamic_cast<const Action_RenamePlayer*>(a));
    case Action::CITY_DESTITUTE:
      return decodeActionCityTooPoorToProduce
	(dynamic_cast<const Action_CityTooPoorToProduce*>(a));
    }

  return;
}

// decode helpers
  
Stack *findStackById(Uint32 id)
{
  for (Playerlist::iterator j = Playerlist::getInstance()->begin(), jend = Playerlist::getInstance()->end();
       j != jend; ++j) {
    Stack *s = (*j)->getStacklist()->getStackById(id);
    if (s)
      return s;
  }
  return 0;
}

Item *findItemById(const std::list<Item *> &l, Uint32 id) 
{
  for (std::list<Item *>::const_iterator i = l.begin(), end = l.end(); i != end; ++i)
    if ((*i)->getId() == id)
      return *i;
  
  return 0;
}


// decoders

void NetworkPlayer::decodeActionMove(const Action_Move *action)
{
  Stack *stack = d_stacklist->getStackById(action->getStackId());
  stack->moveToDest(action->getEndingPosition());
  supdatingStack.emit(stack);
}

void NetworkPlayer::decodeActionSplit(const Action_Split *action)
{
  Stack *stack = d_stacklist->getStackById(action->getStackId());
  
  for (unsigned int i = 0; i < MAX_STACK_SIZE; ++i) {
    Uint32 army_id = action->getGroupedArmyId(i);
    if (army_id == 0)
      continue;

    Army *army = stack->getArmyById(army_id);
    army->setGrouped(false);
  }

  //fixme: the new stack should have an Id of getNewStackId()
  doStackSplit(stack);
}

void NetworkPlayer::decodeActionFight(const Action_Fight *action)
{
  std::list<Stack *> attackers, defenders;
  std::list<Uint32> attacker_army_ids = action->getAttackerArmyIds();
  for (std::list<Uint32>::const_iterator i = attacker_army_ids.begin(),
         end = attacker_army_ids.end(); i != end; ++i)
    attackers.push_back(d_stacklist->getStackById(*i));

  std::list<Uint32> defender_army_ids = action->getDefenderArmyIds();
  for (std::list<Uint32>::const_iterator i = defender_army_ids.begin(),
         end = defender_army_ids.end(); i != end; ++i)
    defenders.push_back(findStackById(*i));

  Fight fight(attackers, defenders, action->getBattleHistory());
  fight.battleFromHistory();
  fight_started.emit(fight);

  cleanupAfterFight(attackers, defenders);
}

void NetworkPlayer::decodeActionJoin(const Action_Join *action)
{
  Stack *receiver = d_stacklist->getStackById(action->getReceivingStackId());
  Stack *joining = d_stacklist->getStackById(action->getJoiningStackId());

  doStackJoin(receiver, joining, false);
}

void NetworkPlayer::decodeActionRuin(const Action_Ruin *action)
{
  Stack *explorer = d_stacklist->getStackById(action->getStackId());
  Ruin *r = Ruinlist::getInstance()->getById(action->getRuinId());
  bool searched = action->getSearchSuccessful();

  Stack* keeper = r->getOccupant();

  // now simulate the fight that might have happened on the other side
  if (keeper) {
    if (searched) {
      // whack the keeper
      for (Stack::iterator i = keeper->begin(); i != keeper->end(); ++i)
        (*i)->setHP(0);
    }
    else {
      // whack the hero
      explorer->getFirstHero()->setHP(0);
    }

    std::list<Stack *> attackers, defenders;
    attackers.push_back(explorer);
    defenders.push_back(keeper);
    
    cleanupAfterFight(attackers, defenders);

    if (searched) {
        r->setOccupant(0);
        delete keeper;
    }
  }

  r->setSearched(searched);

  supdatingStack.emit(0);
}

void NetworkPlayer::decodeActionTemple(const Action_Temple *action)
{
  Stack *stack = d_stacklist->getStackById(action->getStackId());
  Temple *temple = Templelist::getInstance()->getById(action->getTempleId());
  doStackVisitTemple(stack, temple);
}

void NetworkPlayer::decodeActionOccupy(const Action_Occupy *action)
{
  City *city = Citylist::getInstance()->getById(action->getCityId());
  doCityOccupy(city);
}

void NetworkPlayer::decodeActionPillage(const Action_Pillage *action)
{
  City *city = Citylist::getInstance()->getById(action->getCityId());
  int gold, pillaged_army_type;
  doCityPillage(city, gold, &pillaged_army_type);
}

void NetworkPlayer::decodeActionSack(const Action_Sack *action)
{
  City *city = Citylist::getInstance()->getById(action->getCityId());
  int gold;
  std::list<Uint32> sacked_types;
  doCitySack(city, gold, &sacked_types);
  // FIXME: the game class doesn't listen for sack signals, so it doesn't
  // redraw the map as it should
}

void NetworkPlayer::decodeActionRaze(const Action_Raze *action)
{
  City *city = Citylist::getInstance()->getById(action->getCityId());
  doCityRaze(city);
}

void NetworkPlayer::decodeActionUpgrade(const Action_Upgrade *action)
{
  // doesn't exist, not handled
  assert(false);
}

void NetworkPlayer::decodeActionBuy(const Action_Buy *action)
{
  City *city = Citylist::getInstance()->getById(action->getCityId());
  doCityBuyProduction(city, action->getProductionSlot(), 
		      action->getBoughtArmyTypeId());
}

void NetworkPlayer::decodeActionProduction(const Action_Production *action)
{
  City *city = Citylist::getInstance()->getById(action->getCityId());
  doCityChangeProduction(city, action->getSlot());
}

void NetworkPlayer::decodeActionReward(const Action_Reward *action)
{
  Stack *stack = d_stacklist->getStackById(action->getStackId());
  doGiveReward(stack, action->getReward());
  supdatingStack.emit(stack); // make sure we get a redraw
}

void NetworkPlayer::decodeActionQuest(const Action_Quest *action)
{
  QuestsManager *qm = QuestsManager::getInstance();
  switch (Quest::Type(action->getQuestType()))
    {
    case Quest::KILLHERO: 
      qm->createNewKillHeroQuest(action->getHeroId(), action->getData());
      break;
    case Quest::KILLARMIES:
      qm->createNewEnemyArmiesQuest(action->getHeroId(), action->getData(), 
				    action->getVictimPlayerId());
      break;
    case Quest::CITYSACK:
      qm->createNewCitySackQuest(action->getHeroId(), action->getData());
      break;
    case Quest::CITYRAZE:
      qm->createNewCityRazeQuest(action->getHeroId(), action->getData());
      break;
    case Quest::CITYOCCUPY:
      qm->createNewCityOccupyQuest(action->getHeroId(), action->getData());
      break;
    case Quest::KILLARMYTYPE:
      qm->createNewEnemyArmytypeQuest(action->getHeroId(), action->getData());
      break;
    case Quest::PILLAGEGOLD:
      qm->createNewPillageGoldQuest(action->getHeroId(), action->getData());
      break;
    }
}

void NetworkPlayer::decodeActionEquip(const Action_Equip *action)
{
  Stack *stack = d_stacklist->getArmyStackById(action->getHeroId());
  Hero *hero = dynamic_cast<Hero *>(stack->getArmyById(action->getHeroId()));
  Item *item = 0;

  switch (action->getToBackpackOrToGround())
  {
  case Action_Equip::BACKPACK:
    item = findItemById(GameMap::getInstance()->getTile(stack->getPos())->getItems(), action->getItemId());
    doHeroPickupItem(hero, item, stack->getPos());
    break;

  case Action_Equip::GROUND:
    item = findItemById(hero->getBackpack(), action->getItemId());
    doHeroDropItem(hero, item, stack->getPos());
    break;
  }
}

void NetworkPlayer::decodeActionLevel(const Action_Level *action)
{
  Stack *stack = d_stacklist->getArmyStackById(action->getArmyId());
  Army *army = stack->getArmyById(action->getArmyId());

  doLevelArmy(army, Army::Stat(action->getStatToIncrease()));
}

void NetworkPlayer::decodeActionDisband(const Action_Disband *action)
{
  Stack *stack = d_stacklist->getStackById(action->getStackId());
  doStackDisband(stack);
}

void NetworkPlayer::decodeActionModifySignpost(const Action_ModifySignpost *act)
{
  Signpost *sign = Signpostlist::getInstance()->getById(act->getSignpostId());
  doSignpostChange(sign, act->getSignContents());
}

void NetworkPlayer::decodeActionRenameCity(const Action_RenameCity *action)
{
  City *city = Citylist::getInstance()->getById(action->getCityId());
  doCityRename(city, action->getNewCityName());
}

void NetworkPlayer::decodeActionVector(const Action_Vector *action)
{
  City *city = Citylist::getInstance()->getById(action->getCityId());
  doVectorFromCity(city, action->getVectoringDestination());
}

void NetworkPlayer::decodeActionFightOrder(const Action_FightOrder *action)
{
  doSetFightOrder(action->getFightOrder());
}

void NetworkPlayer::decodeActionResign(const Action_Resign *action)
{
  doResign();
}

void NetworkPlayer::decodeActionPlant(const Action_Plant *action)
{
  Stack *stack = d_stacklist->getArmyStackById(action->getHeroId());
  Hero *hero = dynamic_cast<Hero *>(stack->getArmyById(action->getHeroId()));
  Item *item = findItemById(hero->getBackpack(), action->getItemId());
    
  doHeroPlantStandard(hero, item, stack->getPos());
}

void NetworkPlayer::decodeActionProduce(const Action_Produce *action)
{
  //Note: we don't use this to produce new armies in cities
  //That happens during the city->nextTurn method.
  //It is enough that we record the changes in city production, and then
  //let the city->nextTurn method do it's job.
  //This action is used for the Production Report instead, see
  //gui/report-dialog.cpp
 
  //there's no real need to send this action back to the server because
  //only the current player can see her own production report.

  //if we wanted to decode it, we'd do this:
  //City *city = Citylist::getInstance()->getById(action->getCityId());
  //doCityProducesArmy(city);
}

void NetworkPlayer::decodeActionProduceVectored(const Action_ProduceVectored *action)
{
  //Note: we don't use this to produce new vectored army units on the map 
  //That happens during the vectoredunit->nextTurn method.
  //It is enough that we record the changes in city vectoring, and then
  //let the vectoredunitlist->nextTurn method do it's job.
  //This action is used for the Production Report instead, see
  //gui/report-dialog.cpp
 
  //there's no real need to send this action back to the server because
  //only the current player can see her own production report.

  //we couldn't decode this action if we wanted to, because there isn't
  //enough data to make it happen.
}

void NetworkPlayer::decodeActionDiplomacyState(const Action_DiplomacyState *action)
{
  Player *player = Playerlist::getInstance()->getPlayer(action->getOpponentId());
  doDeclareDiplomacy(action->getDiplomaticState(), player);
}

void NetworkPlayer::decodeActionDiplomacyProposal(const Action_DiplomacyProposal *action)
{
  Player *player = Playerlist::getInstance()->getPlayer(action->getOpponentId());
  doProposeDiplomacy(action->getDiplomaticProposal(), player);
}

void NetworkPlayer::decodeActionDiplomacyScore(const Action_DiplomacyScore *action)
{
  Player *player = Playerlist::getInstance()->getPlayer(action->getOpponentId());
  alterDiplomaticRelationshipScore(player, action->getAmountChange());
}

void NetworkPlayer::decodeActionEndTurn(const Action_EndTurn *action)
{
  ending_turn.emit();
}

void NetworkPlayer::decodeActionConquerCity(const Action_ConquerCity *action)
{
  City *city = Citylist::getInstance()->getById(action->getCityId());
  Stack *stack = d_stacklist->getStackById(action->getStackId());
  conquerCity(city, stack);
}

void NetworkPlayer::decodeActionRecruitHero(const Action_RecruitHero *action)
{

  City *city = Citylist::getInstance()->getById(action->getCityId());
  Army *ally = 0;
  if (action->getNumAllies())
    ally = Armysetlist::getInstance()->getArmy(getArmyset(),
                                               action->getAllyArmyType());
  doRecruitHero(action->getHero(), city, action->getCost(), 
		action->getNumAllies(), ally);
}

void NetworkPlayer::decodeActionRenamePlayer(const Action_RenamePlayer *action)
{
  doRename(action->getName());
}

void NetworkPlayer::decodeActionCityTooPoorToProduce(const Action_CityTooPoorToProduce *action)
{
  //this action is only used for reporting purposes.
}

// End of file
