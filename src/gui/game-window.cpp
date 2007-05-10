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

#include <SDL_video.h>

#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/hide.h>

#include <libglademm/xml.h>
#include <gtkmm/widget.h>
#include <gtkmm/menuitem.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/box.h>
#include <gtkmm/progressbar.h>
//#include <gdkmm/cursor.h>
#include <gtkmm/frame.h>
#include <gtkmm/dialog.h>
#include <gtkmm/stock.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/entry.h>

#include "game-window.h"

#include "gtksdl.h"
#include "glade-helpers.h"
#include "image-helpers.h"
#include "error-utils.h"

#include "fight-window.h"
#include "city-window.h"

#include "../ucompose.hpp"
#include "../defs.h"
#include "../sound.h"
#include "../tmp.h"
#include "../File.h"
#include "../game.h"
#include "../GameScenario.h"
#include "../army.h"
#include "../ruin.h"
#include "../input-events.h"
#include "../player.h"
#include "../stacklist.h"
#include "../playerlist.h"
#include "../hero.h"
#include "../temple.h"
#include "../city.h"
#include "../Quest.h"
#include "../QuestsManager.h"
#include "../stack.h"


GameWindow::GameWindow()
{
    sdl_inited = false;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/game-window.glade");

    Gtk::Window *w = 0;
    xml->get_widget("window", w);
    window.reset(w);

    w->signal_delete_event().connect(
	sigc::mem_fun(*this, &GameWindow::on_delete_event));

    xml->get_widget("sdl_container", sdl_container);
    xml->get_widget("stack_info_box", stack_info_box);

    // the map image
    xml->get_widget("map_image", map_image);
    //map_image->property_file() = std::string("gui/example-map.png");
    Gtk::EventBox *map_eventbox;
    xml->get_widget("map_eventbox", map_eventbox);
    map_eventbox->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
			     Gdk::POINTER_MOTION_MASK);
    map_eventbox->signal_button_press_event().connect(
	sigc::mem_fun(*this, &GameWindow::on_map_mouse_button_event));
    map_eventbox->signal_button_release_event().connect(
	sigc::mem_fun(*this, &GameWindow::on_map_mouse_button_event));
    map_eventbox->signal_motion_notify_event().connect(
	sigc::mem_fun(*this, &GameWindow::on_map_mouse_motion_event));

    
    xml->get_widget("stats_label", stats_label);
    stats_text = stats_label->get_text();

    // the control panel
    xml->get_widget("prev_button", prev_button);
    xml->get_widget("next_button", next_button);
    xml->get_widget("next_movable_button", next_movable_button);
    xml->get_widget("center_button", center_button);
    xml->get_widget("defend_button", defend_button);
    xml->get_widget("search_button", search_button);
    xml->get_widget("move_button", move_button);
    xml->get_widget("move_all_button", move_all_button);
    xml->get_widget("end_turn_button", end_turn_button);

    // fill in imagery
    std::vector<Glib::RefPtr<Gdk::Pixbuf> > button_images
	= disassemble_row(File::getMiscFile("various/buttons.png"), 11);
    prev_button->add(*manage(new Gtk::Image(button_images[0])));
    next_button->add(*manage(new Gtk::Image(button_images[1])));
    next_movable_button->add(*manage(new Gtk::Image(button_images[2])));
    center_button->add(*manage(new Gtk::Image(button_images[5])));
    defend_button->add(*manage(new Gtk::Image(button_images[6])));
    search_button->add(*manage(new Gtk::Image(button_images[9])));
    move_button->add(*manage(new Gtk::Image(button_images[3])));
    move_all_button->add(*manage(new Gtk::Image(button_images[4])));
    end_turn_button->add(*manage(new Gtk::Image(button_images[10])));
    
    // connect callbacks for the menu
    xml->connect_clicked("load_game_menuitem",
			 sigc::mem_fun(*this, &GameWindow::on_load_game_activated));
    xml->connect_clicked("save_game_menuitem",
			 sigc::mem_fun(*this, &GameWindow::on_save_game_activated));
    xml->connect_clicked("save_game_as_menuitem", 
			 sigc::mem_fun(*this, &GameWindow::on_save_game_as_activated));
    xml->connect_clicked("resign_game_menuitem", 
			 sigc::mem_fun(*this, &GameWindow::on_resign_game_activated));
    xml->connect_clicked("quit_menuitem", 
			 sigc::mem_fun(*this, &GameWindow::on_quit_activated));
}

