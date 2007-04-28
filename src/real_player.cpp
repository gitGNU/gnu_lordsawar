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
#include "FightDialog.h"
#include "stacklist.h"
#include "citylist.h"
#include "QuestsManager.h"
#include "path.h"
#include "GameMap.h"
#include "army.h"
#include "hero.h"
#include "MoveResult.h"
#include "Configuration.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

RealPlayer::RealPlayer(string name, Uint32 armyset, SDL_Color color,
            Player::Type type)
    :Player(name, armyset, color, type)
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

bool RealPlayer::startTurn()
{
    clearActionlist();

    return true;
}

bool RealPlayer::invadeCity(City* c)
{
    //for the realplayer, this function doesn't do a lot.
    //However, an AI player has to decide here what to do (occupy, raze,
    //pillage)
    sinvadingCity.emit(c);
    soccupyingCity.emit(c);
    return true;
}

bool RealPlayer::recruitHero(Hero* hero, int cost)
{
    // for the realplayer, this function also just raises a signal and looks
    // what to do next. The derived classes or W_Edit actually react on the
    // signal.

    return srecruitingHero.emit(hero, cost);
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

MoveResult *RealPlayer::stackMove(Stack* s, PG_Point dest, bool follow)
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
    while (s->enoughMoves() && (s->getPath()->size() > 1))
    {
        if (stackMoveOneStep(s))
            stepCount++;
        supdatingStack.emit(0);

        // sleep for a specified amount of time
        SDL_Delay(Configuration::s_displaySpeedDelay);
    }

    if ((s->getPath()->size() == 1) && s->enoughMoves())
    //now look for fight targets, joins etc.
    {
        PG_Point pos = **(s->getPath()->begin());
        City* city = Citylist::getInstance()->getObjectAt(pos);
        Stack* target = Stacklist::getObjectAt(pos);

        //first fight_city to avoid ambiguity with fight_army
        if (city && (city->getPlayer() != this) && (!city->isBurnt()))
        {
            Fight::Result result;
            MoveResult *moveResult = new MoveResult(true);
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
            if (Stacklist::defendersInCity(city).empty())
            {
                if (stackMoveOneStep(s))
                    stepCount++;
                invadeCity(city); //determine what to do with city
            }
            else if (s)
            {
                // we didn't suceed in defeating the defenders, but at least our
                // stack has survived. Subtract some MP because fighting takes
                // time.
                s->decrementMoves(2);
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


Fight::Result RealPlayer::stackFight(Stack** attacker, Stack** defender, bool ruin) 
{
    debug("stackFight: player = " << getName()<<" at position "
          <<(*defender)->getPos().x<<","<<(*defender)->getPos().y);

    // save the defender's player for future use
    Player* pd = (*defender)->getPlayer();

    // I suppose, this should be always true, but one can never be sure
    bool attacker_active = ((*attacker) == d_stacklist->getActivestack());
    

    // Fighting goes in three steps:
    // 1. Initializing
    // 2. Fighting
    // 3. Cleaning up (removing dead armies and such)
    
    // First, initiate the stuff. Fortunately, the Fight and FightDialog
    // class do most of it
    FightDialog* fd = new FightDialog(*attacker, *defender, ruin);
    fd->Show();


    // Second, do the fighting

    //the timers of bigmap and smallmap cause flickering of the screen, so
    //we remove them for the duration of the fight
    sinterruptTimers.emit();

    fd->battle();
    fd->Hide();

    
    // Third: cleanup

    // add a fight item about the combat
    Action_Fight* item = new Action_Fight();
    item->fillData(fd->getFight());
    d_actions.push_back(item);

    // get attacker and defender heroes and more...
    const Fight* fight = fd->getFight();
    std::list<Stack*> attackers = fight->getAttackers();
    std::list<Stack*> defenders = fight->getDefenders();
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

    debug("restart timers");
    //don't forget to restart the timers
    scontinueTimers.emit();

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
    bool exists = false;
    for (Stacklist::iterator it = d_stacklist->begin(); it != d_stacklist->end(); it++)
        if ((*it) == (*attacker))
            exists = true;
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

    // remove the dialog only here (clears the fight class as well)
    Fight::Result result = fight->getResult();
    delete fd;
    
    return result;
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

            if(((*sit)->getBattlesNumber())>10 && !((*sit)->getMedalBonus(2)))
            {
                (*sit)->setMedalBonus(2,true);
                // We must recalculate the XPValue of this unit since it got a medal
                (*sit)->setXpReward((*sit)->getXpReward()+1);
                // We get the medal bonus here
                (*sit)->setStat(Army::HP, (*sit)->getStat(Army::HP, false)+4);
                // Emit signal
                snewMedalArmy.emit(*sit);
            }

            debug("Army hits " <<  (*sit)->getNumberHasHit())

            // Only give medals if the unit has attacked often enough, else
            // medals loose the flair of something special; a value of n means
            // roughly to hit an equally strong unit around n times. (note: one
            // hit! An attack can consist of up to strength hits)
            if(((*sit)->getNumberHasHit() > 50) && !(*sit)->getMedalBonus(0))
            {
                (*sit)->setMedalBonus(0,true);
                // We must recalculate the XPValue of this unit since it got a medal
                (*sit)->setXpReward((*sit)->getXpReward()+1);
                // We get the medal bonus here
                (*sit)->setStat(Army::STRENGTH, (*sit)->getStat(Army::STRENGTH, false)+1);
                // Emit signal
                snewMedalArmy.emit(*sit);
            }

            debug("army being hit " <<  (*sit)->getNumberHasBeenHit())

            // Gives the medal for good defense. The more negative the number
            // the more blows the unit evaded. n means roughly avoid n
            // hits from an equally strong unit. Since we want to punish
            // the case of the unit hiding among many others, we set this
            // value quite high.
            if(((*sit)->getNumberHasBeenHit() < -100) && !(*sit)->getMedalBonus(1))
            {
                (*sit)->setMedalBonus(1,true);
                // We must recalculate the XPValue of this unit since it got a medal
                (*sit)->setXpReward((*sit)->getXpReward()+1);
                // We get the medal bonus here
                (*sit)->setStat(Army::DEFENSE, (*sit)->getStat(Army::DEFENSE, false)+1);
                // Emit signal
                snewMedalArmy.emit(*sit);
            }

            for(int i=0;i<3;i++)
            {
                debug("MEDAL[" << i << "]==" << (*sit)->getMedalBonus(i))
            }

            // We reset the hit values after the battle
            (*sit)->setNumberHasHit(0);
            (*sit)->setNumberHasBeenHit(0);

            while((*sit)->canGainLevel())
            {
                // Units not associated to a player never raise levels.
                if ((*sit)->getPlayer() == 0)
                    break;

                //Here this for is to check if army must raise 2 or more levels per time
                //depending on the XP and level itsself

                debug("ADVANCING LEVEL "<< "CANGAINLEVEL== " << (*sit)->canGainLevel())
                (*sit)->getPlayer()->levelArmy(*sit);
            }
            debug("Army new XP=" << (*sit)->getXP())
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
                //Add the XP bonus to the total of the battle;
                total+=(*sit)->getXpReward();
                // here we destroy the army, so we send
                // the signal containing the fight data
                debug("sending sdyingArmy!")
                sdyingArmy.emit(*sit, culprits);
                sit = (*it)->flErase(sit);
                continue;
            }

            if ((*sit)->getStat(Army::ARMY_BONUS) & Army::REGENERATE)
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

bool RealPlayer::stackSearchRuin(Stack* s, Ruin* r)
{
    debug("RealPlayer::stack_search_ruin")

    //throw out impossible actions
    if ((s->getPos().x != r->getPos().x) ||
        (s->getPos().y != r->getPos().y))
    {
        cerr <<_("Error: searching stack and ruin to be searched not on same position\n");
        exit(-1);
    }

    if (r->isSearched())
        return false;

    // start the action item
    Action_Ruin* item = new Action_Ruin();
    item->fillData(r, s);

    Stack* keeper = r->getOccupant();

    stackFight(&s, &keeper, true);

    // did the explorer not win?
    if (keeper && keeper->size())
    {
        item->setSearched(false);
        d_actions.push_back(item);

        return false;
    }

    r->setOccupant(0);
    if (keeper)
        delete keeper;

    // The fight has been done or left out, now comes the reward. Up to now,
    // we only give some gold (automatically done with give_reward())
    r->setSearched(true);
    ssearchingRuin.emit(r, s);

    // actualize the actionlist
    item->setSearched(true);
    d_actions.push_back(item);

    // do we reward a player even when his hero died?
    giveReward(rand() % 1000);
    supdatingStack.emit(0);
    return true;
}

bool RealPlayer::stackVisitTemple(Stack* s, Temple* t)
{
    debug("RealPlayer::stackVisitTemple")

    //abort in case of impossible action
    if (!s || (t->getPos().x != s->getPos().x)
        || (t->getPos().y != s->getPos().y))
    {
        cerr <<_("Stack tried to visit temple at wrong location\n");
        exit(-1);
    }

    // you have your stack blessed (+1 strength)
    s->bless();

    Action_Temple* item = new Action_Temple();
    item->fillData(t, s);
    d_actions.push_back(item);

    svisitingTemple.emit(t, s);
    
    supdatingStack.emit(0);

    return true;
}

Quest* RealPlayer::stackGetQuest(Stack* s, Temple* t)
{
    debug("Realplayer::stackGetQuest")

    // bail out in case of senseless data
    if (!s || !t || (s->getPos().x != t->getPos().x) 
        || (s->getPos().y != t->getPos().y))
    {
        cerr <<_("Stack tried to visit temple at wrong location\n");
        exit(-1);
    }
    
    std::vector<Uint32> heroes;
    s->getHeroes(heroes);
    Quest* q=0;
    
    // Try to assign each hero in turn a quest. If it doesn't work, we assume,
    // he already has one.
    for (unsigned int i = 0; i < heroes.size(); i++)
    {
        q = QuestsManager::getInstance()->createNewQuest(heroes[i]);
        if (q)
            break;
    }

    // couldn't assign a quest for various reasons
    if (!q)
        return 0;

    // Now fill the action item
    Action_Quest* item = new Action_Quest();
    item->fillData(q);
    d_actions.push_back(item);

    return q;
}

bool RealPlayer::cityOccupy(City* c)
{
    debug("cityOccupy")

    c->conquer(this);

    //set the production to the cheapest armytype
    c->setProduction(-1, false);
    if (c->getArmytype(0, false) != -1)
        c->setProduction(0, false);

    Action_Occupy* item = new Action_Occupy();
    item->fillData(c);
    d_actions.push_back(item);

    //to signal that the cities have changed a bit
    supdatingCity.emit(c);

    return true;
}

bool RealPlayer::cityPillage(City* c, int& gold)
{
    gold = 0;
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
        for (i = 0; i < c->getNoOfBasicProd(); i++)
          {
            const Army *a = c->getArmy(i, false);
            if (a != NULL)
              {
                gold += a->getProductionCost() / 2;
                c->removeBasicProd(i);
                break;
              }
          }
      }

    addGold(gold);
    cityOccupy(c);
    return true;
}

bool RealPlayer::citySack(City* c, int& gold)
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
            a = c->getArmy(i, false);
            if (a)
              max++;
          }

        i = c->getNoOfBasicProd() - 1;
        while (max > 1)
          {
            a = c->getArmy(i, false);
            if (a != NULL)
              {
                gold += a->getProductionCost() / 2;
                c->removeBasicProd(i);
                max--;
              }
            i--;
          }
      }

    addGold(gold);
    cityOccupy(c);
    return true;
}

