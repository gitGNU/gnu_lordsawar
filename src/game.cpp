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

#include <config.h>

#include <algorithm>
#include <vector>
#include <assert.h>
#include <SDL.h>
#include <sigc++/functors/mem_fun.h>
#include <sigc++/adaptors/bind.h>

#include "game.h"

#include "ucompose.hpp"
#include "rectangle.h"
#include "sound.h"
#include "GraphicsCache.h"
#include "GameScenario.h"
#include "NextTurn.h"

#include "gamebigmap.h"
#include "smallmap.h"

#include "army.h"
#include "hero.h"
#include "stacklist.h"
#include "citylist.h"
#include "ruinlist.h"
#include "templelist.h"
#include "armysetlist.h"
#include "city.h"
#include "ruin.h"
#include "signpost.h"
#include "temple.h"
#include "GameMap.h"
#include "playerlist.h"
#include "path.h"
#include "Configuration.h"
#include "File.h"
#include "Quest.h"
#include "reward.h"
#include "game-parameters.h"
#include "FogMap.h"


//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
#define debug(x)

Game::Game(GameScenario* gameScenario)
    : d_gameScenario(gameScenario) 
{
    input_locked = false;
    
    // init the bigmap
    bigmap.reset(new GameBigMap);
    bigmap->stack_selected.connect(
	sigc::mem_fun(this, &Game::on_stack_selected));
    bigmap->path_set.connect(
	sigc::mem_fun(this, &Game::update_control_panel));
    bigmap->city_queried.connect(
	sigc::mem_fun(this, &Game::on_city_queried));
    bigmap->ruin_queried.connect(
	sigc::mem_fun(this, &Game::on_ruin_queried));
    bigmap->signpost_queried.connect(
	sigc::mem_fun(this, &Game::on_signpost_queried));
    bigmap->temple_queried.connect(
	sigc::mem_fun(this, &Game::on_temple_queried));
    bigmap->stack_queried.connect(
	sigc::mem_fun(this, &Game::on_stack_queried));

    // init the smallmap
    smallmap.reset(new SmallMap);
    // pass map changes directly through 
    smallmap->map_changed.connect(
	sigc::mem_fun(smallmap_changed,
		      &sigc::signal<void, SDL_Surface *>::emit));
    smallmap->resize(GameMap::get_dim() * 2);

    // connect the two maps
    bigmap->view_changed.connect(
	sigc::mem_fun(smallmap.get(), &SmallMap::set_view));
    smallmap->view_changed.connect(
	sigc::mem_fun(bigmap.get(), &GameBigMap::set_view));

    // get the maps up and running
    bigmap->screen_size_changed();

    // connect player callbacks
    Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::iterator i = pl->begin(); i != pl->end(); ++i)
    {
	Player *p = *i;
        p->sdyingStack.connect(sigc::mem_fun(this, &Game::stackDied));

        if (p->getType() == Player::HUMAN)
        {
            p->snewLevelArmy.connect(sigc::mem_fun(this, &Game::newLevelArmy));
            p->snewMedalArmy.connect(sigc::mem_fun(this, &Game::newMedalArmy));
	    p->srecruitingHero.connect(
		// bind the player
		sigc::bind<0>(
		    sigc::mem_fun(hero_offers_service, &sigc::signal<bool, Player *, Hero *, City *, int>::emit),
		    p));
        }
	
        p->schangingStatus.connect(
	    sigc::mem_fun(this, &Game::update_sidebar_stats));
        p->supdatingStack.connect(sigc::mem_fun(this, &Game::stackUpdate));
	
        p->sinvadingCity.connect(sigc::mem_fun(this, &Game::invading_city));
        p->fight_started.connect(
	    sigc::mem_fun(*this, &Game::on_fight_started));
        p->ruinfight_started.connect(
	    sigc::mem_fun(ruinfight_started, &sigc::signal<void, Stack *, Stack *>::emit));
        p->ruinfight_finished.connect(
	    sigc::mem_fun(ruinfight_finished, &sigc::signal<void, Fight::Result>::emit));
    }
    pl->splayerDead.connect(sigc::mem_fun(this, &Game::on_player_died));

    //set up a NextTurn object
    d_nextTurn = new NextTurn(d_gameScenario->getTurnmode());
    d_nextTurn->splayerStart.connect(
	sigc::mem_fun(this, &Game::init_turn_for_player));
    d_nextTurn->snextRound.connect(
	sigc::mem_fun(d_gameScenario, &GameScenario::nextRound));
    d_nextTurn->supdating.connect(
	sigc::mem_fun(this, &Game::redraw));
            
    center_view_on_city();
    update_control_panel();

    loadHeroTemplates();
}

