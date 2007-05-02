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

#ifndef GUI_SPLASH_WINDOW_H
#define GUI_SPLASH_WINDOW_H

#include <memory>
#include <gtkmm/window.h>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>

#include "../game-parameters.h"


/** The opening window of the game
  * 
  * This is the first window to pop up, where the user selects whether to start
  * a new game, load an old one...
  *
  * The splash screen is also responsible for launching dialogs in response to
  * the menu choices.
  */
class SplashWindow: public sigc::trackable
{
 public:
    SplashWindow();
    ~SplashWindow();
	
    void show();
    void hide();
        
    /** Notification of a finished game
     *
     * This is called when a currently running game is finished. Since the
     * calling function is one of the game lists or windows, we cannot clear
     * them in an instant (this would corrupt the stack), but have to interrupt
     * the game and clear everything when a new game is started. This is a bit
     * crude, as the memory isn't freed while the player has not started a new
     * game, but the alternative of using a timer is equally weird.
     *
     * @param  status  the game delivers a number that indicates how the
     *                 game was won/lost, currently unused
     */
    void gameFinished();

    sigc::signal<void, GameParameters> new_game_requested;
    sigc::signal<void> quit_requested;

 private:
    std::auto_ptr<Gtk::Window> window;
	
    bool on_delete_event(GdkEventAny *e);
    
    void on_new_campaign_clicked();
    void on_new_game_clicked();
    void on_new_network_game_clicked();
    void on_load_game_clicked();
    void on_load_scenario_clicked();
    void on_preferences_clicked();
    void on_quit_clicked();
	
    void on_game_started(GameParameters g);

#if 0
    //! Separate network input thread
    static void *networkThread(void *arg);

    //! Callback for arrival of network data
    bool networkInput();

    //! Network data processing
    void networkData();

    //! Clears the game data
    void clearData();
#endif
        
};


#endif
