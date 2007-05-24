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

#include <sigc++/slot.h>
#include <libglademm/xml.h>
#include <gtkmm/window.h>
#include <gtkmm/image.h>
#include <gtkmm/table.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/stock.h>

#include "splash-window.h"

#include "game-preferences-dialog.h"
#include "load-scenario-dialog.h"
#include "glade-helpers.h"
#include "../Configuration.h"
#include "../defs.h"
#include "../sound.h"
//#include "../netggz.h"

SplashWindow::SplashWindow()
{
#if 0
    d_networkcancelled = false;
    d_networkready = false;
#endif

    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/splash-window.glade");

    Gtk::Window *w = 0;
    xml->get_widget("window", w);
    window.reset(w);

    w->signal_delete_event().connect(
	sigc::mem_fun(*this, &SplashWindow::on_delete_event));
    
    // load background
    Gtk::Image *splash_image
	= manage(new Gtk::Image(Configuration::s_dataPath + "/various/splash_screen.jpg"));

    // the table is a hack to get the image shown behind the buttons
    Gtk::Table *table = 0;
    xml->get_widget("table", table);
    table->attach(*splash_image, 0, 2, 0, 2, Gtk::EXPAND | Gtk::FILL);
    
    xml->connect_clicked("new_game_button",
			 sigc::mem_fun(*this, &SplashWindow::on_new_game_clicked));
    xml->connect_clicked("load_game_button",
			 sigc::mem_fun(*this, &SplashWindow::on_load_game_clicked));
    xml->connect_clicked("load_scenario_button",
			 sigc::mem_fun(*this, &SplashWindow::on_load_scenario_clicked));
    xml->connect_clicked("quit_button",
			 sigc::mem_fun(*this, &SplashWindow::on_quit_clicked));

    Sound::getInstance()->playMusic("intro");
}

SplashWindow::~SplashWindow()
{
    Sound::deleteInstance();
    //clearData();
}

void SplashWindow::show()
{
    window->show_all();
}

void SplashWindow::hide()
{
    window->hide();
}

bool SplashWindow::on_delete_event(GdkEventAny *e)
{
    quit_requested.emit();

    return true;
}

void SplashWindow::on_quit_clicked()
{
    quit_requested.emit();
}

void SplashWindow::on_new_game_clicked()
{
    GamePreferencesDialog d;
    
    d.set_parent_window(*window.get());
    d.game_started.connect(sigc::mem_fun(*this, &SplashWindow::on_game_started));
    
    d.run();
}

void SplashWindow::on_load_game_clicked()
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
	load_requested.emit(filename);
    }
}

void SplashWindow::on_load_scenario_clicked()
{
    LoadScenarioDialog d;
    
    d.set_parent_window(*window.get());
    
    d.run();
    
    std::string filename = d.get_scenario_filename();
    if (!filename.empty())
	load_requested.emit(filename);
}

void SplashWindow::on_game_started(GameParameters g)
{
    new_game_requested.emit(g);
}


#if 0

bool SplashWindow::b_multiplayerGameClicked(PG_Button* btn)
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
        //d_app->sigAppIdle.connect(slot((*this), &SplashWindow::networkInput));

        pthread_t id;
        pthread_create(&id, NULL, &SplashWindow::networkThread, (void*)this);

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

bool SplashWindow::b_loadGameClicked(PG_Button* btn)
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

    RWinGame::swinning.connect(sigc::slot((*this), &SplashWindow::gameFinished));
    RLoseGame::slosing.connect(sigc::slot((*this), &SplashWindow::gameFinished));

    Hide();
        
    d_mainWindow = new PMainWindow(gameScenario, Rectangle(0, 0, Width(), Height()));
    d_mainWindow->squitting.connect(sigc::slot((*this), &SplashWindow::gameFinished));
    d_mainWindow->Show();
    d_mainWindow->loadGame(savegame);

    return true;
}

bool SplashWindow::b_loadscenarioClicked(PG_Button* btn)
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

    RWinGame::swinning.connect(sigc::slot((*this), &SplashWindow::gameFinished));
    RLoseGame::slosing.connect(sigc::slot((*this), &SplashWindow::gameFinished));

    Hide();
        
    d_mainWindow = new PMainWindow(gameScenario, Rectangle(0, 0, Width(), Height()));
    d_mainWindow->squitting.connect(sigc::slot((*this), &SplashWindow::gameFinished));
    d_mainWindow->Show();
    d_mainWindow->startGame();

    return true;
}

bool SplashWindow::b_campaignClicked(PG_Button* btn)
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

void *SplashWindow::networkThread(void *arg)
{
    SplashWindow *splash = (SplashWindow*)arg;

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

bool SplashWindow::networkInput()
{
	//debug("check network!");

	if(GGZ::ref()->data())
		networkData();

	return true;
}

void SplashWindow::networkData()
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

#endif
