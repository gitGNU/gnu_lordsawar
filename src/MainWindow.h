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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <pgthemewidget.h>
#include <pgmenubar.h>
#include <pgpopupmenu.h>
#include <pgapplication.h>
#include "GameScenario.h"

class PG_PopupMenu;
class W_Edit;

/** The game window
  * 
  * MainWindow is the class which describes the game window. However, most
  * actions are done by the W_Edit class. MainWindow only cares for the menu
  * items, the other stuff (clicking on the buttons etc.) is done by W_Edit.
  */

class MainWindow : public PG_ThemeWidget
                    
{
    public:
        /** Constructor
          * 
          * @param scenario     the instance of the played scenario
          * @param rect         the dimensions of the window
          */
        MainWindow(GameScenario* scenario, const PG_Rect& rect);
        ~MainWindow();

        void changeResolution(int w, int h);

        W_Edit* getWedit() {return d_w_edit;};

        //! Initialises the graphic system and starts a game.
        void startGame();

        /** Loads a game
          * 
          * @param filename     the full name of the savegame to be loaded
          */
        void loadGame(std::string filename,bool resetfilename=false);

        /** Stops all game activity
          * 
          * For program flow reasons, the game objects can't be deleted
          * instantly when a game is finished. With this function you can
          * at least freeze the game. It will mainly stop all active timers
          * to prevent strange segfaults...
          */
        void stopGame();

        // CALLBACKS
        bool file_load(PG_PopupMenu::MenuItem* item, PG_Pointer p);
        bool file_save(PG_PopupMenu::MenuItem* item, PG_Pointer p);
        bool file_saveas(PG_PopupMenu::MenuItem* item, PG_Pointer p);
        bool file_quit(PG_PopupMenu::MenuItem* item, PG_Pointer p);
        bool reports_armies(PG_PopupMenu::MenuItem* item, PG_Pointer p);
        bool reports_cities(PG_PopupMenu::MenuItem* item, PG_Pointer p);
        bool reports_gold(PG_PopupMenu::MenuItem* item, PG_Pointer p);
        /** \brief Callback invoked by the upper menu item 'Quests...' */
        bool reports_quests(PG_PopupMenu::MenuItem* item, PG_Pointer p);
        bool options_game(PG_PopupMenu::MenuItem* item, PG_Pointer p);
        bool help_help(PG_PopupMenu::MenuItem* item, PG_Pointer p);
        bool help_about(PG_PopupMenu::MenuItem* item, PG_Pointer p);

        // Event handler 
        bool eventKeyDown(const SDL_KeyboardEvent* key);
                
        SigC::Signal1<void, Uint32> squitting;
        
    private:
        // DATA
        W_Edit* d_w_edit;
        PG_MenuBar* d_menuBar;
        PG_PopupMenu* d_fileMenu;
        PG_PopupMenu* d_reportsMenu;
        PG_PopupMenu* d_optionsMenu;
        PG_PopupMenu* d_helpMenu;

        std::string d_lastsave;
                
        // Helping variables to set the viewrect with the cursor keys
        int arrowx, arrowy; 
};

#endif
