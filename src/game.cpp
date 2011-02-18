// Copyright (C) 2006, 2007, 2008, 2009, 2010 Ben Asselstine
// Copyright (C) 2007, 2008 Ole Laursen
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
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

#include "config.h"

#include <algorithm>
#include <vector>
#include <assert.h>
#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>
#include <sigc++/adaptors/bind.h>

#include "game.h"

#include "ucompose.hpp"
#include "rectangle.h"
#include "sound.h"
#include "GraphicsCache.h"
#include "GameScenario.h"
#include "NextTurnNetworked.h"
#include "NextTurnHotseat.h"
#include "NextTurnPbm.h"

#include "gamebigmap.h"
#include "smallmap.h"

#include "army.h"
#include "fight.h"
#include "hero.h"
#include "heroproto.h"
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
#include "action.h"
#include "game-parameters.h"
#include "FogMap.h"
#include "GameMap.h"
#include "history.h"
#include "pbm-game-server.h"
#include "LocationBox.h"
#include "Backpack.h"
#include "MapBackpack.h"
#include "stacktile.h"

#include "herotemplates.h"
#include "GameScenarioOptions.h"
#include "ai_fast.h"
#include "ai_smart.h"
#include "Sage.h"
#include "Commentator.h"

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

  //now setup the connections that are specific for human players
  if (p->getType() == Player::HUMAN)
    {
      connections[p->getId()].push_back
	(p->sheroGainsLevel.connect(sigc::mem_fun(this, &Game::heroGainsLevel)));
      connections[p->getId()].push_back
	(p->snewMedalArmy.connect(sigc::mem_fun(this, &Game::newMedalArmy)));

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
      connections[p->getId()].push_back
	(p->advice_asked.connect
	 (sigc::mem_fun(advice_asked, &sigc::signal<void, float>::emit)));
      connections[p->getId()].push_back
	(p->smovingStack.connect
	 (sigc::mem_fun(this, &Game::on_stack_starts_moving)));
      connections[p->getId()].push_back
	(p->sstoppingStack.connect
	 (sigc::mem_fun(this, &Game::on_stack_stopped)));
      connections[p->getId()].push_back
	(p->shaltedStack.connect
	 (sigc::mem_fun(this, &Game::on_stack_halted)));
      connections[p->getId()].push_back
	(p->getStacklist()->sgrouped.connect
	 (sigc::mem_fun(this, &Game::on_stack_grouped)));
      connections[p->getId()].push_back
	(p->stole_gold.connect
	 (sigc::mem_fun(stole_gold, &sigc::signal<void, Player*, 
                        guint32>::emit)));
      connections[p->getId()].push_back
	(p->sunk_ships.connect
	 (sigc::mem_fun(sunk_ships, &sigc::signal<void, Player*, 
                        guint32>::emit)));
      connections[p->getId()].push_back
	(p->bags_picked_up.connect
	 (sigc::mem_fun(bags_picked_up, &sigc::signal<void, Hero*, guint32>::emit)));
      connections[p->getId()].push_back
	(p->mp_added_to_hero_stack.connect
	 (sigc::mem_fun(mp_added_to_hero_stack, &sigc::signal<void, Hero*, guint32>::emit)));
    }
      
      
  //now do all of the common connections
      
  connections[p->getId()].push_back
    (p->getStacklist()->snewpos.connect
     (sigc::mem_fun(stack_moves, &sigc::signal<void, Stack*, Vector<int> >::emit)));
  connections[p->getId()].push_back
    (p->srecruitingHero.connect(sigc::mem_fun(this, &Game::recruitHero)));
  connections[p->getId()].push_back
    (p->svisitingTemple.connect
     (sigc::mem_fun(this, &Game::stack_searches_temple)));
  connections[p->getId()].push_back
    (p->ssearchingRuin.connect
     (sigc::mem_fun(this, &Game::stack_searches_ruin)));
  connections[p->getId()].push_back
    (p->getStacklist()->snewpos.connect
     (sigc::mem_fun(this, &Game::stack_arrives_on_tile)));
  connections[p->getId()].push_back
    (p->getStacklist()->soldpos.connect
     (sigc::mem_fun(this, &Game::stack_leaves_tile)));
  connections[p->getId()].push_back
    (p->aborted_turn.connect (sigc::mem_fun
	   (game_stopped, &sigc::signal<void>::emit)));

  connections[p->getId()].push_back
    (p->schangingStats.connect 
     (sigc::mem_fun(this, &Game::update_sidebar_stats)));
        
  connections[p->getId()].push_back
    (p->schangingStatus.connect 
	 (sigc::mem_fun(progress_status_changed, &sigc::signal<void, std::string>::emit)));
        
  connections[p->getId()].push_back
    (p->sbusy.connect (sigc::mem_fun (progress_changed, 
				      &sigc::signal<void>::emit)));
  connections[p->getId()].push_back
    (p->supdatingStack.connect (sigc::mem_fun(this, &Game::stackUpdate)));
  connections[p->getId()].push_back
    (p->sinvadingCity.connect(sigc::mem_fun(this, &Game::invading_city)));
  connections[p->getId()].push_back
    (p->streacheryStack.connect(sigc::mem_fun(this, &Game::maybeTreachery)));
  connections[p->getId()].push_back
    (p->fight_started.connect (sigc::mem_fun(*this, &Game::on_fight_started)));
  connections[p->getId()].push_back
    (p->using_item.connect (sigc::mem_fun(*this, &Game::on_use_item)));
  connections[p->getId()].push_back
    (p->ruinfight_started.connect (sigc::mem_fun(*this, &Game::on_ruinfight_started)));

    //(p->ruinfight_started.connect
     //(sigc::mem_fun
      //(ruinfight_started, &sigc::signal<void, Stack *, Stack *>::emit)));
  connections[p->getId()].push_back
    (p->ruinfight_finished.connect (sigc::mem_fun(*this, &Game::on_ruinfight_finished)));
  //connections[p->getId()].push_back
    //(p->ruinfight_finished.connect
     //(sigc::mem_fun
      //(ruinfight_finished, &sigc::signal<void, Fight::Result>::emit)));
  connections[p->getId()].push_back
    (p->cityfight_finished.connect (sigc::mem_fun(*this, &Game::on_city_fight_finished))); 
  if (p->getType() == Player::NETWORKED && p == Playerlist::getActiveplayer())
    lock_inputs();
  if (p->getType() == Player::HUMAN && p == Playerlist::getActiveplayer())
    unlock_inputs();
}

