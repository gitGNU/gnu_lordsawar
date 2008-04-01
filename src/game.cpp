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
#include "fight.h"
#include "hero.h"
#include "stacklist.h"
#include "citylist.h"
#include "ruinlist.h"
#include "templelist.h"
#include "signpostlist.h"
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
#include "GameMap.h"
#include "history.h"

#include "herotemplates.h"

Game *Game::current_game = 0;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
#define debug(x)
void Game::addPlayer(Player *p)
{

  //disconnect prior players' connections
  for (std::list<sigc::connection>::iterator it = 
       connections[p->getId()].begin(); 
       it != connections[p->getId()].end(); it++) 
    (*it).disconnect();
  connections[p->getId()].clear();

  if (p->getType() == Player::HUMAN)
    {
      connections[p->getId()].push_back
	(p->snewLevelArmy.connect(sigc::mem_fun(this, &Game::newLevelArmy)));
      connections[p->getId()].push_back
	(p->snewMedalArmy.connect(sigc::mem_fun(this, &Game::newMedalArmy)));
      connections[p->getId()].push_back
	(p->srecruitingHero.connect(sigc::mem_fun(this, &Game::recruitHero)));

      connections[p->getId()].push_back
	(p->streachery.connect
	 (sigc::bind<0>
	  (sigc::mem_fun
	   (stack_considers_treachery, 
	    &sigc::signal<bool, Player *, Stack *, Player *, Vector<int> >::emit), p)));
      connections[p->getId()].push_back
	(p->hero_arrives_with_allies.connect
         (sigc::mem_fun
          (hero_arrives, &sigc::signal<void, int>::emit)));
    }
	
      
  connections[p->getId()].push_back
    (p->schangingStatus.connect 
     (sigc::mem_fun(this, &Game::update_sidebar_stats)));
        
  connections[p->getId()].push_back
    (p->supdatingStack.connect (sigc::mem_fun(this, &Game::stackUpdate)));
  connections[p->getId()].push_back
    (p->sinvadingCity.connect(sigc::mem_fun(this, &Game::invading_city)));
  connections[p->getId()].push_back
    (p->streacheryStack.connect(sigc::mem_fun(this, &Game::maybeTreachery)));
  connections[p->getId()].push_back
    (p->fight_started.connect (sigc::mem_fun(*this, &Game::on_fight_started)));
  connections[p->getId()].push_back
    (p->ruinfight_started.connect
     (sigc::mem_fun
      (ruinfight_started, &sigc::signal<void, Stack *, Stack *>::emit)));
  connections[p->getId()].push_back
    (p->ruinfight_finished.connect
     (sigc::mem_fun
      (ruinfight_finished, &sigc::signal<void, Fight::Result>::emit)));
  connections[p->getId()].push_back
    (p->cityfight_finished.connect (sigc::mem_fun(*this, &Game::on_city_fight_finished))); 
  connections[p->getId()].push_back
    (p->advice_asked.connect
     (sigc::mem_fun(advice_asked, &sigc::signal<void, float>::emit)));
}

#define NETWORK_TESTING 1

#include "game-server.h"

GameServer *game_server = 0;

Game::Game(GameScenario* gameScenario)
    : d_gameScenario(gameScenario) 
{
    current_game = this;
    input_locked = false;

#if NETWORK_TESTING
    game_server = new GameServer();
    game_server->start();
#endif
    
    // init the bigmap
    bigmap.reset(new GameBigMap
		 (GameScenario::s_intense_combat, 
		  GameScenario::s_see_opponents_production, 
		  GameScenario::s_see_opponents_stacks, 
		  GameScenario::s_military_advisor));
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
    smallmap->resize();
    smallmap->map_changed.connect(
	sigc::mem_fun(smallmap_changed,
		      &sigc::signal<void, SDL_Surface *>::emit));

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
	addPlayer(p);
    }
    pl->splayerDead.connect(sigc::mem_fun(this, &Game::on_player_died));
    pl->ssurrender.connect(sigc::mem_fun(this, &Game::on_surrender_offered));

    //set up a NextTurn object
    d_nextTurn = new NextTurn(d_gameScenario->getTurnmode(),
			      d_gameScenario->s_random_turns);
    d_nextTurn->splayerStart.connect(
	sigc::mem_fun(this, &Game::init_turn_for_player));
    d_nextTurn->snextRound.connect(
	sigc::mem_fun(d_gameScenario, &GameScenario::nextRound));
    d_nextTurn->snextRound.connect(
	sigc::mem_fun(this, &Game::nextRound));
    d_nextTurn->supdating.connect(
	sigc::mem_fun(this, &Game::redraw));
            
    center_view_on_city();
    update_control_panel();

    HeroTemplates::getInstance();
}