Game::~Game()
{
    delete d_gameScenario;
    delete d_nextTurn;
}

void Game::end_turn()
{
    bigmap->unselect_active_stack();
    clear_stack_info();
    update_control_panel();
    lock_inputs();

    d_nextTurn->endTurn();
}

void Game::update_stack_info()
{
    Stack* stack = Playerlist::getActiveplayer()->getActivestack();

    if (Playerlist::getActiveplayer()->getType() != Player::HUMAN &&
        GameScenario::s_hidden_map == true)
      return;
    stack_info_changed.emit(stack);
}

void Game::clear_stack_info()
{
    stack_info_changed.emit(0);
}

void Game::update_sidebar_stats()
{
    SidebarStats s;
    Player *player = Playerlist::getActiveplayer();

    s.name = player->getName();
    s.gold = player->getGold();
    s.income = s.cities = 0;
    for (Citylist::iterator i = Citylist::getInstance()->begin(),
	     end = Citylist::getInstance()->end(); i != end; ++i)
	if (i->getPlayer() == player)
	{
	    s.income += i->getGold();
	    ++s.cities;
	}

    s.units = 0;
    Stacklist *sl = player->getStacklist();
    for (Stacklist::iterator i = sl->begin(), iend = sl->end(); i != iend; ++i)
	s.units += (*i)->size();
    
    s.turns = d_gameScenario->getRound();
    
    sidebar_stats_changed.emit(s);
}

void Game::redraw()
{
    if (bigmap.get())
	bigmap->draw();
    if (smallmap.get())
	smallmap->draw();
}

void Game::select_prev_stack()
{
    Stack* stack = Playerlist::getActiveplayer()->getStacklist()->setPrev();
    if (stack)
        bigmap->select_active_stack();
}

void Game::select_next_stack()
{
    Stack* stack = Playerlist::getActiveplayer()->getStacklist()->setNext();
    if (stack)
        bigmap->select_active_stack();
}

void Game::select_next_movable_stack()
{
    Stacklist *sl = Playerlist::getActiveplayer()->getStacklist();
    Stack* stack = sl->getNextMovable();
    sl->setActivestack(stack);
    bigmap->select_active_stack();
}

void Game::move_selected_stack()
{
    Player *p = Playerlist::getActiveplayer();
    p->stackMove(p->getActivestack());
}

void Game::move_all_stacks()
{
    Player *player = Playerlist::getActiveplayer();
    Stacklist* sl = player->getStacklist();
    Stack *orig = sl->getActivestack();

    for (Stacklist::iterator i = sl->begin(), end = sl->end(); i != end; ++i)
    {
	Stack &s = **i;
	if (!s.empty() && !s.getPath()->empty() && s.canMove())
	{
	    sl->setActivestack(&s);
            bigmap->select_active_stack();
            player->stackMove(player->getActivestack());
	}
    }
    
    // if the stack still exists, set the active stack to the old one, else to 0
    if (find(sl->begin(), sl->end(), orig) != sl->end())
    {
        sl->setActivestack(orig);
	bigmap->select_active_stack();
    }
    else
    {
        sl->setActivestack(0);
	bigmap->unselect_active_stack();
    }
}

