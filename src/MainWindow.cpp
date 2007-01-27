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

#include <pgmessagebox.h>
#include "MainWindow.h"
#include "w_edit.h"
#include "defs.h"
#include "about.h"
#include "Configuration.h"
#include "goldreport.h"
#include "PG_FileDialog.h"
#include "citiesreport.h"
#include "questsreport.h"
#include "stackreport.h"
#include "OptionsDialog.h"
#include "sound.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

MainWindow::MainWindow(GameScenario* gameScenario, const PG_Rect& rect)
  :PG_ThemeWidget(0, rect, true), d_lastsave(""),arrowx(0),arrowy(0)
{
    // create menus
    d_fileMenu = new PG_PopupMenu(0, 20, 0, 0);
    d_fileMenu->addMenuItem(_("Load"), 0, slot(*this,&MainWindow::file_load));
    d_fileMenu->addMenuItem(_("Save"), 1, slot(*this,&MainWindow::file_save));
    d_fileMenu->addMenuItem(_("Save as"), 2, slot(*this,&MainWindow::file_saveas));
    d_fileMenu->addSeparator();
    d_fileMenu->addMenuItem(_("Quit"), 3, slot(*this,&MainWindow::file_quit));

    d_reportsMenu = new PG_PopupMenu(0, 170, 0, 0);
    d_reportsMenu->addMenuItem(_("Armies"), 0, slot(*this,&MainWindow::reports_armies));
    d_reportsMenu->addMenuItem(_("Cities"), 1, slot(*this,&MainWindow::reports_cities));
    d_reportsMenu->addMenuItem(_("Gold"), 2, slot(*this,&MainWindow::reports_gold));
    d_reportsMenu->addMenuItem(_("Quests"), 3, slot(*this,&MainWindow::reports_quests));
    
    d_optionsMenu = new PG_PopupMenu(0, 0, 0, 0);
    d_optionsMenu->addMenuItem(_("Game options"), 0, slot(*this,&MainWindow::options_game));

    d_helpMenu = new PG_PopupMenu(0, 470, 0, 0);
    d_helpMenu->addMenuItem(_("About"), 1, slot(*this,&MainWindow::help_about));
    d_helpMenu->addSeparator();
    d_helpMenu->addMenuItem(_("Help"), 0, slot(*this,&MainWindow::help_help));

    // create menubar
    d_menuBar = new PG_MenuBar(this, PG_Rect(0, 0, rect.w, 25));
    d_menuBar->Add(_("File"), d_fileMenu);
    d_menuBar->Add(_("Reports"), d_reportsMenu);
    d_menuBar->Add(_("Options"), d_optionsMenu);
    d_menuBar->Add(_("Help"), d_helpMenu);
    
    d_w_edit = new W_Edit(gameScenario, this, PG_Rect(rect.x, rect.y + 25, rect.w, rect.h - 25));

    d_fileMenu->Hide();
    d_reportsMenu->Hide();
    d_optionsMenu->Hide();
    d_helpMenu->Hide();
}

MainWindow::~MainWindow()
{
    delete d_menuBar;
    delete d_fileMenu;
    delete d_reportsMenu;
    delete d_optionsMenu;
    delete d_helpMenu;
    delete d_w_edit;
}

void MainWindow::changeResolution(int w, int h)
{
    if (w>my_width && h>my_height)
    {
        debug("Greater!!")
	debug("w=" << w << " width=" << my_width)
        debug("h=" << h << " height=" << my_width)
        SizeWidget(w,h);
        d_menuBar->SizeWidget(w,25);
        d_w_edit->changeResolution(PG_Rect(0,25,w,h - 25),false);
    }
    else if (w<my_width && h<my_height)
    {
        debug("Lesser!!")
        debug("w=" << w << " width=" << my_width)
        debug("h=" << w << " height=" << my_width)
        d_w_edit->changeResolution(PG_Rect(0,25,w,h - 25),true);
        
        // This is due to a bug in paragui. We get an out of bounds drawing
        // error if we reduce the size of a widget, so pretend the widgets
        // have increased in size
        d_menuBar->my_width = w-1;
        d_menuBar->SizeWidget(w,25);

        my_height = h-1;
        my_width = w-1;
        SizeWidget(w,h);
    }

    Redraw();
}


void MainWindow::startGame()
{
    Sound::getInstance()->haltMusic(false);
    Sound::getInstance()->enableBackground();
    d_w_edit->startGame();
}

void MainWindow::loadGame(std::string filename,bool resetfilename)
{
    Sound::getInstance()->haltMusic(false);
    Sound::getInstance()->enableBackground();
    d_w_edit->loadGame();
    // Andrea : I have added the boolean control value because there where a bug concerning
    // the saving of the tutorial game. Instead of saving it into the .lordsawar directory
    // it saved it into the ...../share/lordsawar/map/ directory
    // Now after loading the tutorial map the filename is reset to "" so that it will save 
    // the file into correct path.
    if (resetfilename) d_lastsave="";
    else d_lastsave = filename;
}

