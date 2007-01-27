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

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <errno.h>

#include "GameScenario.h"
#include "MapCreationDialog.h"
#include "MapGenerator.h"
#include "playerlist.h"
#include "citylist.h"
#include "ruinlist.h"
#include "templelist.h"
#include "stonelist.h"
#include "roadlist.h"
#include "signpostlist.h"
#include "city.h"
#include "ruin.h"
#include "File.h"
#include "armysetlist.h"
#include "stacklist.h"
#include "GameMap.h"
#include "player.h"
#include "Configuration.h"
#include "real_player.h"
#include "ai_dummy.h"
#include "ai_fast.h"
#include "counter.h"
#include "army.h"
#include "QuestsManager.h"
#include "Itemlist.h"
#include "string_tokenizer.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)


GameScenario::GameScenario(std::string name,std::string comment, bool turnmode)
    :d_round(0), d_name(name),d_comment(comment), d_turnmode(turnmode)
{
    Armysetlist::getInstance();
    Itemlist::createInstance();

    if (fl_counter == 0)
        fl_counter = new FL_Counter();
}

// savegame is a filename with absolute path!

GameScenario::GameScenario(string savegame, bool& broken, bool events)
    :d_turnmode(true)
{
    Armysetlist::getInstance();
    Itemlist::createInstance();

    broken = false;
    XML_Helper helper(savegame, ios::in, Configuration::s_zipfiles);

    helper.registerTag("scenario", SigC::slot((*this), &GameScenario::load));
    helper.registerTag("event", SigC::slot((*this), &GameScenario::load));
    helper.registerTag("map", SigC::slot((*this), &GameScenario::load));
    helper.registerTag("playerlist", SigC::slot((*this), &GameScenario::load));
    helper.registerTag("citylist", SigC::slot((*this), &GameScenario::load));
    helper.registerTag("templelist", SigC::slot((*this), &GameScenario::load));
    helper.registerTag("ruinlist", SigC::slot((*this), &GameScenario::load));
    helper.registerTag("signpostlist", SigC::slot((*this), &GameScenario::load));
    helper.registerTag("stonelist", SigC::slot((*this), &GameScenario::load));
    helper.registerTag("roadlist", SigC::slot((*this), &GameScenario::load));
    helper.registerTag("counter", SigC::slot((*this), &GameScenario::load));
    helper.registerTag("questlist", SigC::slot((*this), &GameScenario::load));

    //now parse the document and close the file afterwards
    if (!helper.parse())
    {
        broken = true;
    }

    helper.close();

    //Important: Initialise the events (see Event.h for an explanation)
    if (events)
        for (std::list<Event*>::iterator it = d_events.begin();
                                         it != d_events.end(); it++)
            (*it)->init();
}

GameScenario::~GameScenario()
{
    while (!d_events.empty())
    {
        delete (*d_events.begin());
        d_events.erase(d_events.begin());
    }
    
    // GameMap is a Singleton so we need a function to delete it
    GameMap::deleteInstance();
    Playerlist::deleteInstance();
    Citylist::deleteInstance();
    Templelist::deleteInstance();
    Ruinlist::deleteInstance();
    Signpostlist::deleteInstance();
    Stonelist::deleteInstance();
    QuestsManager::deleteInstance();
    Itemlist::deleteInstance();

    if (fl_counter)
    {
        delete fl_counter;
        fl_counter = 0;
    }
} 

std::string GameScenario::getName(bool translate) const
{
    if (translate)
        return __(d_name);

    return d_name;
}

std::string GameScenario::getComment(bool translate) const
{
    if (translate)
        return __(d_comment);

    return d_comment;
}

void GameScenario::nextRound()
{
    d_round++;

    // autosave to the file "autosave.sav". This is crude, but should work
    //
    // As a more enhanced version: autosave to a temporary file, then rename
    // the file. Avoids screwing up the autosave if something goes wrong
    // (and we have a savefile for debugging)
    if (!saveGame(File::getSavePath() + "tmp.sav"))
    {
        std::cerr<<_("Autosave failed.\n");
        return;
    }
    if (rename(std::string(File::getSavePath() + "tmp.sav").c_str(),
               std::string(File::getSavePath() + "autosave.sav").c_str()))
    {
        char* err = strerror(errno);
        std::cerr <<_("Error while trying to rename the temporary file to autosave.sav\n");
        std::cerr <<_("Error: ") <<err <<std::endl;
    }
}