Game::~Game()
{
  for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
    {
      for (std::list<sigc::connection>::iterator it = connections[i].begin(); 
	   it != connections[i].end(); it++) 
	(*it).disconnect();
      connections[i].clear();
    }
    delete d_gameScenario;
    delete d_nextTurn;
    
    HeroTemplates::deleteInstance();
}

GameScenario *Game::getScenario()
{
  return current_game->d_gameScenario;
}


void Game::end_turn()
{
    unselect_active_stack();
    clear_stack_info();
    update_control_panel();
    lock_inputs();

    d_nextTurn->endTurn();
}

void Game::update_stack_info()
{
    Stack* stack = Playerlist::getActiveplayer()->getActivestack();

    //if (Playerlist::getActiveplayer()->getType() != Player::HUMAN &&
        //GameScenario::s_hidden_map == true)
      //return;
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
    if (player == Playerlist::getInstance()->getNeutral())
      return;

    s.name = player->getName();
    s.gold = player->getGold();
    s.income = s.cities = 0;
    for (Citylist::iterator i = Citylist::getInstance()->begin(),
	     end = Citylist::getInstance()->end(); i != end; ++i)
	if (i->getOwner() == player)
	{
	    s.income += i->getGold();
	    ++s.cities;
	}

    s.units = 0;
    s.upkeep = player->getUpkeep();
    Stacklist *sl = player->getStacklist();
    for (Stacklist::iterator i = sl->begin(), iend = sl->end(); i != iend; ++i)
      s.units += (*i)->size();
    
    s.turns = d_gameScenario->getRound();
    
    sidebar_stats_changed.emit(s);
}

void Game::redraw()
{
    if (bigmap.get())
      {
	bigmap->draw();
      }
    if (smallmap.get())
      {
	//if (Playerlist::getActiveplayer()->getType() == Player::HUMAN ||
	    //GameScenario::s_hidden_map == false)
	  smallmap->draw();
      }
}

void Game::select_next_movable_stack()
{
  Stacklist *sl = Playerlist::getActiveplayer()->getStacklist();
  Stack* stack = sl->getNextMovable();
  sl->setActivestack(stack);
  select_active_stack();
}

void Game::move_selected_stack_dir(int diffx, int diffy)
{
  Stack *stack = Playerlist::getActiveplayer()->getActivestack();
  if (!stack)
    return;
  // Get rid of the old path if there is one
  if (stack->getPath()->size())
    stack->getPath()->flClear();
  //See if we can move there
  Vector<int> dest = stack->getPos();
  dest.x += diffx;
  dest.y += diffy;
  if (stack->getPath()->canMoveThere(stack, dest))
    {
      // Set in a new path
      stack->getPath()->calculate(stack, dest);
      move_selected_stack_along_path();
    }
  else
    {
      Playerlist::getActiveplayer()->getStacklist()->setActivestack(0);
      unselect_active_stack();
    }
}

void Game::move_selected_stack_northwest()
{
  move_selected_stack_dir(-1, -1);
}

void Game::move_selected_stack_north()
{
  move_selected_stack_dir(0, -1);
}

void Game::move_selected_stack_northeast()
{
  move_selected_stack_dir(1, -1);
}

void Game::move_selected_stack_east()
{
  move_selected_stack_dir(1, 0);
}