GameWindow::~GameWindow()
{
    clear_army_buttons();
}

void GameWindow::show()
{
    window->show_all();
}

void GameWindow::hide()
{
    window->hide();
}

namespace 
{
    void surface_attached_helper(GtkSDL *gtksdl, gpointer data)
    {
	static_cast<GameWindow *>(data)->on_sdl_surface_changed();
    }
}

void GameWindow::init(int width, int height)
{
    sdl_widget
	= Gtk::manage(Glib::wrap(gtk_sdl_new(width, height, 0, SDL_SWSURFACE)));

    sdl_widget->set_flags(Gtk::CAN_FOCUS);

    sdl_widget->grab_focus();
    sdl_widget->add_events(Gdk::KEY_PRESS_MASK |
		  Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
	          Gdk::POINTER_MOTION_MASK);

    sdl_widget->signal_button_press_event().connect(
	sigc::mem_fun(*this, &GameWindow::on_sdl_mouse_button_event));
    sdl_widget->signal_button_release_event().connect(
	sigc::mem_fun(*this, &GameWindow::on_sdl_mouse_button_event));
    sdl_widget->signal_motion_notify_event().connect(
	sigc::mem_fun(*this, &GameWindow::on_sdl_mouse_motion_event));
    sdl_widget->signal_key_press_event().connect(
	sigc::mem_fun(*this, &GameWindow::on_sdl_key_event));
    
    // connect to the special signal that signifies that a new surface has been
    // generated and attached to the widget
    g_signal_connect(G_OBJECT(sdl_widget->gobj()), "surface-attached",
		     G_CALLBACK(surface_attached_helper), this);
    
    sdl_container->add(*sdl_widget);
}

void GameWindow::new_game(GameParameters g)
{
    if (g.map_path.empty()) {
	// construct new random scenario if we're not going to load the game
	std::string path = create_and_dump_scenario("random.map", g);
	g.map_path = path;
    }
    
    load_game(g.map_path, true);
}

namespace 
{
    // helper for connecting control panel buttons
    void setup_button(Gtk::Button *button,
		      sigc::slot<void> slot,
		      sigc::signal<void, bool> &game_signal)
    {
	button->signal_clicked().connect(slot);
	game_signal.connect(sigc::mem_fun(button, &Gtk::Widget::set_sensitive));
    }
}

