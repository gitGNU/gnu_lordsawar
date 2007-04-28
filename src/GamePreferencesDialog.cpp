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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.*

#include "GamePreferencesDialog.h"
#include <pglabel.h>
#include <pgfilearchive.h>
#include <pglineedit.h>
#include <pgdropdown.h>
#include "PG_FileDialog.h"
#include "GameScenario.h"
#include "MapConfDialog.h"
#include "player_preferences.h"
#include "File.h"
#include "Configuration.h"
#include <pgapplication.h>

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

std::string turnmode_true = "";
std::string turnmode_false = "";

GamePreferencesDialog::GamePreferencesDialog(PG_Widget* parent, PG_Rect rect)
	:PG_ThemeWidget(parent, rect), d_loaded(false), d_cancelled(false)
{
    debug("GamePreferencesDialog()");

    turnmode_true = _("player's turn");
    turnmode_false = _("new round");

    d_l_type = new PG_Label(this, PG_Rect(40, 10, 100, 20), _("Type"));
    d_l_type->SetFontColor (PG_Color(0, 0, 0));
    d_l_name = new PG_Label(this, PG_Rect(145, 10, 100, 20), _("Name"));
    d_l_name->SetFontColor (PG_Color(0, 0, 0));
    d_l_armyset = new PG_Label(this, PG_Rect(255, 10, 100, 20), _("Armyset"));
    d_l_armyset->SetFontColor (PG_Color(0, 0, 0));

    // predefined names
    vector<string> nameList;
    nameList.push_back("Sirians");
    nameList.push_back("Dark Elves");
    nameList.push_back("Stone Giants");
    nameList.push_back("Kingdoms");
    nameList.push_back("White Dwarves");
    nameList.push_back("Horse Tribes");
    nameList.push_back("Ussyrian Orcs");
    nameList.push_back("Lich King");
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
        PG_Rect p(10, 40 + i*35, 450, 25);
        Player_preferences::Type type = Player_preferences::ANY;
        type = Player_preferences::HUMAN;
        d_player_preferences[i] = new Player_preferences(type, nameList[i], this, p);
        d_player_preferences[i]->playerDataChanged.connect(
                (SigC::slot(*this, &GamePreferencesDialog::slotPlayerDataChanged)));
	}

    d_b_random = new PG_Button(this, PG_Rect(Width() - 310 , 10, 100, 25), _("Random"),0);
    d_b_random->SetFontColor (PG_Color(0, 0, 0));
    d_b_random->SetToggle(true);
    d_b_load = new PG_Button(this, PG_Rect(Width() - 210, 10, 100, 25), _("Load Map"),1);
    d_b_load->SetFontColor (PG_Color(0, 0, 0));
    d_b_load->SetToggle(true);

    //choose tileset and turn mode
    d_l_tiles = new PG_Label(this, PG_Rect(35, 330, 120, 20), _("Choose Tileset"));
    d_l_tiles->SetFontColor (PG_Color(0, 0, 0));
    d_d_tiles = new PG_DropDown(this, PG_Rect(10, 360, 150, 30),3);
    d_d_tiles->SetFontColor (PG_Color(0, 0, 0), 1);
    File::scanTilesets(d_d_tiles); 

    //set the turn mode
    d_l_turnmode = new PG_Label(this, PG_Rect(175, 330, 290, 20),
                                _("Process armies at beginning of"));
    d_l_turnmode->SetFontColor (PG_Color(0, 0, 0));
    d_d_turnmode = new PG_DropDown(this, PG_Rect(175, 360, 170, 30),4);
    d_d_turnmode->SetFontColor (PG_Color(0, 0, 0), 1);
    d_d_turnmode->AddItem(turnmode_true.c_str());
    d_d_turnmode->AddItem(turnmode_false.c_str());
    d_d_turnmode->SetText(turnmode_true.c_str());

    
    d_b_browse = new PG_Button(this, PG_Rect(Width() - 100, 60, 90, 25), _("Browse"),2);
    d_b_browse->SetFontColor (PG_Color(0, 0, 0), 1);
    d_grass = new TerrainConfig(this, PG_Rect(Width() - 310, 60, 300, 20),
                _("Grass"), 0, 99, 78);
    d_water = new TerrainConfig(this, PG_Rect(Width() - 310, 90, 300, 20),
                _("Water"), 0, 99, 7);
    d_swamp = new TerrainConfig(this, PG_Rect(Width() - 310, 120, 300, 20),
                _("Swamp"), 0, 99, 2);
    d_forest = new TerrainConfig(this, PG_Rect(Width() - 310, 150, 300, 20),
                _("Forest"), 0, 99, 3);
    d_hills = new TerrainConfig(this, PG_Rect(Width() - 310, 180, 300, 20),
                _("Hills"), 0, 99, 5);
    d_mountains = new TerrainConfig(this, PG_Rect(Width() - 310, 210, 300, 20),
                _("Mountains"), 0, 99, 5);
    d_cities = new TerrainConfig(this, PG_Rect(Width() - 310, 240, 300, 20),
                _("Cities"), 10, 40, 20);
    d_ruins = new TerrainConfig(this, PG_Rect(Width() - 310, 270, 300, 20),
                _("Ruins"), 15, 30, 25);
    d_temples = new TerrainConfig(this, PG_Rect(Width() - 310, 300, 300, 20),
                _("Temples"), 15, 30, 25);

    d_b_ok = new PG_Button(this, PG_Rect(10, Height() - 40, Width()/2 - 20, 30), _("Start Game"),3);
    d_b_ok->SetFontColor (PG_Color(0, 0, 0));
    d_b_cancel = new PG_Button(this, PG_Rect(Width()/2+10, Height() - 40,Width()/2-20 , 30), _("Back to main menu"),3);
    d_b_cancel->SetFontColor (PG_Color(0, 0, 0));

    //the buttons for the map size
    d_b_normalsize = new PG_Button(this, PG_Rect(Width() - 250, 350, 150, 30), _("Normal size"),5);
    d_b_normalsize->SetFontColor (PG_Color(0, 0, 0));
    d_b_normalsize->SetToggle(true);
    d_b_smallsize = new PG_Button(this, PG_Rect(Width() - 250, 390, 150, 30), _("Small size"),6);
    d_b_smallsize->SetFontColor (PG_Color(0, 0, 0));
    d_b_smallsize->SetToggle(true);
    d_b_tinysize = new PG_Button(this, PG_Rect(Width() - 250, 430, 150, 30), _("Tiny size"),7);
    d_b_tinysize->SetFontColor (PG_Color(0, 0, 0));
    d_b_tinysize->SetToggle(true);
    d_b_normalsize->SetPressed(true);

    
    // if the previous map is still available use this it as default setting
    std::string randommap = File::getSavePath() + "random.map";
    d_edit = new PG_LineEdit(this, PG_Rect(Width() - 310, 60, 200, 25));
    d_edit->SetText(randommap.c_str());

    d_b_random->sigClick.connect(slot(*this, &GamePreferencesDialog::randomClicked));
    d_b_load->sigClick.connect(slot(*this, &GamePreferencesDialog::loadClicked));
    d_b_browse->sigClick.connect(slot(*this, &GamePreferencesDialog::browseClicked));
    d_b_ok->sigClick.connect(slot(*this, &GamePreferencesDialog::okClicked));
    d_b_cancel->sigClick.connect(slot(*this, &GamePreferencesDialog::cancelClicked));
    d_b_normalsize->sigClick.connect(slot(*this, &GamePreferencesDialog::sizeClicked));
    d_b_smallsize->sigClick.connect(slot(*this, &GamePreferencesDialog::sizeClicked));
    d_b_tinysize->sigClick.connect(slot(*this, &GamePreferencesDialog::sizeClicked));
}

