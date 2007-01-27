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

#ifndef SPLASH_H
#define SPLASH_H

#ifdef FL_SOUND
#include <SDL_mixer.h>
#endif
#include <pgthemewidget.h>
#include "GameScenario.h"

class PG_Button;
class PG_Application;
class MainWindow;

/** The opening window of the game
  * 
  * This is the first window to pop up, where the user selects whether to start
  * a new game, load an old one...
  *
  * Besides this, the splash screen is also responsible for setting up the game
  * (in fact, the setup is done by the specialized classes, but the splash
  * screen calls them in their order).
  */

class Splash : public PG_ThemeWidget
{
    public:
        /** The constructor
          * 
          * @param app      solely used to quit the game via app->Quit()
          * @param rect     the rectangle of this window
          */
        Splash(PG_Application* app, const PG_Rect& rect);
        ~Splash();
        
        /** Function which is called by the callbacks and starts a new game
          * 
          * @param gameType     which type (currently only accepts singleplayer)
          * @param ip           the ip of the multiplayer server (unused)
          * @param port         the port at the server (unused)
          * @return true if succesful, false if user aborted
          */
        bool newGame(std::string ip = "", int port = 0);

        //! Callback for quitting the game
        bool b_quitClicked(PG_Button* btn);
        //! Callback for selecting the language
        bool b_langClicked(PG_Button* btn);
        
        //! Callback for starting a campaign (not implemented yet)
        bool b_campaignClicked(PG_Button* btn);

        //! Callback for starting a single player game
        bool b_singleplayerGameClicked(PG_Button* btn);
        //! Callback for starting a (networked) multiplayer game (NIY)
        bool b_multiplayerGameClicked(PG_Button* btn);
        //! Callback for loading a game
        bool b_loadGameClicked(PG_Button* btn);

        //! Callback for loading scenarios
        bool b_loadscenarioClicked(PG_Button* btn);

        //! Callback for cancelling a network wait
        bool b_cancelClicked(PG_Button* btn);

        //! Callback for changing resolution
        bool b_resolutionChanged(bool smaller);

        /** Notification of a finished game
          *
          * This is called when a currently running game is finished. Since
          * the calling function is one of the game lists or windows, we cannot
          * clear them in an instant (this would corrupt the stack), but have to
          * interrupt the game and clear everything when a new game is started.
          * This is a bit crude, as the memory isn't freed while the player has
          * not started a new game, but the alternative of using a timer is
          * equally weird.
          *
          * @param  status  the game delivers a number that indicates how the
          *                 game was won/lost, currently unused
          */
        void gameFinished(Uint32 status);

	//! Separate network input thread
	static void *networkThread(void *arg);

	//! Callback for arrival of network data
        bool networkInput();

        //! Network data processing
        void networkData();

        //! Event Handler 
        bool eventKeyDown(const SDL_KeyboardEvent* key);

    private:
        //! Clears the game data
        void clearData();
        
        // DATA
        MainWindow* d_mainWindow;
        bool d_networkcancelled;
        bool d_networkready;

        // WIDGETS
        PG_Application* d_app;
        PG_Button* d_b_campaign;
        PG_Button* d_b_singleplayer_game;
        PG_Button* d_b_multiplayer_game;
        PG_Button* d_b_scenario;
        PG_Button* d_b_load_game;
        PG_Button* d_b_lang;
        PG_Button* d_b_quit;
        SDL_Surface* d_background;
        PG_ThemeWidget* d_network;
};

#endif // SPLASH_H

// End of file
