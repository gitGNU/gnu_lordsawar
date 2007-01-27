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

#include <stdlib.h>
#include <pglistboxitem.h>
#include <pglabel.h>
#include <pglineedit.h>
#include <pgmessagebox.h>
#include <pgapplication.h>

#include "NewMapDialog.h"
#include "MapGenDialog.h"
#include "../GameMap.h"
#include "../playerlist.h"
#include "../armysetlist.h"
#include "../ai_dummy.h"
#include "../citylist.h"
#include "../ruinlist.h"
#include "../templelist.h"
#include "../signpostlist.h"
#include "../stonelist.h"


E_NewMapDialog::E_NewMapDialog(PG_Widget* parent, PG_Rect rect)
    :PG_Window(parent, rect, _("Create New Map"), PG_Window::MODAL), d_scenario(0)
{
    // dropdown to select the tileset (currently only "default")
    d_tilesets = new PG_DropDown(this, PG_Rect(20, 40, 200, 20), 1);
    d_tilesets->AddItem("default");
    d_tilesets->SetText("default");
    d_tilesets->SetEditable(false);
    d_tilesets->sigSelectItem.connect(SigC::slot(*this, &E_NewMapDialog::tilesetChanged));

    // dropdown to select default creation style; filled when tileset is selected
    // TODO: use MapGenerator to create a map
    new PG_Label(this, PG_Rect(20, 80, 200, 20), _("Select fill style"));
    
    d_types = new PG_DropDown(this, PG_Rect(20, 105, 200, 20), 2);
    d_types->SetEditable(false);

    // set size of the map
    new PG_Label(this, PG_Rect(20, 145, 50, 20), _("Width:"));
    new PG_Label(this, PG_Rect(20, 175, 50, 20), _("Height:"));
    
    d_width = new PG_LineEdit(this, PG_Rect(90, 145, 50, 20), "LineEdit", 3);
    d_width->SetText("100");
    d_width->SetValidKeys("0123456789");
    d_height = new PG_LineEdit(this, PG_Rect(90, 175, 50, 20), "LineEdit", 3);
    d_height->SetText("100");
    d_height->SetValidKeys("0123456789");

    // OK button
    PG_Button* button = new PG_Button(this, PG_Rect(my_width/2 - 50, my_height - 40, 100, 30),
                                      _("Create"), 3);
    button->sigClick.connect(SigC::slot(*this, &E_NewMapDialog::okClicked));

    // fill the 2nd dropdown box
    tilesetChanged(0);
}

E_NewMapDialog::~E_NewMapDialog()
{
}

bool E_NewMapDialog::tilesetChanged(PG_ListBoxBaseItem* item)
{
    std::string name(d_tilesets->GetText());
    
    // get the tileset
    GameMap::getInstance(name);
    TileSet* ts = GameMap::getInstance()->getTileSet();
    
    // now remove all entries from d_types and fill it with new values
    d_types->DeleteAll();

    d_types->AddItem(_("Random Map"));
    for (unsigned int i = 0; i < ts->size();i++)
        d_types->AddItem((*ts)[i]->getName().c_str());
        
    d_types->SelectFirstItem();
    
    return true;
}

bool E_NewMapDialog::okClicked(PG_Button* button)
{
    // transformation should always work because d_width/d_height only accept numbers
    int mapwidth = atoi(d_width->GetText());
    int mapheight = atoi(d_height->GetText());

    if ((mapwidth > 200) || (mapwidth < 20) || (mapheight > 200) || (mapheight < 20))
    {
        PG_MessageBox mb(this, PG_Rect(my_width/2 - 125, my_height/2 - 75, 250, 150),
                        _("Error"), _("Mapsize must be between 20 and 200!"),
                        PG_Rect(85, 110, 80, 30), _("OK"));
        mb.Show();
        mb.RunModal();
        return true;
    }
    
    Hide();

    // resize the GameMap (this could be less ugly)
    GameMap::deleteInstance();
    GameMap::setWidth(mapwidth);
    GameMap::setHeight(mapheight);
    GameMap::getInstance(std::string(d_tilesets->GetText()));

    // this sets up the lists...
    d_scenario = new GameScenario("DefaultName", "DefaultComment", true);

    // ...however we need to do some of the setup by hand.
    // We need to create a neutral player to give cities a player upon creation...
    SDL_Color c;
    c.r = c.g = c.b = 150;c.unused=0;
    Uint32 armyset = Armysetlist::getInstance()->getArmysets()[0];
    Player* neutral = new AI_Dummy("Neutral", armyset, c);
    neutral->setType(Player::AI_DUMMY);
    Playerlist::getInstance()->push_back(neutral);
    Playerlist::getInstance()->setNeutral(neutral);
    Playerlist::getInstance()->nextPlayer();

    //...set up the map
    std::string tilename(d_types->GetText());

    if (tilename != std::string(_("Random Map")))
    {
        // fill the map with the specified terrain
        TileSet* ts = GameMap::getInstance()->getTileSet();
    
        unsigned int index;
        for (index = 0; index < ts->size(); index++)
            if ((*ts)[index]->getName() == tilename)
                break;

        GameMap::getInstance()->fill(index);
    }
    else
    {
        // create a random map
        MapGenerator gen;
        
        // first, fill the generator with data
        int width = PG_Application::GetScreenWidth();
        int height = PG_Application::GetScreenHeight();
        PG_Rect r(width/2 - 200, height/2 - 250, 400, 500);
        E_MapGenDialog* d = new E_MapGenDialog(0, r, &gen);
        d->Show();
        d->RunModal();
        delete d;
        
        gen.makeMap(mapwidth, mapheight);
        GameMap::getInstance()->fill(&gen);

        // now fill the city lists
        const Maptile::Building* build = gen.getBuildings(mapwidth,mapheight);
        for (int j = 0; j < mapheight; j++)
            for (int i = 0; i < mapwidth; i++)
                switch(build[j*mapwidth+i])
                {
                    case Maptile::CITY:
                        Citylist::getInstance()->push_back(City(PG_Point(i,j)));
                        (*Citylist::getInstance()->rbegin()).setPlayer(
                                Playerlist::getInstance()->getNeutral());
                        break;
                    case Maptile::TEMPLE:
                        Templelist::getInstance()->push_back(Temple(PG_Point(i,j)));
                        break;
                    case Maptile::RUIN:
                        Ruinlist::getInstance()->push_back(Ruin(PG_Point(i,j)));
                        break;
                    case Maptile::SIGNPOST:
                        Signpostlist::getInstance()->push_back(Signpost(PG_Point(i,j)));
                        break;
                    case Maptile::STONE:
                        Stonelist::getInstance()->push_back(Stone(PG_Point(i,j)));
			break;
                    case Maptile::NONE:
                        break;
                }
    }
        
    //...this is it, isn't it?
    QuitModal();
    return true;
}
