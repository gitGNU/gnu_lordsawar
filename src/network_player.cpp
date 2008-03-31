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
  Stack *stack = d_stacklist->getStackById(action->d_stack);
  stack->moveToDest(action->d_dest);
  supdatingStack.emit(stack);
}

void NetworkPlayer::decodeActionSplit(const Action_Split *action)
{
  Stack *stack = d_stacklist->getStackById(action->d_orig);
  
  for (unsigned int i = 0; i < MAX_STACK_SIZE; ++i) {
    Uint32 army_id = action->d_armies_moved[i];
    if (army_id == 0)
      continue;

    Army *army = stack->getArmyById(army_id);
    army->setGrouped(false);
  }

  doStackSplit(stack);
}

void NetworkPlayer::decodeActionFight(const Action_Fight *action)
{
  std::list<Stack *> attackers, defenders;
  for (std::list<Uint32>::const_iterator i = action->d_attackers.begin(),
         end = action->d_attackers.end(); i != end; ++i)
    attackers.push_back(d_stacklist->getStackById(*i));

  for (std::list<Uint32>::const_iterator i = action->d_defenders.begin(),
         end = action->d_defenders.end(); i != end; ++i)
    defenders.push_back(findStackById(*i));

  Fight fight(attackers, defenders, action->d_history);
  fight.battleFromHistory();
  fight_started.emit(fight);

  cleanupAfterFight(attackers, defenders);
}

void NetworkPlayer::decodeActionJoin(const Action_Join *action)
{
  Stack *receiver = d_stacklist->getStackById(action->d_orig_id);
  Stack *joining = d_stacklist->getStackById(action->d_joining_id);

  doStackJoin(receiver, joining, false);
}

void NetworkPlayer::decodeActionRuin(const Action_Ruin *action)
{
  Stack *explorer = d_stacklist->getStackById(action->d_stack);
  Ruin *r = Ruinlist::getInstance()->getById(action->d_ruin);
  bool searched = action->d_searched;

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
  Stack *stack = d_stacklist->getStackById(action->d_stack);
  Temple *temple = Templelist::getInstance()->getById(action->d_temple);
  doStackVisitTemple(stack, temple);
}

void NetworkPlayer::decodeActionOccupy(const Action_Occupy *action)
{
  City *city = Citylist::getInstance()->getById(action->d_city);
  doCityOccupy(city);
}

void NetworkPlayer::decodeActionPillage(const Action_Pillage *action)
{
  City *city = Citylist::getInstance()->getById(action->d_city);
  int gold, pillaged_army_type;
  doCityPillage(city, gold, pillaged_army_type);
}

void NetworkPlayer::decodeActionSack(const Action_Sack *action)
{
  City *city = Citylist::getInstance()->getById(action->d_city);
  int gold;
  std::list<Uint32> sacked_types;
  doCitySack(city, gold, &sacked_types);
  // FIXME: the game class doesn't listen for sack signals, so it doesn't
  // redraw the map as it should
}

void NetworkPlayer::decodeActionRaze(const Action_Raze *action)
{
  City *city = Citylist::getInstance()->getById(action->d_city);
  doCityRaze(city);
}

void NetworkPlayer::decodeActionUpgrade(const Action_Upgrade *action)
{
  // doesn't exist, not handled
  assert(false);
}

void NetworkPlayer::decodeActionBuy(const Action_Buy *action)
{
  City *city = Citylist::getInstance()->getById(action->d_city);
  doCityBuyProduction(city, action->d_slot, action->d_prod);
}

void NetworkPlayer::decodeActionProduction(const Action_Production *action)
{
  City *city = Citylist::getInstance()->getById(action->d_city);
  doCityChangeProduction(city, action->d_prod);
}

void NetworkPlayer::decodeActionReward(const Action_Reward *action)
{
  Stack *stack = d_stacklist->getStackById(action->d_stack);
  doGiveReward(stack, action->d_reward);
  supdatingStack.emit(stack); // make sure we get a redraw
}

void NetworkPlayer::decodeActionQuest(const Action_Quest *action)
{
  QuestsManager::getInstance()->createNewQuest(
    action->d_hero, action->d_questtype, action->d_data,
    action->d_victim_player);
}

void NetworkPlayer::decodeActionEquip(const Action_Equip *action)
{
  Stack *stack = d_stacklist->getArmyStackById(action->d_hero);
  Hero *hero = dynamic_cast<Hero *>(stack->getArmyById(action->d_hero));
  Item *item = 0;

  switch (action->d_slot)
  {
  case Action_Equip::BACKPACK:
    item = findItemById(GameMap::getInstance()->getTile(stack->getPos())->getItems(), action->d_item);
    doHeroPickupItem(hero, item, stack->getPos());
    break;

  case Action_Equip::GROUND:
    item = findItemById(hero->getBackpack(), action->d_item);
    doHeroDropItem(hero, item, stack->getPos());
    break;
  }
}

void NetworkPlayer::decodeActionLevel(const Action_Level *action)
{
  Stack *stack = d_stacklist->getArmyStackById(action->d_army);
  Army *army = stack->getArmyById(action->d_army);

  doLevelArmy(army, Army::Stat(action->d_stat));
}

void NetworkPlayer::decodeActionDisband(const Action_Disband *action)
{
  Stack *stack = d_stacklist->getStackById(action->d_stack);
  doStackDisband(stack);
}

void NetworkPlayer::decodeActionModifySignpost(const Action_ModifySignpost *action)
{
  Signpost *signpost = Signpostlist::getInstance()->getById(action->d_signpost);
  doSignpostChange(signpost, action->d_message);
}

void NetworkPlayer::decodeActionRenameCity(const Action_RenameCity *action)
{
  City *city = Citylist::getInstance()->getById(action->d_city);
  doCityRename(city, action->d_name);
}

void NetworkPlayer::decodeActionVector(const Action_Vector *action)
{
  City *city = Citylist::getInstance()->getById(action->d_city);
  doVectorFromCity(city, action->d_dest);
}

void NetworkPlayer::decodeActionFightOrder(const Action_FightOrder *action)
{
  doSetFightOrder(action->d_order);
}

void NetworkPlayer::decodeActionResign(const Action_Resign *action)
{
  doResign();
}

void NetworkPlayer::decodeActionPlant(const Action_Plant *action)
{
  Stack *stack = d_stacklist->getArmyStackById(action->d_hero);
  Hero *hero = dynamic_cast<Hero *>(stack->getArmyById(action->d_hero));
  Item *item = findItemById(hero->getBackpack(), action->d_item);
    
  doHeroPlantStandard(hero, item, stack->getPos());
}

void NetworkPlayer::decodeActionProduce(const Action_Produce *action)
{
  // FIXME: needed?
}

void NetworkPlayer::decodeActionProduceVectored(const Action_ProduceVectored *action)
{
  // FIXME: needed?
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
  City *city = Citylist::getInstance()->getById(action->d_city);
  Stack *stack = d_stacklist->getStackById(action->d_stack);
  conquerCity(city, stack);
}

void NetworkPlayer::decodeActionRecruitHero(const Action_RecruitHero *action)
{
  City *city = Citylist::getInstance()->getById(action->d_city);
  Army *ally = 0;
  if (action->d_allies)
    ally = Armysetlist::getInstance()->getArmy(getArmyset(),
                                               action->d_ally_army_type);
  doRecruitHero(action->d_hero, city, action->d_cost, action->d_allies, ally);
}

// End of file