bool RealPlayer::cityRaze(City* c)
{
    debug("RealPlayer::cityRaze")

    Action_Raze* item = new Action_Raze();
    item->fillData(c);
    d_actions.push_back(item);

    c->setBurnt(true);
    //make sure there are no side effects e.g. when counting the cities of
    //a player
    c->conquer(Playerlist::getInstance()->getNeutral());

    supdatingCity.emit(c);

    return true;
}

bool RealPlayer::cityUpgradeDefense(City* c)
{
    debug("RealPlayer::city_upgrade_defense")

    int needed_gold = c->getDefenseLevel() * 1000;
  
    
    if ((needed_gold >= (int) (1000*CITY_LEVELS)) || (d_gold < needed_gold))
        return false;    //maximum level is CITY_LEVELS or not enough gold

    c->raiseDefense();
    withdrawGold(needed_gold);

    Action_Upgrade* item = new Action_Upgrade();
    item->fillData(c);
    d_actions.push_back(item);

    return true;
}

bool RealPlayer::cityBuyProduction(City* c, int slot, int type, bool advanced)
{
    Uint32 as;
    const Armysetlist* al = Armysetlist::getInstance();

    if (advanced)
        as = getArmyset();
    else
        as = al->getStandardId();

    // sort out unusual values (-1 is allowed and means "scrap production")
    if ((type < -1) || (type >= (int)al->getSize(as)))
        return false;
    
    // return if we don't have enough money
    if ((type != -1) && ((int)al->getArmy(as, type)->getProductionCost() > d_gold))
        return false;

    // return if the city already has the production
    if (c->hasProduction(type, as))
        return false;

    // now we assume everything is ok. Remove old production and set the new one
    if (advanced)
    {
        c->removeAdvancedProd(slot);
        if (!c->addAdvancedProd(slot, type))
            return false;
    }
    else
    {
        c->removeBasicProd(slot);
        if (!c->addBasicProd(slot, type))
            return false;
    }
    
    // and do the rest of the neccessary actions
    withdrawGold(al->getArmy(as, type)->getProductionCost());

    Action_Buy* item = new Action_Buy();
    item->fillData(c, slot, type, advanced);
    d_actions.push_back(item);

    return true;
}

bool RealPlayer::cityChangeProduction(City* c, int slot, bool advanced)
{
    c->setProduction(slot, advanced);

    Action_Production* item = new Action_Production();
    item->fillData(c, slot, advanced);
    d_actions.push_back(item);

    return true;
}

bool RealPlayer::giveReward(int gold)
{
    debug("RealPlayer::give_reward")

    //by now we don't have the infrastructure to give something different
    //from gold, so this function isn't great

    addGold(gold);

    Action_Reward* item = new Action_Reward();
    item->fillData(gold);
    d_actions.push_back(item);

    return true;
}

bool RealPlayer::stackMoveOneStep(Stack* s)
{
    if (!s)
        return false;
    
    if (!s->enoughMoves())
        return false;

    PG_Point dest = *(s->getPath()->front());

    int needed_moves = GameMap::getInstance()->getTile(dest.x,dest.y)->getMoves();
    Uint32 maptype = GameMap::getInstance()->getTile(dest.x,dest.y)->getMaptileType();
    City* c = Citylist::getInstance()->getObjectAt(dest.x, dest.y);

    for (Stack::iterator it = s->begin(); it != s->end(); it++)
    //calculate possible move boni for each army
    {
        if (c != 0)
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

// End of file