void GameWindow::load_game(const std::string &file_path, bool start)
{
    bool broken;
    GameScenario* game_scenario = new GameScenario(file_path, broken, true);
    
    if (broken)
	show_fatal_error(_("Map was broken when re-reading. Exiting..."));

    // set up misc stuff
#if 0
    RWinGame::swinning.connect(sigc::slot((*this), &Splash::gameFinished));
    RLoseGame::slosing.connect(sigc::slot((*this), &Splash::gameFinished));
#endif
    Sound::getInstance()->haltMusic(false);
    Sound::getInstance()->enableBackground();

    game.reset(new Game(game_scenario));

    // connect signals to and from control panel buttons
    setup_button(prev_button,
		 sigc::mem_fun(game.get(), &Game::select_prev_stack),
		 game->can_select_prev_stack);
    setup_button(next_button,
		 sigc::mem_fun(game.get(), &Game::select_next_stack),
		 game->can_select_next_stack);
    setup_button(next_movable_button,
		 sigc::mem_fun(game.get(), &Game::select_next_movable_stack),
		 game->can_select_next_movable_stack);
    setup_button(center_button,
		 sigc::mem_fun(game.get(), &Game::center_selected_stack),
		 game->can_center_selected_stack);
    setup_button(defend_button,
		 sigc::mem_fun(game.get(), &Game::defend_selected_stack),
		 game->can_defend_selected_stack);
    setup_button(search_button,
		 sigc::mem_fun(game.get(), &Game::search_selected_stack),
		 game->can_search_selected_stack);
    setup_button(move_button,
		 sigc::mem_fun(game.get(), &Game::move_selected_stack),
		 game->can_move_selected_stack);
    setup_button(move_all_button,
		 sigc::mem_fun(game.get(), &Game::move_all_stacks),
		 game->can_move_all_stacks);
    setup_button(end_turn_button,
		 sigc::mem_fun(game.get(), &Game::end_turn),
		 game->can_end_turn);

    // setup game callbacks
    //game->game_over.connect(sigc::mem_fun(*this, ))
    game->sidebar_stats_changed.connect(
	sigc::mem_fun(*this, &GameWindow::on_sidebar_stats_changed));
    game->smallmap_changed.connect(
	sigc::mem_fun(*this, &GameWindow::on_smallmap_changed));
    game->stack_info_changed.connect(
	sigc::mem_fun(*this, &GameWindow::on_stack_info_changed));
    game->map_tip_changed.connect(
	sigc::mem_fun(*this, &GameWindow::on_map_tip_changed));
    game->ruin_searched.connect(
	sigc::mem_fun(*this, &GameWindow::on_ruin_searched));
    game->fight_started.connect(
	sigc::mem_fun(*this, &GameWindow::on_fight_started));
    game->ruinfight_started.connect(
	sigc::mem_fun(*this, &GameWindow::on_ruinfight_started));
    game->ruinfight_finished.connect(
	sigc::mem_fun(*this, &GameWindow::on_ruinfight_finished));
    game->hero_offers_service.connect(
	sigc::mem_fun(*this, &GameWindow::on_hero_offers_service));
    game->temple_visited.connect(
	sigc::mem_fun(*this, &GameWindow::on_temple_visited));
    game->quest_assigned.connect(
	sigc::mem_fun(*this, &GameWindow::on_quest_assigned));
    game->city_defeated.connect(
	sigc::mem_fun(*this, &GameWindow::on_city_defeated));
    game->city_pillaged.connect(
	sigc::mem_fun(*this, &GameWindow::on_city_pillaged));
    game->city_sacked.connect(
	sigc::mem_fun(*this, &GameWindow::on_city_sacked));
    game->city_visited.connect(
	sigc::mem_fun(*this, &GameWindow::on_city_visited));

    
    game->startTimers();
    if (start)
      game->startGame();
    else
      game->loadGame();
}

bool GameWindow::on_delete_event(GdkEventAny *e)
{
    on_resign_game_activated();
    
    return true;
}

namespace 
{
    MouseButtonEvent to_input_event(GdkEventButton *e)
    {
	MouseButtonEvent m;
	m.pos = make_vector(int(e->x), int(e->y));

	if (e->button == 1)
	    m.button = MouseButtonEvent::LEFT_BUTTON;
	else if (e->button == 3)
	    m.button = MouseButtonEvent::RIGHT_BUTTON;
	else
	    m.button = MouseButtonEvent::MIDDLE_BUTTON;

	if (e->type == GDK_BUTTON_PRESS)
	    m.state = MouseButtonEvent::PRESSED;
	else if (e->type == GDK_BUTTON_RELEASE)
	    m.state = MouseButtonEvent::RELEASED;
	
	return m;
    }

