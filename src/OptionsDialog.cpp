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

#include <string>
#include <pglistboxitem.h>
#include "File.h"
#include "Configuration.h"
#include "OptionsDialog.h"
#include "MainWindow.h"
#include "w_edit.h"
#include "defs.h"
#include "sound.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

OptionsDialog::OptionsDialog(PG_Widget* parent, PG_Rect rect)
    :PG_Window(parent, rect, _("Game settings"),PG_Window::MODAL)
{
    debug("OptionsDialog()");

    // To the left, we place a listbox (not the nicest thing, but works) to
    // select the general type of options to adjust, to the right part, the
    // single buttons etc. are placed.

    // Left: The listbox; it takes the first 200 pixels width
    d_submenu = new PG_ListBox(this, PG_Rect(20, 30, 180, my_height - 80));
    d_submenu->SetMultiSelect(false);
    new PG_ListBoxItem(d_submenu, 20, _("Video"), 0, (void*)VIDEO);
    new PG_ListBoxItem(d_submenu, 20, _("Audio"), 0, (void*)AUDIO);
    new PG_ListBoxItem(d_submenu, 20, _("Misc"), 0, (void*)GENERAL);


    // now the other items. First, the items for video mode
    d_fullscreen = new PG_CheckButton(this, PG_Rect(220, 60, 150, 20),_("Fullscreen"),0);
    if (Configuration::s_fullScreen)
        d_fullscreen->SetPressed();

    d_resolution = new PG_DropDown(this,  PG_Rect(220, 90, 150, 25),0);
    d_resolution->SetIndent(5);
    d_resolution->SetEditable(false);
    char val[11]; val[10] = '\0';
    snprintf(val, 10, "%dx%d",Configuration::s_width,Configuration::s_height);
    d_resolution->SetText(val);

    d_resolution->AddItem("800x600");
    d_resolution->AddItem("1024x768");
    d_resolution->AddItem("1280x1024");
    d_resolution->AddItem("1400x1050");
    d_resolution->AddItem("1600x1200");

    // next, the elements for audio setup (TBD)
    d_musicenable = new PG_CheckButton(this, PG_Rect(230, 60, 250, 30), _("Enable music"), 0);
    if (Configuration::s_musicenable)
        d_musicenable->SetPressed();

    d_musicvolume = new TerrainConfig(this, PG_Rect(230, 90, 250, 30), _("Volume:"),
                                      0, 128, Configuration::s_musicvolume);

    // finally, the general things
    d_smooth_scrolling = new PG_CheckButton(this, PG_Rect(230, 60, 250, 30), _("Smooth scrolling"),0);
    d_show_next_player = new PG_CheckButton(this, PG_Rect(230, 90, 300, 30), _("Show next player popup"),0);
    d_speeddelay = new TerrainConfig(this, PG_Rect(230, 120, 300, 30), _("Delay:"), 0, 1000,
                                            Configuration::s_displaySpeedDelay);

    if(Configuration::s_smoothScrolling)
        d_smooth_scrolling->SetPressed();

    if(Configuration::s_showNextPlayer)
        d_show_next_player->SetPressed();


    // finally, place the buttons
    PG_Rect myrect(5, my_height - 40, 0, 30);
    d_b_ok = new PG_Button(this, myrect, _("OK"));
    d_b_ok->SizeWidget(d_b_ok->GetTextWidth()+40,30,true);

    myrect.x += d_b_ok->my_width + 50;
    d_b_cancel = new PG_Button(this, myrect, _("Cancel"));
    d_b_cancel->SizeWidget(d_b_cancel->GetTextWidth()+40,30,true);

    myrect.x += d_b_cancel->my_width + 50;
    d_b_save = new PG_Button(this, myrect, _("Save"));
    d_b_save->SizeWidget(d_b_save->GetTextWidth()+40,30,true);

    d_b_ok->sigClick.connect(slot(*this, &OptionsDialog::okClicked));
    d_b_cancel->sigClick.connect(slot(*this, &OptionsDialog::cancelClicked));
    d_b_save->sigClick.connect(slot(*this, &OptionsDialog::saveClicked));
    d_submenu->sigSelectItem.connect(slot(*this, &OptionsDialog::modeSelected));

    d_submenu->SelectFirstItem();
}

OptionsDialog::~OptionsDialog()
{
}