void Game::on_stack_starts_moving(Stack *stack)
{
  if (Playerlist::getActiveplayer()->getType() == Player::HUMAN)
    lock_inputs();
}

void Game::on_stack_stopped()
{
  if (Playerlist::getActiveplayer()->getType() == Player::HUMAN)
    unlock_inputs();
}

void Game::on_stack_halted(Stack *stack)
{
  if (Playerlist::getActiveplayer()->getType() == Player::HUMAN)
    unlock_inputs();
  if (stack == NULL)
    return;
  bigmap->reset_path_calculator(stack);
  //tell gamebigmap that a stack just stopped
}
void Game::on_stack_grouped(Stack *stack, bool grouped)
{
  bigmap->reset_path_calculator(stack);
  //tell gamebigmap that we just grouped/ungrouped a stack.
  return;
}
#include "game-server.h"

Game::Game(GameScenario* gameScenario, NextTurn *nextTurn)
    : d_gameScenario(gameScenario), d_nextTurn(nextTurn)
{
    current_game = this;
    input_locked = false;

    // init the bigmap
    bigmap.reset(new GameBigMap
		 (GameScenario::s_intense_combat, 
		  GameScenario::s_see_opponents_production, 
		  GameScenario::s_see_opponents_stacks, 
		  GameScenario::s_military_advisor));
    bigmap->stack_selected.connect(
	sigc::mem_fun(this, &Game::on_stack_selected));
    bigmap->stack_grouped_or_ungrouped.connect(
	sigc::mem_fun(this, &Game::on_stack_grouped_or_ungrouped));
    bigmap->path_set.connect(
	sigc::mem_fun(this, &Game::update_control_panel));
    bigmap->city_visited.connect(
	sigc::mem_fun(this, &Game::on_city_visited));
    bigmap->city_queried.connect(
	sigc::mem_fun(this, &Game::on_city_queried));
    bigmap->city_unqueried.connect(
	sigc::mem_fun(this, &Game::on_city_unqueried));
    bigmap->ruin_queried.connect(
	sigc::mem_fun(this, &Game::on_ruin_queried));
    bigmap->signpost_queried.connect(
	sigc::mem_fun(this, &Game::on_signpost_queried));
    bigmap->temple_queried.connect(
	sigc::mem_fun(this, &Game::on_temple_queried));
    bigmap->stack_queried.connect(
	sigc::mem_fun(this, &Game::on_stack_queried));
    bigmap->stack_unqueried.connect(
	sigc::mem_fun(this, &Game::on_stack_unqueried));

    // init the smallmap
    smallmap.reset(new SmallMap);
    // pass map changes directly through 
    smallmap->resize();
    smallmap->map_changed.connect(
	sigc::mem_fun(smallmap_changed,
		      &sigc::signal<void, Glib::RefPtr<Gdk::Pixmap>, 
		      Gdk::Rectangle>::emit));

    // connect the two maps
    bigmap->view_changed.connect(
	sigc::mem_fun(smallmap.get(), &SmallMap::set_view));
    bigmap->map_changed.connect(
	sigc::mem_fun(bigmap_changed,
		      &sigc::signal<void, Glib::RefPtr<Gdk::Pixmap> >::emit));
    smallmap->view_changed.connect(
	sigc::mem_fun(bigmap.get(), &GameBigMap::set_view));

    // get the maps up and running
    Playerlist::getInstance()->setViewingplayer(Playerlist::getActiveplayer());
    bigmap->screen_size_changed(Gtk::Allocation(0,0,320,200));

    // connect player callbacks
    Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::iterator i = pl->begin(); i != pl->end(); ++i)
    {
	Player *p = *i;
	addPlayer(p);
    }
    if (gameScenario->getPlayMode() == GameScenario::HOTSEAT)
      pl->splayerDead.connect(sigc::mem_fun(this, &Game::on_player_died));
    pl->ssurrender.connect(sigc::mem_fun(this, &Game::on_surrender_offered));

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
  //only human players hit this.
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
    s.income = player->getIncome();
    s.cities = Citylist::getInstance()->countCities(player);

    s.units = 0;
    s.upkeep = player->getUpkeep() + player->getCostOfUnitsProducedThisTurn();
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
	bigmap->draw(Playerlist::getViewingplayer());
      }
    if (smallmap.get())
      {
	//if (Playerlist::getActiveplayer()->getType() == Player::HUMAN ||
	    //GameScenario::s_hidden_map == false)
	smallmap->draw(Playerlist::getActiveplayer());
      }
}