void Game::defend_selected_stack()
{
    Player *player = Playerlist::getActiveplayer();
    Stack *stack = player->getActivestack();
    assert(stack);
    stack->setDefending(true);

    stack = player->getStacklist()->getNextMovable();
    player->getStacklist()->setActivestack(stack);
    
    if (stack)
        bigmap->select_active_stack();
    else
	bigmap->unselect_active_stack();
}

void Game::disband_selected_stack()
{
    Stack *stack = Playerlist::getActiveplayer()->getActivestack();
    bigmap->unselect_active_stack();
    Playerlist::getActiveplayer()->getStacklist()->setActivestack(0);
    clear_stack_info();
    update_control_panel();
    stack->flClear();
}

void Game::center_selected_stack()
{
    Stack *stack = Playerlist::getActiveplayer()->getActivestack();
    if (stack) 
        bigmap->select_active_stack();
}

void Game::search_selected_stack()
{
    Player *player = Playerlist::getActiveplayer();
    Stack* stack = player->getActivestack();

    Ruin* ruin = Ruinlist::getInstance()->getObjectAt(stack->getPos());
    Temple* temple = Templelist::getInstance()->getObjectAt(stack->getPos());

    if (ruin && !ruin->isSearched() && stack->getGroupMoves() > 0 &&
        ((ruin->isHidden() == true && ruin->getOwner() == player) ||
         ruin->isHidden() == false))
    {
        Reward *reward;

	reward = player->stackSearchRuin(stack, ruin);
        if (!reward)
        {
	    redraw();
	    update_stack_info();
	    update_control_panel();
            return;
        }

	ruin_searched.emit(ruin, stack, reward);
	
        update_sidebar_stats();
    }
    else if (temple && temple->searchable() && stack->getGroupMoves() > 0)
    {
        int blessCount;
        blessCount = player->stackVisitTemple(stack, temple);
	bool wants_quest = temple_visited.emit(stack->hasHero(), temple, blessCount);
        if (wants_quest)
        {
            Quest *q = player->stackGetQuest(stack, temple);
            Hero* hero = dynamic_cast<Hero*>(stack->getFirstHero());

            if (q)
              {
                for (Stack::iterator it = stack->begin(); it != stack->end(); it++)
                    if ((*it)->getId() == q->getHeroId())
                        hero = dynamic_cast<Hero*>(*it);

              }
	    quest_assigned.emit(hero, q);
        }
    }
}

// the parameter is currently not used (=0), but may be used for more detailed
// descriptions later on
void Game::stackUpdate(Stack* s)
{
    if (!s)
        s = Playerlist::getActiveplayer()->getActivestack();

    if (s)
	bigmap->center_view(s->getPos());

    redraw();

    update_stack_info();
    update_control_panel();
}

// s is currently unused, but can later be filled with reasonable data
void Game::stackDied(Stack* s)
{
    bigmap->unselect_active_stack();
    smallmap->draw();
    update_control_panel();
}


Army::Stat Game::newLevelArmy(Army* a)
{
    // don't show a dialog if computer or enemy's armies advance
    if ((a->getPlayer()->getType() != Player::HUMAN) ||
        (a->getPlayer() != Playerlist::getInstance()->getActiveplayer()))
        return Army::STRENGTH;

    return army_gains_level.emit(a);
}

void Game::newMedalArmy(Army* a)
{
    // We don't want to have medal awards of computer players displayed
    if (!a->getPlayer()
	|| (a->getPlayer()->getType() != Player::HUMAN)
	|| a->getPlayer() != Playerlist::getInstance()->getActiveplayer())
        return;

    medal_awarded_to_army.emit(a);
    update_stack_info();
}

void Game::on_stack_selected(Stack* s)
{
    update_stack_info();
    update_control_panel();
}

