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
//#include "RuinSearchDialog.h"
//#include "CityOccupationDialog.h"
//#include "FightDialog.h"
//#include "cityinfo.h"
//#include "hero_offer.h"
//#include "QuestsManager.h"
//#include "ArmyLevelDialog.h"
//#include "ArmyMedalDialog.h"
#include "events/ERound.h"
#include "events/ENextTurn.h"
#include "events/RUpdate.h"
#include "events/RCenter.h"
#include "events/RCenterObj.h"
#include "events/RRaiseEvent.h"
#include "events/RActEvent.h"
#include "events/RWinGame.h"


//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
#define debug(x)

Game::Game(GameScenario* gameScenario)
    : d_gameScenario(gameScenario), d_lock(false)
{
    map_view.x = map_view.y = 0;
    
    // init the bigmap
    bigmap.reset(new BigMap);
    bigmap->view_changed.connect(
	sigc::mem_fun(*this, &Game::on_bigmap_view_changed));
    bigmap->stack_selected.connect(
	sigc::mem_fun((*this), &Game::on_stack_selected));
    bigmap->path_set.connect(
	sigc::mem_fun((*this), &Game::update_control_panel));
    bigmap->city_selected.connect(
	sigc::mem_fun((*this), &Game::on_city_selected));
    bigmap->ruin_selected.connect(
	sigc::mem_fun((*this), &Game::on_ruin_selected));
    bigmap->signpost_selected.connect(
	sigc::mem_fun((*this), &Game::on_signpost_selected));
    bigmap->temple_selected.connect(
	sigc::mem_fun((*this), &Game::on_temple_selected));
    bigmap->mouse_on_tile.connect(
	sigc::mem_fun(*this, &Game::on_mouse_on_tile));

    // init the smallmap
    smallmap.reset(new SmallMap);
    smallmap->view_changed.connect(
	sigc::mem_fun(*this, &Game::on_smallmap_view_changed));
    smallmap->resize(Vector<int>(GameMap::getWidth(), GameMap::getHeight()) * 2);
    // pass map changes directly through 
    smallmap->map_changed.connect(
	sigc::mem_fun(smallmap_changed,
		      &sigc::signal<void, SDL_Surface *>::emit));
    
    // get them up and running
    size_changed();

    // connect player callbacks
    const Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::const_iterator i = pl->begin(); i != pl->end(); ++i)
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
		    sigc::mem_fun(hero_offers_service, &sigc::signal<bool, Player *, Hero *, int>::emit),
		    p));
        }
	
        p->schangingStatus.connect(
	    sigc::mem_fun(this, &Game::update_sidebar_stats));
        p->supdatingStack.connect(sigc::mem_fun(this, &Game::stackUpdate));
	
        p->sinvadingCity.connect(sigc::mem_fun(this, &Game::invading_city));
        p->fight_started.connect(
	    sigc::mem_fun(fight_started, &sigc::signal<void, Fight &>::emit));
    }

    //set up a NextTurn object
    d_nextTurn = new NextTurn(d_gameScenario->getTurnmode());
    d_nextTurn->splayerStart.connect(
	sigc::mem_fun(this, &Game::init_turn_for_player));
    d_nextTurn->snextRound.connect(
	sigc::mem_fun(*d_gameScenario, &GameScenario::nextRound));
    d_nextTurn->supdating.connect(
	sigc::mem_fun(bigmap.get(), &BigMap::draw));
            
    connectEvents();

    center_view_on_city();
    update_control_panel();

#if 0
    // load other pictures
    d_pic_turn_start = File::getMiscPicture("ship.jpg", false);
    d_pic_winGame = File::getMiscPicture("win.jpg", false);

    // mask pics need a special format
    SDL_Surface* tmp = File::getMiscPicture("win_mask.png");
    d_pic_winGameMask = SDL_CreateRGBSurface(SDL_SWSURFACE, tmp->w, tmp->h,
                                        32, 0xFF000000, 0xFF0000, 0xFF00, 0xFF);
    SDL_SetAlpha(tmp, 0, 0);
    SDL_BlitSurface(tmp, 0, d_pic_winGameMask, 0);
    SDL_FreeSurface(tmp);