    MouseMotionEvent to_input_event(GdkEventMotion *e)
    {
	MouseMotionEvent m;
	m.pos = make_vector(int(e->x), int(e->y));
	
	m.pressed[MouseMotionEvent::LEFT_BUTTON] = e->state & GDK_BUTTON1_MASK;
	m.pressed[MouseMotionEvent::MIDDLE_BUTTON] = e->state & GDK_BUTTON2_MASK;
	m.pressed[MouseMotionEvent::RIGHT_BUTTON] = e->state & GDK_BUTTON3_MASK;
	    
	return m;
    }
}


bool GameWindow::on_sdl_mouse_button_event(GdkEventButton *e)
{
    if (e->type != GDK_BUTTON_PRESS && e->type != GDK_BUTTON_RELEASE)
	return true;	// useless event

    if (game.get())
	game->mouse_button_event(to_input_event(e));
    
    return true;
}

bool GameWindow::on_sdl_mouse_motion_event(GdkEventMotion *e)
{
    if (game.get())
	game->mouse_motion_event(to_input_event(e));
    
    return true;
}

bool GameWindow::on_sdl_key_event(GdkEventKey *e)
{
    if (game.get()) {
	KeyPressEvent k;
	// FIXME: fill in
	game->key_press_event(k);
    }
    
    return true;
}

bool GameWindow::on_map_mouse_button_event(GdkEventButton *e)
{
    if (e->type != GDK_BUTTON_PRESS && e->type != GDK_BUTTON_RELEASE)
	return true;	// useless event
    
    if (game.get())
	game->smallmap_mouse_button_event(to_input_event(e));
    
    return true;
}

bool GameWindow::on_map_mouse_motion_event(GdkEventMotion *e)
{
    if (game.get())
	game->smallmap_mouse_motion_event(to_input_event(e));
    
    return true;
}

void GameWindow::on_sdl_surface_changed()
{
    if (!sdl_inited) {
	sdl_inited = true;
	sdl_initialized.emit();
    }

    if (game.get()) {
	game->size_changed();
	game->redraw();
    }
}

void GameWindow::on_load_game_activated()
{
}

void GameWindow::on_save_game_activated()
{
}

void GameWindow::on_save_game_as_activated()
{
}

void GameWindow::on_resign_game_activated()
{
    // FIXME: ask
    bool end = true;

    if (end) {
	if (game.get())
	{
	    game->stopGame();
	    game->stopTimers();
	    game.reset();
	}
	Sound::getInstance()->disableBackground();
	
	game_ended.emit();
    }
}

void GameWindow::on_quit_activated()
{
    // FIXME: quick hack
    on_resign_game_activated();
    quit_requested.emit();
}

void GameWindow::on_army_toggled(Gtk::ToggleButton *toggle, Army *army)
{
    army->setGrouped(toggle->get_active());
    ensure_one_army_button_active();
    set_army_button_tooltip(toggle);
}

void GameWindow::on_army_button_has_size()
{
    // fix height to prevent flickering
    stack_info_box->property_height_request() = stack_info_box->get_height();
}

void GameWindow::clear_army_buttons()
{
    for (army_buttons_type::iterator i = army_buttons.begin(),
	     end = army_buttons.end(); i != end; ++i)
	delete *i;
    army_buttons.clear();
}

void GameWindow::ensure_one_army_button_active()
{
    if (army_buttons.empty())
	return;
    
    // determine number of active buttons
    int sum = 0;
    for (army_buttons_type::iterator i = army_buttons.begin(),
	     end = army_buttons.end(); i != end; ++i)
	if ((*i)->get_active())
	    ++sum;

    if (sum == 0)
    {
	// must have at least one active, so pick the first
	army_buttons.front()->set_active();
	sum = 1;
    }
}

void GameWindow::set_army_button_tooltip(Gtk::ToggleButton *toggle)
{
    Glib::ustring tip;
    
    if (toggle->get_active())
	tip = _("Click to prevent this army from following the stack");
    else
	tip = _("Click to let this army follow the stack");

    tooltips.set_tip(*toggle, tip);
}

