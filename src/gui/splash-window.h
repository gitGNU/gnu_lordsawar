//  Copyright (C) 2007, Ole Laursen
//  Copyright (C) 2007, 2008 Ben Asselstine
//
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#ifndef GUI_SPLASH_WINDOW_H
#define GUI_SPLASH_WINDOW_H

#include <memory>
#include <gtkmm.h>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>

#include "game-parameters.h"


#include "decorated.h"
/** The opening window of the game
  * 
  * This is the first window to pop up, where the user selects whether to start
  * a new game, load an old one...
  *
  * The splash screen is also responsible for launching dialogs in response to
  * the menu choices.
  */
class SplashWindow: public Decorated
{
 public:
    SplashWindow();
    ~SplashWindow();
	
    void show();
    void hide();
        
    Gtk::Window *get_window() {return window.get();}

    sigc::signal<void> sdl_initialized;
    sigc::signal<void, std::string, unsigned short, std::string> new_remote_network_game_requested;
    sigc::signal<void, GameParameters, int, std::string > new_hosted_network_game_requested;
    sigc::signal<void, GameParameters> new_pbm_game_requested;
    sigc::signal<void, GameParameters> new_game_requested;
    sigc::signal<void, std::string> load_requested;
    sigc::signal<void> quit_requested;

 private:
    std::auto_ptr<Gtk::Window> window;
    Gtk::Button *crash_button;
    Gtk::Button *load_game_button;
    Gtk::Button *load_scenario_button;
    Gtk::Button *quit_button;
    Gtk::Button *new_network_game_button;
    Gtk::Button *new_pbm_game_button;
    Gtk::Button *preferences_button;
	    
    bool sdl_inited;
    Gtk::Container *sdl_container;
    Gtk::Widget *sdl_widget;
    bool on_delete_event(GdkEventAny *e);
    void on_window_closed();
  
    std::string network_game_nickname;
    
    void on_new_network_game_clicked();
    void on_new_pbm_game_clicked();
    void on_load_game_clicked();
    void on_load_scenario_clicked();
    void on_preferences_clicked();
    void on_quit_clicked();
    void on_rescue_crashed_game_clicked();
	
    void on_game_started(GameParameters g);
    void on_network_game_created(GameParameters g);
    void on_pbm_game_created(GameParameters g);
    void on_network_game_selected(std::string ip, unsigned short port);

#if 0
    //! Separate network input thread
    static void *networkThread(void *arg);

    //! Callback for arrival of network data
    bool networkInput();

    //! Network data processing
    void networkData();
#endif
        
 public:
    void on_sdl_surface_changed();
};

#endif