GamePreferencesDialog::~GamePreferencesDialog()
{
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
        delete d_player_preferences[i];
    delete d_l_type;
    delete d_l_name;
    delete d_l_armyset;
    delete d_l_turnmode;
    delete d_d_turnmode;
    delete d_b_random;
    delete d_b_load;
    delete d_b_browse;
    delete d_b_ok;
    delete d_b_cancel;
    delete d_b_normalsize;
    delete d_b_smallsize;
    delete d_b_tinysize;
    delete d_grass;
    delete d_water;
    delete d_swamp;
    delete d_forest;
    delete d_hills;
    delete d_mountains;
    delete d_cities;
    delete d_ruins;
    delete d_temples;
}

void GamePreferencesDialog::initGUI()
{

    debug("initGUI");
    if (d_loaded)
    {
        d_grass->hide();
        d_water->hide();
        d_swamp->hide();
        d_forest->hide();
        d_hills->hide();
        d_mountains->hide();
        d_cities->hide();
        d_ruins->hide();
        d_temples->hide();
        d_b_normalsize->Hide();
        d_b_smallsize->Hide();
        d_b_tinysize->Hide();
        d_l_tiles->Hide();
        d_d_tiles->Hide();
        d_l_turnmode->Hide();
        d_d_turnmode->Hide();
        d_l_type->Hide();
        d_l_name->Hide();
        d_l_armyset->Hide();
        for (unsigned int i = 0; i < MAX_PLAYERS; i++)
            d_player_preferences[i]->Hide();
        
        d_edit->Show();
        d_b_browse->Show();
        d_b_load->SetPressed(true);
        d_b_random->SetPressed(false);
    }
    else
    {
        d_grass->show();
        d_water->show();
        d_swamp->show();
        d_forest->show();
        d_hills->show();
        d_mountains->show();
        d_cities->show();
        d_ruins->show();
        d_temples->show();
        d_b_normalsize->Show();
        d_b_smallsize->Show();
        d_b_tinysize->Show();
        d_l_tiles->Show();
        d_d_tiles->Show();
        d_l_turnmode->Show();
        d_d_turnmode->Show();
        d_l_type->Show();
        d_l_name->Show();
        d_l_armyset->Show();
        for (unsigned int i = 0; i < MAX_PLAYERS; i++)
            d_player_preferences[i]->Show();
        
        d_edit->Hide();
        d_b_browse->Hide();
        d_b_load->SetPressed(false);
        d_b_random->SetPressed(true);
    }
}