bool GameScenario::saveGame(string filename, string extension) const
{
    bool retval = true;
    string goodfilename=filename;
    
    stringTokenizer * strtoken= new stringTokenizer(filename,"/.\\ ");
       
    if (strtoken->getLastToken() == extension)
    {
        debug(_("The Filename is well formed"))
        std::cerr <<"";  //dummy call if debug statement is commented out
    }
    else 
    {
        debug(_("The Filename lacks the extension --> ") << strtoken->getLastToken())
        goodfilename += "." + extension;
    }

    delete strtoken;

    XML_Helper helper(goodfilename, ios::out, Configuration::s_zipfiles);

    //start writing
    retval &= helper.begin(LORDSAWAR_SAVEGAME_VERSION);
    retval &= helper.openTag("freelords");
    
    //if retval is still true it propably doesn't change throughout the rest
    //now save the single object's data
    retval &= fl_counter->save(&helper);
    retval &= GameMap::getInstance()->save(&helper);
    retval &= Playerlist::getInstance()->save(&helper);
    retval &= Citylist::getInstance()->save(&helper);
    retval &= Templelist::getInstance()->save(&helper);
    retval &= Ruinlist::getInstance()->save(&helper);
    retval &= Signpostlist::getInstance()->save(&helper);
    retval &= Stonelist::getInstance()->save(&helper);
    retval &= Roadlist::getInstance()->save(&helper);
    retval &= QuestsManager::getInstance()->save(&helper);

    //save the private GameScenario data last due to dependencies
    retval &= helper.openTag("scenario");
    retval &= helper.saveData("name", d_name);
    retval &= helper.saveData("comment", d_comment);
    retval &= helper.saveData("turn", d_round);
    retval &= helper.saveData("turnmode", d_turnmode);
    
    std::list<Event*>::const_iterator it;
    for (it = d_events.begin(); it != d_events.end(); it++)
        (*it)->save(&helper);
            
    retval &= helper.closeTag();
    
    if (!retval)
    {
        std::cerr <<_("GameScenario: Something went wrong with saving.\n");
        return false;
    }
    
    retval &= helper.closeTag();

    helper.close();

    if (retval)
        return true;
        
    std::cerr <<_("GameScenario: Something went wrong with saving.\n");
    return false;
}

bool GameScenario::load(std::string tag, XML_Helper* helper)
{
    if (tag == "scenario")
    {
        if (helper->getVersion() != LORDSAWAR_SAVEGAME_VERSION)
        {
            cerr <<_("savefile has wrong version, we want ");
            std::cerr <<LORDSAWAR_SAVEGAME_VERSION <<",\n";
            cerr <<_("savefile offers ") <<helper->getVersion() <<".\n";
            return false;
        }
    
        debug("loading scenario")
        helper->getData(d_round, "turn");
        helper->getData(d_turnmode, "turnmode");
        helper->getData(d_name, "name");
        helper->getData(d_comment, "comment");

        //TODO: for later when it becomes crucial: deal with loading of
        //something else than simple games
        return true;
    }

    if (tag == "event")
    {
        Event* ev = Event::loadEvent(helper);
        if (!ev)
            return false;
        d_events.push_back(ev);
        return true;
    }

    if (tag == "counter")
    {
        debug("loading counter")
        fl_counter = new FL_Counter(helper);
        return true;
    }

    if (tag == "map")
    {
        debug("loading map")
        GameMap::getInstance(helper);
        return true;
    }

    if (tag == "playerlist")
    {
        debug("loading players");
        Playerlist::getInstance(helper);
        return true;
    }

    if (tag == "citylist")
    {
        debug("loading cities")

        Citylist::getInstance(helper);
        return true;
    }

    if (tag == "templelist")
    {
        debug("loading temples")
        Templelist::getInstance(helper);
        return true;
    }

    if (tag == "ruinlist")
    {
        debug("loading ruins")
        Ruinlist::getInstance(helper);
        return true;
    }

    if (tag == "signpostlist")
    {
        debug("loading signposts")
            Signpostlist::getInstance(helper);
        return true;
    }

    if (tag == "stonelist")
    {
        debug("loading stones")
        Stonelist::getInstance(helper);
        return true;
    }

    if (tag == "roadlist")
    {
        debug("loading roads")
        Roadlist::getInstance(helper);
        return true;
    }

    if (tag == "questlist")
    {
        debug("loading quests")
        QuestsManager::getInstance(helper);
        return true;
    }

    return false;
}

void GameScenario::addEvent(Event* event)
{
    d_events.push_back(event);
}

void GameScenario::removeEvent(Event* event)
{
    for (std::list<Event*>::iterator it = d_events.begin(); it != d_events.end(); it++)
        if ((*it) == event)
        {
            delete event;
            d_events.erase(it);
            return;
        }
}

void GameScenario::deactivateEvents()
{
    for (std::list<Event*>::iterator it = d_events.begin(); it != d_events.end(); it++)
        (*it)->setActive(false);
}
