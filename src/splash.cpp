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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "splash.h"
#include <iostream>
#include <SDL_image.h>
#include "MainWindow.h"
#include "defs.h"
#include "armysetlist.h"
#include "stacklist.h"
#include "playerlist.h"
#include "Configuration.h"
#include "MultiPlayerModeDialog.h"
#include "GraphicsCache.h"
#include "PG_FileDialog.h"
#include "CreateScenario.h"
#include "File.h"
#include "events/RWinGame.h"
#include "events/RLoseGame.h"
#include <pgapplication.h>
#include <pgmessagebox.h>
#include "LangDialog.h"
#include "netggz.h"
#include "w_edit.h"
#include "ScenariosDialog.h"
#include "sound.h"
#include <pthread.h>
#include "GamePreferencesDialog.h"

#ifdef __WIN32__
  // on mingw32, fd_set lives in winsock.h
  #include <winsock.h>
#else 
  #include <sys/select.h>
#endif

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

Splash::Splash(PG_Application* app, const Rectangle& rect)
    :PG_ThemeWidget(0, rect), d_mainWindow(0), d_networkcancelled(false), d_networkready(false)
{
    d_app = app;
    d_background = File::getMiscPicture("splash_screen.jpg", false);
    SetBackground(d_background, 2);
    
	if(!Configuration::s_ggz)
    {
        d_b_campaign = new PG_Button(this,
            Rectangle(rect.w - 210, rect.h - 250, 180, 30), _("New Campaign"),0);
	d_b_campaign->SetFontColor (PG_Color(0, 0, 0));
        d_b_singleplayer_game = new PG_Button(this,
            Rectangle(rect.w - 210, rect.h - 215, 180, 30), _("New Game"),1);
	d_b_singleplayer_game->SetFontColor (PG_Color(0, 0, 0));
        d_b_load_game = new PG_Button(this,
            Rectangle(rect.w - 210, rect.h - 180, 180, 30), _("Load Game"),4);
	d_b_load_game->SetFontColor (PG_Color(0, 0, 0));
        d_b_scenario = new PG_Button(this,
            Rectangle(rect.w - 210, rect.h - 145, 180, 30), _("Load Scenario"),3);
	d_b_scenario->SetFontColor (PG_Color(0, 0, 0));
    }
    else
    {
        d_b_campaign = 0;
        d_b_singleplayer_game = 0;
        d_b_load_game = 0;
        d_b_scenario = 0;
    }
    d_b_multiplayer_game = new PG_Button(this,
            Rectangle(rect.w - 210, rect.h - 110, 180, 30), _("New Network Game"),2);
    d_b_multiplayer_game->SetFontColor (PG_Color(0, 0, 0));
    d_b_lang = new PG_Button(this, Rectangle(rect.w - 210, rect.h - 75, 180, 30),
            _("Language"), 4);
    d_b_lang->SetFontColor (PG_Color(0, 0, 0));
#ifdef __WIN32__
    d_b_lang->SetTransparency(100);
    d_b_lang->EnableReceiver(false);
#endif

    d_b_quit = new PG_Button(this,
            Rectangle(rect.w - 210, rect.h - 40, 180, 30), _("Quit"),5);
    d_b_quit->SetFontColor (PG_Color(0, 0, 0));


    if(d_b_campaign)
        d_b_campaign->sigClick.connect(slot(*this, &Splash::b_campaignClicked));
    if(d_b_singleplayer_game)
        d_b_singleplayer_game->sigClick.connect(slot(*this, &Splash::b_singleplayerGameClicked));
    d_b_multiplayer_game->sigClick.connect(slot(*this, &Splash::b_multiplayerGameClicked));
    if(d_b_load_game)
        d_b_load_game->sigClick.connect(slot(*this, &Splash::b_loadGameClicked));
    if(d_b_scenario)
        d_b_scenario->sigClick.connect(slot(*this, &Splash::b_loadscenarioClicked));

    d_b_lang->sigClick.connect(slot(*this, &Splash::b_langClicked));
    d_b_quit->sigClick.connect(slot(*this, &Splash::b_quitClicked));

    W_Edit::sigChangeResolution.connect(slot(*this, &Splash::b_resolutionChanged));

    Sound::getInstance()->playMusic("intro");
}

Splash::~Splash()
{
    Sound::deleteInstance();
    SDL_FreeSurface(d_background);
    clearData();
    delete d_b_campaign;
    delete d_b_singleplayer_game;
    delete d_b_multiplayer_game;
    delete d_b_load_game;
    delete d_b_scenario;
    delete d_b_quit;
}