void Game::select_next_movable_stack()
{
  Stacklist *sl = Playerlist::getActiveplayer()->getStacklist();
  Stack* stack = sl->getNextMovable();
  sl->setActivestack(stack);
  select_active_stack();
}

void Game::move_map_dir(int diffx, int diffy)
{
  smallmap->move_map_in_dir(Vector<int>(diffx, diffy));
}

void Game::move_selected_stack_dir(int diffx, int diffy)
{
  static bool currently_moving = false;
  if (currently_moving == true)
    return;
  currently_moving = true;
  Stack *stack = Playerlist::getActiveplayer()->getActivestack();
  if (!stack)
    return;
  // Get rid of the old path if there is one
  if (stack->getPath()->size())
    stack->getPath()->clear();
  //See if we can move there
  Vector<int> dest = stack->getPos();
  dest += Vector<int>(diffx, diffy);
  int mp = stack->getPath()->calculate(stack, dest);
  if (mp > 0)
    move_selected_stack_along_path();
  else
    {
      Playerlist::getActiveplayer()->setActivestack(0);
      unselect_active_stack();
    }
  currently_moving = false;
}

void Game::move_selected_stack_northwest()
{
  if (Playerlist::getActiveplayer()->getActivestack() != NULL)
    move_selected_stack_dir(-1, -1);
  else
    move_map_dir(-1,-1);
}

void Game::move_selected_stack_north()
{
  if (Playerlist::getActiveplayer()->getActivestack() != NULL)
    move_selected_stack_dir(0, -1);
  else
    move_map_dir(0, -1);
}