void Game::on_city_queried (City* c, bool brief)
{
    if (c)
    {
	Player *player = c->getPlayer();
	
	if (brief)
	{
	    Glib::ustring str;

	    if (c->isCapital())
		str = String::ucompose(_("%1 (capital city)"), c->getName());
	    else
		str = c->getName();
	    str += "\n";
	    if (player != Playerlist::getInstance()->getNeutral())
	    {
		str += String::ucompose(_("Under rule of %1"), player->getName());
		str += "\n";
	    }
	    str += String::ucompose(_("Income: %1"), c->getGold());
	    str += "\n";
	    str += String::ucompose(_("Defense: %1"), c->getDefenseLevel());
	    if (c->isBurnt())
	    {
		str += "\n";
		str += _("Status: razed!");
	    }

	    MapTipPosition mpos = bigmap->map_tip_position(c->get_area());
	    map_tip_changed.emit(str, mpos);
	}
	else
	{
	    city_visited.emit(c);

	    // some visible city properties (razed) may have changed
	    redraw();
	}
    }
    else
	map_tip_changed.emit("", MapTipPosition());
}

void Game::on_ruin_queried (Ruin* r)
{
    if (r)
    {
	Glib::ustring str;

	str = r->getName();
	str += "\n";
	if (r->isSearched())
	    // note to translators: whether a ruin has been searched
	    str += _("Explored");
	else
	    // note to translators: whether a ruin has been searched
	    str += _("Unexplored");
	    
	MapTipPosition mpos = bigmap->map_tip_position(r->get_area());
	map_tip_changed.emit(str, mpos);
    }
    else
	map_tip_changed.emit("", MapTipPosition());
}

void Game::on_signpost_queried (Signpost* s)
{
    if (s)
    {
	Glib::ustring str;

	str = s->getName();
	    
	MapTipPosition mpos = bigmap->map_tip_position(s->get_area());
	map_tip_changed.emit(str, mpos);
    }
    else
	map_tip_changed.emit("", MapTipPosition());
}

void Game::on_stack_queried (Stack* s)
{
    if (s)
    {
        Rectangle r = s->get_area();
        //r.w *= s->size();
	MapTipPosition mpos = bigmap->map_tip_position(r);
	stack_tip_changed.emit(s, mpos);
    }
    else
	stack_tip_changed.emit(NULL, MapTipPosition());
}

void Game::on_temple_queried (Temple* t)
{
    if (t)
    {
	Glib::ustring str;

	str = t->getName();
	    
	MapTipPosition mpos = bigmap->map_tip_position(t->get_area());
	map_tip_changed.emit(str, mpos);
    }
    else
	map_tip_changed.emit("", MapTipPosition());
}

void Game::looting_city(City* city, int &gold)
{
    Citylist *clist = Citylist::getInstance();
    Playerlist *plist = Playerlist::getInstance();
    Player *attacker = plist->getActiveplayer();
    Player *defender = city->getPlayer();
    int amt = (defender->getGold() / (2 * clist->countCities (defender)) * 2);
    // give (Enemy-Gold/(2Enemy-Cities)) to the attacker 
    // and then take away twice that from the defender.
    // the idea here is that some money is taken in the invasion
    // and other monies are lost forever
    defender->withdrawGold (amt);
    amt /= 2;
    attacker->addGold (amt);
    gold = amt;
    return;
}

void Game::invading_city(City* city)
{
    Playerlist *plist = Playerlist::getInstance();
    Player *player = plist->getActiveplayer();
    int gold = 0;
    // if a computer makes it's turn and occupied a city, we shouldn't
    // show a modal dialog :)

    // loot the city
    // if the attacked city isn't neutral, loot some gold
    if (city->getPlayer() != plist->getNeutral())
      looting_city (city, gold);

    if (!input_locked)
    {
        redraw();
	CityDefeatedAction a = city_defeated.emit(city, gold);
        gold = 0;

	switch (a) {
	case CITY_DEFEATED_OCCUPY:
	    player->cityOccupy(city);
	    break;
	    
	case CITY_DEFEATED_RAZE:
	    player->cityRaze(city);
            city_razed.emit (city);
	    break;
	    
	case CITY_DEFEATED_PILLAGE:
            int pillaged_army_type;
	    player->cityPillage(city, gold, pillaged_army_type);
	    city_pillaged.emit(city, gold, pillaged_army_type);
	    break;

	case CITY_DEFEATED_SACK:
            std::list<Uint32> sacked_types;
	    player->citySack(city, gold, &sacked_types);
	    city_sacked.emit(city, gold, sacked_types);
	    break;
	}
	
	if (!city->isBurnt())
	    city_visited.emit(city);
    }
   
    Playerlist::getInstance()->checkPlayers();
    redraw();
    update_stack_info();
    update_sidebar_stats();
    update_control_panel();
}

