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

#include <iomanip>
#include <queue>
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
#include <gtkmm/entry.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include "game-window.h"

#include "gtksdl.h"
#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "error-utils.h"

#include "fight-window.h"
#include "city-window.h"
#include "army-gains-level-dialog.h"
#include "hero-dialog.h"
#include "hero-offer-dialog.h"

#include "../ucompose.hpp"
#include "../defs.h"
#include "../sound.h"
#include "../tmp.h"
#include "../File.h"
#include "../game.h"
#include "../GameScenario.h"
#include "../army.h"
#include "../ruin.h"
#include "../player.h"
#include "../stacklist.h"
#include "../playerlist.h"
#include "../hero.h"
#include "../temple.h"
#include "../city.h"
#include "../Quest.h"
#include "../QuestsManager.h"
#include "../stack.h"
#include "../GraphicsCache.h"
#include "../events/RWinGame.h"
#include "../events/RLoseGame.h"
#include "../events/RMessage.h"
#include "../QuestsManager.h"
#include "../Quest.h"


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
    xml->get_widget("stats_box", stats_box);
    stack_info_box->hide();
    stats_box->show();

    // the map image
    xml->get_widget("map_image", map_image);
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

    // the stats
    xml->get_widget("stats_label", stats_label);
    stats_text = stats_label->get_text();
    Gtk::Image *image;
    xml->get_widget("cities_stats_image", image);
    image->property_file() = File::getMiscFile("various/items.png");
    xml->get_widget("gold_stats_image", image);
    image->property_file() = File::getMiscFile("various/items.png");
    xml->get_widget("income_stats_image", image);
    image->property_file() = File::getMiscFile("various/items.png");
    
    xml->get_widget("cities_stats_label", cities_stats_label);
    xml->get_widget("gold_stats_label", gold_stats_label);
    xml->get_widget("income_stats_label", income_stats_label);

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
    prev_button->show_all();
    next_button->show_all();
    next_movable_button->show_all();
    center_button->show_all();
    defend_button->show_all();
    search_button->show_all();
    move_button->show_all();
    move_all_button->show_all();
    end_turn_button->show_all();
    
    sdl_container->show_all();
    window->show();
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

    setup_game(g.map_path);
    game->startGame();
}

void GameWindow::load_game(std::string file_path)
{
    setup_game(file_path);
    game->loadGame();
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

void GameWindow::setup_game(std::string file_path)
{
    stop_game();
    
    bool broken;
    GameScenario* game_scenario = new GameScenario(file_path, broken, true);
    
    if (broken)
	// FIXME: we should not die here, but simply return to the splash screen
	show_fatal_error(_("Map was broken when re-reading. Exiting..."));

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
    game->next_player_turn.connect(
	sigc::mem_fun(*this, &GameWindow::on_next_player_turn));
    game->hero_arrives.connect(
	sigc::mem_fun(*this, &GameWindow::on_hero_brings_allies));
    game->medal_awarded_to_army.connect(
	sigc::mem_fun(*this, &GameWindow::on_medal_awarded_to_army));
    game->army_gains_level.connect(
	sigc::mem_fun(*this, &GameWindow::on_army_gains_level));
    game->game_loaded.connect(
	sigc::mem_fun(*this, &GameWindow::on_game_loaded));

    // misc callbacks
    RWinGame::swinning.connect(sigc::mem_fun(this, &GameWindow::on_game_won));
    RLoseGame::slosing.connect(sigc::mem_fun(this, &GameWindow::on_game_lost));
    RMessage::message_requested.connect(sigc::mem_fun(this, &GameWindow::on_message_requested));
    
    QuestsManager *q = QuestsManager::getInstance();
    q->quest_completed.connect(
	sigc::mem_fun(this, &GameWindow::on_quest_completed));
    q->quest_expired.connect(
	sigc::mem_fun(this, &GameWindow::on_quest_expired));
}

bool GameWindow::on_delete_event(GdkEventAny *e)
{
    on_resign_game_activated();
    
    return true;
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
    Gtk::FileChooserDialog chooser(*window.get(), _("Choose Game to Load"));
    Gtk::FileFilter sav_filter;
    sav_filter.add_pattern("*.sav");
    chooser.set_filter(sav_filter);
    chooser.set_current_folder(Configuration::s_savePath);

    chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
    chooser.set_default_response(Gtk::RESPONSE_ACCEPT);
	
    chooser.show_all();
    int res = chooser.run();
    
    if (res == Gtk::RESPONSE_ACCEPT)
    {
	std::string filename = chooser.get_filename();
	chooser.hide();
	load_game(filename);
    }
}

void GameWindow::on_save_game_activated()
{
    if (current_save_filename.empty())
	on_save_game_as_activated();
    else
    {
	if (game.get())
	{
	    bool success = game->saveGame(current_save_filename);
	    if (!success)
		show_error(_("Game was not saved!"));
	}
    }
}

void GameWindow::on_save_game_as_activated()
{
    Gtk::FileChooserDialog chooser(*window.get(), _("Choose a Name"),
				   Gtk::FILE_CHOOSER_ACTION_SAVE);
    Gtk::FileFilter sav_filter;
    sav_filter.add_pattern("*.sav");
    chooser.set_filter(sav_filter);
    chooser.set_current_folder(Configuration::s_savePath);

    chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    chooser.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_ACCEPT);
    chooser.set_default_response(Gtk::RESPONSE_ACCEPT);
    
    chooser.show_all();
    int res = chooser.run();
    
    if (res == Gtk::RESPONSE_ACCEPT)
    {
	std::string filename = chooser.get_filename();
	chooser.hide();

	current_save_filename = filename;

	if (game.get())
	{
	    bool success = game->saveGame(current_save_filename);
	    if (!success)
		show_error(_("Error saving game!"));
	}
    }
}

