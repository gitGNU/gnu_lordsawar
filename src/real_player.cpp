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
#include <SDL_timer.h>

#include "real_player.h"
#include "playerlist.h"
#include "armysetlist.h"
//#include "FightDialog.h"
#include "stacklist.h"
#include "citylist.h"
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
#include "GameScenario.h"
#include "game-parameters.h"
#include "signpost.h"
#include "history.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

RealPlayer::RealPlayer(string name, Uint32 armyset, SDL_Color color,
		       Player::Type type, int player_no)
    :Player(name, armyset, color, type, player_no)
{
}

RealPlayer::RealPlayer(const Player& player)
    :Player(player)
{
    d_type = HUMAN;
}

RealPlayer::RealPlayer(XML_Helper* helper)
    :Player(helper)
{
}

RealPlayer::~RealPlayer()
{
}

bool RealPlayer::save(XML_Helper* helper) const
{
    // This may seem a bit dumb, but allows derived players (especially
    // AI's) to save additional data, such as character types or so.
    bool retval = true;
    retval &= helper->openTag("player");
    retval &= Player::save(helper);
    retval &= helper->closeTag();

    return retval;
}

bool RealPlayer::initTurn()
{
    clearActionlist();
    History_StartTurn* item = new History_StartTurn();
    d_history.push_back(item);
    return true;
}

bool RealPlayer::startTurn()
{
    return true;
}

bool RealPlayer::invadeCity(City* c)
{
    //for the realplayer, this function doesn't do a lot.
    //However, an AI player has to decide here what to do (occupy, raze,
    //pillage)
    sinvadingCity.emit(c);
    return true;
}

bool RealPlayer::recruitHero(Hero* hero, City *city, int cost)
{
    // for the realplayer, this function also just raises a signal and looks
    // what to do next.

    History_HeroEmerges *item = new History_HeroEmerges();
    item->fillData(hero, city);
    d_history.push_back(item);

    return srecruitingHero.emit(hero, city, cost);
}

bool RealPlayer::levelArmy(Army* a)
{
    if (!a->canGainLevel())
        return false;

    // the standard human player just raises the signal and fills in the
    // data for the w_edit object
    Army::Stat stat = snewLevelArmy.emit(a);

    a->gainLevel(stat);

    Action_Level* item = new Action_Level();
    item->fillData(a->getId(), stat);
    d_actions.push_back(item);
    
    return true;
}

bool RealPlayer::stackSplit(Stack* s)
{
    debug("RealPlayer::stackSplit("<<s->getId()<<")")

    Army* ungrouped = s->getFirstUngroupedArmy();
    if (!ungrouped)        //no armies to split
    {
        return false;
    }

    bool all_ungrouped = true;    //the whole army would be split
    for (Stack::iterator it = s->begin(); it != s->end(); it++)
    {
        if ((*it)->isGrouped())
        {
            all_ungrouped = false;
        }
    }
    if (all_ungrouped)
    {
        return false;
    }

    Stack* new_stack = new Stack(this, s->getPos());

    while (ungrouped)
    {
        new_stack->push_back(ungrouped);
        s->erase(find(s->begin(), s->end(), ungrouped));

        ungrouped->setGrouped(true);
        ungrouped = s->getFirstUngroupedArmy();
    }

    d_stacklist->push_back(new_stack);

    Action_Split* item = new Action_Split();
    item->fillData(s, new_stack);
    d_actions.push_back(item);

    return true;
}

bool RealPlayer::stackJoin(Stack* receiver, Stack* joining, bool grouped)
{
    debug("RealPlayer::stackJoin("<<receiver->getId()<<","<<joining->getId()<<")")

    if ((receiver == 0) || (joining == 0))
        return false;

    if ((receiver->size() + joining->size()) > 8)
        return false;
    
    // now if grouped is set to false, ungroup all the receiving stack's armies
    // (by default, only the joining stacks armies will continue to move)
    // Note that the computer player silently ignores the grouped value and always
    // moves all armies in a stack.
    for (Stack::iterator it = receiver->begin(); it != receiver->end(); it++)
        (*it)->setGrouped(grouped);

    for (Stack::iterator it = joining->begin(); it != joining->end(); it++)
    {
        receiver->push_back(*it);
        (*it)->setGrouped(true);
    }

    Action_Join* item = new Action_Join();
    item->fillData(receiver, joining);
    d_actions.push_back(item);

    joining->clear();    //clear only erases the pointers not the armies
    d_stacklist->flRemove(joining);
    supdatingStack.emit(0);

    return true;
}


bool RealPlayer::signpostChange(Signpost *s, std::string message)
{
  if (!s)
    return false;
  s->setName(message);
  Action_ModifySignpost* item = new Action_ModifySignpost();
  item->fillData(s, message);
  d_actions.push_back(item);
  return true;
}

bool RealPlayer::cityRename(City *c, std::string name)
{
  if (!c)
    return false;
  c->setName(name);
  Action_RenameCity* item = new Action_RenameCity();
  item->fillData(c, name);
  d_actions.push_back(item);
  return true;
}

bool RealPlayer::stackDisband(Stack* s)
{
    debug("player::stackDisband(Stack*)")
    if (!s)
      s = getActivestack();
    getStacklist()->setActivestack(0);
    Action_Disband* item = new Action_Disband();
    item->fillData(s);
    d_actions.push_back(item);
    sdyingStack.emit(s);
    s->getPlayer()->getStacklist()->deleteStack(s);
    return true;
}