void GameWindow::on_sidebar_stats_changed(SidebarStats s)
{
    Glib::ustring n = String::ucompose(
	stats_text, s.name, s.gold, s.income, s.cities, s.units, s.turns);
    
    stats_label->set_text(n);
}

void GameWindow::on_smallmap_changed(SDL_Surface *map)
{
    map_image->property_pixbuf() = to_pixbuf(map);
}

void GameWindow::on_stack_info_changed(StackInfo s)
{
    clear_army_buttons();

    for (StackInfo::armies_type::iterator i = s.armies.begin(),
	     end = s.armies.end(); i != end; ++i)
    {
	// construct a toggle button
	Army *army = *i;
	Gtk::VBox *toggle_box = manage(new Gtk::VBox);
	
	// image
	toggle_box->add(*manage(new Gtk::Image(to_pixbuf(army->getPixmap()))));
	// hit points graph
	Gtk::ProgressBar *progress = manage(new Gtk::ProgressBar);
	progress->set_fraction(double(army->getHP()) / army->getStat(Army::HP));
	progress->property_width_request() = army->getPixmap()->w;
	progress->property_height_request() = 12;
	toggle_box->pack_start(*progress, Gtk::PACK_SHRINK, 4);
	// number of moves
	Glib::ustring moves_str = String::ucompose("%1", army->getMoves());
	toggle_box->add(*manage(new Gtk::Label(moves_str,
					       Gtk::ALIGN_CENTER, Gtk::ALIGN_TOP)));

	// the button itself
	Gtk::ToggleButton *toggle = new Gtk::ToggleButton;
	toggle->add(*toggle_box);
	toggle->set_active(army->isGrouped());
	set_army_button_tooltip(toggle);
	toggle->signal_toggled().connect(
	    sigc::bind(sigc::mem_fun(*this, &GameWindow::on_army_toggled),
		       toggle, army));
	toggle->signal_size_allocate().connect_notify(
	    sigc::hide(sigc::mem_fun(*this, &GameWindow::on_army_button_has_size)));
	
	// add it
	stack_info_box->pack_start(*toggle, Gtk::PACK_SHRINK);
	army_buttons.push_back(toggle);

    }
    ensure_one_army_button_active();
    stack_info_box->show_all();
}

void GameWindow::on_map_tip_changed(Glib::ustring tip, MapTipPosition pos)
{
    if (tip.empty())
	hide_map_tip();
    else
	show_map_tip(tip, pos);
}

void GameWindow::show_map_tip(Glib::ustring msg, MapTipPosition pos)
{
    // init the map tip
    map_tip.reset(new Gtk::Window(Gtk::WINDOW_POPUP));
    Gtk::Label *l = manage(new Gtk::Label);
    l->set_padding(6, 6);
    l->set_text(msg);

    Gtk::Frame *f = manage(new Gtk::Frame);
    f->property_shadow_type() = Gtk::SHADOW_ETCHED_OUT;
    f->add(*l);

    map_tip->add(*f);
    f->show_all();
    
    // get screen position
    Vector<int> p;
    sdl_widget->get_window()->get_origin(p.x, p.y);
    p += pos.pos;

    Vector<int> size(0, 0);
    map_tip->get_size(size.x, size.y);
    
    switch (pos.justification)
    {
    case MapTipPosition::LEFT:
	map_tip->set_gravity(Gdk::GRAVITY_NORTH_WEST);
	break;
    case MapTipPosition::RIGHT:
	map_tip->set_gravity(Gdk::GRAVITY_NORTH_EAST);
	p.x -= size.x;
	break;
    case MapTipPosition::TOP:
	map_tip->set_gravity(Gdk::GRAVITY_NORTH_WEST);
	break;
    case MapTipPosition::BOTTOM:
	map_tip->set_gravity(Gdk::GRAVITY_SOUTH_WEST);
	p.y -= size.y;
	break;
    }

    // and action
    map_tip->move(p.x, p.y);
    map_tip->show();
}

