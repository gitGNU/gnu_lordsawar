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

#ifndef W_EDIT_H
#define W_EDIT_H

#include <pgwidget.h>
#include <pglabel.h>
#include <pgbutton.h>
#include <sigc++/sigc++.h>
#include "GameScenario.h"
#include "hero_offer.h"
#include "NextTurn.h"
#include "bigmap.h"
#include "tooltip.h"
#include "scroller.h"
#ifdef HAVE_CONFIG_H
#include "config.h" // to check several macros
#endif

class SmallMap;
class Stackinfo;
class Hero;

/** The superclass for the main game screen
  * 
  * Two classes care for the running of the game on the graphical side. One is
  * MainWindow, which basically only deals with reacting to menu selections, the
  * other one is W_Edit. It controls all game widgets, such as the BigMap, the
  * SmallMap, the ArmyInfo display etc. and guards e.g. the pointer to the
  * GameScenario instance of the current game. As you can see from this listing,
  * this class does a bunch of administrative tasks as well.
  *
  * Most of the callbacks of this class deal with player interactions. They
  * refer to the buttons on the right side which allow the player to select the
  * next stack etc.
  *
  * I have to admit the code here has grown too much. Perhaps there is some time
  * for a radical cut somewhen in the future.
  */

class W_Edit : public PG_Widget
{
    public:
    /** Constructs a new main window with all subwidgets (Bigmap et al.)
      * 
      * @param gameScenario     the scenario instance of the current game
      * @param parent           the parent widget
      * @param rect             the rectangle for the game screen
      */
    W_Edit(GameScenario* gameScenario, PG_Widget* parent,PG_Rect rect);
    ~W_Edit();

    //! Function to resize the w_edit according to the current Resolution.
    void changeResolution(PG_Rect rect,bool smaller);

    //! Function that actually places the items according to current resolution
    void placeWidgets(PG_Rect rect);

    //! Callback which makes the previous stack in the player's stacklist active
    bool b_prevClicked(PG_Button* btn);
    //! Callback which makes the next stack in the player's stacklist active
    bool b_nextClicked(PG_Button* btn);
    //! Callback which makes the next stack that can move in the player's stacklist active
    bool b_nextwithmoveClicked(PG_Button* btn);
    //! Callback which sets the active stack's defend value to true
    bool b_defendClicked(PG_Button* btn);
    //! Callback which has the active stack follow its predefined path
    bool b_moveClicked(PG_Button* btn);
    //! Callback which has all stacks follow their predefined path
    bool b_moveAllClicked(PG_Button* btn);
    //! Callback which has the active stack search its location
    bool b_searchClicked(PG_Button* btn);
    //! Callback which ends the player's turn
    bool b_nextTurnClicked(PG_Button* btn);
    //! The same as defendClicked + nextClicked
    bool b_defendAndNextClicked(PG_Button* btn);
    //! The same as defendClicked + nextwithMoveClicked
    bool b_defendAndNextWithMoveClicked(PG_Button* btn);
    //! Callback which centers the map on the active stack
    bool b_centerCurrentClicked(PG_Button* btn);

    //! Callback which scrolls the bigmap around
    //bool b_scrollClicked(PG_Button* btn);

    //! Callback which redraws the map and checks if the active stack has died
    bool stackRedraw();
    

    /** Cares for correct connecting of the events. There are enough signals to
      * care for that a separate function seems justified.
      */
    void connectEvents();
    
    //! This function has to be called to initiate the game flow
    void startGame();

    //! Starts an already loaded and set up game (some details vary to startGame)
    void loadGame();

    //! Stops the game. This mainly stops the timers and such.
    void stopGame();
    
    /** Called whenever a human player occupies a city. It opens a dialog where
      * the player can select whether to raze, occupy or pillage the city.
      *
      * @param city         the occupied city
      */
    void cityOccupied(City* city);

    //! Sets up the graphics for the next turn (opening dialog etc.)
    void nextTurn();

    //! Called whenever bigmap realizes a new stack has been selected
    void bigmapStackSelected(Stack* s);

    //! Called whenever a stack has moved, updates the map etc.
    void stackUpdate(Stack* s);

    //! Called whenever a stack has died; updates bigmap as well
    void stackDied(Stack* s);

    //! Callback when the army of a human player reaches a new level.
    Army::Stat newLevelArmy(Army* a);

    //! Callback when an army gets a new medal.
    void newMedalArmy(Army* a);