bool RealPlayer::stackMove(Stack* s)
{
    debug("player::stackMove(Stack*)")

    if (s->getPath()->empty())
    {
        return false;
    }

    Path::iterator it = s->getPath()->end();
    it--;
    MoveResult *result = stackMove(s, **(it), true);
    bool ret = result->moveSucceeded();
    delete result;
    result = 0;
    return ret;
}

MoveResult *RealPlayer::stackMove(Stack* s, Vector<int> dest, bool follow)
{
    debug("player::stack_move()");
    //if follow is set to true, follow an already calculated way, else
    //calculate it here
    if (!follow)
    {
        s->getPath()->calculate(s, dest);
    }
    else
    {
        s->getPath()->checkPath(s);
    }

    if (s->getPath()->empty())
    {
        return new MoveResult(false);        //way to destination blocked
    }

    int stepCount = 0;
    while (s->getPath()->size() > 1 && s->enoughMoves())
    {
        if (stackMoveOneStep(s))
            stepCount++;
        supdatingStack.emit(0);

        // sleep for a specified amount of time
        SDL_Delay(Configuration::s_displaySpeedDelay);
    }

    if (s->getPath()->size() == 1 && s->enoughMoves())
    //now look for fight targets, joins etc.
    {
        Vector<int> pos = **(s->getPath()->begin());
        City* city = Citylist::getInstance()->getObjectAt(pos);
        Stack* target = Stacklist::getObjectAt(pos);

        //first fight_city to avoid ambiguity with fight_army
        if (city && (city->getPlayer() != this) && (!city->isBurnt()))
        {
            Fight::Result result;
            MoveResult *moveResult = new MoveResult(true);
	    if (stackMoveOneStep(s))
	      stepCount++;
            vector<Stack*> def_in_city = Stacklist::defendersInCity(city);
            if (!def_in_city.empty())
            {
                // This is a hack to circumvent the limitations of stackFight.
                // Create a dummy stack at the target position if neccessary
                // and start a fight with this dummy stack.
                if (!target)
                    target = new Stack(city->getPlayer(), pos);
                result = stackFight(&s, &target, false);
                if (target && target->empty())
                    delete target;
            }
            else
                result = Fight::ATTACKER_WON;

            moveResult->setFightResult(result);

            // We may only take the city if we have defeated all defenders
            if (result == Fight::ATTACKER_WON)
            {
                invadeCity(city); //determine what to do with city

                History_CityWon *item = new History_CityWon();
                item->fillData(city);
                d_history.push_back(item);
		if (s->hasHero())
		  {
		    History_HeroCityWon *another = new History_HeroCityWon();
		    Hero *hero = dynamic_cast<Hero *>(s->getFirstHero());
		    another->fillData(hero, city);
		    d_history.push_back(another);
		  }
            }
            else
            {
                // we didn't suceed in defeating the defenders
                //if this is a neutral city, and we're playing with 
                //active neutral cities, AND it hasn't already been attacked
                //then it's production gets turned on
                Player *neu = city->getPlayer(); //neutral player
                if (GameScenario::s_neutral_cities == GameParameters::ACTIVE &&
                    neu == Playerlist::getInstance()->getNeutral() &&
                    city->getProductionIndex() == -1)
                {
                  //great, then let's turn on the production.
                  //well, we already made a unit, and we want to produce more
                  //of it.
                  Stack *o = neu->getStacklist()->getObjectAt(city->getPos());
                  if (o)
                    {
                      int army_type = o->getStrongestArmy()->getType();
                      for (int i = 0; i < 4; i++)
                        {
                          if (city->getArmytype(i) == army_type)
                            {
                              // hey, we found the droid we were looking for
                              city->setProduction(i);
                              break;
                            }
                        }
                    }
                }
            }
            moveResult->setStepCount(stepCount);
            supdatingStack.emit(0);
            
            return moveResult;
        }
        
        //another friendly stack => join it
        else if (target && target->getPlayer() == this)
        {
            if (stackMoveOneStep(s))
                stepCount++;
            stackJoin(Stacklist::getAmbiguity(s), s, false);

            supdatingStack.emit(0);
            SDL_Delay(Configuration::s_displaySpeedDelay);
    
            MoveResult *moveResult = new MoveResult(true);
            moveResult->setStepCount(stepCount);
            moveResult->setJoin(true);
            return moveResult;
        }
        
        //enemy stack => fight
        else if (target)
        {
            MoveResult *moveResult = new MoveResult(true);
        
            Fight::Result result = stackFight(&s, &target, false);
            moveResult->setFightResult(result);
            if (!target)
            {
                if (stackMoveOneStep(s))
                    stepCount++;
            }
            else if (s)
                s->decrementMoves(2);
            
            supdatingStack.emit(0);
            moveResult->setStepCount(stepCount);
            return moveResult;
        }
        
        //else
        if (stackMoveOneStep(s))
            stepCount++;

        supdatingStack.emit(0);
        SDL_Delay(Configuration::s_displaySpeedDelay);
    
        MoveResult *moveResult = new MoveResult(true);
        moveResult->setStepCount(stepCount);
        return moveResult;
    }

    //If there is another stack where we landed, join it. We can't have two
    //stacks share the same maptile
    if (Stacklist::getAmbiguity(s))
    {
        stackJoin(Stacklist::getAmbiguity(s), s, false);
        supdatingStack.emit(0);
        MoveResult *moveResult = new MoveResult(true);
        moveResult->setStepCount(stepCount);
        moveResult->setJoin(true);
        return moveResult;
    }

    return new MoveResult(true);
}

