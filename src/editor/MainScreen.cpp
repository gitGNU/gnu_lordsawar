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

#include <pgmenubar.h>
#include <pgmessagebox.h>

#include "MainScreen.h"
#include "NewMapDialog.h"
#include "Bigmap.h"
#include "Smallmap.h"
#include "PlayerDialog.h"
#include "ScenarioDialog.h"
#include "FindDialog.h"
#include "EventDialog.h"

#include "../defs.h"
#include "../GameMap.h"
#include "../File.h"
#include "../PG_FileDialog.h"
#include "../Configuration.h"
#include "../GraphicsCache.h"
#include "../citiesreport.h"
#include "../stackreport.h"


E_MainScreen::E_MainScreen(PG_Application* app, const PG_Rect r)
    :PG_ThemeWidget(0, r), d_app(app), d_bigmap(0), d_smallmap(0),
    d_b_erase(0), d_b_stack(0), d_b_city(0), d_b_ruin(0), d_b_temple(0),
    d_b_signpost(0), d_b_stone(0), d_b_road(0), d_l_tilepos(0), 
    d_filename(""), d_scenario(0)
{
    // Children are removed automatically in the paragui destructor or when 
    // the program exits, so we don't need to care for them once they have
    // been set up.
    
    PG_MenuBar* menu = new PG_MenuBar(this, PG_Rect(0,0,r.w,25));
    
    // first, the "File" menu
    PG_PopupMenu* popup = new PG_PopupMenu(0, 20, 0, 0);
    popup->addSeparator();
    popup->addMenuItem(_("New"), 0, slot(*this, &E_MainScreen::file_new));
    popup->addMenuItem(_("Load"), 1, slot(*this, &E_MainScreen::file_load));
    popup->addMenuItem(_("Save"), 2, slot(*this, &E_MainScreen::file_save));
    popup->addMenuItem(_("Save as"), 3, slot(*this, &E_MainScreen::file_saveAs));
    popup->addMenuItem(_("Quit"), 4, slot(*this, &E_MainScreen::file_quit));
    menu->Add(_("File"), popup);

    popup = new PG_PopupMenu(0, 20, 0, 0);
    popup->addSeparator();
    popup->addMenuItem(_("Scenario"), 0, slot(*this, &E_MainScreen::edit_scenario));
    popup->addMenuItem(_("Players"), 0, slot(*this, &E_MainScreen::edit_players));
    popup->addMenuItem(_("Events"), 0, slot(*this, &E_MainScreen::edit_events));
    menu->Add(_("Edit"), popup);

    popup = new PG_PopupMenu(0, 20, 0, 0);
    popup->addSeparator();
    popup->addMenuItem(_("By ID"), 0, slot(*this, &E_MainScreen::find_id));
    menu->Add(_("Find"), popup);
    
    popup = new PG_PopupMenu(0, 30, 0, 0);
    popup->addSeparator();
    popup->addMenuItem(_("stacks"), 0, slot(*this, &E_MainScreen::report_stacks));
    popup->addMenuItem(_("cities"), 0, slot(*this, &E_MainScreen::report_cities));
    menu->Add(_("Reports"), popup);

    for (int i = 0; i < 2; i++)
        d_b_pointer[i] = 0;

    // these surfaces will be used several times
    d_s_pointer[0] = File::getEditorPic("button_1x1");
    d_s_pointer[1] = File::getEditorPic("button_3x3");
    d_erasepic = File::getEditorPic("button_erase");
    d_stackpic = File::getEditorPic("button_stack");
    d_ruinpic = File::getMapsetPicture("default", "misc/ruin.png");
    d_signpostpic = File::getMapsetPicture("default", "misc/signpost.png");
    d_stonepic = File::getMapsetPicture("default", "misc/stones.png");
    d_templepic = File::getMapsetPicture("default", "misc/temples.png");
    d_roadpic = File::getMapsetPicture("default", "misc/roads.png");
    d_citypic = File::getEditorPic("button_castle");
}

E_MainScreen::~E_MainScreen()
{
    for (int i = 0; i < 2; i++)
        SDL_FreeSurface(d_s_pointer[i]);

    SDL_FreeSurface(d_erasepic);
    SDL_FreeSurface(d_stackpic);
    SDL_FreeSurface(d_ruinpic);
    SDL_FreeSurface(d_signpostpic);
    SDL_FreeSurface(d_stonepic);
    SDL_FreeSurface(d_templepic);
    SDL_FreeSurface(d_roadpic);
    SDL_FreeSurface(d_citypic);
}