void Game::lock_inputs()
{
    // don't accept modifying user input from now on
    bigmap->set_input_locked(true);
    smallmap->set_input_locked(true);
    input_locked = true;
    update_control_panel();
}

void Game::unlock_inputs()
{
    bigmap->set_input_locked(false);
    smallmap->set_input_locked(false);
    input_locked = false;
    update_control_panel();
}

void Game::update_control_panel()
{
    if (input_locked)
    {
	can_select_prev_stack.emit(false);
	can_select_next_stack.emit(false);
	can_select_next_movable_stack.emit(false);
	can_center_selected_stack.emit(false);
	can_defend_selected_stack.emit(false);
	can_search_selected_stack.emit(false);
	can_move_selected_stack.emit(false);
	can_move_all_stacks.emit(false);
	can_end_turn.emit(false);
	can_disband_stack.emit(false);
	
        return;
    }
    
    Player *player = Playerlist::getActiveplayer();
    Stacklist* sl = player->getStacklist();
    
    bool all_defending = true;
    for (Stacklist::iterator i = sl->begin(); i != sl->end(); ++i)
        if (!(*i)->getDefending() && *i != sl->getActivestack())
        {
            all_defending = false;
            break;
        }
    
    can_select_prev_stack.emit(!all_defending);
    can_select_next_stack.emit(!all_defending);

    bool all_immobile = true;
    for (Stacklist::iterator i = sl->begin(); i != sl->end(); ++i)
        if (!(*i)->getDefending() && (*i)->canMove()
	    && *i != sl->getActivestack())
	{
	    all_immobile = false;
	    break;
	}
    can_select_next_movable_stack.emit(!all_defending && !all_immobile);

    // if any stack can move, enable the moveall button
    can_move_all_stacks.emit(sl->enoughMoves());

    Stack *stack = player->getActivestack();

    can_defend_selected_stack.emit(stack != 0);
    can_center_selected_stack.emit(stack != 0);
    
    if (stack)
    {
	can_move_selected_stack.emit(stack->getPath()->size() > 0
				     && stack->enoughMoves());
	
        if (stack->getGroupMoves() > 0)
        {
            Temple *temple;
            temple = Templelist::getInstance()->getObjectAt(stack->getPos());
            if (stack->hasHero())
            {
                Ruin *ruin
		    = Ruinlist::getInstance()->getObjectAt(stack->getPos());

	        can_search_selected_stack.emit(
		    (ruin && !ruin->isSearched()) || temple);
            }
            else
            {
	        can_search_selected_stack.emit(temple);
            }
        }
	can_disband_stack.emit(true);
    }
    else
    {
	can_search_selected_stack.emit(false);
	can_move_selected_stack.emit(false);
	can_disband_stack.emit(false);
    }

    can_end_turn.emit(true);
}

GameBigMap &Game::get_bigmap()
{
    assert(bigmap.get());
    return *bigmap.get();
}

SmallMap &Game::get_smallmap()
{
    assert(smallmap.get());
    return *smallmap.get();
}

void Game::startGame()
{

    debug ("start_game()");
    lock_inputs();

    d_nextTurn->start();
    update_control_panel();
}

