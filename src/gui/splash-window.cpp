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

#include <config.h>

#include <sigc++/slot.h>
#include <libglademm/xml.h>
#include <gtkmm/window.h>
#include <gtkmm/dialog.h>
#include <gtkmm/image.h>
#include <gtkmm/table.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include "splash-window.h"
#include "driver.h"

#include "game-preferences-dialog.h"
#include "load-scenario-dialog.h"
#include "glade-helpers.h"
#include "Configuration.h"
#include "defs.h"
#include "sound.h"
#include "File.h"
#include "GameScenario.h"
#include "playerlist.h"
#include "network-game-selector-dialog.h"
//#include "netggz.h"
#include "gtksdl.h"
#include "main-preferences-dialog.h"

namespace
{
  void surface_attached_helper(GtkSDL *gtksdl, gpointer data)
    {
      static_cast<SplashWindow*>(data)->on_sdl_surface_changed();
    }
}
SplashWindow::SplashWindow()
{
  network_game_nickname = "";
  sdl_inited = false;
#if 0
    d_networkcancelled = false;
    d_networkready = false;
#endif

    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/splash-window.glade");

    Gtk::Window *w = 0;
    xml->get_widget("window", w);
    window.reset(w);
    decorate(window.get(), File::getMiscFile("various/back.bmp"));
    window_closed.connect(sigc::mem_fun(window.get(), &Gtk::Window::hide));

    xml->get_widget("sdl_container", sdl_container);
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

    xml->connect_clicked("new_network_game_button",
			 sigc::mem_fun(*this, &SplashWindow::on_new_network_game_clicked));
    xml->connect_clicked("new_pbm_game_button",
			 sigc::mem_fun(*this, &SplashWindow::on_new_pbm_game_clicked));
    xml->connect_clicked("preferences_button",
			 sigc::mem_fun(*this, &SplashWindow::on_preferences_clicked));
    Sound::getInstance()->playMusic("intro");

    if (Configuration::s_autosave_policy == 1)
      {
	Gtk::VBox *button_box;
	xml->get_widget("button_box", button_box);
  
	std::string filename = File::getSavePath() + "autosave.sav";
	FILE *fileptr = fopen (filename.c_str(), "r");
	if (fileptr)
	  {
	    fclose (fileptr);
	    crash_button = Gtk::manage(new Gtk::Button());
	    crash_button->set_label(_("Rescue Crashed Game"));
	    button_box->pack_start(*crash_button, true, true, 0);
	    crash_button->signal_clicked().connect(sigc::mem_fun(*this, &SplashWindow::on_rescue_crashed_game_clicked));
	    button_box->reorder_child(*crash_button, 0);
	  }
      }

  sdl_widget = Gtk::manage(Glib::wrap(gtk_sdl_new(1,1,0,SDL_SWSURFACE)));
  sdl_widget->grab_focus();
  sdl_widget->add_events(Gdk::KEY_PRESS_MASK | Gdk::BUTTON_PRESS_MASK | 
			 Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK | 
			 Gdk::LEAVE_NOTIFY_MASK);

      // connect to the special signal that signifies that a new surface has been
      // generated and attached to the widget 
  g_signal_connect(G_OBJECT(sdl_widget->gobj()), "surface-attached", 
     G_CALLBACK(surface_attached_helper), this); 
      
      sdl_container->add(*sdl_widget); 
}

SplashWindow::~SplashWindow()
{
    Sound::deleteInstance();
    //clearData();
}

void SplashWindow::show()
{
    sdl_container->show_all();
    window->show_all();
}

void SplashWindow::hide()
{
  sdl_container->hide();
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

void SplashWindow::on_rescue_crashed_game_clicked()
{
  delete crash_button;
  std::string filename = File::getSavePath() + "autosave.sav";
  load_requested.emit(filename);
}

void SplashWindow::on_new_game_clicked()
{
    GamePreferencesDialog d(GameScenario::HOTSEAT);
    
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

void SplashWindow::on_new_network_game_clicked()
{
  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + 
				"/new-network-game-dialog.glade");
  std::auto_ptr<Gtk::Dialog> dialog;
  Gtk::Dialog *d;
  Gtk::RadioButton *client_radiobutton;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  xml->get_widget("client_radiobutton", client_radiobutton);
  dialog->set_transient_for(*window.get());
  Gtk::Entry *nick_entry;
  xml->get_widget("nick_entry", nick_entry);
  std::string nick;
  if (getenv("USER"))
    nick = getenv("USER");
  else if (network_game_nickname != "")
    nick = network_game_nickname;
  else
    nick = "guest";
  nick_entry->set_text(nick);
  nick_entry->set_activates_default(true);
  int response = dialog->run();
  dialog->hide();
  if (response == 0) //we hit okay
    {
      network_game_nickname = nick_entry->get_text();
      if (client_radiobutton->get_active() == true)
	{
	  NetworkGameSelectorDialog ngsd;
	  ngsd.game_selected.connect(sigc::mem_fun(*this, &SplashWindow::on_network_game_selected));
	  ngsd.run();
	}
      else
	{
	  //okay, we're a server.
	  GamePreferencesDialog gpd(GameScenario::NETWORKED);
    
	  gpd.set_parent_window(*window.get());
	  gpd.set_title(_("New Networked Game"));
	  gpd.game_started.connect(sigc::mem_fun(*this, &SplashWindow::on_network_game_created));
	  gpd.run(network_game_nickname);
	  gpd.hide();
	  return;
	}
    }

}

void SplashWindow::on_new_pbm_game_clicked()
{
  GamePreferencesDialog gpd(GameScenario::PLAY_BY_MAIL);
    
  gpd.set_parent_window(*window.get());
  gpd.set_title(_("New Play By Mail game"));
  gpd.game_started.connect(sigc::mem_fun(*this, &SplashWindow::on_pbm_game_created));
  gpd.run();
  gpd.hide();
}

void SplashWindow::on_load_scenario_clicked()
{
    LoadScenarioDialog d;
    
    d.set_parent_window(*window.get());
    
    d.run();
    d.hide();
    
    std::string filename = d.get_scenario_filename();
    if (!filename.empty())
      {
	GamePreferencesDialog gp(filename);
	gp.set_parent_window(*window.get());
	gp.game_started.connect(sigc::mem_fun(*this, &SplashWindow::on_game_started));
    
	gp.run();
      } 
	//load_requested.emit(filename);
}

void SplashWindow::on_network_game_selected(std::string ip, unsigned short port)
{
  new_remote_network_game_requested.emit(ip, port, network_game_nickname);
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
void
SplashWindow::on_sdl_surface_changed()
{
  if (!sdl_inited)
    {
      sdl_inited = true;
      sdl_initialized.emit();
    }
}

void SplashWindow::on_network_game_created(GameParameters g)
{
  new_hosted_network_game_requested.emit(g, LORDSAWAR_PORT, 
					 network_game_nickname);
}

void SplashWindow::on_pbm_game_created(GameParameters g)
{
  new_pbm_game_requested.emit(g);
}
    
void SplashWindow::on_preferences_clicked()
{
  MainPreferencesDialog d;
  d.set_parent_window(*window.get());
  d.run();
  d.hide();
}