/*
 *
 * To help factor in the advantage of hero experience/strength and 
 * ruin-monster strength as well as the stack strength, I think you'll 
 * find it'll be easier to calculate in terms of the odds of failure [than
 * the odds of success].  A new hero (minimum strength) with nothing in 
 * the stack to help him might have 10-20% odds of failure at a wimpy ruin.
 * The same novice hero facing a dragon in the ruin might have 50% odds of 
 * failure.  So a rule of thumb would be to start with a 25% chance of
 * failure.  The odds would be doubled by the worst monster and halved by 
 * the easiest.  I agree that a strength-9 hero with 8 in the stack should i
 * definitely be at 99%.  A reasonable formula might be:
 *
 * OddsOfFailure = BaseOdds * MonsterFactor * StackFactor * HeroFactor,
 *
 * with
 *        BaseOdds = 0.25
 * and
 *        MonsterFactor = 2, 1 or 0.5 depending on hard vs. easy
 * and
 *        StackFactor = (9 - SizeOfStack)/8,
 * and
 *        HeroFactor = (10-StrengthOfHero)/5.
 */
Fight::Result ruinfight (Stack **attacker, Stack **defender)
{
  Stack *loser;
  Fight::Result result;
  Uint32 hero_strength, monster_strength;
  hero_strength = (*attacker)->getFirstHero()->getStat(Army::STRENGTH, true);
  monster_strength = (*defender)->getStrongestArmy()->getStat(Army::STRENGTH, true);
  float base_factor = 0.25;
  float stack_factor = (9.0 - (*attacker)->size()) / 8.0;
  float hero_factor = (10.0 - hero_strength) / 5.0;
  float monster_factor;
  if (monster_strength >= 8)
    monster_factor = 2.0;
  else if (monster_strength >= 6)
    monster_factor = 1.0;
  else
    monster_factor = 0.5;
  float fail = base_factor * monster_factor * stack_factor * hero_factor;

  if (rand() % 100 > (int)(fail * 100.0))
    {
      result = Fight::ATTACKER_WON;
      loser = *defender;
      for (Stack::iterator sit = loser->begin(); sit != loser->end();)
        {
          (*sit)->setHP (0);
          sit++;
        }
    }
  else
    {
      result = Fight::DEFENDER_WON;
      loser = *attacker;
      loser->getFirstHero()->setHP(0); /* only the hero dies */
    }
        
  return result;
}

Fight::Result RealPlayer::stackRuinFight (Stack **attacker, Stack **defender)
{
    Fight::Result result = Fight::DRAW;
    if (*defender == NULL)
      return Fight::ATTACKER_WON;
    debug("stackRuinFight: player = " << getName()<<" at position "
          <<(*defender)->getPos().x<<","<<(*defender)->getPos().y);

    ruinfight_started.emit(*attacker, *defender);
    result = ruinfight (attacker, defender);
    ruinfight_finished.emit(result);

    // cleanup
    
    // add a ruin fight item about the combat
    //Action_RuinFight* item = new Action_RuinFight();
    //item->fillData(*attacker, *defender, result);
    //d_actions.push_back(item);
    /* FIXME: do we need an Action_RuinFight? */

    // get attacker and defender heroes and more...
    std::list<Stack*> attackers;
    attackers.push_back(*attacker);
    std::list<Stack*> defenders;
    defenders.push_back(*defender);
    std::vector<Uint32> attackerHeroes, defenderHeroes;
    
    getHeroes(attackers, attackerHeroes);
    getHeroes(defenders, defenderHeroes);

    // here we calculate also the total XP to add when a player have a battle
    // clear dead defenders
    debug("clean dead defenders");
    double defender_xp = removeDeadArmies(defenders, attackerHeroes);

    // and dead attackers
    debug("clean dead attackers");
    double attacker_xp = removeDeadArmies(attackers, defenderHeroes);

    // after a fight: if the attacker's stack won - emit
    // supdatingStack signal, else emit sdyingStack

    debug("after fight: attackers empty? " << attackers.empty()
          << "(" << attackers.size() << ")");


    //only emit this signal when the defender has not won!
    if (!attackers.empty())
    {
        debug("attacker won: supdatingStack");
        supdatingStack.emit(0);
        if (defender_xp != 0)
            updateArmyValues(attackers, defender_xp);
    }
    else
    {
        debug("attacker lost: sdyingStack");
        //sdyingStack.emit(0);crapola
    }
    if (attacker_xp != 0)
        updateArmyValues(defenders, attacker_xp);

    return result;
}