void Game::move_selected_stack_west()
{
  move_selected_stack_dir(-1, 0);
}

void Game::move_selected_stack_southwest()
{
  move_selected_stack_dir(-1, 1);
}

void Game::move_selected_stack_south()
{
  move_selected_stack_dir(0, 1);
}

void Game::move_selected_stack_southeast()
{
  move_selected_stack_dir(1, 1);
}

void Game::move_selected_stack_along_path()
{
  Stack *stack = Playerlist::getActiveplayer()->getActivestack();
  if (stack->isGrouped() == false)
    Playerlist::getActiveplayer()->stackSplit(stack);

  Playerlist::getActiveplayer()->stackMove(stack);

  //maybe we joined another stack
  stack = Playerlist::getActiveplayer()->getActivestack();
  if (stack && stack->canMove() == false)
    {
      Playerlist::getActiveplayer()->getStacklist()->setActivestack(0);
      unselect_active_stack();
    }
}

void Game::move_all_stacks()
{
  Player *player = Playerlist::getActiveplayer();
  Stacklist* sl = player->getStacklist();

  for (Stacklist::iterator i = sl->begin(), end = sl->end(); i != end; ++i)
    {
      Stack &s = **i;
      if (!(s.empty()) && !(s.getPath()->empty()) && s.enoughMoves())
	{
	  sl->setActivestack(&s);
	  select_active_stack();
	  if (player->getActivestack()->isGrouped() == false)
	    player->stackSplit(player->getActivestack());
	  player->stackMove(player->getActivestack());
	  i = sl->begin();
	}
    }

  if (sl->getActivestack()->canMove() == false)
    {
      Playerlist::getActiveplayer()->getStacklist()->setActivestack(0);
      unselect_active_stack();
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
    select_active_stack();
  else
    unselect_active_stack();
}

void Game::park_selected_stack()
{
  Player *player = Playerlist::getActiveplayer();
  Stack *stack = player->getActivestack();
  assert(stack);
  stack->setParked(true);

  stack = player->getStacklist()->getNextMovable();
  player->getStacklist()->setActivestack(stack);

  if (stack)
    select_active_stack();
  else
    unselect_active_stack();
}

void Game::deselect_selected_stack()
{
  Player *player = Playerlist::getActiveplayer();
  player->getStacklist()->setActivestack(0);
  unselect_active_stack();
}

void Game::center_selected_stack()
{
  Stack *stack = Playerlist::getActiveplayer()->getActivestack();
  if (stack) 
    select_active_stack();
}

void Game::search_selected_stack()
{
  Player *player = Playerlist::getActiveplayer();
  Stack* stack = player->getActivestack();

  Ruin* ruin = Ruinlist::getInstance()->getObjectAt(stack->getPos());
  Temple* temple = Templelist::getInstance()->getObjectAt(stack->getPos());

  if (ruin && !ruin->isSearched() && stack->hasHero() &&
      stack->getFirstHero()->getMoves() > 0 &&
      ((ruin->isHidden() == true && ruin->getOwner() == player) ||
       ruin->isHidden() == false))
    {
      Reward *reward;

      reward = player->stackSearchRuin(stack, ruin);
      if (ruin->hasSage() == true)
	{
	  sage_visited.emit(ruin, stack);
	  reward = ruin->getReward();
	}
	  
      if (reward)
	{
	  player->giveReward(Playerlist::getActiveplayer()->getActivestack(),
			     reward);
	  //FIXME: delete this reward, but don't delete the item, or map
	  redraw();
	  update_stack_info();
	  update_control_panel();
	  ruin_searched.emit(ruin, stack, reward);
	}
      else
	{
	  redraw();
	  update_stack_info();
	  update_control_panel();
	}



      update_sidebar_stats();
    }
  else if (temple && temple->searchable() && stack->getGroupMoves() > 0)
    {
      int blessCount;
      blessCount = player->stackVisitTemple(stack, temple);
      bool wants_quest = temple_searched.emit(stack->hasHero(), temple, blessCount);
      if (wants_quest)
	{
	  Quest *q = player->stackGetQuest
	    (stack, temple, 
	     GameScenario::s_razing_cities != GameParameters::NEVER);
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

void Game::stackUpdate(Stack* s)
{
  if (!s)
    s = Playerlist::getActiveplayer()->getActivestack();

  //FIXME: if player is not to be observed, bail now
  if (s)
    smallmap->center_view_on_tile(s->getPos(), true);

  redraw();

  update_stack_info();
  update_control_panel();

  // sleep for a specified amount of time
  SDL_Delay(Configuration::s_displaySpeedDelay);
}

Army::Stat Game::newLevelArmy(Army* a)
{
  // don't show a dialog if computer or enemy's armies advance
  if ((a->getOwner()->getType() != Player::HUMAN) ||
      (a->getOwner() != Playerlist::getInstance()->getActiveplayer()))
    return Army::STRENGTH;

  return army_gains_level.emit(a);
}

void Game::newMedalArmy(Army* a)
{
  // We don't want to have medal awards of computer players displayed
  if (!a->getOwner()
      || (a->getOwner()->getType() != Player::HUMAN)
      || a->getOwner() != Playerlist::getInstance()->getActiveplayer())
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
      Player *player = c->getOwner();

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

void Game::on_ruin_queried (Ruin* r, bool brief)
{
  if (r)
    {
      if (brief)
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
	{
	  ruin_visited.emit(r);
	}
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
      MapTipPosition mpos = bigmap->map_tip_position(s->getPos());
      stack_tip_changed.emit(s, mpos);
    }
  else
    stack_tip_changed.emit(NULL, MapTipPosition());
}

void Game::on_temple_queried (Temple* t, bool brief)
{
  if (t)
    {
      if (brief)
	{
	  Glib::ustring str;

	  str = t->getName();

	  MapTipPosition mpos = bigmap->map_tip_position(t->get_area());
	  map_tip_changed.emit(str, mpos);
	}
      else
	{
	  temple_visited.emit(t);
	}
    }
  else
    map_tip_changed.emit("", MapTipPosition());
}

void Game::looting_city(City* city, int &gold)
{
  Citylist *clist = Citylist::getInstance();
  Playerlist *plist = Playerlist::getInstance();
  Player *attacker = plist->getActiveplayer();
  Player *defender = city->getOwner();
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

void Game::invading_city(City* city, int gold)
{
  Player *player = Playerlist::getInstance()->getActiveplayer();
  
  if (player->getType() == Player::HUMAN)
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
	player->deteriorateDiplomaticRelationship (5);
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
      can_select_next_movable_stack.emit(false);
      can_center_selected_stack.emit(false);
      can_defend_selected_stack.emit(false);
      can_park_selected_stack.emit(false);
      can_deselect_selected_stack.emit(false);
      can_search_selected_stack.emit(false);
      can_inspect_selected_stack.emit(false);
      can_plant_standard_selected_stack.emit(false);
      can_move_selected_stack.emit(false);
      can_move_selected_stack_along_path.emit(false);
      can_move_all_stacks.emit(false);
      can_group_ungroup_selected_stack.emit(false);
      can_end_turn.emit(false);
      can_disband_stack.emit(false);
      can_change_signpost.emit(false);
      can_see_history.emit(false);

      return;
    }

  Player *player = Playerlist::getActiveplayer();
  Stacklist* sl = player->getStacklist();

  bool all_defending_or_parked = true;
  for (Stacklist::iterator i = sl->begin(); i != sl->end(); ++i)
    if (!(*i)->getDefending() && !(*i)->getParked() 
	&& *i != sl->getActivestack())
      {
	all_defending_or_parked = false;
	break;
      }

  bool all_immobile = true;
  for (Stacklist::iterator i = sl->begin(); i != sl->end(); ++i)
    if (!(*i)->getDefending() && !(*i)->getParked() && (*i)->canMove()
	&& *i != sl->getActivestack())
      {
	all_immobile = false;
	break;
      }
  can_select_next_movable_stack.emit(!all_defending_or_parked && !all_immobile);

  // if any stack can move, enable the moveall button
  can_move_all_stacks.emit(sl->enoughMoves());

  Stack *stack = player->getActivestack();

  can_defend_selected_stack.emit(stack != 0);
  can_park_selected_stack.emit(stack != 0);
  can_deselect_selected_stack.emit(stack != 0);
  can_center_selected_stack.emit(stack != 0);

  if (stack)
    {
      can_move_selected_stack_along_path.emit
	(!stack->getPath()->empty() && stack->enoughMoves() ||
	 (!stack->getPath()->empty() && stack->getPath()->getMovesExhaustedAtPoint() > 0));

      /*
       * a note about searching.
       * ruins can be searched by stacks that have a hero, and when the
       * hero has moves left.  also the ruin must be unexplored.
       * temples can be searched by any stack, when the stack has 
       * movement left.
       */
      if (stack->getGroupMoves() > 0)
	{
	  Temple *temple;
	  temple = Templelist::getInstance()->getObjectAt(stack->getPos());
	  can_search_selected_stack.emit(temple);
	  can_move_selected_stack.emit(true);
	}

      if (stack->hasHero())
	{
	  Ruin *ruin = Ruinlist::getInstance()->getObjectAt(stack->getPos());
	  if (stack->getFirstHero()->getMoves() > 0 && ruin)
	    can_search_selected_stack.emit(!ruin->isSearched());

	  can_inspect_selected_stack.emit(true);
	  //does the hero have the player's standard?
	  for (Stack::iterator it = stack->begin(); it != stack->end(); it++)
	    {
	      if ((*it)->isHero())
		{
		  Hero *hero = dynamic_cast<Hero*>((*it));
		  std::list<Item*> backpack = hero->getBackpack();
		  for (std::list<Item*>::iterator i = backpack.begin(), 
		       end = backpack.end(); i != end; ++i)
		    {
		      if ((*i)->isPlantable() && 
			  (*i)->getPlantableOwner() == player)
			{
			  //can't plant on city/ruin/temple/signpost
			  Citylist *cl = Citylist::getInstance();
			  City *city = cl->getObjectAt(stack->getPos());
			  Templelist *tl = Templelist::getInstance();
			  Temple *temple = tl->getObjectAt(stack->getPos());
			  Ruinlist *rl = Ruinlist::getInstance();
			  Ruin *ruin = rl->getObjectAt(stack->getPos());
			  Signpostlist *spl = Signpostlist::getInstance();
			  Signpost *sign = spl->getObjectAt(stack->getPos());
			  if (!city && !temple && !ruin && !sign)
			    {
			      GameMap *gm = GameMap::getInstance();
			      std::list<Item*> items;
			      Vector<int> pos = stack->getPos();
			      items = gm->getTile(pos)->getItems();
			      std::list<Item*>::iterator iit;
			      bool standard_already_planted = false;
			      for (iit = items.begin(); iit != items.end();
				   iit++)
				{
				  if ((*iit)->getPlanted())
				    standard_already_planted = true;
				}
			      //are there any other standards here?
			      if (standard_already_planted == false)
				can_plant_standard_selected_stack.emit(true);
			    }
			}
		    }
		}
	    }
	}
      else
	{
	  can_inspect_selected_stack.emit(false);
	  can_plant_standard_selected_stack.emit(false);
	}

      if (Signpostlist::getInstance()->getObjectAt(stack->getPos()))
	can_change_signpost.emit(true);

      can_disband_stack.emit(true);
      can_group_ungroup_selected_stack.emit(true);
    }
  else
    {
      can_search_selected_stack.emit(false);
      can_move_selected_stack.emit(false);
      can_move_selected_stack_along_path.emit(false);
      can_disband_stack.emit(false);
      can_group_ungroup_selected_stack.emit(false);
      can_inspect_selected_stack.emit(false);
      can_plant_standard_selected_stack.emit(false);
    }

  if (d_gameScenario->getRound() > 0)
    can_see_history.emit(true);
  else
    can_see_history.emit(false);

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
  if (Playerlist::getInstance()->countPlayersAlive())
    update_control_panel();
}

void Game::loadGame()
{
  Player *player = Playerlist::getActiveplayer();
  if (player->getType() == Player::HUMAN)
    {
      //human players want access to the controls and an info box
      unlock_inputs();
      player->getStacklist()->setActivestack(0);
      center_view_on_city();
      update_sidebar_stats();
      update_control_panel();
      update_stack_info();
      game_loaded.emit(player);
    }

  d_nextTurn->setContinuingTurn();
  d_nextTurn->start();
#if 0
  else
    {
      lock_inputs();
      update_sidebar_stats();
      player->startTurn();
      d_nextTurn->endTurn();
      if (Playerlist::getInstance()->countPlayersAlive())
	update_control_panel();
    }
#endif
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

void Game::init_turn_for_player(Player* p)
{
  Playerlist* pl = Playerlist::getInstance();

  if (GameScenario::s_hidden_map && p->getType() == Player::HUMAN)
    {
      smallmap->blank();
      bigmap->blank();
    }
  next_player_turn.emit(p, d_gameScenario->getRound() + 1);
  center_view_on_city();
  if (p->getType() == Player::HUMAN)
    {
      unlock_inputs();

      update_sidebar_stats();
      update_stack_info();
      update_control_panel();

      //if (d_gameScenario->getRound() == 0)
	//{
	  //Citylist *clist = Citylist::getInstance();
	  //city_visited.emit(clist->getFirstCity(p));
	//}

      // update the diplomacy icon if we've received a proposal
      bool proposal_received = false;
      for (Playerlist::iterator it = pl->begin(); it != pl->end(); ++it)
	{
	  if ((*it) == pl->getNeutral())
	    continue;
	  if ((*it) == p)
	    continue;
	  if((*it)->isDead())
	    continue;
	  if ((*it)->getDiplomaticProposal(p) != Player::NO_PROPOSAL)
	    {
	      proposal_received = true;
	      break;
	    }
	}
      received_diplomatic_proposal.emit(proposal_received);
    }
  else
    {
      SDL_Delay(250);
    }
}

void Game::on_player_died(Player *player)
{
  const Playerlist* pl = Playerlist::getInstance();
  if (pl->getNoOfPlayers() <= 1)
    game_over.emit(pl->getFirstLiving());
  else
    player_died.emit(player);
}

void Game::on_fight_started(Fight &fight)
{
#if 0
  Player* pd = fight.getDefenders().front()->getOwner();
  Player* pa = fight.getAttackers().front()->getOwner();
  if ((pa->getType() == Player::HUMAN || pd->getType() == Player::HUMAN) ||
      (pa->getType() != Player::HUMAN && pd->getType() != Player::HUMAN 
       && pd != Playerlist::getInstance()->getNeutral()))
  {

    //short circuit the battle sequence
    if (pa->getType() != Player::HUMAN && pd->getType() != Player::HUMAN &&
        GameScenario::s_hidden_map == true)
      return;
  }
  else
    return; //short circuit the battle sequence
#endif
  
  //FIXME: zoom the map here if we're attacking an observable human, 
  //from an unobservable computer player
  bigmap->setFighting(true);
  bigmap->draw();
  fight_started.emit(fight);
  bigmap->setFighting(false);
  bigmap->draw();
}

void Game::center_view_on_city()
{
  const Player* p = Playerlist::getInstance()->getActiveplayer();

  if (p == Playerlist::getInstance()->getNeutral())
    return;
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN &&
      GameScenario::s_hidden_map == true)
    return;
  //FIXME: if player is not to be observed, bail now
  // preferred city is a capital city that belongs to the player 
  for (Citylist::iterator i = Citylist::getInstance()->begin();
       i != Citylist::getInstance()->end(); i++)
    {
      if (i->getOwner() == p && i->isCapital() &&
	  i->getCapitalOwner() == p)
	{
	  smallmap->center_view_on_tile(i->getPos(), 
					!GameScenario::s_hidden_map);
	  return;
	}
    }

  // okay, then find any city that belongs to the player and center on it
  for (Citylist::iterator i = Citylist::getInstance()->begin();
       i != Citylist::getInstance()->end(); i++)
    {
      if (i->getOwner() == p)
	{
	  if (Playerlist::isFinished())
	    return;
	  smallmap->center_view_on_tile(i->getPos(), 
					!GameScenario::s_hidden_map);
	  break;
	}
    }
}
void Game::select_active_stack()
{
  //if (Playerlist::getActiveplayer()->getType() != Player::HUMAN &&
      //GameScenario::s_hidden_map == true)
    //return;
  Player *p = Playerlist::getInstance()->getActiveplayer();
  smallmap->center_view_on_tile (p->getActivestack()->getPos(), true);
  bigmap->select_active_stack();
}
void Game::unselect_active_stack()
{
  bigmap->unselect_active_stack();
}

bool Game::maybeTreachery(Stack *stack, Player *them, Vector<int> pos)
{
  Player *me = stack->getOwner();
  bool treachery = me->treachery (stack, them, pos);
  if (treachery == false)
    return false;
  me->proposeDiplomacy (Player::NO_PROPOSAL, them);
  me->declareDiplomacy (Player::AT_WAR, them);
  them->proposeDiplomacy (Player::NO_PROPOSAL, me);
  them->declareDiplomacy (Player::AT_WAR, me);
  History_DiplomacyTreachery *item = new History_DiplomacyTreachery();
  item->fillData(them);
  me->getHistorylist()->push_back(item);

  me->deteriorateDiplomaticRelationship (5);
  them->improveDiplomaticRelationship (2, me);

  return true;
}

void Game::nextRound()
{
  Playerlist::getInstance()->nextRound
    (GameScenario::s_diplomacy, &GameScenario::s_surrender_already_offered);
}
    
void Game::on_surrender_offered(Player *recipient)
{
  Playerlist *plist = Playerlist::getInstance();
  if (enemy_offers_surrender.emit(plist->countPlayersAlive() - 1))
    {
      surrender_answered.emit(true);
      game_over.emit(recipient);
    }
  else
    surrender_answered.emit(false);
}

void Game::recalculate_moves_for_stack(Stack *s)
{
  if (!s)
    s = Playerlist::getActiveplayer()->getActivestack();
  if (s)
    {
      s->getPath()->recalculate(s);
      redraw();
      update_control_panel();
    }
}
    
void Game::on_city_fight_finished(City *city, Fight::Result result)
{
  if (result != Fight::ATTACKER_WON)
    {
      // we didn't suceed in defeating the defenders
      //if this is a neutral city, and we're playing with 
      //active neutral cities, AND it hasn't already been attacked
      //then it's production gets turned on
      Player *neu = city->getOwner(); //neutral player
      if (GameScenario::s_neutral_cities == GameParameters::ACTIVE &&
	  neu == Playerlist::getInstance()->getNeutral() &&
	  city->getActiveProductionSlot() == -1)
	{
	  //great, then let's turn on the production.
	  //well, we already made a unit, and we want to produce more
	  //of it.
	  Stack *o = neu->getStacklist()->getObjectAt(city->getPos());
	  if (o)
	    {
	      int army_type = o->getStrongestArmy()->getType();
	      for (int i = 0; i < city->getMaxNoOfProductionBases(); i++)
		{
		  if (city->getArmytype(i) == army_type)
		    {
		      // hey, we found the droid we were looking for
		      city->setActiveProductionSlot(i);
		      break;
		    }
		}
	    }
	}
    }
  return;
}
    
bool Game::recruitHero(Hero *hero, City *city, int gold)
{
  bool retval = hero_offers_service (city->getOwner(), hero, city, gold);
  if (d_gameScenario->getRound() == 0)
    city_visited.emit(city);
  return retval;
}