bool Splash::b_resolutionChanged(bool smaller)
{
   
    int w=Configuration::s_width;
    int h=Configuration::s_height;

    debug("SPLASH w=" << w << " h=" << h)
    if (!smaller) 
    {
        d_app->InitScreen(w,h, 16,Configuration::s_flags);
        SizeWidget(w,h);
    }

    if (d_b_campaign) d_b_campaign->MoveWidget(Rectangle(w - 210, h - 250, 180, 30));
    if (d_b_singleplayer_game) d_b_singleplayer_game->MoveWidget(Rectangle(w - 210, h - 215, 180, 30));
    if (d_b_load_game) d_b_load_game->MoveWidget(Rectangle(w - 210, h - 180, 180, 30));
    if (d_b_scenario) d_b_scenario->MoveWidget(Rectangle(w - 210,h - 145, 180, 30));
    d_b_multiplayer_game->MoveWidget(Rectangle(w - 210,h - 110, 180, 30));
    d_b_lang->MoveWidget(Rectangle(w - 210, h - 75, 180, 30));
    d_b_quit->MoveWidget(Rectangle(w - 210, h - 40, 180, 30));

    if (smaller) 
    {
        d_app->InitScreen(w,h, 16,Configuration::s_flags);
        my_height = h-1;
        my_width = w-1;  
        SizeWidget(w,h);
    }
    debug("I arrive here.")
  
    return true;
}

bool Splash::b_quitClicked(PG_Button* btn)
{
	d_app->Quit();
	return true;
}

bool Splash::b_langClicked(PG_Button* btn)
{
    // First, have the user choose the language
    LangDialog dialog(this, Rectangle(my_width/2 - 150, my_height/2 - 150, 300, 300));
    dialog.Show();
    dialog.RunModal();
    dialog.Hide();

    // Then, rename all buttons
    if (d_b_campaign)
        d_b_campaign->SetText(_("New Campaign"));
    if (d_b_singleplayer_game)
        d_b_singleplayer_game->SetText(_("New Game"));
    if (d_b_load_game)
        d_b_load_game->SetText(_("Load Game"));
    if (d_b_scenario)
        d_b_scenario->SetText(_("Load Scenario"));
    
    d_b_multiplayer_game->SetText(_("New Network Game"));
    d_b_lang->SetText(_("Language"));
    d_b_quit->SetText(_("Quit"));
    
    return true;
}

bool Splash::b_cancelClicked(PG_Button* btn)
{
	d_network->QuitModal();
	d_networkcancelled = true;
	return true;
}

bool Splash::b_singleplayerGameClicked(PG_Button* btn)
{
	clearData();
	if (newGame());
        return true;

	return true;
}

bool Splash::newGame(std::string ip, int port)
{
    CreateScenario *creator = new CreateScenario();
    PGamePreferencesDialog* dialog = 0;
    
    //CreateScenario requires to define a maptype
    creator->setMaptype(CreateScenario::NORMAL);

    //the player has to choose which player to activate etc.
    
    dialog = new PGamePreferencesDialog(0, Rectangle(0, 0,
            PG_Application::GetScreenWidth(),
            PG_Application::GetScreenHeight()));

    dialog->Show();
    dialog->initGUI();
    if(Configuration::s_ggz)
    {
        dialog->restrictPlayers(GGZ::ref()->seats());
        if(!GGZ::ref()->host())
        {
            dialog->restrictSettings();
        }
        for(int i = 0; i < GGZ::ref()->seats(); i++)
        {
            dialog->setPlayerName(i, GGZ::ref()->name(i));
        }
    }
    dialog->RunModal();
    dialog->Hide();

    if (dialog->cancelled())
    {
        delete dialog;
        return false;
    }

    if (!dialog->loadedMap())
        dialog->fillData(creator);

    //now create the map and dump the created map under ~savepath/random.map
    std::string randomfilename = File::getSavePath();
    randomfilename += "random.map";
    
    //!dialog means testing game
    if (!dialog || (!dialog->loadedMap()))
    {
        creator->create();
        creator->dump(randomfilename);
    }
    delete creator;

	//if in GGZ mode, wait for other players
    if(Configuration::s_ggz)
    {
#ifdef WITH_GGZ
        int fd = GGZ::ref()->fd();
        ggz_write_int(fd, 43);
#endif

        Rectangle rect = Rectangle(0, 0, Width(), Height());
        d_network = new PG_ThemeWidget(0, rect);
        SDL_Surface* background = File::getMiscPicture("network_screen.jpg", false);
        d_network->SetBackground(background, 2);

        PG_Button* cancel = new PG_Button(d_network,
                Rectangle(rect.w - 210, rect.h - 40, 180, 30), _("Cancel"),5);
        cancel->sigClick.connect(slot((*this), &Splash::b_cancelClicked));

        Hide();
        //d_network->Show();
        d_network->Show(true);
        //d_network->RunModal();
        while((!d_networkcancelled) && (!d_networkready))
        {
            // FIXME:
            // process events!
        }

        d_network->Hide();
        SDL_FreeSurface(background);
        delete d_network;

        if (d_networkcancelled)
        {
            Show();
            delete dialog;
            return false;
        }
    }

    //and finally load the "savegame", set up the main window and start
    bool broken;
    GameScenario* gameScenario = 0;
    
    if (dialog->loadedMap())
        gameScenario = new GameScenario(dialog->getFilename(), broken, true);
    else
        gameScenario = new GameScenario(randomfilename, broken, true);

    if (broken)
    {
        std::cerr <<_("random.map was broken when re-reading. Exiting...\n");
        exit(-1);
    }

    //set up misc stuff
    RWinGame::swinning.connect(sigc::slot((*this), &Splash::gameFinished));
    RLoseGame::slosing.connect(sigc::slot((*this), &Splash::gameFinished));

    Hide();
    
    if (dialog)
        delete dialog;
    
    d_mainWindow = new PMainWindow(gameScenario,
                                    Rectangle(0, 0, Width(), Height()));
    d_mainWindow->squitting.connect(sigc::slot((*this),
                                    &Splash::gameFinished));
    d_mainWindow->Show();
    d_mainWindow->startGame();

    return true;
}

