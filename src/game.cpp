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

#include "bigmap.h"
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


//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
#define debug(x)

Game::Game(GameScenario* gameScenario)
    : d_gameScenario(gameScenario), input_locked(false)
{
    map_view.x = map_view.y = 0;
    
    // init the bigmap
    bigmap.reset(new BigMap);
    bigmap->view_changed.connect(
	sigc::mem_fun(this, &Game::on_bigmap_view_changed));
    bigmap->stack_selected.connect(
	sigc::mem_fun(this, &Game::on_stack_selected));
    bigmap->path_set.connect(
	sigc::mem_fun(this, &Game::update_control_panel));
    bigmap->city_selected.connect(
	sigc::mem_fun(this, &Game::on_city_selected));
    bigmap->ruin_selected.connect(
	sigc::mem_fun(this, &Game::on_ruin_selected));
    bigmap->signpost_selected.connect(
	sigc::mem_fun(this, &Game::on_signpost_selected));
    bigmap->temple_selected.connect(
	sigc::mem_fun(this, &Game::on_temple_selected));
    bigmap->mouse_on_tile.connect(
	sigc::mem_fun(this, &Game::on_mouse_on_tile));

    // init the smallmap
    smallmap.reset(new SmallMap);
    smallmap->view_changed.connect(
	sigc::mem_fun(*this, &Game::on_smallmap_view_changed));
    smallmap->resize(Vector<int>(GameMap::getWidth(), GameMap::getHeight()) * 2);
    // pass map changes directly through 
    smallmap->map_changed.connect(
	sigc::mem_fun(smallmap_changed,
		      &sigc::signal<void, SDL_Surface *>::emit));
    
    // get the maps up and running
    size_changed();

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
	    sigc::mem_fun(fight_started, &sigc::signal<void, Fight &>::emit));
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
	sigc::mem_fun(*d_gameScenario, &GameScenario::nextRound));
    d_nextTurn->supdating.connect(
	sigc::mem_fun(bigmap.get(), &BigMap::draw));
            
    center_view_on_city();
    update_control_panel();

    loadHeroTemplates();
}

Game::~Game()
{
    delete d_gameScenario;
    delete d_nextTurn;
}

void Game::redraw()
{
    if (bigmap.get())
	bigmap->draw();
    if (smallmap.get())
	smallmap->draw();
}

void Game::size_changed()
{
    SDL_Surface *v = SDL_GetVideoSurface();
    int ts = GameMap::getInstance()->getTileSet()->getTileSize();
    
    map_view.w = v->w / ts;
    map_view.h = v->h / ts;

    map_view.pos = clip(Vector<int>(0,0), map_view.pos,
			GameMap::get_dim() - map_view.dim);

    // inform the maps of the new size
    bigmap->set_view(map_view);
    smallmap->set_view(map_view);
}

void Game::smallmap_mouse_button_event(MouseButtonEvent e)
{
    if (smallmap.get())
	smallmap->mouse_button_event(e);
}

void Game::smallmap_mouse_motion_event(MouseMotionEvent e)
{
    if (smallmap.get())
	smallmap->mouse_motion_event(e);
}

void Game::mouse_button_event(MouseButtonEvent e)
{
    if (bigmap.get())
	bigmap->mouse_button_event(e);
}

void Game::mouse_motion_event(MouseMotionEvent e)
{
    if (bigmap.get())
	bigmap->mouse_motion_event(e);
}

void Game::key_press_event(KeyPressEvent e)
{
}

void Game::end_turn()
{
    bigmap->unselect_active_stack();
    clear_stack_info();
    update_control_panel();
    lock_inputs();

    d_nextTurn->endTurn();
}

void Game::center_view(Vector<int> p)
{
    map_view.pos = clip(Vector<int>(0, 0), p - map_view.dim / 2,
			GameMap::get_dim() - map_view.dim);

    smallmap->set_view(map_view);
    bigmap->set_view(map_view);
}

void Game::on_mouse_on_tile(Vector<int> pos)
{
    //std::cerr << "mouse on tile " << pos.x << " " << pos.y << std::endl;
}

void Game::on_bigmap_view_changed(Rectangle view)
{
    map_view = view;
    if (smallmap.get())
	smallmap->set_view(map_view);
}

void Game::on_smallmap_view_changed(Rectangle view)
{
    map_view = view;
    if (bigmap.get())
	bigmap->set_view(map_view);
}