bool GamePreferencesDialog::browseClicked(PG_Button* btn)
{
    debug("browse clicked")
    
    PG_FileDialog fd(0, PG_FileDialog::PG_OPEN, true, Configuration::s_savePath,
                        "*.map");
    
    fd.Show();
    fd.RunModal();
    fd.Hide();

    d_fileName = fd.getSelectedFile();
    if (!d_fileName.empty())
    {
        d_edit->SetText(d_fileName.c_str());
    }
    return true;
}

bool GamePreferencesDialog::randomClicked(PG_Button* btn)
{
    d_loaded = false;
    initGUI();
    return true;
}

bool GamePreferencesDialog::loadClicked(PG_Button* btn)
{
    d_loaded = true;
    initGUI();
    return true;
}

bool GamePreferencesDialog::cancelClicked(PG_Button* btn)
{
    QuitModal();
    d_cancelled = true;
    return true;
}

bool GamePreferencesDialog::okClicked(PG_Button* btn)
{
    //this is a workaround for a really strange bug where you would get a
    //segfault if you had activated an edit box and pressed the start button
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
        d_player_preferences[i]->Hide();
    d_edit->Hide();
    d_d_tiles->Hide();
    d_d_turnmode->Hide();
        
    QuitModal();
    return true;
}

bool GamePreferencesDialog::sizeClicked(PG_Button* widget)
{
    //set all buttons to not pressed except the caller
    d_b_normalsize->SetPressed(false);
    d_b_smallsize->SetPressed(false);
    d_b_tinysize->SetPressed(false);

    widget->SetPressed(true);

    // adjust the other sliders
    if (widget == d_b_normalsize)
    {
            d_cities->setValue(20);
            d_ruins->setValue(25);
            d_temples->setValue(25);
    }
    else if (widget == d_b_smallsize)
    {
            d_cities->setValue(15);
            d_ruins->setValue(20);
            d_temples->setValue(20);
    }
    else if (widget == d_b_tinysize)
    {
            d_cities->setValue(10);
            d_ruins->setValue(15);
            d_temples->setValue(15);
    }

    return true;
}

void GamePreferencesDialog::slotPlayerDataChanged()
{
    debug("slotPlayerDataChanged()");
    playerDataChanged.emit();
}

void GamePreferencesDialog::setPlayerName(int player, std::string name)
{
    if(player < 0) return;
    if(player > 7) return;

    Player_preferences *p = d_player_preferences[player];
    p->setName(name);
}

void GamePreferencesDialog::restrictPlayers(int number)
{
    if(number < 0) number = 0;
    if(number > 7) number = 7;

    for (int i = 0; i < number; i++)
        d_player_preferences[i]->Show();
    for (unsigned int i = number; i < MAX_PLAYERS; i++)
        d_player_preferences[i]->Hide();
}

void GamePreferencesDialog::restrictSettings()
{
    // this is similar to initGUI() in load mode,
    // but it is not the same!

    d_grass->hide();
    d_water->hide();
    d_swamp->hide();
    d_forest->hide();
    d_hills->hide();
    d_mountains->hide();
    d_cities->hide();
    d_ruins->hide();
    d_temples->hide();
    d_b_normalsize->Hide();
    d_b_smallsize->Hide();
    d_b_tinysize->Hide();
    d_l_tiles->Hide();
    d_d_tiles->Hide();
    d_l_turnmode->Hide();
    d_d_turnmode->Hide();
    d_l_type->Hide();
    d_l_name->Hide();
    d_l_armyset->Hide();

    d_edit->Hide();
    d_b_browse->Hide();
    d_b_load->Hide();
    d_b_random->Hide();
}