Fight::Result RealPlayer::stackFight(Stack** attacker, Stack** defender, bool ruin) 
{
    debug("stackFight: player = " << getName()<<" at position "
          <<(*defender)->getPos().x<<","<<(*defender)->getPos().y);
    if (ruin)
      return RealPlayer::stackRuinFight (attacker, defender);

    // save the defender's player for future use
    Player* pd = (*defender)->getPlayer();

    // I suppose, this should be always true, but one can never be sure
    bool attacker_active = *attacker == d_stacklist->getActivestack();

    Fight fight(*attacker, *defender);
    fight_started.emit(fight);

    // cleanup
    
    // add a fight item about the combat
    Action_Fight* item = new Action_Fight();
    item->fillData(&fight);
    d_actions.push_back(item);

    // get attacker and defender heroes and more...
    std::list<Stack*> attackers = fight.getAttackers();
    std::list<Stack*> defenders = fight.getDefenders();
    std::vector<Uint32> attackerHeroes, defenderHeroes;
    
    getHeroes(attackers, attackerHeroes);
    getHeroes(defenders, defenderHeroes);

    // here we calculate also the total XP to add when a player have a battle
    // clear dead defenders
    debug("clean dead defenders");
    double defender_xp = removeDeadArmies(defenders, attackerHeroes);

    // and dead attackers
    debug("clean dead attackers");
    double attacker_xp = removeDeadArmies(attackers, defenderHeroes);

    // after a fight: if the attacker's stack won - emit
    // supdatingStack signal, else emit sdyingStack

    debug("after fight: attackers empty? " << attackers.empty()
          << "(" << attackers.size() << ")");


    //only emit this signal when the defender has not won!
    if (!attackers.empty())
    {
        debug("attacker won: supdatingStack");
        supdatingStack.emit(0);
        if (defender_xp != 0)
            updateArmyValues(attackers, defender_xp);
    }
    else
    {
        debug("attacker lost: sdyingStack");
        sdyingStack.emit(0);
    }
    if (attacker_xp != 0)
        updateArmyValues(defenders, attacker_xp);

    // Set the attacker and defender stack to 0 if neccessary. This is a great
    // help for the functions calling stackFight (e.g. if a stack attacks
    // another stack and destroys it without winning the battle, it may take the
    // position of this stack)

    // First, the attacker...
    bool exists =
	std::find(d_stacklist->begin(), d_stacklist->end(), *attacker)
	!= d_stacklist->end();
#if 0
    bool exists = false;
    for (Stacklist::iterator it = d_stacklist->begin(); it != d_stacklist->end(); it++)
        if ((*it) == (*attacker))
	{
            exists = true;
	    break;
	}
#endif
    
    if (!exists)
    {
        (*attacker) = 0;
        if (attacker_active)
            d_stacklist->setActivestack(0);
    }

    // ...then the defender.
    exists = false;
    if (pd)
        for (Stacklist::iterator it = pd->getStacklist()->begin();
             it != pd->getStacklist()->end(); it++)
        {
            if ((*it) == (*defender))
                exists = true;
        }
    else
        exists = true;
    if (!exists)
        (*defender) = 0;

    return fight.getResult();
}


void RealPlayer::updateArmyValues(std::list<Stack*>& stacks, double xp_sum)
{
    std::list<Stack*>::iterator it;
    double numberarmy = 0;

    for (it = stacks.begin(); it != stacks.end(); it++)
        numberarmy += (*it)->size();

    for (it = stacks.begin(); it != stacks.end(); )
      {
        debug("Stack: " << (*it))

        for (Stack::iterator sit = (*it)->begin(); sit != (*it)->end();)
          {
            debug("Army: " << (*sit))

            // here we adds XP
            (*sit)->gainXp((double)((xp_sum)/numberarmy));
            debug("Army gets " << (double)((xp_sum)/numberarmy) << " XP")

            // here we adds 1 to number of battles
            (*sit)->setBattlesNumber((*sit)->getBattlesNumber()+1);
            debug("Army battles " <<  (*sit)->getBattlesNumber())

            // medals only go to non-ally armies.
            if ((*it)->hasHero() && (*sit)->isHero() == false && 
                (*sit)->getAwardable() == false)
              {
                if(((*sit)->getBattlesNumber())>10 && 
                   !((*sit)->getMedalBonus(2)))
                  {
                    (*sit)->setMedalBonus(2,true);
                    // We must recalculate the XPValue of this unit since it 
                    // got a medal
                    (*sit)->setXpReward((*sit)->getXpReward()+1);
                    // We get the medal bonus here
                    (*sit)->setStat(Army::STRENGTH, (*sit)->getStat(Army::STRENGTH, false)+1);
                    // Emit signal
                    snewMedalArmy.emit(*sit);
                  }

                debug("Army hits " <<  (*sit)->getNumberHasHit())

                // Only give medals if the unit has attacked often enough, else
                // medals lose the flair of something special; a value of n 
                // means roughly to hit an equally strong unit around n 
                // times. (note: one hit! An attack can consist of up to 
                // strength hits)
                if(((*sit)->getNumberHasHit()>50) && !(*sit)->getMedalBonus(0))
                  {
                    (*sit)->setMedalBonus(0,true);
                    // We must recalculate the XPValue of this unit since it
                    // got a medal
                    (*sit)->setXpReward((*sit)->getXpReward()+1);
                    // We get the medal bonus here
                    (*sit)->setStat(Army::STRENGTH, (*sit)->getStat(Army::STRENGTH, false)+1);
                    // Emit signal
                    snewMedalArmy.emit(*sit);
                  }

                debug("army being hit " <<  (*sit)->getNumberHasBeenHit())

                // Gives the medal for good defense. The more negative the 
                // number the more blows the unit evaded. n means roughly 
                // avoid n hits from an equally strong unit. Since we want 
                // to punish the case of the unit hiding among many others, 
                // we set this value quite high.
                if(((*sit)->getNumberHasBeenHit() < -100) && !(*sit)->getMedalBonus(1))
                  {
                    (*sit)->setMedalBonus(1,true);
                    // We must recalculate the XPValue of this unit since it 
                    // got a medal
                    (*sit)->setXpReward((*sit)->getXpReward()+1);
                    // We get the medal bonus here
                    (*sit)->setStat(Army::STRENGTH, (*sit)->getStat(Army::STRENGTH, false)+1);
                    // Emit signal
                    snewMedalArmy.emit(*sit);
                  }
                debug("Army hits " <<  (*sit)->getNumberHasHit())

                for(int i=0;i<3;i++)
                  {
                    debug("MEDAL[" << i << "]==" << (*sit)->getMedalBonus(i))
                  }
              }

              // We reset the hit values after the battle
              (*sit)->setNumberHasHit(0);
              (*sit)->setNumberHasBeenHit(0);

              if ((*sit)->isHero())
                {
                  while((*sit)->canGainLevel())
                    {
                      // Units not associated to a player never raise levels.
                      if ((*sit)->getPlayer() == 
                          Playerlist::getInstance()->getNeutral())
                        break;

                      //Here this for is to check if army must raise 2 or more 
                      //levels per time depending on the XP and level itself

                      debug("ADVANCING LEVEL "<< "CANGAINLEVEL== " << (*sit)->canGainLevel())
                      (*sit)->getPlayer()->levelArmy(*sit);
                    }
                  debug("Army new XP=" << (*sit)->getXP())
                }
              sit++;
            }
          it++;
      }
}