    //! Callback when the game is finished; display a dialog
    void gameFinished();
    

    /** Locks all widgets (They don't react to user command) during computer
      * turns.
      */
    void lockScreen();

    //! Unlocks the widgets after a computer turn
    void unlockScreen();

    //! centers the screen, this way we don't have to make bigmap publicly accessible
    void centerScreen(const PG_Point pos){d_bigmap->centerView(pos);}

    /** Stops the timers of all subwidgets (esp. bigmap and smallmap). Used when
      * showing several dialogs, because the timers would cause a flickering
      * otherwise.
      */
    void stopTimers();

    //! Restarts the timers of the subwidgets when they have been stopped.
    void startTimers();
    
    //! Moves the viewrect by arrow[i] tiles in [i]-direction
    void helpSmallmap(int arrowx ,int arrowy); 
        
    //! Crude Callback function for closing the player announcement
    bool eventMouseMotion(const SDL_MouseMotionEvent* event);
        
    //! Hack to keep GameScenario invisible from MainWindow
    bool save(std::string file){return d_gameScenario->saveGame(file);}

    //! Function that puts all units on the tile into the current active stack
    void selectAllStack();

    //! Function that unselects the current active stack
    void unselectStack();

    //init the buttons (used in bigmap to activate the correct buttons 
    //when clicking on a stack for the first time
    void initButtons() {checkButtons();} 

    //! This signal is connected by Splash to the correct function.
    static SigC::Signal1<bool,bool> sigChangeResolution;

    private:
    //! Centers the map on the first city of the active player
    void displayFirstCity();

    //! Draws the announcement of the next player
    void pictureNextPlayer();

    //! Callback when a hero offers his services to a human player
    bool heroJoins(Hero* hero, int gold);

    //! Checks the buttons below the smallmap and (de-)activates them
    void checkButtons();

    //! Updates the display of the player's gold ressources and the turn number
    void updateStatus();

    //! callback when the user moves the mouse over the bigmap to a new tile
    void movingMouse(PG_Point pos);

    /** Callback from the NextTurn class. This function checks whether the
      * player is human or not. In the first case, it unlocks the screen etc.
      * This callback style way is neccessary for ..ehm.. internal reasons.
      *
      * @param p            the player to be checked
      */
    bool checkPlayer(Player* p);

    //! Loads the button images
    void loadImages();

    // Data
    GameScenario* d_gameScenario;
    NextTurn* d_nextTurn;
    BigMap* d_bigmap;
    SmallMap* d_smallmap;
    Stackinfo* d_stackinfo;
    ToolTip* d_tp;

    //Hero_offer * d_heroOffer;
    PG_Label* l_waiting;
    PG_Label* l_turns;
    PG_Label* l_gold;
    SDL_Surface* d_pic_prev[2];
    SDL_Surface* d_pic_next[2];
    SDL_Surface* d_pic_nextwithmove[2];
    SDL_Surface* d_pic_move[2];
    SDL_Surface* d_pic_moveall[2];
    SDL_Surface* d_pic_centerCurrent[2];
    SDL_Surface* d_pic_defend[2];
    SDL_Surface* d_pic_defendAndNext[2];
    SDL_Surface* d_pic_defendAndNextwithmove[2];
    SDL_Surface* d_pic_search[2];
    SDL_Surface* d_pic_nextTurn[2];
    SDL_Surface* d_library;
    SDL_Surface* d_pic_library;
    SDL_Surface* d_pic_winGame, *d_pic_winGameMask;
    SDL_Surface* d_pic_logo;

    PG_Button* d_b_prev;
    PG_Button* d_b_next;
    PG_Button* d_b_nextwithmove;
    PG_Button* d_b_move;
    PG_Button* d_b_moveall;
    PG_Button* d_b_centerCurrent;
    PG_Button* d_b_defend;
    PG_Button* d_b_defendAndNext;
    PG_Button* d_b_defendAndNextwithmove;
    PG_Button* d_b_search;
    PG_Button* d_b_nextTurn;
    PG_Label* d_l_tilepos;
    
    PG_Button* d_b_scroll[8];
    PG_ThemeWidget* d_border[8];

    PG_Label* d_fllogo;

    SDL_Surface* d_scrollsurf[8];
    SDL_Surface* d_scrollsurfon[8];
    SDL_Surface* d_bordersurf[4];

    PG_Rect myrect;

    bool d_allDefending;
    bool d_lock;
};

#endif // W_EDIT_H

// End of file