int GamePreferencesDialog::noPlayers()
{
	int n = 0;
	
	for (unsigned int i = 0; i < MAX_PLAYERS; i++)
		if (d_player_preferences[i]->isActive())
			n++;
	
	return n;
}

bool GamePreferencesDialog::fillData(CreateScenario* creator)
{
    int w = 100, h = 100, noSignposts, noStones;
    //fill all player data
    SDL_Color color;
    Player::Type type;

    //first insert the neutral player
    unsigned int set = (Armysetlist::getInstance()->getArmysets())[0];
    
    color.r = color.g = color.b = 220;color.unused=0;
    creator->addNeutral("Neutral", set, color, Player::AI_DUMMY);

    //then fill the other players
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
        if (!d_player_preferences[i]->isActive())
            continue;

        switch(i)
        {
            case 0:color.r = 252; color.b = 252; color.g = 252; break;
            case 1:color.r = 80; color.b = 28; color.g = 172; break;
            case 2:color.r = 252; color.b = 32; color.g = 236; break;
            case 3:color.r = 92; color.b = 208; color.g = 92; break;
            case 4:color.r = 252; color.b = 0; color.g = 160;break;
            case 5:color.r = 44; color.b = 252; color.g = 184; break;
            case 6:color.r = 196; color.b = 0; color.g = 28; break;
            case 7:color.r = color.g = color.b = 0; break;
        }
        
        if (d_player_preferences[i]->isComputer())
        {
            if (d_player_preferences[i]->isEasy()) {
                type = Player::AI_FAST;
            }
            else
            {
                type = Player::AI_SMART;
            }
        }
        else
            type = Player::HUMAN;

        creator->addPlayer(d_player_preferences[i]->getName(),
                            d_player_preferences[i]->getArmyset(),
                            color, type);
    }

    //now fill in some map information
    //first, cities, temples, ruins
    creator->setMapTiles(std::string(d_d_tiles->GetText()));
    creator->setNoCities(d_cities->getValue());
    creator->setNoRuins(d_ruins->getValue());
    creator->setNoTemples(d_temples->getValue());

    //now the terrain. The scenario generator also accepts input with a sum of
    //more than 100%, so the thing is rather easy here
    creator->setPercentages(d_grass->getValue(), d_water->getValue(),
                            d_forest->getValue(), d_swamp->getValue(),
                            d_hills->getValue(), d_mountains->getValue());

    //finally, tell CreateScenario the map size
    if (d_b_normalsize->GetPressed())
    {
	w = 100;
	h = 100;
    }
    if (d_b_smallsize->GetPressed())
    {
	w = 70;
	h = 70;
    }
    if (d_b_tinysize->GetPressed())
    {
	w = 50;
	h = 75;
    }
    noSignposts = 
	    static_cast<int>(w * h * (d_grass->getValue() / 100.0) * 0.003);
    creator->setNoSignposts(noSignposts);
    noStones = static_cast<int>(w * h * (d_grass->getValue() / 100.0) * 0.0022);
    creator->setNoStones(noStones);
    creator->setWidth(w);
    creator->setHeight(h);

    //and tell it the turn mode
    creator->setTurnmode(false);
    if (std::string(d_d_turnmode->GetText()) == turnmode_true)
        creator->setTurnmode(true);
    
    return true;
}


bool GamePreferencesDialog::eventKeyDown(const SDL_KeyboardEvent* key)
{	
    // Check , which key was pressed
    switch (key->keysym.sym)
    {		
        case SDLK_f:		        
	  //d_w_edit->lockScreen();
             if(Configuration::s_fullScreen)
	     {
                 Configuration::s_flags&= ~SDL_FULLSCREEN;	
                 PG_Application::GetApp()->InitScreen(Configuration::s_width, Configuration::s_height, 16,Configuration::s_flags);
                 Configuration::s_fullScreen=false; 
	     }
             else 
	     {
                 Configuration::s_flags|=SDL_FULLSCREEN;	
                 PG_Application::GetApp()->InitScreen(Configuration::s_width, Configuration::s_height, 16,Configuration::s_flags);
                 Configuration::s_fullScreen=true; 
	     }
             break;
	 default:
	     break;	
    }	
    return true;
}

// End of file