void GameWindow::hide_map_tip()
{
    map_tip.reset();
}

void GameWindow::on_ruin_searched(Ruin *ruin, int gold_found)
{
    std::auto_ptr<Gtk::Dialog> dialog;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/ruin-searched-dialog.glade");

	
    Gtk::Dialog *d;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    dialog->set_transient_for(*window.get());
    
    dialog->set_title(ruin->getName());

    Gtk::Image *image;
    xml->get_widget("ruin_image", image);
    image->property_file() = File::getMiscFile("various/ruin_1.jpg");

    Gtk::Label *label;
    xml->get_widget("label", label);
    
    Glib::ustring s = label->get_text();
    s += "\n\n";
    s += String::ucompose(ngettext("The loot is worth %1 gold piece.",
				   "The loot is worth %1 gold pieces.",
				   gold_found), gold_found);
    label->set_text(s);
    
    dialog->show_all();
    dialog->run();
}

void GameWindow::on_ruinfight_started(Stack *attackers, Stack *defenders)
{
//so and so encounters a wolf...
    std::auto_ptr<Gtk::Dialog> dialog;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/ruinfight-started-dialog.glade");
	
    Gtk::Dialog *d;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    dialog->set_transient_for(*window.get());
    
    dialog->set_title(_("Searching"));

    Gtk::Label *label;
    xml->get_widget("label", label);
    Glib::ustring s = label->get_text();
    s = "\n\n";
    s += attackers->getFirstHero()->getName() + " encounters a ";
    s += defenders->getStrongestArmy()->getName() + "...";
    label->set_text(s);

    dialog->show_all();
    dialog->run();
}
void GameWindow::on_ruinfight_finished(Fight::Result result)
{
    std::auto_ptr<Gtk::Dialog> dialog;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/ruinfight-finished-dialog.glade");
	
    Gtk::Dialog *d;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    dialog->set_transient_for(*window.get());
    
    if (result == Fight::ATTACKER_WON)
      dialog->set_title(_("Hero Victorious"));
    else
      dialog->set_title(_("Hero Defeated"));

    Gtk::Label *label;
    xml->get_widget("label", label);
    Glib::ustring s = label->get_text();
    s = "\n\n";
    if (result == Fight::ATTACKER_WON)
      s += _("...and is victorious!");
    else
      s += _("...and is slain by it!");
    label->set_text(s);

    dialog->show_all();
    dialog->run();
}

void GameWindow::on_fight_started(Fight &fight)
{
    FightWindow d(fight);

    d.set_parent_window(*window.get());
    d.run();
}

bool GameWindow::on_hero_offers_service(Player *player, Hero *hero, City *city, int gold)
{
    std::auto_ptr<Gtk::Dialog> dialog;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/hero-offer-dialog.glade");

	
    Gtk::Dialog *d;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    dialog->set_transient_for(*window.get());
    
    dialog->set_title(String::ucompose(_("Hero offer for %1"),
				       player->getName()));

    Gtk::RadioButton *radio;
    Gtk::Image *image;
    xml->get_widget("hero_image", image);
    if (hero->getGender() == Army::MALE)
      {
        image->property_file() = File::getMiscFile("various/recruit_male.png");
        xml->get_widget("hero_male", radio);
      }
    else
      {
        image->property_file() = File::getMiscFile("various/recruit_female.png");
        xml->get_widget("hero_female", radio);
      }
    radio->set_active(true);
    
    Gtk::Entry *entry;
    xml->get_widget("name", entry);
    entry->set_text(hero->getName());

    Gtk::Label *label;
    xml->get_widget("label", label);
    
    Glib::ustring s;
    if (gold > 0)
	s = String::ucompose(
	    ngettext("A hero in %2 wants to join you for %1 gold piece!",
		     "A hero in %2 wants to join you for %1 gold pieces!",
		     gold), gold, city->getName().c_str());
    else
	s = String::ucompose(_("A hero in %1 wants to join you!"), city->getName().c_str());
    label->set_text(s);
    
    Sound::getInstance()->playMusic("hero", 1);
    dialog->show_all();
    int response = dialog->run();
    Sound::getInstance()->haltMusic();

    if (response == 0)		// accepted
      {
        hero->setName(entry->get_text());
        xml->get_widget("hero_male", radio);
        if (radio->get_active() == true)
          hero->setGender(Hero::MALE);
        else
          hero->setGender(Hero::FEMALE);
	return true;
      }
    else
	return false;
}