void Game::move_selected_stack_northeast()
{
  if (Playerlist::getActiveplayer()->getActivestack() != NULL)
    move_selected_stack_dir(1, -1);
  else
    move_map_dir(1, -1);
}

void Game::move_selected_stack_east()
{
  if (Playerlist::getActiveplayer()->getActivestack() != NULL)
    move_selected_stack_dir(1, 0);
  else
    move_map_dir(1, 0);
}

void Game::move_selected_stack_west()
{
  if (Playerlist::getActiveplayer()->getActivestack() != NULL)
    move_selected_stack_dir(-1, 0);
  else
    move_map_dir(-1, 0);
}

void Game::move_selected_stack_southwest()
{
  if (Playerlist::getActiveplayer()->getActivestack() != NULL)
    move_selected_stack_dir(-1, 1);
  else
    move_map_dir(-1, 1);
}

void Game::move_selected_stack_south()
{
  if (Playerlist::getActiveplayer()->getActivestack() != NULL)
    move_selected_stack_dir(0, 1);
  else
    move_map_dir(0, 1);
}

void Game::move_selected_stack_southeast()
{
  if (Playerlist::getActiveplayer()->getActivestack() != NULL)
    move_selected_stack_dir(1, 1);
  else
    move_map_dir(1, 1);
}

void Game::move_selected_stack_along_path()
{
  Stack *stack = Playerlist::getActiveplayer()->getActivestack();

  Playerlist::getActiveplayer()->stackMove(stack);

  //maybe we joined another stack
  stack = Playerlist::getActiveplayer()->getActivestack();
  if (stack && stack->canMove() == false)
    {
      Playerlist::getActiveplayer()->setActivestack(0);
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
	  bool moved = player->stackMove(player->getActivestack());
	  if (!moved)
	    break;
	  i = sl->begin();
	}
    }

  if (sl->getActivestack()->canMove() == false)
    {
      Playerlist::getActiveplayer()->setActivestack(0);
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
  player->setActivestack(stack);

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
  player->setActivestack(stack);

  if (stack)
    select_active_stack();
  else
    unselect_active_stack();
}

void Game::deselect_selected_stack()
{
  Player *player = Playerlist::getActiveplayer();
  player->setActivestack(0);
  unselect_active_stack();
}

void Game::center_selected_stack()
{
  Stack *stack = Playerlist::getActiveplayer()->getActivestack();
  if (stack) 
    select_active_stack();
}