void RealPlayer::getHeroes(const std::list<Stack*> stacks, std::vector<Uint32>& dst)
{
    std::list<Stack*>::const_iterator it;
    for (it = stacks.begin(); it != stacks.end(); it++)
        (*it)->getHeroes(dst);
}

double RealPlayer::removeDeadArmies(std::list<Stack*>& stacks,
                                  std::vector<Uint32>& culprits)
{
    double total=0;
    Player *owner = NULL;
    if (stacks.empty() == 0)
    {
        owner = (*stacks.begin())->getPlayer();
        debug("Owner = " << owner);
        if (owner)
            debug("Owner of the stacks: " << owner->getName()
                  << ", his stacklist = " << owner->getStacklist());
    }
    for (unsigned int i = 0; i < culprits.size(); i++)
        debug("Culprit: " << culprits[i]);

    std::list<Stack*>::iterator it;
    for (it = stacks.begin(); it != stacks.end(); )
    {
        debug("Stack: " << (*it))
        for (Stack::iterator sit = (*it)->begin(); sit != (*it)->end();)
        {
            debug("Army: " << (*sit))
            if ((*sit)->getHP() <= 0)
            {
                debug("Army: " << (*sit)->getName())
                debug("Army: " << (*sit)->getXpReward())
                if ((*sit)->isHero())
                {
                    //one of our heroes died
                    //drop hero's stuff
                    Hero *h = static_cast<Hero *>(*sit);
                    //now record the details of the death
                    GameMap *gm = GameMap::getInstance();
                    Maptile *tile = gm->getTile((*it)->getPos());
                    if (tile->getBuilding() == Maptile::RUIN)
                    {
                        History_HeroKilledSearching* item;
                        item = new History_HeroKilledSearching();
                        item->fillData(h);
                        h->getPlayer()->getHistorylist()->push_back(item);
                        heroDropAllItems (h, (*it)->getPos());
                    }
                    else if (tile->getBuilding() == Maptile::CITY)
                    {
		        Citylist *clist = Citylist::getInstance();
                        City* c = clist->getObjectAt((*it)->getPos());
                        History_HeroKilledInCity* item;
                        item = new History_HeroKilledInCity();
                        item->fillData(h, c);
                        h->getPlayer()->getHistorylist()->push_back(item);
                        heroDropAllItems (h, (*it)->getPos());
                    }
                    else //somewhere else
                    {
                        History_HeroKilledInBattle* item;
                        item = new History_HeroKilledInBattle();
                        item->fillData(h);
                        h->getPlayer()->getHistorylist()->push_back(item);
                        heroDropAllItems (h, (*it)->getPos());
                    }
                }
                //Add the XP bonus to the total of the battle;
                total+=(*sit)->getXpReward();
                // here we destroy the army, so we send
                // the signal containing the fight data
                debug("sending sdyingArmy!")
                sdyingArmy.emit(*sit, culprits);
                sit = (*it)->flErase(sit);
                continue;
            }

            // heal this army to full hitpoints
            (*sit)->heal((*sit)->getStat(Army::HP));

            sit++;
        }
        
        debug("Is stack empty?")
            
        if ((*it)->empty())
        {
            if (owner)
            {
                debug("Removing this stack from the owner's stacklist");
                owner->deleteStack(*it);
            }
            else // there is no owner - like for the ruin's occupants
                debug("No owner for this stack - do stacklist too");

            debug("Removing from the vector too (the vetor had "
                  << stacks.size() << " elt)");
            it = stacks.erase(it);
        }
        else
            it++;
    }
    debug("after removeDead: size = " << stacks.size());
    return total;
}