bool GameWindow::on_temple_visited(Temple *temple)
{
    std::auto_ptr<Gtk::Dialog> dialog;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/temple-visit-dialog.glade");
	
    Gtk::Dialog *d;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    dialog->set_transient_for(*window.get());
    
    dialog->set_title(temple->getName());

    dialog->show_all();
    int response = dialog->run();

    if (response == 0)		// accepted a quest
	return true;
    else
	return false;
}

void GameWindow::on_quest_assigned(Hero *hero, Quest *quest)
{
    std::auto_ptr<Gtk::Dialog> dialog;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/quest-assigned-dialog.glade");
	
    Gtk::Dialog *d;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    dialog->set_transient_for(*window.get());
    
    dialog->set_title(String::ucompose(_("Quest - %1"), hero->getName()));

    Gtk::Label *label;
    xml->get_widget("label", label);
    Glib::ustring s;
    if (quest)
	s = quest->getDescription();
    else
	s = _("This hero already has a quest.");
    label->set_text(s);

    dialog->show_all();
    dialog->run();
}

static bool
hero_has_quest_here (Stack *s, City *c, bool *sack, bool *raze)
{
  Player *p = Playerlist::getActiveplayer();
  std::vector<Quest*> questlist;
  std::vector<Hero*> heroes;
  *sack = false;
  *raze = false;

  QuestsManager *q_mgr = QuestsManager::getInstance();
  q_mgr->getPlayerQuests(p, questlist, heroes);
  /* loop over all quests */
  /* for each quest, check the quest type */
  for (std::vector<Quest*>::iterator i = questlist.begin();
       i != questlist.end(); ++i)
    {
      switch ((*i)->getType())
        {
        case Quest::CITYSACK:
        case Quest::CITYRAZE:
          /* now check if the quest's hero is in our stack */
          for (Stack::iterator it = s->begin(); it != s->end(); ++it)
            {
              if ((*it)->isHero())
                {
                  if ((*it)->getId() == (*i)->getHeroId())
                    {
                      /* hey we found one, set the corresponding boolean */
                      if ((*i)->getType() == Quest::CITYSACK)
                        *sack = true;
                      else if ((*i)->getType() == Quest::CITYRAZE)
                        *raze = true;
                    }
                }
            }
          break;
        }
    }
  if ((*raze) || (*sack))
    return true;
  else
    return false;
} 