void Game::loadGame()
{
    Player *player = Playerlist::getActiveplayer();
    if (player->getType() == Player::HUMAN)
    {
        //human players want access to the controls and an info box
        unlock_inputs();
        stack_info_changed.emit(player->getActivestack());
        center_view_on_city();
        update_sidebar_stats();
        update_control_panel();

	game_loaded.emit(player);
    }       
    else
    {
        lock_inputs();
        update_sidebar_stats();
        player->startTurn();
        d_nextTurn->endTurn();
        update_control_panel();
    }
}

void Game::stopGame()
{
    d_nextTurn->stop();
    Playerlist::finish();
}

bool Game::saveGame(std::string file)
{
    return d_gameScenario->saveGame(file);
}


/*
 *
 * what are the chances of a hero showing up?
 *
 * 1 in 6 if you have enough gold, where "enough gold" is...
 *
 * ... 1500 if the player already has a hero, then:  1500 is generally 
 * enough to buy all the heroes.  I forget the exact distribution of 
 * hero prices but memory says from 1000 to 1500.  (But, if you don't 
 * have 1500 gold, and the price is less, you still get the offer...  
 * So, calculate price, compare to available gold, then decided whether 
 * or not to offer...)
 *
 * ...500 if all your heroes are dead: then prices are cut by about 
 * a factor of 3.
 */
void Game::maybeRecruitHero (Player *p)
{
  City *city;
  Playerlist *plist = Playerlist::getInstance();
  int gold_needed = 0;
  //give the player a hero if it's the first round.
  //otherwise we get a hero based on chance
  //a hero costs a random number of gold pieces
  if (d_gameScenario->getRound() == 0)
    gold_needed = 0;
  else
    {
      bool exists = false;
      Stacklist *stacklist = p->getStacklist();
      for (Stacklist::iterator it = stacklist->begin(); 
           it != stacklist->end(); it++)
        if ((*it)->hasHero())
            exists = true; 

      gold_needed = (rand() % 500) + 1000;
      if (exists == false)
        gold_needed /= 3;
    }
    
  //we set the chance of some hero recruitment to, ehm, 10 percent
  if (((((rand() % 6) == 0) && (gold_needed < p->getGold())) 
       || gold_needed == 0)
      && (p != plist->getNeutral()))
    {
      int num = rand() % d_herotemplates[p->getId()].size();
      Hero *templateHero = d_herotemplates[p->getId()][num];
      Hero* newhero = new Hero(*templateHero);
      if (gold_needed == 0)
        city = Citylist::getInstance()->getFirstCity(p);
      else
        {
          std::vector<City*> cities;
          Citylist* cl = Citylist::getInstance();
          for (Citylist::iterator it = cl->begin(); it != cl->end(); ++it)
            if (!(*it).isBurnt() && (*it).getPlayer() == p)
              cities.push_back(&(*it));
          if (cities.empty())
            return;
          city = cities[rand() % cities.size()];
        }

      bool accepted = p->recruitHero(newhero, city, gold_needed);
      if (accepted)
        {
	  newhero->setPlayer(p);

          int alliesCount;
          GameMap::getInstance()->addArmy(city, newhero);
          /* now maybe add a few allies */
          if (gold_needed > 1300)
            alliesCount = 3;
          else if (gold_needed > 1000)
            alliesCount = 2;
          else if (gold_needed > 800)
            alliesCount = 1;
          else
            alliesCount = 0;

          if (alliesCount > 0)
            {
              const Army *army = Reward_Allies::randomArmyAlly();
              if (army)
                {
                  Reward_Allies::addAllies(p, city->getPos(), army,alliesCount);
                  if (p->getType() == Player::HUMAN)
                    hero_arrives.emit(alliesCount);
                }
            }
          if (gold_needed == 0)
            {
              std::string name = p->getName() + " " + _("Standard");
              Item *battle_standard = new Item (name, true, p);
              battle_standard->setBonus(Item::ADD1STACK);
              newhero->addToBackpack(battle_standard, 0);
            }
          p->withdrawGold(gold_needed);
          p->supdatingStack.emit(0);
        }
       else
        delete newhero;
    }
  return;
}