void MainWindow::stopGame()
{
    //This function stops all current game activity. The background is that
    //MainWindow doesn't automatically know that a game has ended, so it has
    //to be told this fact.
    d_w_edit->stopGame();
    Sound::getInstance()->disableBackground();
}

// This event handler is from Thomas Plonka
// Scrolling of the map with the keybord arrows and reports with the keyboard.
// It is a paragui event and no't a SDL event.
bool MainWindow::eventKeyDown(const SDL_KeyboardEvent* key)
{	
	// Check , which key was pressed and change the variables
	// and the position of the viewrect , or start reports	
	switch (key->keysym.sym)
	{		
	        case ' ':
			d_w_edit->selectAllStack();
		    break;
	        case SDLK_ESCAPE:
			d_w_edit->unselectStack();
		    break; 			
		case SDLK_LEFT:
			arrowx = -1;
			break;
		case SDLK_RIGHT:
			arrowx = 1;
			break;
		case SDLK_UP:
			arrowy = -1;
			break;
		case SDLK_DOWN:
			arrowy = 1;
			break;
		case SDLK_a:	
                        reports_armies(0,0);
			break;
		case SDLK_c:	
                        reports_cities(0,0);
			break;
		case SDLK_g:
                        reports_gold(0,0);
			break;
		case SDLK_l:
                        file_load(0,0);
			break;
		case SDLK_q:
                        reports_quests(0,0);
			break;
		case SDLK_s:
                        file_saveas(0,0);
			break;
                case SDLK_f:		        
		        d_w_edit->lockScreen();
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
                        d_w_edit->unlockScreen();
			break;
	        case SDLK_e:
                        if ((key->keysym.mod & KMOD_RALT ) || (key->keysym.mod & KMOD_LALT ))
                              d_w_edit->b_nextTurnClicked(NULL);
			break;
		default:
			break;	
	}
	// connection to smallmap
	d_w_edit->helpSmallmap(arrowx, arrowy);
	
	// If "arrowx" and "arrowy" are not set to "0" ,
	// diagonal scrolling with the keyboard-arrows
	arrowx = 0;
	arrowy = 0;
					
	return true;	
}

bool MainWindow::file_load(PG_PopupMenu::MenuItem* item, PG_Pointer p)
{
	//to make sure no timers interfere with the graphics painting routines
    d_w_edit->stopTimers();

    //if (PlayerReallyWantsToLoadGame)  (to be implemented later)
    PG_FileDialog opendialog(0, PG_FileDialog::PG_OPEN, true, Configuration::s_savePath,
            "*.sav", _("LordsAWar savegames"));
    opendialog.Show();
    opendialog.RunModal();


    std::string savefile = opendialog.getSelectedFile();

    if (savefile.empty()) return true;

    //we delete d_w_edit only here to have a nice background during the
    //open dialog :)
    delete d_w_edit;

    bool broken;
    GameScenario* gameScenario = new GameScenario(savefile, broken, true);

    if (broken)
    {
        std::cerr <<_("Couldn't load savefile ") <<savefile <<_(". Bailing out.\n");
        exit(-1);
    }

    d_w_edit = new W_Edit(gameScenario, this,
                    PG_Rect(my_xpos, my_ypos + 25, my_width, my_height - 25));
    d_w_edit->startTimers();
    d_w_edit->Show();
    d_w_edit->loadGame();

    d_lastsave = savefile;

    return true;
}
bool MainWindow::file_save(PG_PopupMenu::MenuItem* item, PG_Pointer p)
{
    if (d_lastsave.empty())
    {
        file_saveas(item, p);   // open dialog for file selection etc.
        return true;
    }

    // tell GameScenario to save the game
    bool success = d_w_edit->save(d_lastsave);

    //timers interfere with the graphics drawing routine
    d_w_edit->stopTimers();

    if (success)
    {
        PG_MessageBox mb(this, PG_Rect(my_width/2-100, my_height/2-75, 200, 150), _("Save Game"),
                _("Game Saved."), PG_Rect(60, 110, 80, 30), _("OK"));
        mb.Show();
        mb.RunModal();
        mb.Hide();
    }
    else
    {
        PG_MessageBox mb(this,PG_Rect(my_width/2-100, my_height/2-75, 200, 150), _("Save Game"),
                _("Error saving. Game not saved"),
                PG_Rect(60, 110, 80, 30), _("OK"));
        mb.Show();
        mb.RunModal();
        mb.Hide();
    }

    d_w_edit->startTimers();

    return true;
}