bool E_MainScreen::file_new(PG_PopupMenu::MenuItem* item, PG_Pointer p)
{
    // Query if the user wants to create a new map
    PG_MessageBox mb(this, PG_Rect(my_width/2 - 150, my_height/2 - 75, 300, 150),
                    "", _("Do you really want to create a new map (All changes will be lost)?"),
                    PG_Rect(60, 110, 50, 30), _("Yes"),
                    PG_Rect(200, 110, 50, 30), _("No"));
    mb.Show();

    if (mb.RunModal() == 2)
        // equals "No"
        return true;
    
    mb.Hide();
    d_filename = "";

    
    // remove the old data constructs before creating a new map
    clearMap();
    
    
    // E_NewMapDialog also sets up the new map and the lists
    E_NewMapDialog d(this, PG_Rect(my_width/2-150, my_height/2-100, 300, 250));
    d.Show();
    d.RunModal();
    d_scenario = d.getScenario();
    

    // initialises Bigmap and Smallmap etc.
    setupInterface();
    
    return true;
}

bool E_MainScreen::file_load(PG_PopupMenu::MenuItem* item, PG_Pointer p)
{
    PG_MessageBox mb(this, PG_Rect(my_width/2 - 150, my_height/2 - 75, 300, 150),
                    "", _("Do you really want to load a map (All changes will be lost)?"),
                    PG_Rect(60, 110, 50, 30), _("Yes"),
                    PG_Rect(200, 110, 50, 30), _("No"));
    mb.Show();

    if (mb.RunModal() == 2)
        // equals "No"
        return true;
    
    mb.Hide();

    clearMap();
    
    PG_FileDialog opendialog(0, PG_FileDialog::PG_OPEN, true, Configuration::s_savePath,
            "*.map", _("Freelords maps"));
    opendialog.Show();
    opendialog.RunModal();


    std::string mapfile = opendialog.getSelectedFile();
    if (mapfile.empty()) return true;

    bool broken;
    d_scenario = new GameScenario(mapfile, broken, false);

    if (broken)
    {
        std::cerr <<_("Couldn't load map ") <<mapfile <<_(". Bailing out.\n");
        exit(-1);
    }

    d_filename = mapfile;

    setupInterface();
    
    return true;
}

bool E_MainScreen::file_save(PG_PopupMenu::MenuItem* item, PG_Pointer p)
{
    // if no file has previously been loaded/saved, this works like saveAs
    if (d_filename.empty())
        return file_saveAs(item, p);
    
    bool success = d_scenario->saveGame(d_filename, "map");
    
    if (success)
    {
        PG_MessageBox mb(this, PG_Rect(200, 200, 200, 150), _("Save"),
                _("Map Saved"), PG_Rect(60, 110, 80, 30), _("OK"));
        mb.Show();
        mb.RunModal();
        mb.Hide();
    }
    else
    {
        PG_MessageBox mb(this, PG_Rect(200, 200, 200, 150), _("Save"),
                _("Error saving. Map not saved"),
                PG_Rect(60, 110, 80, 30), _("OK"));
        mb.Show();
        mb.RunModal();
        mb.Hide();
    }
    
    return true;
}

bool E_MainScreen::file_saveAs(PG_PopupMenu::MenuItem* item, PG_Pointer p)
{
    PG_FileDialog savedialog(0, PG_FileDialog::PG_SAVE, true, Configuration::s_savePath,
            "*.map", _("Freelords map files"));
    savedialog.Show();
    savedialog.RunModal();
    std::string savefile = savedialog.getSelectedFile();

    //nothing selected
    if (savefile.empty()) return true;

    //tell GameScenario to save the game
    bool success = d_scenario->saveGame(savefile, "map");
    
    if (success)
    {
        PG_MessageBox mb(this, PG_Rect(200, 200, 200, 150), _("Save"),
                _("Map Saved."), PG_Rect(60, 110, 80, 30), _("OK"));
        mb.Show();
        mb.RunModal();
        mb.Hide();

        d_filename = savefile;
    }
    else
    {
        PG_MessageBox mb(this, PG_Rect(200, 200, 200, 150), _("Save"),
                _("Error saving. Map not saved"),
                PG_Rect(60, 110, 80, 30), _("OK"));
        mb.Show();
        mb.RunModal();
        mb.Hide();
    }

    return true;
}

bool E_MainScreen::file_quit(PG_PopupMenu::MenuItem* item, PG_Pointer p)
{
    d_app->Quit();
    return true;
}