int
Game::loadHeroTemplates()
{
  FILE *fileptr = fopen (File::getMiscFile("heronames").c_str(), "r");
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int retval;
  int gender;
  int side;
  size_t bytesread;
  char *tmp;
  const Armysetlist* al = Armysetlist::getInstance();
  const Army* herotype;

  // list all the army types that are heroes.
  std::vector<const Army*> heroes;
  Player *p = Playerlist::getInstance()->getNeutral();
  for (unsigned int j = 0; j < al->getSize(p->getArmyset()); j++)
    {
      const Army *a = al->getArmy (p->getArmyset(), j);
      if (a->isHero())
        heroes.push_back(a);
    }

  if (fileptr == NULL)
    return -1;
  while ((read = getline (&line, &len, fileptr)) != -1)
    {
      retval = sscanf (line, "%d%d%n", &side, &gender, &bytesread);
      if (retval != 2)
        {
          free (line);
          return -2;
        }
      while (isspace(line[bytesread]) && line[bytesread] != '\0')
        bytesread++;
      tmp = strchr (&line[bytesread], '\n');
      if (tmp)
        tmp[0] = '\0';
      if (strlen (&line[bytesread]) == 0)
        {
          free (line);
          return -3;
        }
      if (side < 0 || side > (int) MAX_PLAYERS)
        {
          free (line);
          return -4;
        }

      herotype = heroes[rand() % heroes.size()];
      Hero *newhero = new Hero (*herotype, "", NULL);
      if (gender)
        newhero->setGender(Hero::MALE);
      else
        newhero->setGender(Hero::FEMALE);
      newhero->setName (&line[bytesread]);
      d_herotemplates[side].push_back (newhero);
    }
  if (line)
    free (line);
  fclose (fileptr);
  return 0;
}

bool Game::init_turn_for_player(Player* p)
{
    // FIXME: Currently this function only checks for a human player. You
    // can also have it check for e.g. escape key pressed to interrupt
    // an AI-only game to save/quit.


    next_player_turn.emit(p, d_gameScenario->getRound() + 1);
    if (p->getType() == Player::HUMAN)
    {
	unlock_inputs();
    
        Stack* stack = p->getActivestack();
	if (stack != NULL)
	    bigmap->center_view(stack->getPos());
	else
	    center_view_on_city();

	update_sidebar_stats();
	update_stack_info();
	update_control_panel();
    

        maybeRecruitHero(p);
    
        if (d_gameScenario->getRound() == 0)
          {
            Citylist *clist = Citylist::getInstance();
	    city_visited.emit(clist->getFirstCity(p));
          }


        return true;
    }
    else
    {
	center_view_on_city();
	SDL_Delay(250);
        Game::maybeRecruitHero(p);
	return false;
    }
}

void Game::on_player_died(Player *player)
{
    const Playerlist* pl = Playerlist::getInstance();
    if (pl->getNoOfPlayers() <= 1)
	game_over.emit(pl->getFirstLiving());
    else if (player->getType() == Player::HUMAN)
	player_died.emit(player);
}

void Game::on_fight_started(Fight &fight)
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN &&
      GameScenario::s_hidden_map == true)
    return;
  fight_started.emit(fight);
}

void Game::center_view_on_city()
{
    const Player* p = Playerlist::getInstance()->getActiveplayer();
    
    // preferred city is a capital city that belongs to the player 
    for (Citylist::iterator i = Citylist::getInstance()->begin();
            i != Citylist::getInstance()->end(); i++)
    {
        if (i->getPlayer() == p && i->isCapital())
        {
	    bigmap->center_view(i->getPos());
            return;
        }
    }

    // okay, then find any city that belongs to the player and center on it
    for (Citylist::iterator i = Citylist::getInstance()->begin();
        i != Citylist::getInstance()->end(); i++)
    {
        if (i->getPlayer() == p)
        {
	    bigmap->center_view(i->getPos());
            break;
        }
    }
}