Reward* RealPlayer::stackSearchRuin(Stack* s, Ruin* r)
{
    Reward *retReward;
    debug("RealPlayer::stack_search_ruin")

    //throw out impossible actions
    if ((s->getPos().x != r->getPos().x) ||
        (s->getPos().y != r->getPos().y))
    {
        cerr <<_("Error: searching stack and ruin to be searched not on same position\n");
        exit(-1);
    }

    if (r->isSearched())
        return NULL;

    // start the action item
    Action_Ruin* item = new Action_Ruin();
    item->fillData(r, s);

    Stack* keeper = r->getOccupant();

    if (keeper)
      {
        stackFight(&s, &keeper, true);

        // did the explorer not win?
        if (keeper && keeper->size())
          {
            item->setSearched(false);
            d_actions.push_back(item);

            return NULL;
          }

        r->setOccupant(0);
        if (keeper)
          delete keeper;
      }

     if (r->hasSage())
      {
        History_FoundSage* item = new History_FoundSage();
        item->fillData(dynamic_cast<Hero *>(s->getFirstHero()));
        d_history.push_back(item);
      }
     else
      {
       // The fight has been done or left out, now comes the reward. Up to now,
       int num = rand() % 3;
        if (num == 0)
          {
            int gold = rand() % 1000;
            Reward_Gold *reward = new Reward_Gold(gold);
            giveReward(NULL, reward);
	    retReward = reward;
          }
        else if (num == 1)
          {
            int num = (rand() % 8) + 1;
            const Army *a = Reward_Allies::randomArmyAlly();
            Reward_Allies *reward = new Reward_Allies(a, num);
            giveReward(getActivestack(), reward);
	    retReward = reward;
          }
        else if (num == 2)
          {
            Reward *itemReward = Rewardlist::getInstance()->popRandomItemReward();
            if (itemReward)
              {
                giveReward(getActivestack(), itemReward);
	        retReward = itemReward;
              }
            else //no items left to give!
              {
                int gold = rand() % 1000;
                Reward_Gold *reward = new Reward_Gold(gold);
                giveReward(NULL, reward);
	        retReward = reward;
              }
          }
/*
 * we can't give a ruin for searching a ruin
 * mostly because there's no map to show where the new ruin is
 * but also because if we just searched a ruin, we shouldn't be asked
 * to search another ruin.
        else if (num == 3)
          {
            Reward *ruinReward = Rewardlist::getInstance()->popRandomRuinReward();
            if (ruinReward)
              {
                giveReward(getActivestack(), ruinReward);
	        retReward = ruinReward;
              }
            else //no ruins left to give!
              {
                int gold = rand() % 1000;
                Reward_Gold *reward = new Reward_Gold(gold);
                giveReward(NULL, reward);
	        retReward = reward;
              }
          }
*/
      }

    ssearchingRuin.emit(r, s, retReward);

    r->setSearched(true);

    // actualize the actionlist
    item->setSearched(true);
    d_actions.push_back(item);

    supdatingStack.emit(0);
    return retReward;
}

int RealPlayer::stackVisitTemple(Stack* s, Temple* t)
{
    int count;
    debug("RealPlayer::stackVisitTemple")

    //abort in case of impossible action
    if (!s || (t->getPos().x != s->getPos().x)
        || (t->getPos().y != s->getPos().y))
    {
        cerr <<_("Stack tried to visit temple at wrong location\n");
        exit(-1);
    }

    // you have your stack blessed (+1 strength)
    count = s->bless();

    Action_Temple* item = new Action_Temple();
    item->fillData(t, s);
    d_actions.push_back(item);

    svisitingTemple.emit(t, s);
    
    supdatingStack.emit(0);

    return count;
}

Quest* RealPlayer::stackGetQuest(Stack* s, Temple* t)
{
    QuestsManager *qm = QuestsManager::getInstance();
    debug("Realplayer::stackGetQuest")

    // bail out in case of senseless data
    if (!s || !t || (s->getPos().x != t->getPos().x) 
        || (s->getPos().y != t->getPos().y))
    {
        cerr <<_("Stack tried to visit temple at wrong location\n");
        exit(-1);
    }
    
    std::vector<Quest*> quests = qm->getPlayerQuests(Playerlist::getActiveplayer());
    if (quests.size() > 0)
      return NULL;

    Quest* q=0;
    if (s->getFirstHero())
      {
        q = qm->createNewQuest(s->getFirstHero()->getId());
      }

    // couldn't assign a quest for various reasons
    if (!q)
        return 0;

    // Now fill the action item
    Action_Quest* action = new Action_Quest();
    action->fillData(q);
    d_actions.push_back(action);

    // and record it for posterity
    History_HeroQuestStarted * history = new History_HeroQuestStarted();
    history->fillData(dynamic_cast<Hero *>(s->getFirstHero()));
    d_history.push_back(history);
    return q;
}

bool RealPlayer::cityOccupy(City* c, bool emit)
{
    c->conquer(this);

    //set the production to the cheapest armytype
    c->setProduction(-1);
    if (c->getArmytype(0) != -1)
        c->setProduction(0);

    Action_Occupy* item = new Action_Occupy();
    item->fillData(c);
    d_actions.push_back(item);

    if (emit)
      soccupyingCity.emit(c, getActivestack());
    //to signal that the cities have changed a bit
    supdatingCity.emit(c);

    return true;
}

bool RealPlayer::cityOccupy(City* c)
{
    debug("cityOccupy")

    return cityOccupy (c, true);
}

bool RealPlayer::cityPillage(City* c, int& gold, int& pillaged_army_type)
{
    gold = 0;
    pillaged_army_type = -1;
    debug("RealPlayer::cityPillage")

    Action_Pillage* item = new Action_Pillage();
    item->fillData(c);
    d_actions.push_back(item);
    
    // get rid of the most expensive army type and trade it in for 
    // half it's cost
    // it is presumed that the last army type is the most expensive

    if (c->getNoOfBasicProd() > 0)
      {
        int i;
        unsigned int max_cost = 0;
        int slot = -1;
        for (i = 0; i < c->getNoOfBasicProd(); i++)
          {
            const Army *a = c->getArmy(i);
            if (a != NULL)
              {
		if (a->getProductionCost() == 0)
		  {
                    slot = i;
		    break;
		  }
                if (a->getProductionCost() > max_cost)
                  {
                    max_cost = a->getProductionCost();
                    slot = i;
                  }
              }
          }
        if (slot > -1)
          {
            const Army *a = c->getArmy(slot);
            pillaged_army_type = a->getType();
	    if (a->getProductionCost() == 0)
	      gold += 1500;
	    else
	      gold += a->getProductionCost() / 2;
            c->removeBasicProd(slot);
          }
      }

    addGold(gold);
    std::list<Uint32> sacked_types;
    sacked_types.push_back (pillaged_army_type);
    spillagingCity.emit(c, Playerlist::getActiveplayer()->getActivestack(), 
                        gold, sacked_types);
    return cityOccupy (c, false);
}

