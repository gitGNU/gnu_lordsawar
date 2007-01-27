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

#ifndef E_MAINSCREEN_H
#define E_MAINSCREEN_H

#include <iostream>
#include <list>
#include <pgthemewidget.h>
#include <pgpopupmenu.h>
#include <pgapplication.h>

class E_Bigmap;
class E_Smallmap;
class GameScenario;
class E_PlayerDialog;
class E_ScenarioDialog;

/** This class is the top-level widget of the editor. It has a menu, contains
  * a bigmap and a smallmap and several buttons.
  */

class E_MainScreen : public PG_ThemeWidget
{
    public:
        //! Default paragui constructor
        E_MainScreen(PG_Application* app, const PG_Rect r);
        ~E_MainScreen();

        // Callbacks
        
        //! create a new map
        bool file_new(PG_PopupMenu::MenuItem* item, PG_Pointer p);
        //! load a map
        bool file_load(PG_PopupMenu::MenuItem* item, PG_Pointer p);
        //! save a map
        bool file_save(PG_PopupMenu::MenuItem* item, PG_Pointer p);
        //! save a map under a certain filename
        bool file_saveAs(PG_PopupMenu::MenuItem* item, PG_Pointer p);
        //! quit the program
        bool file_quit(PG_PopupMenu::MenuItem* item, PG_Pointer p);
        
        //! edit the scenario variables
        bool edit_scenario(PG_PopupMenu::MenuItem* item, PG_Pointer p);
        //! edit the players
        bool edit_players(PG_PopupMenu::MenuItem* item, PG_Pointer p);
        //! edit the events
        bool edit_events(PG_PopupMenu::MenuItem* item, PG_Pointer p);

        //! find something on the map
        bool find_id(PG_PopupMenu::MenuItem* item, PG_Pointer p);

        //! report of the stacks
        bool report_stacks(PG_PopupMenu::MenuItem* item, PG_Pointer p);
        //! report of the stacks
        bool report_cities(PG_PopupMenu::MenuItem* item, PG_Pointer p);

        //! button for terrain painting has been clicked
        bool pointerClicked(PG_Button* btn);
        //! button for a terrain selection has been clicked
        bool terrainClicked(PG_Button* btn);
        //! callback when the user moves the mouse over the bigmap to a new tile
        void movingMouse(PG_Point pos);

    private:
        //! Removes all data structures associated with a given map.
        void clearMap();

        //! Sets up the buttons etc. when creating a new map.
        void setupInterface();

        //! Changes the status of the bigmap appropriately
        void changeBigmapStatus();
        
        // Data
        PG_Application* d_app;
        E_Bigmap* d_bigmap;
        E_Smallmap* d_smallmap;

        PG_Button* d_b_pointer[2];
        PG_Button* d_b_erase;
        PG_Button* d_b_stack;
        PG_Button* d_b_city;
        PG_Button* d_b_ruin;
        PG_Button* d_b_temple;
        PG_Button* d_b_signpost;
        PG_Button* d_b_stone;
        PG_Button* d_b_road;
        std::list<PG_Button*> d_b_terrain;
        std::list<PG_Label*> d_l_terrain;
        PG_Label* d_l_tilepos;
        
        SDL_Surface* d_s_pointer[2];
        SDL_Surface* d_erasepic;
        SDL_Surface* d_stackpic;
        SDL_Surface* d_citypic;
        SDL_Surface* d_ruinpic;
        SDL_Surface* d_templepic;
        SDL_Surface* d_signpostpic;
        SDL_Surface* d_stonepic;
        SDL_Surface* d_roadpic;
        
        std::string d_filename;
        
        GameScenario* d_scenario;
};

#endif //E_MAINSCREEN_H