bool OptionsDialog::okClicked(PG_Button* btn)
{
    Hide();

    /* Transfer button state to configuration */
    Configuration::s_showNextPlayer = d_show_next_player->GetPressed();
    Configuration::s_smoothScrolling = d_smooth_scrolling->GetPressed();
    Configuration::s_displaySpeedDelay = d_speeddelay->getValue();

    // set audio stuff
#ifdef FL_SOUND
    Mix_VolumeMusic(d_musicvolume->getValue());
    if (d_musicenable->GetPressed() != Configuration::s_musicenable)
    {
        // stop/restart music
        Sound::getInstance()->haltMusic();
        Sound::getInstance()->disableBackground();
        if (d_musicenable->GetPressed())        // music was enabled
        {
            Configuration::s_musicenable = d_musicenable->GetPressed();
            Sound::getInstance()->enableBackground();
        }
    }
#endif
    Configuration::s_musicenable = d_musicenable->GetPressed();
    Configuration::s_musicvolume = d_musicvolume->getValue();

    bool thesameres=false;            
    bool thesamefs=false;            

    if (d_fullscreen->GetPressed())
    {
        Configuration::s_flags|=SDL_FULLSCREEN;
        if (Configuration::s_fullScreen==true) 
            thesamefs=true;
        else
        {
        thesamefs=false;
            Configuration::s_fullScreen=true;
        }
    }
    else 
    {
        Configuration::s_flags&= ~SDL_FULLSCREEN;
        if (Configuration::s_fullScreen==false) 
            thesamefs=true;
        else
        {
            thesamefs=false;
            Configuration::s_fullScreen=false;
        }
    }
    
    string tmp=d_resolution->GetText();
    debug("String= -->" << tmp) 
    if (tmp.find("800x")!=string::npos)  
    {
        debug(tmp.find("800x")) 
        debug(tmp.find("x")) 
        debug(tmp.size())
        debug(tmp) 
        debug("inside 800x600")
        if (Configuration::s_width==800 && Configuration::s_height == 600)
            thesameres=true;   
        Configuration::s_width = 800;
        Configuration::s_height = 600;
    }    
    else if (tmp.find("1024x")!=string::npos)
    {
        debug("inside 1024x768") 
        debug(tmp.find("1024x")) 
        debug(tmp.size()) 
        if (Configuration::s_width==1024 && Configuration::s_height == 768)
            thesameres=true;   
        Configuration::s_width = 1024;
        Configuration::s_height = 768;
    }
    else if (tmp.find("1280x")!=string::npos)
    {
        debug("inside 1280x1024") 
        debug(tmp.find("1280x")) 
        debug(tmp.size()) 
        if (Configuration::s_width==1280 && Configuration::s_height == 1024)
            thesameres=true;   
        Configuration::s_width = 1280;
        Configuration::s_height = 1024;
    }
    else if (tmp.find("1400x")!=string::npos)
    {
        debug("inside 1400x1050") 
        debug(tmp.find("1400x")) 
        debug(tmp.size()) 
        if (Configuration::s_width==1400 && Configuration::s_height == 1050)
            thesameres=true;   
        Configuration::s_width = 1400;
        Configuration::s_height = 1050;
    }
    else if (tmp.find("1600x")!=string::npos)
    {
        debug("inside 1600x1200") 
        debug(tmp.find("1600x")) 
        debug(tmp.size()) 
        if (Configuration::s_width==1600 && Configuration::s_height == 1200)
            thesameres=true;   
        Configuration::s_width = 1600;
        Configuration::s_height = 1200;
    }

    if (!thesameres || !thesamefs) 
    {    
       cerr << Configuration::s_width << " -- " <<Configuration::s_height << endl;
       dynamic_cast<MainWindow*>(GetParent())->getWedit()->lockScreen();
       PG_Application::GetApp()->InitScreen(Configuration::s_width, Configuration::s_height, 16,Configuration::s_flags);       
       dynamic_cast<MainWindow*>(GetParent())->changeResolution(Configuration::s_width, Configuration::s_height);
       dynamic_cast<MainWindow*>(GetParent())->getWedit()->unlockScreen();
    }

    QuitModal();
    return true;
}

bool OptionsDialog::cancelClicked(PG_Button* btn)
{
    QuitModal();
    return true;
}

bool OptionsDialog::saveClicked(PG_Button* btn)
{

    okClicked(btn);
    
#ifndef __WIN32__
    char* home = getenv("HOME");
    Configuration::saveConfigurationFile(std::string(home) + "/.lordsawarrc");
#else
    Configuration::saveConfigurationFile("./lordsawarrc");
#endif

    QuitModal();
    return true;
}

bool OptionsDialog::modeSelected(PG_ListBoxBaseItem* item)
{
    d_fullscreen->Hide();
    d_resolution->Hide();
    d_musicenable->Hide();
    d_musicvolume->hide();
    d_smooth_scrolling->Hide();
    d_show_next_player->Hide();
    d_speeddelay->hide();

    unsigned long mode = (unsigned long)item->GetUserData();

    switch(mode)
    {
        case VIDEO:
            d_fullscreen->Show();
            d_resolution->Show();
            break;
        case AUDIO:
            d_musicenable->Show();
            d_musicvolume->show();
            break;
        case GENERAL:
            d_smooth_scrolling->Show();
            d_show_next_player->Show();
            d_speeddelay->show();
            break;
        default:
            break;
    }

    return true;
}

// End of file