bool E_MainScreen::edit_scenario(PG_PopupMenu::MenuItem* item, PG_Pointer p)
{
    if (!d_bigmap)
        return true;

    E_ScenarioDialog d(this, PG_Rect(my_width/2-250, my_height/2 - 200,
                                  230, 180), d_scenario);
    d.Show();
    d.RunModal();
    d.Hide();

    return true;
}

bool E_MainScreen::edit_players(PG_PopupMenu::MenuItem* item, PG_Pointer p)
{
    // if no scenario has been loaded, ignore the call
    if (!d_bigmap)
        return true;
    
    E_PlayerDialog d(this, PG_Rect(my_width/2 - 225, my_height/2 - 200, 450, 400));
    d.Show();
    d.RunModal();

    GraphicsCache::getInstance()->clear();  // in case player colors changed
    d_bigmap->Redraw();
    d_smallmap->Redraw();

    return true;
}

bool E_MainScreen::edit_events(PG_PopupMenu::MenuItem* item, PG_Pointer p)
{
    if (!d_bigmap)
        return true;

    E_EventDialog d(this, PG_Rect(my_width/2-250, my_height/2 - 200,
                                  500, 400), d_scenario);
    d.Show();
    d.RunModal();
    d.Hide();

    return true;
}

bool E_MainScreen::find_id(PG_PopupMenu::MenuItem* item, PG_Pointer p)
{
    if (!d_bigmap)
        return true;
    
    E_FindDialog d(this, PG_Rect(my_width/2-110, my_height/2-75, 220, 150), d_bigmap);
    d.Show();
    d.RunModal();

    return true;
}

bool E_MainScreen::report_stacks(PG_PopupMenu::MenuItem* item, PG_Pointer p)
{
    if (!d_bigmap)
        return true;
    
    StackReport report(this, PG_Rect(my_width/2-250, my_height/2-200, 530, 400));
    report.sselectingStack.connect(SigC::slot(*d_bigmap, &E_Bigmap::centerView));
    report.Show();
    report.RunModal();
    report.Hide();

    return true;
}

bool E_MainScreen::report_cities(PG_PopupMenu::MenuItem* item, PG_Pointer p)
{
    if (!d_bigmap)
        return true;
    
    CitiesReport report(this, PG_Rect(Width()/2 - 250, Height()/2 - 200, 530, 400),true);
    report.sselectingCity.connect(SigC::slot(*d_bigmap, &E_Bigmap::centerView));
    report.Show();
    report.RunModal();
    report.Hide();

    return true;
}

bool E_MainScreen::pointerClicked(PG_Button* btn)
{
    bool d_pressed = btn->GetPressed();

    // deselect all buttons and reselect the one afterwards
    for (int i = 0; i < 2; i++)
        d_b_pointer[i]->SetPressed(false);
    d_b_erase->SetPressed(false);
    d_b_stack->SetPressed(false);
    d_b_city->SetPressed(false);
    d_b_ruin->SetPressed(false);
    d_b_signpost->SetPressed(false);
    d_b_stone->SetPressed(false);
    d_b_temple->SetPressed(false);
    d_b_road->SetPressed(false);

    // if the button was pressed before, we leave it unpressed now
    if (d_pressed)
        btn->SetPressed(true);

    changeBigmapStatus();
    
    return true;
}

bool E_MainScreen::terrainClicked(PG_Button* btn)
{
    // first, deselect all terain buttons, then push the one that was clicked
    std::list<PG_Button*>::iterator it;
    for (it = d_b_terrain.begin(); it != d_b_terrain.end(); it++)
        (*it)->SetPressed(false);
    btn->SetPressed(true);

    changeBigmapStatus();
    
    return true;
}

void E_MainScreen::movingMouse(PG_Point pos)
{
    if (pos.x < 0 || pos.y < 0)
    {
        d_l_tilepos->SetText("");
        return;
    }
    
    char buffer[31]; buffer[30]='\0';
    snprintf(buffer, 30, "(%i,%i)", pos.x, pos.y);
    d_l_tilepos->SetText(buffer);
}