void GameWindow::on_resign_game_activated()
{
    // FIXME: ask
    bool end = true;

    if (end) {
	stop_game();
	game_ended.emit();
    }
}

void GameWindow::on_quit_activated()
{
    // FIXME: ask
    bool end = true;

    if (end) {
	stop_game();
	quit_requested.emit();
    }
}

void GameWindow::stop_game()
{
    Sound::getInstance()->disableBackground();
    if (game.get())
    {
	game->stopGame();
	game.reset();
	current_save_filename = "";
    }
}

void GameWindow::on_game_won(Uint32 status)
{
    std::auto_ptr<Gtk::Dialog> dialog;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/game-won-dialog.glade");
	
    Gtk::Dialog *d;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    dialog->set_transient_for(*window.get());

    Gtk::Image *image;
    xml->get_widget("image", image);
    
    SDL_Surface *win_unmasked = File::getMiscPicture("win.jpg", false);
    // mask pics need a special format
    SDL_Surface* tmp = File::getMiscPicture("win_mask.png");
    SDL_Surface *mask = SDL_CreateRGBSurface(
	SDL_SWSURFACE, tmp->w, tmp->h, 32, 0xFF000000, 0xFF0000, 0xFF00, 0xFF);
    SDL_SetAlpha(tmp, 0, 0);
    SDL_BlitSurface(tmp, 0, mask, 0);
    SDL_FreeSurface(tmp);
    
    SDL_Surface* win = GraphicsCache::getInstance()->applyMask(
	win_unmasked, mask, Playerlist::getActiveplayer());

    image->property_pixbuf() = to_pixbuf(win);
    dialog->show_all();
    dialog->run();

    SDL_FreeSurface(win);
    SDL_FreeSurface(win_unmasked);
    SDL_FreeSurface(mask);

    stop_game();
    game_ended.emit();
}

void GameWindow::on_game_lost(Uint32 status)
{
    std::auto_ptr<Gtk::Dialog> dialog;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/game-lost-dialog.glade");
	
    Gtk::Dialog *d;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    dialog->set_transient_for(*window.get());
    
    dialog->show_all();
    dialog->run();

    stop_game();
    game_ended.emit();
}

void GameWindow::on_message_requested(std::string msg)
{
    // FIXME: this is a bit crude, maybe beef it up
    Gtk::MessageDialog dialog(*window.get(), msg);
    dialog.show_all();
    dialog.run();
}

void GameWindow::on_army_toggled(Gtk::ToggleButton *toggle, Army *army)
{
    army->setGrouped(toggle->get_active());
    ensure_one_army_button_active();
}

bool GameWindow::on_army_button_event(GdkEventButton *e,
				      Gtk::ToggleButton *toggle, Army *army)
{
    // if a hero is right-clicked, pop up the hero dialog, otherwise show the
    // army info tip
    MouseButtonEvent event = to_input_event(e);
    if (event.button == MouseButtonEvent::RIGHT_BUTTON
	&& event.state == MouseButtonEvent::PRESSED) {

	Hero *hero = dynamic_cast<Hero *>(army);
	if (hero)
	{
	    HeroDialog d(hero, currently_selected_stack->getPos());

	    d.set_parent_window(*window.get());
	    d.run();
	}
	else
	{
	    show_army_info_tip(toggle, army);
	}
	
	return true;
    }
    else if (event.button == MouseButtonEvent::RIGHT_BUTTON
	     && event.state == MouseButtonEvent::RELEASED) {
	army_info_tip.reset();
	return true;
    }
    
    return false;
}