bool RealPlayer::citySack(City* c, int& gold, std::list<Uint32> *sacked_types)
{
    gold = 0;
    debug("RealPlayer::citySack")

    Action_Sack* item = new Action_Sack();
    item->fillData(c);
    d_actions.push_back(item);
    
    //trade in all of the army types except for one
    //presumes that the army types are listed in order of expensiveness
  
    if (c->getNoOfBasicProd() > 1)
      {
        const Army *a;
        int i, max = 0;
        for (i = 0; i < c->getNoOfBasicProd(); i++)
          {
            a = c->getArmy(i);
            if (a)
              max++;
          }

        i = c->getNoOfBasicProd() - 1;
        while (max > 1)
          {
            a = c->getArmy(i);
            if (a != NULL)
              {
                sacked_types->push_back(a->getType());
		if (a->getProductionCost() == 0)
		  gold += 1500;
		else
		  gold += a->getProductionCost() / 2;
                c->removeBasicProd(i);
                max--;
              }
            i--;
          }
      }

    addGold(gold);
    ssackingCity.emit(c, Playerlist::getActiveplayer()->getActivestack(), gold,
                      *sacked_types);
    return cityOccupy (c, false);
}

bool RealPlayer::cityRaze(City* c)
{
    debug("RealPlayer::cityRaze")

    Action_Raze* action = new Action_Raze();
    action->fillData(c);
    d_actions.push_back(action);

    //ugh, put a similar history event to the action on the history list
    History_CityRazed* history = new History_CityRazed();
    history->fillData(c);
    d_history.push_back(history);

    c->setBurnt(true);

    supdatingCity.emit(c);

    srazingCity.emit(c, Playerlist::getActiveplayer()->getActivestack());
    return true;
}

bool RealPlayer::cityBuyProduction(City* c, int slot, int type)
{
    Uint32 as;
    const Armysetlist* al = Armysetlist::getInstance();

    as = c->getPlayer()->getArmyset();

    // sort out unusual values (-1 is allowed and means "scrap production")
    if ((type < -1) || (type >= (int)al->getSize(as)))
        return false;
    
    // return if we don't have enough money
    if ((type != -1) && ((int)al->getArmy(as, type)->getProductionCost() > d_gold))
        return false;

    // return if the city already has the production
    if (c->hasProduction(type, as))
        return false;

    c->removeBasicProd(slot);
    if (!c->addBasicProd(slot, type))
        return false;
    
    // and do the rest of the neccessary actions
    withdrawGold(al->getArmy(as, type)->getProductionCost());

    Action_Buy* item = new Action_Buy();
    item->fillData(c, slot, type);
    d_actions.push_back(item);

    return true;
}

bool RealPlayer::cityChangeProduction(City* c, int slot)
{
    c->setProduction(slot);

    Action_Production* item = new Action_Production();
    item->fillData(c, slot);
    d_actions.push_back(item);

    return true;
}

bool RealPlayer::giveReward(Stack *s, Reward *reward)
{
    debug("RealPlayer::give_reward")

    switch (reward->getType())
      {
      case Reward::GOLD:
        addGold(dynamic_cast<Reward_Gold*>(reward)->getGold());
	break;
      case Reward::ALLIES:
        {
          const Army *a = dynamic_cast<Reward_Allies*>(reward)->getArmy();

          Reward_Allies::addAllies(s->getPlayer(), s->getPos(), a,
                       dynamic_cast<Reward_Allies*>(reward)->getNoOfAllies());
        }
	break;
      case Reward::ITEM:
        static_cast<Hero*>(s->getFirstHero())->addToBackpack(
          dynamic_cast<Reward_Item*>(reward)->getItem());
	break;
      case Reward::RUIN:
        //assign the hidden ruin to this player
        Ruin *r = dynamic_cast<Reward_Ruin*>(reward)->getRuin();
        r->setHidden(true);
        r->setOwner(this);

	break;
      }

    Action_Reward* item = new Action_Reward();
    item->fillData(reward);
    d_actions.push_back(item);
    //FIXME: get rid of this reward now that we're done with it

    return true;
}