void Game::update_stack_info()
{
    Stack* stack = Playerlist::getActiveplayer()->getActivestack();
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
        int cur_gold = player->getGold();

        if (!(player->stackSearchRuin(stack, ruin)))
        {
            stackRedraw();
            return;
        }

        stackRedraw();
        // this also includes the gold-only hack
        int gold_added = player->getGold() - cur_gold;

	ruin_searched.emit(ruin, stack, gold_added);
	
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
    debug("Game: stackUpdate()");

    bigmap->stackMoved(s);

    update_stack_info();
    update_control_panel();
}

// s is currently unused, but can later be filled with reasonable data
void Game::stackDied(Stack* s)
{
    debug("stackDied()");
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

bool Game::stackRedraw()
{
    bigmap->draw();
    update_stack_info();
    update_control_panel();
    return true;
}

void Game::on_stack_selected(Stack* s)
{
    update_stack_info();
    update_control_panel();
}

void Game::on_city_selected(City* c, MapTipPosition pos, bool brief)
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
	    
	    map_tip_changed.emit(str, pos);
	}
	else if (player == Playerlist::getActiveplayer() && !c->isBurnt())
	{
	    city_visited.emit(c);

	    // some visible city properties (razed) may have changed
	    bigmap->draw();
	}
    }
    else
	map_tip_changed.emit("", pos);
}

void Game::on_ruin_selected(Ruin* r, MapTipPosition pos)
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
	    
	map_tip_changed.emit(str, pos);
    }
    else
	map_tip_changed.emit("", pos);
}

void Game::on_signpost_selected(Signpost* s, MapTipPosition pos)
{
    if (s)
    {
	Glib::ustring str;

	str = s->getName();
	    
	map_tip_changed.emit(str, pos);
    }
    else
	map_tip_changed.emit("", pos);
}

void Game::on_temple_selected(Temple* t, MapTipPosition pos)
{
    if (t)
    {
	Glib::ustring str;

	str = t->getName();
	    
	map_tip_changed.emit(str, pos);
    }
    else
	map_tip_changed.emit("", pos);
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
        bigmap->draw();
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
	    player->cityPillage(city, gold);
	    city_pillaged.emit(city, gold);
	    break;

	case CITY_DEFEATED_SACK:
	    player->citySack(city, gold);
	    city_sacked.emit(city, gold);
	    break;
	}
	
	if (!city->isBurnt())
	    city_visited.emit(city);
    }
   
    Playerlist::getInstance()->checkPlayers();
    bigmap->draw();
    update_stack_info();
    update_sidebar_stats();
    update_control_panel();
}

void Game::lock_inputs()
{
    //don't accept modifying user input from now on
    bigmap->set_accept_events(false);
    input_locked = true;
    update_control_panel();
}

void Game::unlock_inputs()
{
    bigmap->set_accept_events(true);
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
    }
    else
    {
	can_search_selected_stack.emit(false);
	can_move_selected_stack.emit(false);
    }

    can_end_turn.emit(true);
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
            Reward_Allies::addAllies(p, city->getPos(), army, alliesCount);
            if (p->getType() == Player::HUMAN)
              hero_arrives.emit(alliesCount);
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
  Uint32 heroset = al->getHeroId();
  const Army* herotype;


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
      herotype = al->getArmy(heroset, rand() % al->getSize (heroset));
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
	    center_view(stack->getPos());
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

void Game::center_view_on_city()
{
    const Player* p = Playerlist::getInstance()->getActiveplayer();
    
    // preferred city is a capital city that belongs to the player 
    for (Citylist::iterator i = Citylist::getInstance()->begin();
            i != Citylist::getInstance()->end(); i++)
    {
        if (i->getPlayer() == p && i->isCapital())
        {
	    center_view(i->getPos());
            return;
        }
    }

    // okay, then find any city that belongs to the player and center on it
    for (Citylist::iterator i = Citylist::getInstance()->begin();
        i != Citylist::getInstance()->end(); i++)
    {
        if (i->getPlayer() == p)
        {
	    center_view(i->getPos());
            break;
        }
    }
}

// This function makes all armies part of the stack.
void Game::selectAllStack()
{
    Stack *s = Playerlist::getActiveplayer()->getStacklist()->getActivestack();
    if (s)
    {
    	s->selectAll();
	update_stack_info();
    }

    return;
}