void Game::search_stack(Stack *stack, bool &gotquest, bool &stackdied)
{
  Player *player = Playerlist::getActiveplayer();
  Ruin* ruin = GameMap::getRuin(stack);
  Temple* temple = GameMap::getTemple(stack);

  if (ruin && !ruin->isSearched() && stack->hasHero() &&
      stack->getFirstHero()->getMoves() > 0 &&
      ((ruin->isHidden() == true && ruin->getOwner() == player) ||
       ruin->isHidden() == false))
    {
      Reward *reward;
      reward = player->stackSearchRuin(stack, ruin, stackdied);
      if (stackdied)
        return;
      if (ruin->hasSage() == true)
	{
          Sage *sage = ruin->generateSage();
          if (player->isComputer() == false)
            reward = sage_visited.emit(ruin, sage, stack);
          else
            reward = player->chooseReward(ruin, sage, stack);
	}
	  
      if (reward)
	{
          StackReflist *stacks = new StackReflist();
	  player->giveReward(stack, reward, stacks);
          delete stacks;
	  //FIXME: delete this reward, but don't delete the item, or map
	  redraw();
	  update_stack_info();
	  update_control_panel();
          if (player->isComputer() == false)
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
  else if (temple && temple->searchable() && stack->getMoves() > 0)
    {
      int blessCount;
      blessCount = player->stackVisitTemple(stack, temple);
      bool wants_quest;
      Hero *hero = stack->getFirstHeroWithoutAQuest();
      if (player->isComputer() == false)
        wants_quest = temple_searched.emit(hero, temple, blessCount);
      else
        wants_quest = player->chooseQuest(hero);
      if (wants_quest && stack->hasHero())
	{
	  Quest *q = player->heroGetQuest 
            (hero, temple, 
             GameScenario::s_razing_cities != GameParameters::NEVER);

	  if (q)
            {
              gotquest = true;
              if (player->isComputer() == false)
                {
                  Hero *hero;
                  Army *a = stack->getArmyById(q->getHeroId());
                  if (a)
                    hero = dynamic_cast<Hero*>(a);
                  quest_assigned.emit(hero, q);
                }
            }
	}
    }
}

void Game::select_item_to_use()
{
  Player *active = Playerlist::getActiveplayer();
  //emit a signal that makes a dialog appear that lets us pick an item to use.
  std::list<Item*> items = active->getUsableItems();
  if (items.size() == 0)
    return;
  Item *item = select_item.emit(items);
  if (item != NULL)
    on_use_item(item);
}

void Game::on_use_item(Item *item)
{
  Player *active = Playerlist::getActiveplayer();
  Stack *stack = NULL;
  Hero *hero = NULL;
  active->getItemHolder(item, &stack, &hero);
  Player *victim = NULL;

  //ask the user a series of questions on how to use the item
  if (item->usableOnVictimPlayer())
    victim = select_item_victim_player.emit();
          
  // now we act on the answers and execute the usage of the item
  if (item->usableOnVictimPlayer() == true && victim)
    active->heroUseItem(hero, item, victim);

  if (item->usableOnVictimPlayer() == false)
    active->heroUseItem(hero, item, NULL);
}

void Game::search_selected_stack()
{
  Player *player = Playerlist::getActiveplayer();
  Stack* stack = player->getActivestack();
  bool stack_died = false;
  bool got_quest = false;
  search_stack(stack, got_quest, stack_died);
  return;
}

void Game::stackUpdate(Stack* s)
{
  if (!s)
    s = Playerlist::getActiveplayer()->getActivestack();

  //if player is not to be observed, bail now
  if (s != NULL && s->getOwner()->isObservable() == false)
      return;

  if (s)
    smallmap->center_view_on_tile(s->getPos(), true);

  //redraw();

  update_stack_info();
  update_control_panel();

}

Army::Stat Game::heroGainsLevel(Hero * h)
{
  // don't show a dialog if computer or enemy's armies advance
  if (h->getOwner()->isComputer() == true ||
      h->getOwner() != Playerlist::getInstance()->getActiveplayer())
    return Playerlist::getInstance()->getActiveplayer()->chooseStat(h);

  return hero_gains_level.emit(h);
}

void Game::newMedalArmy(Army* a, int medaltype)
{
  // We don't want to have medal awards of computer players displayed
  if (!a->getOwner()
      || (a->getOwner()->getType() != Player::HUMAN)
      || a->getOwner() != Playerlist::getInstance()->getActiveplayer())
    return;

  medal_awarded_to_army.emit(a, medaltype);
  update_stack_info();
}

void Game::on_stack_grouped_or_ungrouped(Stack *s)
{
  //this only happens when we double-click on a stack on the bigmap.
  update_stack_info();
  update_control_panel();
}

void Game::on_stack_selected(Stack* s)
{
  update_stack_info();
  update_control_panel();
}

void Game::on_city_queried (Vector<int> tile, City *c)
{
  MapTipPosition mpos = bigmap->map_tip_position(tile);
  city_tip_changed.emit(c, mpos);
}

void Game::on_city_unqueried ()
{
  city_tip_changed.emit(NULL, MapTipPosition());
}

void Game::on_city_visited(City* c)
{
  if (c)
    {
      city_visited.emit(c);
      // some visible city properties (razed) may have changed
      redraw();
    }
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

	  MapTipPosition mpos = bigmap->map_tip_position(r->getArea());
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

      MapTipPosition mpos = bigmap->map_tip_position(s->getArea());
      map_tip_changed.emit(str, mpos);
    }
  else
    map_tip_changed.emit("", MapTipPosition());
}

void Game::on_stack_unqueried ()
{
  stack_tip_changed.emit(NULL, MapTipPosition());
}
void Game::on_stack_queried (Vector<int> tile)
{
  MapTipPosition mpos = bigmap->map_tip_position(tile);
  stack_tip_changed.emit(GameMap::getStacks(tile), mpos);
}

void Game::on_temple_queried (Temple* t, bool brief)
{
  if (t)
    {
      if (brief)
	{
	  Glib::ustring str;

	  str = t->getName();

	  MapTipPosition mpos = bigmap->map_tip_position(t->getArea());
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
	//the razing just happened in the are-you-sure dialog, and the user
	//was sure.
	city_razed.emit(city);
	player->deteriorateDiplomaticRelationship (5);
	break;

      case CITY_DEFEATED_PILLAGE:
	  {
	    int pillaged_army_type = -1;
	    player->cityPillage(city, gold, &pillaged_army_type);
	    city_pillaged.emit(city, gold, pillaged_army_type);
	  }
	break;

      case CITY_DEFEATED_SACK:
	std::list<guint32> sacked_types;
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
  bigmap->reset_zoom();
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
      can_use_item.emit(false);
      can_plant_standard_selected_stack.emit(false);
      can_move_selected_stack.emit(false);
      can_move_selected_stack_along_path.emit(false);
      can_move_all_stacks.emit(false);
      can_group_ungroup_selected_stack.emit(false);
      can_end_turn.emit(false);
      can_disband_stack.emit(false);
      can_change_signpost.emit(false);
      can_see_history.emit(false);
      can_see_diplomacy.emit(false);

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
	((!stack->getPath()->empty() && stack->enoughMoves()) ||
	 (!stack->getPath()->empty() && stack->getPath()->getMovesExhaustedAtPoint() > 0));

      if (stack->getMoves() > 0)
        can_move_selected_stack.emit(true);

      can_plant_standard_selected_stack.emit(GameMap::can_plant_flag(stack));

      can_search_selected_stack.emit(GameMap::can_search(stack));
          
      can_use_item.emit(player->hasUsableItem());

      if (GameMap::getSignpost(stack))
	can_change_signpost.emit(true);

      can_disband_stack.emit(true);
      can_group_ungroup_selected_stack.emit(true);
    }
  else
    {
      can_move_selected_stack.emit(false);
      can_move_selected_stack_along_path.emit(false);
      can_disband_stack.emit(false);
      can_group_ungroup_selected_stack.emit(false);
      can_plant_standard_selected_stack.emit(false);
    }

  if (d_gameScenario->getRound() > 1)
    can_see_history.emit(true);
  else
    can_see_history.emit(false);
    
  can_see_diplomacy.emit(GameScenarioOptions::s_diplomacy);

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
      
  center_view_on_city();
  update_sidebar_stats();
  update_control_panel();
  update_stack_info();
  lock_inputs();

  if (d_gameScenario->getPlayMode() != GameScenario::NETWORKED)
    d_nextTurn->start();
      
  if (Playerlist::getInstance()->countPlayersAlive())
    update_control_panel();
}

void Game::loadGame()
{
  //if pbm and human
  //if pbm and not human, then bail

  Player *player = Playerlist::getActiveplayer();
  if (!player)
    {
      Playerlist::getInstance()->nextPlayer();
      player = Playerlist::getActiveplayer();
    }

  if (player->getType() == Player::HUMAN && (d_gameScenario->getPlayMode() == GameScenario::HOTSEAT))
    {
      //human players want access to the controls and an info box
      unlock_inputs();
      player->setActivestack(0);
      center_view_on_city();
      update_sidebar_stats();
      update_control_panel();
      update_stack_info();
      game_loaded.emit(player);
      if (player->getType() == Player::HUMAN)
	d_nextTurn->setContinuingTurn();
    }
  else if (player->getType() == Player::HUMAN && 
	   d_gameScenario->getPlayMode() == GameScenario::PLAY_BY_MAIL)
    {
      if (player->hasAlreadyInitializedTurn())
	{
	  unlock_inputs();
	  player->setActivestack(0);
	  center_view_on_city();
	  update_sidebar_stats();
	  update_control_panel();
	  update_stack_info();
	  game_loaded.emit(player);
	  player->loadPbmGame();
	  d_nextTurn->setContinuingTurn();
	}
      else
	{
	  lock_inputs();
	}
    }
  else
    lock_inputs();

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
}

bool Game::saveGame(std::string file)
{
  return d_gameScenario->saveGame(file);
}

void Game::blank(bool on)
{
  if (GameScenarioOptions::s_hidden_map == true)
    {
      bigmap->blank(on);
      smallmap->blank(on);
    }
}

void Game::init_turn_for_player(Player* p)
{
  Playerlist* pl = Playerlist::getInstance();

  blank(true);

  printf("init turn for player %s\n", p->getName().c_str());
  next_player_turn.emit(p, d_gameScenario->getRound());

  if (p->getType() == Player::NETWORKED)
    {
      remote_next_player_turn.emit();
      return;
    }
  blank(false);

  if (p->isObservable() == true)
    center_view_on_city();

  if (p->getType() == Player::HUMAN)
    {
      if (Commentator::getInstance()->hasComment() == true)
        {
          std::vector<std::string> comments =
            Commentator::getInstance()->getComments(p);
          if (comments.size() > 0)
            commentator_comments.emit(comments[rand() % comments.size()]);
        }
    }

  p->maybeRecruitHero();

  if (p->getType() == Player::HUMAN)
    {
      unlock_inputs();

      update_sidebar_stats();
      update_stack_info();
      update_control_panel();
      redraw();

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
      //check to see if we've turned off production due to destitution.
      bool destitute = false;
      if (p->countDestituteCitiesThisTurn() > 0)
        destitute = true;
      city_too_poor_to_produce.emit(destitute);
    }
  else
    {
      //SDL_Delay(250);
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
  
  //don't show the battle if the ai is attacking neutral
  bool ai_attacking_neutral = false;
  if (fight.getDefenders().front()->getOwner() == Playerlist::getInstance()->getNeutral() && Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    ai_attacking_neutral = true;

  //show the battle if we're attacking an observable player
  bool attacking_observable_player = false;
  if (fight.getDefenders().front()->getOwner()->isObservable())
    attacking_observable_player = true;

  //don't show the battle if we're ai and we're on a hidden map
  bool ai_attacking_on_hidden_map = false;
  if (fight.getAttackers().front()->getOwner()->getType() != Player::HUMAN &&
      GameScenario::s_hidden_map == true)
    ai_attacking_on_hidden_map = true;

  if ((Playerlist::getActiveplayer()->isObservable() == true ||
      attacking_observable_player) && !ai_attacking_neutral &&
      !ai_attacking_on_hidden_map)
    {
      Vector<int> pos = fight.getAttackers().front()->getPos();
      if (GameScenario::s_hidden_map == false)
	smallmap->center_view_on_tile(pos, true);
      fight_started.emit(Fight::calculateFightBox(fight), fight);
    }
  else if ((Playerlist::getActiveplayer()->isObservable() == true ||
      attacking_observable_player) && ai_attacking_neutral &&
      !ai_attacking_on_hidden_map)
    {
      Vector<int> pos = fight.getAttackers().front()->getPos();
      if (GameScenario::s_hidden_map == false)
	smallmap->center_view_on_tile(pos, true);
      abbreviated_fight_started.emit(Fight::calculateFightBox(fight));
    }
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
      City *c = *i;
      if (c->getOwner() == p && c->isCapital() &&
	  c->getCapitalOwner() == p)
	{
	  smallmap->center_view_on_tile(c->getPos(), 
					!GameScenario::s_hidden_map);
	  return;
	}
    }

  // okay, then find any city that belongs to the player and center on it
  for (Citylist::iterator i = Citylist::getInstance()->begin();
       i != Citylist::getInstance()->end(); i++)
    {
      City *c = *i;
      if (c->getOwner() == p)
	{
	  smallmap->center_view_on_tile(c->getPos(), 
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
  bool treachery = false;
  if (me->isComputer())
    {
      if (me->getType() == Player::AI_FAST)
        {
          AI_Fast *ai = dynamic_cast<AI_Fast*>(me);
          treachery = ai->chooseTreachery (stack, them, pos);
        }
      else if (me->getType() == Player::AI_SMART)
        {
          AI_Smart *ai = dynamic_cast<AI_Smart*>(me);
          treachery = ai->chooseTreachery (stack, them, pos);
        }
    }
  if (treachery == false)
    return false;
  me->proposeDiplomacy (Player::NO_PROPOSAL, them);
  me->declareDiplomacy (Player::AT_WAR, them);
  them->proposeDiplomacy (Player::NO_PROPOSAL, me);
  them->declareDiplomacy (Player::AT_WAR, me);
  History_DiplomacyTreachery *item = new History_DiplomacyTreachery();
  item->fillData(them);
  me->addHistory(item);

  me->deteriorateDiplomaticRelationship (5);
  them->improveDiplomaticRelationship (2, me);

  return true;
}

void Game::nextRound()
{
  if (GameServer::getInstance()->isListening())
    {
      Playerlist::getInstance()->nextRound
        (GameScenarioOptions::s_diplomacy, 
         &GameScenarioOptions::s_surrender_already_offered);
    }
}
    
void Game::on_surrender_offered(Player *recipient)
{
  Playerlist *plist = Playerlist::getInstance();
  if (enemy_offers_surrender.emit(plist->countPlayersAlive() - 1))
    {
      plist->surrender();
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
	  Stack *o = GameMap::getStacks(city->getPos())->getFriendlyStack(neu);
	  if (o)
	    {
	      int army_type = o->getStrongestArmy()->getTypeId();
	      for (guint32 i = 0; i < city->getMaxNoOfProductionBases(); i++)
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
    
bool Game::recruitHero(HeroProto *hero, City *city, int gold)
{
  bool retval; 
  if (city->getOwner()->isComputer())
    retval = city->getOwner()->chooseHero (hero, city, gold);
  else
    {
      retval = hero_offers_service.emit (city->getOwner(), hero, city, gold);
      if (d_gameScenario->getRound() == 1)
        city_visited.emit(city);
    }
  return retval;
}
    
bool Game::saveTurnFile(std::string turnfile)
{
  bool broken;
  //trigger the GameServer to spit out a set of networkactions and networkhistory events for the active player, into a file.
  if (d_gameScenario->getPlayMode() == GameScenario::PLAY_BY_MAIL)
    PbmGameServer::getInstance()->endTurn(turnfile, broken);
  return true;
}

void Game::inhibitAutosaveRemoval(bool inhibit)
{
  if (d_gameScenario)
    d_gameScenario->inhibitAutosaveRemoval(inhibit);
}

void Game::endOfGameRoaming(Player *winner)
{
  Playerlist* pl = Playerlist::getInstance();
  pl->setWinningPlayer(winner);

  winner->immobilize();
  d_gameScenario->s_see_opponents_stacks = true;
  d_gameScenario->s_see_opponents_production = true;
  bigmap->d_see_opponents_stacks = true;
  bigmap->d_see_opponents_production = true;
  center_view_on_city();

  unlock_inputs();

  update_sidebar_stats();
  update_stack_info();
  update_control_panel();
  redraw();
}

void Game::stack_arrives_on_tile(Stack *stack, Vector<int> tile)
{
  StackTile *stile = GameMap::getInstance()->getTile(tile)->getStacks();
  stile->arriving(stack);
}

void Game::stack_leaves_tile(Stack *stack, Vector<int> tile)
{
  StackTile *stile = GameMap::getInstance()->getTile(tile)->getStacks();
  bool left = stile->leaving(stack);
  if (left == false)
    {
      if (stack == NULL)
	{
	  printf("stack is %p\n", stack);
	  printf("WTFFF!!!!!!!!!!!!!!!!!!!!\n");
	  return;
	}
    }
}
    
bool Game::stack_searches_ruin(Ruin *ruin, Stack *stack)
{
  bool stack_died = false;
  bool hero_got_quest = false;
  search_stack(stack, hero_got_quest, stack_died);
  return stack_died;
}
    
bool Game::stack_searches_temple(Temple *temple, Stack *stack)
{
  bool stack_died = false;
  bool hero_got_quest = false;
  search_stack(stack, hero_got_quest, stack_died);
  return hero_got_quest;
}

void Game::on_ruinfight_started(Stack *attacker, Stack *defender)
{
  if (Playerlist::getActiveplayer()->isComputer() == false)
    ruinfight_started.emit(attacker, defender);
}

void Game::on_ruinfight_finished(Fight::Result result)
{
  if (Playerlist::getActiveplayer()->isComputer() == false)
    ruinfight_finished.emit(result);
}