bool MainWindow::file_saveas(PG_PopupMenu::MenuItem* item, PG_Pointer p)
{
    //temporarily stop all timers
    d_w_edit->stopTimers();

    PG_FileDialog savedialog(0, PG_FileDialog::PG_SAVE, true, Configuration::s_savePath,
            "*.sav", _("LordsAWar savegames"));
    savedialog.Show();
    savedialog.RunModal();
    string savefile = savedialog.getSelectedFile();

    //nothing selected
    if (savefile.empty()) return true;

    //tell GameScenario to save the game
    bool success = d_w_edit->save(savefile);
    
    if (success)
    {
        PG_MessageBox mb(this, PG_Rect(my_width/2-100, my_height/2-75, 200, 150), _("Save Game"),
                _("Game Saved."),
                PG_Rect(60, 110, 80, 30), _("OK"));
        mb.Show();
        mb.RunModal();
        mb.Hide();

        d_lastsave = savefile;
    }
    else
    {
        PG_MessageBox mb(this, PG_Rect(my_width/2-100, my_height/2-75, 200, 150), _("Save Game"),
                _("Error saving. Game not saved"),
                PG_Rect(60, 110, 80, 30), _("OK"));
        mb.Show();
        mb.RunModal();
        mb.Hide();
    }

    d_w_edit->startTimers();

    return true;
}

bool MainWindow::file_quit(PG_PopupMenu::MenuItem* item, PG_Pointer p)
{
    d_w_edit->stopTimers();
    squitting.emit(0);
    return true;
}

bool MainWindow::reports_armies(PG_PopupMenu::MenuItem* item, PG_Pointer p)
{
    debug("Showing army report")

    d_w_edit->stopTimers();

    StackReport report(this, PG_Rect(Width()/2 - 250, Height()/2 - 200, 530, 400));
    report.sselectingStack.connect(SigC::slot(*d_w_edit, &W_Edit::centerScreen));

    report.Show();
     //some parts of the report which we don't want anyone to see have been
    //displayed by calling the show event. We simulate a user action here
    //to redraw the whole thing.
    report.b_upClicked(0);
    report.RunModal();
    report.Hide();

    d_w_edit->startTimers();

    return true;
}

bool MainWindow::reports_quests(PG_PopupMenu::MenuItem* item, PG_Pointer p)
{
    debug("Showing quests report")

    d_w_edit->stopTimers();

    QuestsReport report(this, PG_Rect(Width()/2 - 250, Height()/2 - 200, 
                        535, 400));
    report.Show();
     //some parts of the report which we don't want anyone to see have been
    //displayed by calling the show event. We simulate a user action here
    //to redraw the whole thing.
    report.b_upClicked(0);
    report.RunModal();
    report.Hide();

    d_w_edit->startTimers();
    return true;
}

bool MainWindow::reports_cities(PG_PopupMenu::MenuItem* item, PG_Pointer p)
{
    debug("Showing city report")

    d_w_edit->stopTimers();

    CitiesReport report(this, PG_Rect(Width()/2 - 250, Height()/2 - 200, 530, 400));
    report.sselectingCity.connect(SigC::slot(*d_w_edit, &W_Edit::centerScreen));

    report.Show();
     //some parts of the report which we don't want anyone to see have been
    //displayed by calling the show event. We simulate a user action here
    //to redraw the whole thing.
    report.b_upClicked(0);
    report.RunModal();
    report.Hide();

    d_w_edit->startTimers();

    return true;
}

bool MainWindow::reports_gold(PG_PopupMenu::MenuItem* item, PG_Pointer p)
{
    debug("Showing gold report")

    d_w_edit->stopTimers();

    GoldReport report(this, PG_Rect(Width()/2 - 150, Height()/2 - 200,
                    300, 300));
    report.Show();
    report.RunModal();
    report.Hide();

    d_w_edit->startTimers();

    return true;
}

bool MainWindow::options_game(PG_PopupMenu::MenuItem* item, PG_Pointer p)
{
    d_w_edit->stopTimers();

    OptionsDialog opts(this, PG_Rect(Width()/2 - 250, Height()/2 - 150, 540, 300));

    opts.Show();
    opts.RunModal();
    opts.Hide();

    d_w_edit->startTimers();

    return true;
}

bool MainWindow::help_help(PG_PopupMenu::MenuItem* item, PG_Pointer p)
{
    std::cerr << _("not implemented yet\n");
    return true;
}

bool MainWindow::help_about(PG_PopupMenu::MenuItem* item, PG_Pointer p)
{
    debug("Showing About Dialog")

    d_w_edit->stopTimers();

    int x=Width()/2;
    int y=Height()/2;
    
    AboutDialog about(this, PG_Rect(x/2-2, y/2-15,
                  x+4, y+30));
    about.Show();
    about.RunModal();
    about.Hide();

    d_w_edit->startTimers();

    return true;
}

// End of file