bool Splash::b_multiplayerGameClicked(PG_Button* btn)
{
    clearData();

    if(Configuration::s_ggz)
    {
        GGZ::ref()->init();
        bool success = GGZ::ref()->connect();
        if(!success)
        {
            std::cerr << _("GGZ initialization failed. Exiting.\n") << std::endl;
            exit(-1);
            return false;
        }
        std::cout << "GGZ: playing with " << GGZ::ref()->seats() << " players." << std::endl;

        //d_app->EnableAppIdleCalls(true);
        //d_app->sigAppIdle.connect(slot((*this), &Splash::networkInput));

        pthread_t id;
        pthread_create(&id, NULL, &Splash::networkThread, (void*)this);

        newGame();
        return false;
    }
    else
    {
        PG_MessageBox mb(GetParent(), Rectangle(200, 200, 200, 150),
                _("Play on GGZ"),
                _("Please launch LordsAWar from a GGZ Gaming Zone client."),
                Rectangle(60, 100, 80, 30), _("OK"));
        mb.Show();
        mb.RunModal();
        mb.Hide();
    }

    /*MultiPlayerModeDialog dialog(this, Rectangle(20, 20, 350, 230));
    dialog.Show();
    dialog.RunModal();
    dialog.Hide();

    if (dialog.getResult())
    {
        if (dialog.getMode())
        {
            newGame(GameScenario::MULTI_PLAYER_HOST, "", dialog.getPort());
        }
        else
        {  
            newGame(GameScenario::MULTI_PLAYER_CLIENT, dialog.getIP(), dialog.getPort());
        }
    }*/

    return true;
}

bool Splash::b_loadGameClicked(PG_Button* btn)
{
    PG_FileDialog opendialog(0, PG_FileDialog::PG_OPEN, true,
            Configuration::s_savePath, "*.sav", _("LordsAWar savegames"));
    opendialog.Show();
    opendialog.RunModal();
    std::string savegame = opendialog.getSelectedFile();

    //no file selected
    if (savegame.empty())
    {
        return true;
    }
    
    clearData();

    bool broken;
    GameScenario* gameScenario = new GameScenario(savegame, broken, true);

    if (broken)
    {
        std::cerr <<savegame <<_(": Could not load savegame.\n");
        PG_MessageBox mb(this, Rectangle(my_width/2 - 100, my_height/2 - 100, 200, 150),
                _("Error"),
                _("An error occured while loading the savegame."),
                Rectangle(60, 100, 80, 30), _("OK"));
        mb.Show();
        mb.RunModal();
        mb.Hide();
        delete gameScenario;
        return true;
    }

    RWinGame::swinning.connect(sigc::slot((*this), &Splash::gameFinished));
    RLoseGame::slosing.connect(sigc::slot((*this), &Splash::gameFinished));

    Hide();
        
    d_mainWindow = new PMainWindow(gameScenario, Rectangle(0, 0, Width(), Height()));
    d_mainWindow->squitting.connect(sigc::slot((*this), &Splash::gameFinished));
    d_mainWindow->Show();
    d_mainWindow->loadGame(savegame);

    return true;
}