void E_MainScreen::clearMap()
{
    // remove bigmap and smallmap first (in case they need some of the data)
    if (d_bigmap)
    {
        delete d_bigmap;
        d_bigmap = 0;
    }
    if (d_smallmap)
    {
        delete d_smallmap;
        d_smallmap = false;
    }

    // GameScenario will kill all game lists when removed
    if (d_scenario)
    {
        delete d_scenario;
        d_scenario = 0;
    }

    // furthermore, clear the pointers and terrain buttons
    for (int i = 0; i < 2; i++)
        if (d_b_pointer[i])
        {
            delete d_b_pointer[i];
            d_b_pointer[i] = 0;
        }
    while (!d_b_terrain.empty())
    {
        delete *(d_b_terrain.begin());
        d_b_terrain.erase(d_b_terrain.begin());
        delete *(d_l_terrain.begin());
        d_l_terrain.erase(d_l_terrain.begin());
    }
    if (d_b_erase)
        delete d_b_erase;
    if (d_b_stack)
        delete d_b_stack;
    if (d_b_city)
        delete d_b_city;
    if (d_b_ruin)
        delete d_b_ruin;
    if (d_b_signpost)
        delete d_b_signpost;
    if (d_b_stone)
        delete d_b_stone;
    if (d_b_temple)
        delete d_b_temple;
    if (d_b_road)
        delete d_b_road;
    if (d_l_tilepos)
        delete d_l_tilepos;
}

void E_MainScreen::setupInterface()
{
    if (d_bigmap || d_smallmap || !d_scenario)
        return;

    // We want to place the smallmap in the upper right corner and the bigmap on
    // the upper left side. Since the upper map limit is 200 tiles, we reserve a
    // bit more than 200 pixels for the smallmap.
    int width = GameMap::getInstance()->getWidth();
    int height = GameMap::getInstance()->getHeight();
    int tilesize = GameMap::getInstance()->getTileSet()->getTileSize();

    PG_Rect smallrect, bigrect;
    smallrect.x = my_width - 116 - width/2;
    smallrect.y = 116 - height/2;
    smallrect.w = width + 2;         //+2 for the border
    smallrect.h = height + 2;

    bigrect.x = 15;
    bigrect.y = 45;
    bigrect.w = (my_width - 240);
    bigrect.w -= bigrect.w % tilesize;
    bigrect.h = (my_height - 40);
    bigrect.h -= bigrect.h % tilesize;

    int tilex = bigrect.w/tilesize;
    int tiley = bigrect.h/tilesize;

    // Now create the maps and link them together
    d_bigmap = new E_Bigmap(this, bigrect);
    d_smallmap = new E_Smallmap(this, smallrect, tilex, tiley);

    d_bigmap->schangingView.connect(SigC::slot(*d_smallmap, &E_Smallmap::centerView));
    d_bigmap->schangingMap.connect(SigC::slot(*d_smallmap, &E_Smallmap::Redraw));
    d_bigmap->smovingMouse.connect(SigC::slot(*this, &E_MainScreen::movingMouse));
    d_smallmap->schangingView.connect(SigC::slot(*d_bigmap, &E_Bigmap::centerView));

    // next, create the buttons for the pointers...
    for (int i = 0; i < 2; i++)
    {
        PG_Rect r(bigrect.x+bigrect.w+52+i*tilesize, smallrect.y+smallrect.h+30, 40, 40);
        d_b_pointer[i] = new PG_Button(this, r, "");
        d_b_pointer[i]->SetIcon(d_s_pointer[i]);
        d_b_pointer[i]->SetToggle(true);
        d_b_pointer[i]->sigClick.connect(slot(*this, &E_MainScreen::pointerClicked));
    }

    // ...and the terrain selection...
    TileSet* tset = GameMap::getInstance()->getTileSet();
    for (unsigned int i = 0; i < tset->size(); i++)
    {
        PG_Rect rb(bigrect.x+bigrect.w+80, smallrect.y+smallrect.h+100+i*35, 25, 25);
        PG_Rect rl(rb.x + rb.w + 20, rb.y + 5, 100, 20);

        // first the button...
        PG_Button* b = new PG_Button(this, rb, "");
        b->SetIcon((*tset)[i]->getSurface(0));
        b->SetToggle(true);
        b->sigClick.connect(slot(*this, &E_MainScreen::terrainClicked));
        d_b_terrain.push_back(b);

        // ...then the label
        PG_Label* l = new PG_Label(this, rl, (*tset)[i]->getName().c_str());
        d_l_terrain.push_back(l);
        
        // already show the buttons, this saves a useless loop lateron
        b->Show();
        l->Show();
    }
    
    // ...and the other items
    d_b_erase = new PG_Button(this, PG_Rect(bigrect.x+bigrect.w+180, smallrect.y+smallrect.h+30, 40, 40));
    d_b_erase->SetToggle(true);
    d_b_erase->SetIcon(d_erasepic);
    d_b_erase->sigClick.connect(slot(*this, &E_MainScreen::pointerClicked));

    PG_Rect buttonrect(bigrect.x+bigrect.w+30, smallrect.y+smallrect.h+320, 50, 50);
    d_b_stack = new PG_Button(this, buttonrect);
    d_b_stack->SetToggle(true);
    d_b_stack->SetIcon(d_stackpic);
    d_b_stack->sigClick.connect(slot(*this, &E_MainScreen::pointerClicked));
    
    buttonrect.x += 50;
    d_b_city = new PG_Button(this, buttonrect);
    d_b_city->SetToggle(true);
    d_b_city->SetIcon(d_citypic);
    d_b_city->sigClick.connect(slot(*this, &E_MainScreen::pointerClicked));

    buttonrect.x += 50;
    d_b_ruin = new PG_Button(this, buttonrect);
    d_b_ruin->SetToggle(true);
    d_b_ruin->SetIcon(d_ruinpic);
    d_b_ruin->sigClick.connect(slot(*this, &E_MainScreen::pointerClicked));
    
    buttonrect.x += 50;
    d_b_temple = new PG_Button(this, buttonrect);
    d_b_temple->SetToggle(true);
    d_b_temple->SetIcon(d_templepic);
    d_b_temple->sigClick.connect(slot(*this, &E_MainScreen::pointerClicked));

    buttonrect.x -= 150;
    buttonrect.y += 50;
    d_b_signpost = new PG_Button(this, buttonrect);
    d_b_signpost->SetToggle(true);
    d_b_signpost->SetIcon(d_signpostpic);
    d_b_signpost->sigClick.connect(slot(*this, &E_MainScreen::pointerClicked));
    
    buttonrect.x += 50;
    d_b_stone = new PG_Button(this, buttonrect);
    d_b_stone->SetToggle(true);
    d_b_stone->SetIcon(GraphicsCache::getInstance()->getStonePic(8));
    //d_b_stone->SetIcon(d_stonepic);    
    d_b_stone->sigClick.connect(slot(*this, &E_MainScreen::pointerClicked));
    
     buttonrect.x += 50;
     d_b_road = new PG_Button(this, buttonrect);
     d_b_road->SetToggle(true);
     d_b_road->SetText("road");
     //d_b_road->SetIcon(d_roadpic);
     d_b_road->sigClick.connect(slot(*this, &E_MainScreen::pointerClicked));
   
    buttonrect.x = bigrect.x+bigrect.w - 60;
    buttonrect.y = bigrect.y - 20;
    buttonrect.w = 50;
    buttonrect.h = 20;
    d_l_tilepos = new PG_Label(this, buttonrect, "");
    d_l_tilepos->SetAlignment(PG_Label::CENTER);

    // select the first terrain type as default
    terrainClicked(*d_b_terrain.begin());

    // show everything
    d_bigmap->Show();
    d_smallmap->Show();
    d_b_erase->Show();
    d_b_stack->Show();
    d_b_city->Show();
    d_b_ruin->Show();
    d_b_signpost->Show();
    d_b_stone->Show();
    d_b_road->Show();
    d_b_temple->Show();
    d_l_tilepos->Show();

    for (int i = 0; i < 2; i++)
        d_b_pointer[i]->Show();
}