#endif
}

Game::~Game()
{
    //smallmap as well as bigmap have timers running, so first stop the timers
    //and then wait some microseconds to make sure we don't get problems with
    //the timers
    stopTimers();
    SDL_Delay(1);

    delete d_gameScenario;
    delete d_nextTurn;
    
    //delete d_stackinfo;

#if 0
    SDL_FreeSurface(d_pic_turn_start);
    SDL_FreeSurface(d_pic_winGame);
    SDL_FreeSurface(d_pic_logo);
#endif
}

void Game::redraw()
{
    std::cerr << "REDRAWING" << std::endl;
    
    if (bigmap.get())
	bigmap->draw();
}

void Game::size_changed()
{
    SDL_Surface *v = SDL_GetVideoSurface();
    int ts = GameMap::getInstance()->getTileSet()->getTileSize();
    
    map_view.w = v->w / ts;
    map_view.h = v->h / ts;

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
    lockScreen();

    d_nextTurn->endTurn();
}

void Game::center_view(Vector<int> p)
{
    p = clip(Vector<int>(0, 0), p - map_view.dim / 2,
	     GameMap::get_dim() - map_view.dim);
    
    map_view.x = p.x;
    map_view.y = p.y;

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

    StackInfo s;
    if (stack)
    {
	for (Stack::iterator i = stack->begin(), end = stack->end(); i != end; ++i)
	{
	    s.armies.push_back(*i);
	}
    }
    
    stack_info_changed.emit(s);
}

void Game::clear_stack_info()
{
    StackInfo s;
    stack_info_changed.emit(s);
}