CityDefeatedAction GameWindow::on_city_defeated(City *city, int gold)
{
    std::auto_ptr<Gtk::Dialog> dialog;
    if (gold)
      on_city_looted (city, gold);
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/city-defeated-dialog.glade");

	
    Gtk::Dialog *d;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    dialog->set_transient_for(*window.get());
    
    Gtk::Image *image;
    xml->get_widget("city_image", image);
    image->property_file() = File::getMiscFile("various/city_occupied.jpg");
    image->show();

    Gtk::Label *label;
    xml->get_widget("label", label);
    Gtk::Requisition req = image->size_request();
    label->set_size_request(req.width);
    
    Glib::ustring name;
    Player *p = Playerlist::getActiveplayer();
    Army *h = p->getStacklist()->getActivestack()->getFirstHero();
    if (h)
	name = h->getName();
    else
	name = p->getName(false);

    Glib::ustring s;
    switch (rand() % 4)
    {
      case 0: s = _("%1, you have triumphed in the battle of %2."); break;
      case 1: s = _("%1, you have claimed victory in the battle of %2."); break;
      case 2: s = _("%1, you have shown no mercy in the battle of %2."); break;
      case 3: s = _("%1, you have slain the foe in the battle of %2."); break;
    }

    s = String::ucompose(s, name, city->getName());
    s += "\n\n";
    s += label->get_text();
    label->set_text(s);

    if (h) /* if there was a hero in the stack */
      {
        bool sack, raze;
        if (hero_has_quest_here (p->getStacklist()->getActivestack(), city, 
                                 &sack, &raze))
          {
            Gtk::Button *button;
            if (sack)
              {
                xml->get_widget("sack_button", button);
                button->set_label(">" + button->get_label() +"<");
              }
            if (raze)
              {
                xml->get_widget("raze_button", button);
                button->set_label(">" + button->get_label() +"<");
              }
          }
      }

    if (city->getNoOfBasicProd() <= 0) {
	Gtk::Button *b;
	xml->get_widget("pillage_button", b);
	b->hide();
    }

    if (city->getNoOfBasicProd() <= 1) {
	Gtk::Button *b;
	xml->get_widget("sack_button", b);
	b->hide();
    }
    
    dialog->show();

    int response = dialog->run();
    switch (response) {
    default:
    case 0: return CITY_DEFEATED_OCCUPY;
    case 1: return CITY_DEFEATED_RAZE;
    case 2: return CITY_DEFEATED_PILLAGE;
    case 3: return CITY_DEFEATED_SACK;
    }
}

void GameWindow::on_city_looted (City *city, int gold)
{
    std::auto_ptr<Gtk::Dialog> dialog;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/city-looted-dialog.glade");
	
    Gtk::Dialog *d;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    dialog->set_transient_for(*window.get());
    
    dialog->set_title(String::ucompose(_("%1 Looted"), city->getName()));

    Gtk::Label *label;
    xml->get_widget("label", label);
    Glib::ustring s = label->get_text();
    s += "\n\n";
    s += String::ucompose(
	ngettext("Your armies loot %1 gold piece.",
		 "Your armies loot %1 gold pieces.", gold), gold);
    label->set_text(s);

    dialog->show_all();
    dialog->run();
}
void GameWindow::on_city_pillaged(City *city, int gold)
{
    std::auto_ptr<Gtk::Dialog> dialog;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/city-pillaged-dialog.glade");
	
    Gtk::Dialog *d;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    dialog->set_transient_for(*window.get());
    
    dialog->set_title(String::ucompose(_("Pillaged %1"), city->getName()));

    Gtk::Label *label;
    xml->get_widget("label", label);
    Glib::ustring s = label->get_text();
    s += "\n\n";
    s += String::ucompose(
	ngettext("The loot is worth %1 gold piece.",
		 "The loot is worth %1 gold pieces.",
		 gold), gold);
    label->set_text(s);

    dialog->show_all();
    dialog->run();
}

void GameWindow::on_city_sacked(City *city, int gold)
{
    std::auto_ptr<Gtk::Dialog> dialog;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/city-sacked-dialog.glade");
	
    Gtk::Dialog *d;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    dialog->set_transient_for(*window.get());
    
    dialog->set_title(String::ucompose(_("Sacked %1"), city->getName()));

    Gtk::Label *label;
    xml->get_widget("label", label);
    Glib::ustring s = label->get_text();
    s += "\n\n";
    s += String::ucompose(
	ngettext("The loot is worth %1 gold piece.",
		 "The loot is worth %1 gold pieces.",
		 gold), gold);
    label->set_text(s);

    dialog->show_all();
    dialog->run();
}

void GameWindow::on_city_visited(City *city)
{
    CityWindow d(city);

    d.set_parent_window(*window.get());
    d.run();
}