void E_MainScreen::changeBigmapStatus()
{
    // first, decide what status we have
    E_Bigmap::STATUS status = E_Bigmap::NONE;

    if (d_b_pointer[0]->GetPressed())
        status = E_Bigmap::AREA1x1;
    if (d_b_pointer[1]->GetPressed())
        status = E_Bigmap::AREA3x3;
    if (d_b_erase->GetPressed())
        status = E_Bigmap::ERASE;
    if (d_b_stack->GetPressed())
        status = E_Bigmap::STACK;
    if (d_b_city->GetPressed())
        status = E_Bigmap::CITY;
    if (d_b_ruin->GetPressed())
        status = E_Bigmap::RUIN;
    if (d_b_signpost->GetPressed())
        status = E_Bigmap::SIGNPOST;
    if (d_b_stone->GetPressed())
        status = E_Bigmap::STONE;
    if (d_b_temple->GetPressed())
        status = E_Bigmap::TEMPLE;
    if (d_b_road->GetPressed())
        status = E_Bigmap::ROAD;

    // now find out the terrain type(the type doesn't matter in case of
    // non-terrain)
    int i = 0;
    std::list<PG_Button*>::iterator it;
    for (it = d_b_terrain.begin(); it != d_b_terrain.end(); it++, i++)
        if ((*it)->GetPressed())
            break;

    d_bigmap->changeStatus(status, i);
}