bool RealPlayer::stackMoveOneStep(Stack* s)
{
    int needed_moves;
    if (!s)
        return false;
    
    if (!s->enoughMoves())
        return false;

    Vector<int> dest = *(s->getPath()->front());

    Uint32 maptype = GameMap::getInstance()->getTile(dest.x,dest.y)->getMaptileType();
    City* to_city = Citylist::getInstance()->getObjectAt(dest.x, dest.y);
    City* on_city = Citylist::getInstance()->getObjectAt(s->getPos().x, s->getPos().y);
    bool on_water = (GameMap::getInstance()->getTile(s->getPos().x,s->getPos().y)->getMaptileType() == Tile::WATER);
    bool to_water = (GameMap::getInstance()->getTile(dest.x,dest.y)->getMaptileType() == Tile::WATER);
    bool ship_load_unload = false;
    //here we mark the armies as being on or off a boat
    if (!s->isFlying())
      {
        if ((on_water && to_city) || (on_city && to_water))
          {
            ship_load_unload = true;
            for (Stack::iterator it = s->begin(); it != s->end(); it++)
              {
                if (to_water && 
                     ((*it)->getStat(Army::MOVE_BONUS) & Tile::WATER) == 0)
                  (*it)->setInShip(true);
                else
                  (*it)->setInShip(false);
              }
          }
      }
    else
      {
        for (Stack::iterator it = s->begin(); it != s->end(); it++)
          (*it)->setInShip(false);
      }
    needed_moves = GameMap::getInstance()->getTile(dest.x,dest.y)->getMoves();

    for (Stack::iterator it = s->begin(); it != s->end(); it++)
    //calculate possible move boni for each army
    {
        if (ship_load_unload)
          {
            (*it)->decrementMoves((*it)->getMoves());
            continue;
          }
        if (to_city != 0)
        //cities cost one MP
        {
            (*it)->decrementMoves(1);
            continue;
        }

        if ((*it)->getStat(Army::MOVE_BONUS) == maptype)
        //if army has move bonus, it takes only 2 move points...
        {
            (*it)->decrementMoves(2);
            continue;
        }
        //else the whole
        (*it)->decrementMoves(needed_moves);
    }

    s->moveOneStep();    //this is only for updating positions etc.

    d_fogmap->alterFogRadius(dest, s->getMaxSight(), FogMap::OPEN);

    //! signal that we have moved the stack
    smovingStack.emit(s);

    Action_Move* item = new Action_Move();
    item->fillData(s, dest);
    d_actions.push_back(item);

    return true;
}

bool RealPlayer::vectorFromCity(City * c, Vector<int> dest)
{
  c->setVectoring(dest);
  Action_Vector* item = new Action_Vector();
  item->fillData(c, dest);
  d_actions.push_back(item);
  return true;
}

void RealPlayer::setFightOrder(std::list<Uint32> order) 
{
  d_fight_order = order;
  Action_FightOrder * item = new Action_FightOrder();
  item->fillData(order);
  d_actions.push_back(item);
}

void RealPlayer::resign() 
{
  //disband all stacks
  getStacklist()->flClear();

  //raze all cities
  Citylist *cl = Citylist::getInstance();
  for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
    {
      if ((*it).getPlayer() == this)
        (*it).setBurnt(true);
    }

  Action_Resign* item = new Action_Resign();
  item->fillData();
  d_actions.push_back(item);
    
  withdrawGold(getGold()); //empty the coffers!

  getStacklist()->setActivestack(0);
  supdatingStack.emit(0);
}

bool RealPlayer::heroPlantStandard(Stack* s)
{
    debug("player::heroPlantStandard(Stack*)")
    if (!s)
      s = getActivestack();
    for (Stack::iterator it = s->begin(); it != s->end(); it++)
      {
        if ((*it)->isHero())
	  {
	    Hero *hero = dynamic_cast<Hero*>((*it));
            std::list<Item*> backpack = hero->getBackpack();
            for (std::list<Item*>::iterator i = backpack.begin(), 
	         end = backpack.end(); i != end; ++i)
	      {
		if ((*i)->isPlantable() && (*i)->getPlantableOwner() == this)
		  {
		    //drop the item, and plant it
		    (*i)->setPlanted(true);
                    GameMap *gm = GameMap::getInstance();
	            gm->getTile(s->getPos())->addItem(*i);
	            hero->removeFromBackpack(*i);
                    Action_Plant * item = new Action_Plant();
                    item->fillData(hero->getId(), (*i)->getId());
                    d_actions.push_back(item);
                    break;
		  }
	      }
	  }
      }
    return true;
}

bool RealPlayer::heroDropAllItems(Hero *h, Vector<int> pos)
{
  std::list<Item*> backpack = h->getBackpack();
  for (std::list<Item*>::iterator i = backpack.begin(), end = backpack.end();
    i != end; ++i)
    heroDropItem(h, *i, pos);
  return true;
}

bool RealPlayer::heroDropItem(Hero *h, Item *i, Vector<int> pos)
{
  GameMap::getInstance()->getTile(pos)->addItem(i);
  h->removeFromBackpack(i);
  Action_Equip* item = new Action_Equip();
  item->fillData(h->getId(), i->getId(), Action_Equip::BACKPACK);
  d_actions.push_back(item);
  return true;
}

bool RealPlayer::heroPickupItem(Hero *h, Item *i, Vector<int> pos)
{
  GameMap::getInstance()->getTile(pos)->removeItem(i);
  h->addToBackpack(i, 0);
  Action_Equip* item = new Action_Equip();
  item->fillData(h->getId(), i->getId(), Action_Equip::GROUND);
  d_actions.push_back(item);
  return true;
}

bool RealPlayer::heroCompletesQuest(Hero *h)
{
    // record it for posterity
    History_HeroQuestCompleted* item = new History_HeroQuestCompleted();
    item->fillData(h);
    d_history.push_back(item);
    return true;
}

Uint32 RealPlayer::getScore()
{
  //go get our last published score in the history
  Uint32 score = 0;
  std::list<History*>::iterator it = d_history.begin();
  for (; it != d_history.end(); it++)
    {
      if ((*it)->getType() == History::SCORE)
	score = static_cast<History_Score*>(*it)->getScore();
    }
  return score;
}
// End of file