void GameWindow::show_army_info_tip(Gtk::ToggleButton *toggle, Army *army)
{
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/army-info-window.glade");

    Gtk::Window *w = 0;
    xml->get_widget("window", w);
    army_info_tip.reset(w);

    Gtk::Image *army_image;
    xml->get_widget("army_image", army_image);
    army_image->property_pixbuf() = to_pixbuf(army->getPixmap());

    // fill in terrain image
    Gtk::Image *terrain_image;
    xml->get_widget("terrain_image", terrain_image);
    //terrain_image->property_pixbuf() = to_pixbuf(army->getPixmap());
    terrain_image->hide();

    // fill in info
    Gtk::Label *info_label;
    xml->get_widget("info_label", info_label);
    Glib::ustring s;
    s += army->getName();
    s += "\n";
    // note to translators: %1 is melee strength, %2 is ranged strength
    s += String::ucompose(_("Attack: %1"),
			  army->getStat(Army::STRENGTH));
    s += "\n";
    // note to translators: %1 is remaining moves, %2 is total moves
    s += String::ucompose(_("Moves: %1/%2"),
			  army->getMoves(), army->getStat(Army::MOVES));
    s += "\n";
    s += String::ucompose(_("Upkeep: %1"), army->getUpkeep());
    info_label->set_text(s);
    
    // move into correct position
    army_info_tip->get_child()->show();
    Vector<int> p(0, 0);
    toggle->get_window()->get_origin(p.x, p.y);
    if (toggle->has_no_window())
    {
	Gtk::Allocation a = toggle->get_allocation();
	p.x += a.get_x();
	p.y += a.get_y();
    }
    Vector<int> size(0, 0);
    army_info_tip->get_size(size.x, size.y);
    army_info_tip->set_gravity(Gdk::GRAVITY_SOUTH);
    p.y -= size.y + 2;
	
    army_info_tip->move(p.x, p.y);
    army_info_tip->show();
}