bool Splash::b_loadscenarioClicked(PG_Button* btn)
{
    debug(_("Load Scenario Clicked!"));

    clearData();
    
    string filename="";
    ScenariosDialog scd(GetParent(), Rectangle(200,70,500,460),&filename);
    scd.Show();
    scd.RunModal();
    scd.Hide();

    if (filename=="") 
        return true; 

    cerr << "FILENAME=" << filename << "!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
    std::string savegame = Configuration::s_dataPath+string("/map/")+filename+string(".map");

    bool broken;
    GameScenario* gameScenario = new GameScenario(savegame, broken, true);

    if (broken)
    {
        std::cerr <<savegame <<_(": Could not load scenario.\n");
        PG_MessageBox mb(this, Rectangle(my_width/2 - 100, my_height/2 - 100, 200, 150),
                _("Error"),
                _("An error occured while loading the savegame."),
                Rectangle(60, 100, 80, 30), _("OK"));
        mb.Show();
        mb.RunModal();
        mb.Hide();
        delete gameScenario;
        return true;
    }

    RWinGame::swinning.connect(sigc::slot((*this), &Splash::gameFinished));
    RLoseGame::slosing.connect(sigc::slot((*this), &Splash::gameFinished));

    Hide();
        
    d_mainWindow = new PMainWindow(gameScenario, Rectangle(0, 0, Width(), Height()));
    d_mainWindow->squitting.connect(sigc::slot((*this), &Splash::gameFinished));
    d_mainWindow->Show();
    d_mainWindow->startGame();

    return true;
}

bool Splash::b_campaignClicked(PG_Button* btn)
{
    clearData();
    PG_MessageBox mb(GetParent(), Rectangle(200, 200, 200, 150),
            _("Not implemented"),
            _("Campaigns are not implemented yet."),
            Rectangle(60, 100, 80, 30), _("OK"));
    mb.Show();
    mb.RunModal();
    mb.Hide();
    return true;
}

void Splash::clearData()
{
    if (d_mainWindow)
    {
        delete d_mainWindow;
        d_mainWindow = 0;
    }

    // some of the references here use player pointers which are obsolete by now,
    // beside it assumes some specific terrain set and such
    GraphicsCache::deleteInstance();
}

void Splash::gameFinished(Uint32 status)
{
    debug("gameFinished()");

    //the memory for gameScenario and mainWindow is freed as soon as a
    //new game is started

    d_mainWindow->stopGame();
    d_mainWindow->Hide();

    Show();

    Sound::getInstance()->playMusic("intro");
}

void *Splash::networkThread(void *arg)
{
    Splash *splash = (Splash*)arg;

    int fd;
    int cfd;
    fd_set set;
    int ret;
    struct timeval tv;
    int maxfd;

    while(1)
    {
        fd = GGZ::ref()->fd();
        cfd = GGZ::ref()->controlfd();

        FD_ZERO(&set);
        FD_SET(cfd, &set);

        maxfd = cfd;
        if(fd != -1)
        {
            FD_SET(fd, &set);
            if(fd > maxfd) maxfd = fd;
        }

        tv.tv_sec = 10;
        tv.tv_usec = 0;

        ret = select(maxfd + 1, &set, NULL, NULL, /*&tv*/NULL);
        printf("GGZ (thread) ## check (seats: %i)\n", GGZ::ref()->seats());

        if(ret > 0)
        {
            if(FD_ISSET(fd, &set))
            {
                printf("GGZ (thread) ## got data\n");
                splash->networkInput();
            }
            if(FD_ISSET(cfd, &set))
            {
                printf("GGZ (thread) ## control data arrived\n");
                GGZ::ref()->dispatch();
                if(GGZ::ref()->playing())
                {
                    printf("GGZ (thread) ## we're finally playing!\n");
                }
            }
        }
        else if(ret == -1)
        {
            printf("GGZ (thread) ## select error\n");
            GGZ::ref()->deinit();
            return NULL;
        }
    }

    return NULL;
}

bool Splash::networkInput()
{
	//debug("check network!");

	if(GGZ::ref()->data())
		networkData();

	return true;
}

void Splash::networkData()
{
#ifdef WITH_GGZ
	int fd, ret, op;

	fd = GGZ::ref()->fd();

	ret = ggz_read_int(fd, &op);
	printf("GGZ ## received opcode: %i (%i)\n", op, ret);
	if(ret < 0)
	{
		GGZ::ref()->deinit();
		return;
	}

	// Test only:
	// 42 (s->c): init
	// 43 (c->s): i'm ready, please transit from WAITING to PLAYING
	// 44 (s->c): all are ready, transition done + data here

	if(op == 42)
	{
		debug("opcode 42: initialisation!");
	}
	else if(op == 44)
	{
		debug("opcode 44: game start!");
		//b_cancelClicked(NULL);
		//d_network->Hide();
		//Show();
		d_networkready = 1;
	}
	else
	{
		debug("unknown opcode!");
	}
#endif
}

bool Splash::eventKeyDown(const SDL_KeyboardEvent* key)
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