void Game::connectEvents()
{
    std::list<Event*> elist = d_gameScenario->getEventlist();
    ERound* eround;
    ENextTurn* eturn;

    // first, connect the static signals appropriately
    RUpdate::supdating.connect(sigc::mem_fun(*this, &Game::redraw));
    RCenter::scentering.connect(sigc::mem_fun(*this, &Game::center_view));
    RCenterObj::scentering.connect(sigc::mem_fun(*this, &Game::center_view));
    RRaiseEvent::sgettingEvents.connect(sigc::mem_fun(*d_gameScenario,
                                        &GameScenario::getEventlist));
    RActEvent::sgettingEvents.connect(sigc::mem_fun(*d_gameScenario,
                                        &GameScenario::getEventlist));
    RWinGame::swinDialog.connect(sigc::mem_fun(*this, &Game::gameFinished));


    // and connect some of the events
    for (std::list<Event*>::iterator it = elist.begin(); it != elist.end(); it++)
    {
        switch((*it)->getType())
        {
            //the round event needs rather much help with its signals
            case Event::ROUND:
                eround = dynamic_cast<ERound*>(*it);
                eround->sgettingRound.connect(sigc::mem_fun(*d_gameScenario,
                                                        &GameScenario::getRound));
                d_nextTurn->snextTurn.connect(sigc::mem_fun(*eround, &ERound::trigger));
                break;

            case Event::NEXTTURN:
                eturn = dynamic_cast<ENextTurn*>(*it);
                d_nextTurn->snextTurn.connect(sigc::mem_fun(*eturn, &ENextTurn::trigger));
                break;
                
            default:
                continue;
        }
    }
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

// pop up message window for next player
void Game::pictureNextPlayer()
{
    debug("pictureNextPlayer()");

#if 0
    // create and run the next player popup
    char buf[101]; buf[100] = '\0';
    snprintf(buf, 100, _("Next Player: %s"),
             Playerlist::getInstance()->getActiveplayer()->getName().c_str());
    
    Popup* nextPlayer = new Popup(this, Rectangle((my_width-300)/2,(my_height-200)/2, 300, 200));
    new PG_Label(nextPlayer, Rectangle(10, 10, 300, 20), buf);
    
    nextPlayer->SetIcon(d_pic_turn_start);
    nextPlayer->Show();
    nextPlayer->RunModal();
    
    delete nextPlayer;
#endif
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
    std::vector<unsigned int> movedid;
    std::vector<unsigned int>::iterator it;
    Player *player = Playerlist::getActiveplayer();
    Stacklist* sl = player->getStacklist();
    Stack *orig = sl->getActivestack();

    for (Stacklist::iterator i = sl->begin(), end = sl->end(); i != end; ++i)
    {
	Stack &s = **i;
	if (s.canMove() && s.getPath()->size() > 0)
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

    if (ruin && !ruin->isSearched())
    {
        int cur_gold = player->getGold();

#if 0
        snprintf(buf,sizeof(buf), _("%s encounters a %s..."),
             stack->getFirstHero()->getName().c_str(), 
             ruin->getOccupant()->getStrongestArmy()->getName().c_str());
        PG_MessageBox mb(this, 
                         PG_Rect((my_width-w1)/2, (my_height-h1)/2, w1, h1), 
                         _("Searching..."), buf,
                         PG_Rect(80, 70, 80, 30), _("OK"));
#endif
	
        if (!(player->stackSearchRuin(stack, ruin)))
        {
            stackRedraw();
            return;
        }

        stackRedraw();
        // this also includes the gold-only hack
        int gold_added = player->getGold() - cur_gold;

	ruin_searched.emit(ruin, gold_added);
	
        update_sidebar_stats();
    }
    else if (temple && temple->searchable())
    {
        player->stackVisitTemple(stack, temple);
	bool wants_quest = temple_visited.emit(temple);
        if (wants_quest)
        {
            Quest *q = player->stackGetQuest(stack, temple);
            Hero* hero = dynamic_cast<Hero*>(stack->getFirstHero());

            if (q)
                for (Stack::iterator it = stack->begin(); it != stack->end(); it++)
                    if ((*it)->getId() == q->getHeroId())
                        hero = dynamic_cast<Hero*>(*it);

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
    debug("NEWLEVEL Dialog SHOW")

    // Don't show this dialog if computer or enemy's armies advance
    if ((a->getPlayer()->getType() != Player::HUMAN) ||
        (a->getPlayer() != Playerlist::getInstance()->getActiveplayer()))
        return Army::STRENGTH;

#if 0
    ArmyLevelDialog dialog(a, 0, Rectangle(200, 100, 430, 260));
    dialog.Show();
    dialog.RunModal();
    dialog.Hide();

#if 0
    d_stackinfo->Redraw();
#endif
    bigmap->redraw();

    return dialog.getResult();
#endif
    return Army::STRENGTH;
}

void Game::newMedalArmy(Army* a)
{
    // We don't want to have medal awards of computer players displayed
    if (!a->getPlayer() || (a->getPlayer()->getType() != Player::HUMAN) ||
        (a->getPlayer() != Playerlist::getInstance()->getActiveplayer()))
        return;
    
    debug("NEWMedal Dialog SHOW")
    std::cerr << "NEWMedal Dialog SHOW" << std::endl;
#if 0
    ArmyMedalDialog dialog(a, 0, Rectangle(200, 100, 230, 230));
    dialog.Show();
    dialog.RunModal();
    dialog.Hide();
 
    d_stackinfo->Redraw();
#endif
}

void Game::gameFinished()
{
    // timers would interfere with the dialog, so stop them
    stopTimers();

#if 0
    // show a nice dialog in the center of the screen
    
    Rectangle r;
    r.w = d_pic_winGame->w;
    r.h = d_pic_winGame->h;
    r.x = my_width/2 - r.w/2;
    r.y = my_height/2 - r.h/2;
    
    Popup* win = new Popup(this, r);
    SDL_Surface* winpic = GraphicsCache::getInstance()->applyMask(
                                        d_pic_winGame, d_pic_winGameMask,
                                        Playerlist::getActiveplayer());
    win->SetIcon(winpic);
    
    win->Show();
    win->RunModal();

    delete win;
    SDL_FreeSurface(winpic);
#endif
    // We don't restart the timers, the game should be over now.
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

void Game::on_city_selected(City* c, MapTipPosition pos)
{
    if (c)
    {
	if (c->isFriend(Playerlist::getActiveplayer()) && !c->isBurnt())
	{
	    city_visited.emit(c);

	    // some visible city properties (defense level) may have changed
	    bigmap->draw();
	}
	else
	{
	    Glib::ustring str;

	    if (c->isCapital())
		str = String::ucompose(_("%1 (capital)"), c->getName());
	    else
		str = c->getName();

	    str += "\n";
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
	    str += _("Searched");
	else
	    // note to translators: whether a ruin has been searched
	    str += _("Not searched");
	    
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

void Game::invading_city(City* city)
{
    // if a computer makes it's turn and occupied a city, we shouldn't
    // show a modal dialog :)

    if (!d_lock)
    {
        bigmap->draw();
	CityDefeatedAction a = city_defeated.emit(city);
	Player *player = Playerlist::getInstance()->getActiveplayer();

	int gold;
	switch (a) {
	case CITY_DEFEATED_OCCUPY:
	    player->cityOccupy(city);
	    break;
	    
	case CITY_DEFEATED_RAZE:
	    player->cityRaze(city);
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
    update_control_panel();
}

void Game::lockScreen()
{
    debug("lockScreen()");

    //don't accept modifying user input from now on
    bigmap->setEnable(false);
    d_lock = true;
    stopTimers();   //on computer turns, strange things may happen if the timers
                    //are still active
}

void Game::unlockScreen()
{
    debug("unlockScreen()");
    bigmap->setEnable(true);
    d_lock = false;
    startTimers();
}

void Game::stopTimers()
{
#if 0
    bigmap->interruptTimer();
    d_smallmap->interruptTimer();
#endif
}

void Game::startTimers()
{
    //never start timers with the computer player's turn
    if (d_lock)
        return;
#if 0
    bigmap->restartTimer();
    d_smallmap->restartTimer();
#endif
}

void Game::update_control_panel()
{
#if 0 // FIXME
    if (d_lock)
        return;
#endif

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
	
        if (stack->hasHero())
        {
            Ruin *ruin
		= Ruinlist::getInstance()->getObjectAt(stack->getPos());
            Temple *temple
		= Templelist::getInstance()->getObjectAt(stack->getPos());

	    can_search_selected_stack.emit(
		(ruin && !ruin->isSearched()) || temple);
        }
    }
    else
    {
	can_search_selected_stack.emit(false);
	can_move_selected_stack.emit(false);
    }
}

void Game::startGame()
{
    debug ("start_game()");
    lockScreen();

    d_nextTurn->start();
    update_control_panel();
}

void Game::loadGame()
{
    Player *player = Playerlist::getActiveplayer();
    if (player->getType() == Player::HUMAN)
    {
	char buf[101];
	buf[100] = '\0';
        //human players want access to the controls and an info box
        unlockScreen();
        bigmap->unselect_active_stack();
	clear_stack_info();
        center_view_on_city();
        update_sidebar_stats();
        update_control_panel();

#if 0
	snprintf(buf, 100, _("%s, your turn continues."), 
		Playerlist::getInstance()->getActiveplayer()->getName().c_str());
        PG_MessageBox mb(this, Rectangle(my_width/2-100, my_height/2-87, 200, 150), _("Load Game"),
                buf, Rectangle(60, 110, 80, 30), _("OK"));
        mb.Show();
        mb.RunModal();
        mb.Hide();
#endif
    }       
    else
    {
        lockScreen();
        update_sidebar_stats();
        player->startTurn();
        d_nextTurn->endTurn();
        update_control_panel();
    }
}

void Game::stopGame()
{
    stopTimers();
    d_nextTurn->stop();
    d_gameScenario->deactivateEvents();
    Playerlist::finish();
}

bool Game::init_turn_for_player(Player* p)
{
    // FIXME: Currently this function only checks for a human player. You
    // can also have it check for e.g. escape key pressed to interrupt
    // an AI-only game to save/quit.

    if (p->getType() == Player::HUMAN)
    {
	unlockScreen();
    
	if (Stack* stack = p->getActivestack())
	    center_view(stack->getPos());
	else
	    center_view_on_city();
    
	update_sidebar_stats();
	update_stack_info();
	update_control_panel();
    
	if (Configuration::s_showNextPlayer)
	{
	    pictureNextPlayer();
	}

        return true;
    }
    else
    {
	center_view_on_city();
	SDL_Delay(250);
	
	return false;
    }
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