void GameWindow::on_army_button_has_size()
{
    // fix height to prevent flickering
    int height = stack_info_box->get_height();
    stack_info_box->property_height_request() = height;
    stats_box->property_height_request() = height;
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

void GameWindow::on_sidebar_stats_changed(SidebarStats s)
{
    Glib::ustring n = String::ucompose(
	stats_text, s.name, s.gold, s.income, s.cities, s.units, s.turns);
    
    stats_label->set_text(n);

    cities_stats_label->set_text(String::ucompose("%1", s.cities));
    gold_stats_label->set_text(String::ucompose("%1", s.gold));
    income_stats_label->set_text(String::ucompose("%1", s.income));
}

void GameWindow::on_smallmap_changed(SDL_Surface *map)
{
    map_image->property_pixbuf() = to_pixbuf(map);
}

void GameWindow::on_stack_info_changed(Stack *s)
{
    clear_army_buttons();

    currently_selected_stack = s;

    if (!s)
 	show_stats();
    else
	show_stack(s);
}

void GameWindow::show_stats()
{
    stack_info_box->hide();
    stats_box->show();
}

void GameWindow::show_stack(Stack *s)
{
    stats_box->hide();

    for (Stack::iterator i = s->begin(), end = s->end(); i != end; ++i)
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
	toggle->signal_toggled().connect(
	    sigc::bind(sigc::mem_fun(*this, &GameWindow::on_army_toggled),
		       toggle, army));
	toggle->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
	toggle->signal_button_press_event().connect(
	    sigc::bind(sigc::mem_fun(*this, &GameWindow::on_army_button_event),
		       toggle, army), false);
	toggle->signal_button_release_event().connect(
	    sigc::bind(sigc::mem_fun(*this, &GameWindow::on_army_button_event),
		       toggle, army), false);
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
    Gtk::Frame *f = manage(new Gtk::Frame);
    f->property_shadow_type() = Gtk::SHADOW_ETCHED_OUT;

    Gtk::Label *l = manage(new Gtk::Label);
    l->set_padding(6, 6);
    l->set_text(msg);
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

void GameWindow::on_hero_brings_allies (int numAllies)
{
    std::auto_ptr<Gtk::Dialog> dialog;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/hero-brings-allies-dialog.glade");
	
    Gtk::Dialog *d;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    dialog->set_transient_for(*window.get());
    
    dialog->set_title(_("Hero brings allies!"));

    Gtk::Label *label;
    xml->get_widget("label", label);
    Glib::ustring s;
    s = String::ucompose(
	ngettext("The hero brings %1 ally!",
		 "The hero brings %1 allies!", numAllies), numAllies);
    label->set_text(s);

    dialog->show_all();
    dialog->run();
}

bool GameWindow::on_hero_offers_service(Player *player, Hero *hero, City *city, int gold)
{
    HeroOfferDialog d(player, hero, city, gold);
    d.set_parent_window(*window.get());
    return d.run();
}


bool GameWindow::on_temple_visited(Temple *temple, int blessCount)
{
    std::auto_ptr<Gtk::Dialog> dialog;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/temple-visit-dialog.glade");
	
    Gtk::Dialog *d;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    dialog->set_transient_for(*window.get());
    
    dialog->set_title(temple->getName());

    Gtk::Label *l;
    xml->get_widget("label", l);

    Glib::ustring s;
    s += String::ucompose(
	ngettext("%1 army has been blessed!",
		 "%1 armies have been blessed!", blessCount), blessCount);
    l->set_text(s);
    l->set_text(l->get_text() + "\n" +
                _("Seek more blessings in far temples!") + "\n\n" +
                _("Do you seek a quest?"));

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

void GameWindow::on_next_player_turn(Player *player, unsigned int turn_number)
{
    std::auto_ptr<Gtk::Dialog> dialog;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/next-player-turn-dialog.glade");
	
    Gtk::Dialog *d;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    dialog->set_transient_for(*window.get());

    Gtk::Image *image;
    xml->get_widget("image", image);
    image->property_file() = File::getMiscFile("various/ship.jpg");
    
    Gtk::Label *label;
    xml->get_widget("label", label);
    Glib::ustring s = String::ucompose(_("%1\nTurn %2"), player->getName(), 
                                       turn_number);
    label->set_text(s);

    dialog->show_all();
    dialog->run();
}

void GameWindow::on_medal_awarded_to_army(Army *army)
{
    std::auto_ptr<Gtk::Dialog> dialog;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/medal-awarded-dialog.glade");
	
    Gtk::Dialog *d;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    dialog->set_transient_for(*window.get());

    Gtk::Image *image;
    xml->get_widget("image", image);
    image->property_pixbuf() = to_pixbuf(army->getPixmap());
    
    Gtk::Label *label;
    xml->get_widget("label", label);
    Glib::ustring s;
    s += String::ucompose(_("%1 is awarded a medal!"), army->getName());
    s += "\n\n";
    s += String::ucompose(_("Experience: %1"), std::setprecision(2), army->getXP());
    s += "\n";
    s += String::ucompose(_("Level: %1"), army->getLevel());
    label->set_text(s);

    dialog->show_all();
    dialog->run();
}

Army::Stat GameWindow::on_army_gains_level(Army *army)
{
    ArmyGainsLevelDialog d(army);

    d.set_parent_window(*window.get());
    d.run();
    
    return d.get_selected_stat();
}

void GameWindow::on_game_loaded(Player *player)
{
    std::auto_ptr<Gtk::Dialog> dialog;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/game-loaded-dialog.glade");
	
    Gtk::Dialog *d;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    dialog->set_transient_for(*window.get());

    Gtk::Label *label;
    xml->get_widget("label", label);
    Glib::ustring s;
    s += String::ucompose(_("%1, your turn continues."), player->getName());
    label->set_text(s);

    dialog->show_all();
    dialog->run();
}

void GameWindow::on_quest_completed(Quest *quest, int gold)
{
    std::auto_ptr<Gtk::Dialog> dialog;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/quest-completed-dialog.glade");
	
    Gtk::Dialog *d;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    dialog->set_transient_for(*window.get());

    Gtk::Label *label;
    xml->get_widget("label", label);
    Glib::ustring s;
    s += String::ucompose(_("%1 completed the quest!"),
			  quest->getHero()->getName());
    s += "\n\n";

    // add messages from the quest
    std::queue<std::string> msgs;
    quest->getSuccessMsg(msgs);
    while (!msgs.empty())
    {
        s += msgs.front();
        s += "\n\n";
	msgs.pop();
    }

    s += String::ucompose(
	ngettext("You have been rewarded with %1 gold piece.",
		 "You have been rewarded with %1 gold pieces.",
		 gold), gold);

    label->set_text(s);

    dialog->show_all();
    dialog->run();
}

void GameWindow::on_quest_expired(Quest *quest)
{
    std::auto_ptr<Gtk::Dialog> dialog;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/quest-expired-dialog.glade");
	
    Gtk::Dialog *d;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    dialog->set_transient_for(*window.get());

    Gtk::Label *label;
    xml->get_widget("label", label);
    Glib::ustring s;
    s += String::ucompose(_("%1 did not complete the quest."),
			  quest->getHero()->getName());
    s += "\n\n";

    // add messages from the quest
    std::queue<std::string> msgs;
    quest->getExpiredMsg(msgs);
    while (!msgs.empty())
    {
        s += msgs.front();
	msgs.pop();
	if (!msgs.empty())
	    s += "\n\n";
    }

    label->set_text(s);

    dialog->show_all();
    dialog->run();
}

