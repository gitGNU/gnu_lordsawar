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

#ifndef GamePreferencesDialog_H
#define GamePreferencesDialog_H

#include <pgwindow.h>
#include <pglineedit.h>

#include "GameScenario.h"
#include "player.h"
#include "CreateScenario.h"
#include "MapConfDialog.h"

class PG_DropDown;

class Player_preferences;
class TerrainConfig;
class CreateScenario;

/** Class for setting up player preferences
  *
  * GamePreferencesDialog is the class where the player, or players in a
  * multiplayergame, decide which parameters he/they want to play with
  */

class GamePreferencesDialog : public PG_ThemeWidget
{
	public:
        /** Dialog constructor
          * 
          * @param parent   the parent widget (inherited from PG_Widget)
          * @param rect     the rect for the widget (silently ignored)
          */
        GamePreferencesDialog(PG_Widget* parent, PG_Rect rect);
        ~GamePreferencesDialog();

        //! Switches the GUI between random map settings and loading of a map
        void initGUI();

        // SIGNALS
        SigC::Signal0<void> playerDataChanged;

        // SLOTS
        void slotPlayerDataChanged();

		//! Restrict the number of players (for GGZ)
		void restrictPlayers(int number);

		//! Restrict most user interface elements (for GGZ)
		void restrictSettings();

        //! Changes the name of a player (for GGZ)
        void setPlayerName(int player, std::string name);

        //! Get the number of selected players
        int noPlayers();

        /** Fill the given scenario creator object with the selected data
          * 
          * This function tells the creator especially the player details, the
          * terrain distribution and the global settings.
          *
          * @param creator      the scenario creator to be filled with data
          * @return true on success
          */
        bool fillData(CreateScenario* creator);
	
        // CALLBACK FUNCTIONS
        bool randomClicked(PG_Button* btn);
        bool loadClicked(PG_Button* btn);
        bool browseClicked(PG_Button* btn);
        bool okClicked(PG_Button* btn);
        bool cancelClicked(PG_Button* btn);
        bool sizeClicked(PG_Button* btn);

        // ACCESSORS
        bool loadedMap() const {return d_loaded;}
        std::string getFilename(){return std::string(d_edit->GetText());}
        bool cancelled() const {return d_cancelled;}
	
        //! Event Handler 
        bool eventKeyDown(const SDL_KeyboardEvent* key);
	
	private:
	// WIDGETS
        PG_Label* d_l_type;
        PG_Label* d_l_name;
        PG_Label* d_l_armyset;
        PG_Label* d_l_tiles;
        PG_Label* d_l_turnmode;
        Player_preferences* d_player_preferences[MAX_PLAYERS];
        PG_Button* d_b_random;
        PG_Button* d_b_load;
        PG_Button* d_b_browse;
        PG_Button* d_b_ok;
        PG_Button* d_b_cancel;
        PG_Button* d_b_normalsize;
        PG_Button* d_b_smallsize;
        PG_Button* d_b_tinysize;
        PG_DropDown* d_d_tiles;
        PG_DropDown* d_d_turnmode;
        PG_LineEdit* d_edit;
        TerrainConfig* d_grass;
        TerrainConfig* d_water;
        TerrainConfig* d_swamp;
        TerrainConfig* d_forest;
        TerrainConfig* d_hills;
        TerrainConfig* d_mountains;
        TerrainConfig* d_cities;
        TerrainConfig* d_ruins;
        TerrainConfig* d_temples;

        // DATA
        bool d_loaded;
        bool d_cancelled;
        std::string d_fileName;    // filename of map (if loaded)
};

#endif

// End of file
