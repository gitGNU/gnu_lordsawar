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

#include <sstream>

#include "CreateScenario.h"
#include "army.h"
#include "GameMap.h"
#include "counter.h"
#include "real_player.h"
#include "ai_fast.h"
#include "ai_smart.h"
#include "ai_dummy.h"
#include "File.h"
#include "MapCreationDialog.h"
#include "MapGenerator.h"
#include "QuestsManager.h"
#include "Configuration.h"
#include "events/EPlayerDead.h"
#include "events/EKillAll.h"
#include "events/ENextTurn.h"
#include "events/RMessage.h"
#include "events/RActEvent.h"
#include "events/RWinGame.h"
#include "events/RLoseGame.h"
#include "events/CPlayer.h"
#include "events/CLiving.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

CreateScenario::CreateScenario(PG_Widget* uncle)
    :d_scenario(0), d_generator(0), d_uncle(uncle)
{
    debug("CreateScenario::CreateScenario")
   
   //default value
    setWidth(100);
    setHeight(100);
    
    //make sure that objects are deleted
    GameMap::deleteInstance();
    Playerlist::deleteInstance();
    Templelist::deleteInstance();
    Ruinlist::deleteInstance();
    Citylist::deleteInstance();

    QuestsManager::deleteInstance();

    fl_counter = new FL_Counter();

    // Fill the namelists 
    bool success = true;
    
    std::ifstream file(File::getMiscFile("citynames").c_str());
    success &= loadNames(d_citynames, file);
    file.close();

    file.open(File::getMiscFile("templenames").c_str());
    success &= loadNames(d_templenames, file);
    file.close();

    file.open(File::getMiscFile("ruinnames").c_str());
    success &= loadNames(d_ruinnames, file);
    file.close();

    file.open(File::getMiscFile("signposts").c_str());
    success &= loadNames(d_signposts, file);
    file.close();

    if (!success)
    {
        std::cerr <<"CreateScenario: Didn't succeed in reading object names. Aborting!\n";
        exit(-1);
    }
}

CreateScenario::~CreateScenario()
{
    debug("CreateScenario::~CreateScenario")

    if (d_generator)
        delete d_generator;

    if (d_scenario)
        delete d_scenario;
}

void CreateScenario::setMaptype(MapType type)
{
    debug("CreateScenario::setMaptype")

    //this function currently does almost nothing. It's purpose is to initialize
    //the right MapGenerator (e.g. islands, mountain chains, normal etc.).
    //Currently we have only a normal d_generator, so it is initialized here

    if (d_generator)
        delete d_generator;

    d_generator = new MapGenerator();
}

void CreateScenario::setPercentages(int pgrass, int pwater, int pforest,
                                    int pswamp, int phills, int pmountains)
{
    debug("CreateScenario::setPercentages")

    if (!d_generator)
        setMaptype(NORMAL);

    //handle input with !=100% sum
    int sum = pgrass + pwater + pforest + pswamp +phills + pmountains;

    if (sum != 100)
    {
        double factor = 100 / static_cast<double>(sum);  

        pwater = static_cast<int>(pwater * factor);
        pforest = static_cast<int>(pforest * factor);
        pswamp = static_cast<int>(pswamp * factor);
        phills = static_cast<int>(phills * factor);
        pmountains = static_cast<int>(pmountains * factor);
    }

    //the multiplication doesn't round up, so the figures should be OK now, the
    //missing percentage is implicitely added to the grass part.
    d_generator->setPercentages(pwater, pforest, pswamp, phills, pmountains);
}

void CreateScenario::setMapTiles(std::string tilesname)
{
    debug("CreateScenario::setMapTiles")
    d_tilesname = tilesname;
}

void CreateScenario::setNoCities(int nocities)
{
    debug("CreateScenario::setNoCities")

    if (!d_generator)
        setMaptype(NORMAL);

    d_generator->setNoCities(nocities);
}

void CreateScenario::setNoRuins(int noruins)
{
    debug("CreateScenario::setNoRuins")

    if (!d_generator)
        setMaptype(NORMAL);

    d_generator->setNoRuins(noruins);
}

void CreateScenario::setNoSignposts (int nosignposts)
{
    debug("CreateScenario::setNoSignposts")

    if (!d_generator)
        setMaptype(NORMAL);

    d_generator->setNoSignposts(nosignposts);
}

void CreateScenario::setNoStones(int nostones)
{
    debug("CreateScenario::setNoStones")

    if (!d_generator)
        setMaptype(NORMAL);

    d_generator->setNoStones(nostones);
}

void CreateScenario::setNoTemples(int notemples)
{
    debug("CreateScenario::setNoTemples")

    if (!d_generator)
        setMaptype(NORMAL);

    d_generator->setNoTemples(notemples);
}

void CreateScenario::setWidth(int width)
{
    debug("CreateScenario::setWidth")

    if (width < 0)
    {
        std::cerr <<_("CreateScenario:: wrong width given\n");
        return;
    }

    d_width = width;

    //IMPORTANT!!
    GameMap::setWidth(width);
}
void CreateScenario::setHeight(int height)
{
    debug("CreateScenario::setHeight")

    if (height < 0)
    {
        std::cerr <<_("CreateScenario:: wrong height given\n");
        return;
    }

    d_height = height;

    //IMPORTANT!!
    GameMap::setHeight(height);
}

Player* CreateScenario::addPlayer(std::string name, Uint32 armyset,
                                SDL_Color color, Player::Type type)
{
    debug("CreateScenario::addPlayer")

    Player* p = Player::create(name, armyset, color, type);
    Playerlist::getInstance()->push_back(p);

    return p;
}

bool CreateScenario::addNeutral(std::string name, Uint32 armyset,
                                SDL_Color color, Player::Type type)
{
    // for consistency, we only allow exactly one neutral player
    if (Playerlist::getInstance()->getNeutral() != 0)
        return false;

    Player* p = addPlayer(name, armyset, color, type);
    Playerlist::getInstance()->setNeutral(p);
    return true;
}

int CreateScenario::getNoPlayers() const
{
    return Playerlist::getInstance()->size();
}

Player* CreateScenario::getPlayer(int number) const
{
    debug("CreateScenario::getPlayer")

    if (number >= static_cast<int>(Playerlist::getInstance()->size()))
        return 0;

    Playerlist::iterator it;
    for (it = Playerlist::getInstance()->begin(); number > 0; number--)
        it++;

    return (*it);
}

int CreateScenario::getNoCities() const
{
    if (!d_generator)
        return -1;

    return d_generator->getNoCities();
}

int CreateScenario::getNoRuins() const
{
    if (!d_generator)
        return -1;

    return d_generator->getNoRuins();
}

int CreateScenario::getNoStones() const
{
    if (!d_generator)
        return -1;

    return d_generator->getNoStones();
}

int CreateScenario::getNoTemples() const
{
    if (!d_generator)
        return -1;

    return d_generator->getNoTemples();
}

std::string CreateScenario::getMapTiles() const
{
    debug("CreateScenario::getMapTiles")
    return d_tilesname;
}

bool CreateScenario::create()
{
    debug("CreateScenario::create")

    d_scenario = new GameScenario("AutoGenerated", "AutoGenerated", d_turnmode);

    PG_Rect rect;
    rect.x = (d_uncle->Width() / 2) - 150;
    rect.y = (d_uncle->Height() / 2) - 50;
    rect.w = 300;
    rect.h = 100;
    MapCreationDialog dialog(d_uncle, rect);
    dialog.Show();

    dialog.setProgress(0, _("Creating map"));
    if (!createMap())
        return false;

    dialog.setProgress(30, _("Distributing players"));
    if (!distributePlayers())
        return false;

    dialog.setProgress(50, _("Setting up cities"));
    if (!setupCities())
        return false;

    dialog.setProgress(60, _("Setting up temples"));
    if (!setupTemples())
        return false;
    
    dialog.setProgress(70, _("Setting up ruins"));
    if (!setupRuins())
        return false;

    dialog.setProgress(80, _("Setting up signposts"));
    if (!setupSignposts())
        return false;

    dialog.setProgress(90, _("Setting up players"));
    if (!setupPlayers())
        return false;

    dialog.setProgress(95, _("Setting up victory conditions"));
    if (!setupEvents())
        return false;
    
    dialog.Hide();

    return true;
}

bool CreateScenario::dump(std::string filename) const
{
    debug("CreateScenario::dump")

    if (d_scenario)
        return d_scenario->saveGame(filename, "map");

    return false;
}

bool CreateScenario::createMap()
{
    debug("CreateScenario::createMap")

    const Maptile::Building* map;
    
    //have the generator make the map...
    d_generator->makeMap(d_width, d_height);
    
    //...fill the terrain...
    GameMap::getInstance(d_tilesname)->fill(d_generator);

    //...and create cities, temples, ruins ,signposts, and stones
    map = d_generator->getBuildings(d_width, d_height);
    
    for (int y = 0; y < d_height; y++)
        for (int x = 0; x < d_width; x++)
        {
            switch (map[y*d_width + x])
            {
                case Maptile::STONE:
                    Stonelist::getInstance()->push_back(Stone(PG_Point(x,y)));
                    break;
                case Maptile::SIGNPOST:
                    Signpostlist::getInstance()->push_back(Signpost(PG_Point(x,y)));
                    break;
                case Maptile::TEMPLE:
                    Templelist::getInstance()->push_back(Temple(PG_Point(x,y)));
                    break;
                case Maptile::RUIN:
		    Ruinlist::getInstance()->push_back(Ruin(PG_Point(x,y)));
		    break;
                case Maptile::CITY:
                    Citylist::getInstance()->push_back(City(PG_Point(x,y)));
                case Maptile::NONE:
		    break;
            }
        }
    //the other details such as giving names are done later
    
    return true;
}

bool CreateScenario::distributePlayers()
{
    debug("CreateScenario::distributePlayers")

    //how many cities we skip after a player assignment
    int cityskip = (Citylist::getInstance()->size() - 1) / (Playerlist::getInstance()->size() - 1);
    
    int skipping = 0;
    Citylist* cl = Citylist::getInstance();
    const Playerlist* pl = Playerlist::getInstance();

    Playerlist::const_iterator pit = pl->begin();
    if ((*pit) == pl->getNeutral())
        pit++;

    for (Citylist::iterator cit = cl->begin(); cit != cl->end(); cit++, skipping++)
    {
        if ((skipping >= cityskip) && (pit != pl->end()))
        {
            // distribute capitals for the players
            (*cit).setPlayer(*pit);
            (*cit).setCapital(true);
            skipping = 0;

            pit++;
            if ((*pit) == pl->getNeutral())
                pit++;
        }
        else
            (*cit).setPlayer(pl->getNeutral());
    }

    return true;
}

bool CreateScenario::setupCities()
{
    debug("CreateScenario::setupCities")

    for (Citylist::iterator it = Citylist::getInstance()->begin();
        it != Citylist::getInstance()->end(); it++)
    {
        //1. set a reasonable cityname
        int randno = rand() % d_citynames.size();
        (*it).setName(d_citynames[randno]);

        //remove the used name
        d_citynames[randno] = d_citynames[d_citynames.size() - 1];
        d_citynames.pop_back();

        //2. distribute the income a bit (TBD)

        //3. set the city production
        (*it).setRandomArmytypes();
        (*it).produceLargestArmy();
        if ((*it).getPlayer() == Playerlist::getInstance()->getNeutral())
            (*it).setProduction(-1, false);
        else
        {
            (*it).setProduction(0, false);
            //raise the city defense to the highest level
            for (; (*it).raiseDefense(););
        }
    }

    return true;
}

bool CreateScenario::setupTemples()
{
    Templelist* tl = Templelist::getInstance();
    for (Templelist::iterator it = tl->begin(); it != tl->end(); it++)
    {
        // set a temple name
        int randno = rand() % d_templenames.size();
        (*it).setName(d_templenames[randno]);

        // set a random temple type
        int type= (int) ((TEMPLE_TYPES*1.0) * (rand() / (RAND_MAX + 1.0)));
        (*it).setType(type);

        //remove the used name
        d_templenames[randno] = d_templenames[d_templenames.size() - 1];
        d_templenames.pop_back();
    }

    return true;
}

bool CreateScenario::setupRuins()
{
    debug("CreateScenario::setupRuins")

    //The aim of this function is to put a strong stack as sentinel in all
    //ruins.

    for (Ruinlist::iterator it = Ruinlist::getInstance()->begin();
        it != Ruinlist::getInstance()->end(); it++)
    {
        // set a ruin name
        int randno = rand() % d_ruinnames.size();
        (*it).setName(d_ruinnames[randno]);

        //remove the used name
        d_ruinnames[randno] = d_ruinnames[d_ruinnames.size() - 1];
        d_ruinnames.pop_back();

        // and set a guardian
        Stack* s;
        const Army* a = 0;
        PG_Point pos = (*it).getPos();
        
        //create a strong stack:
        s = new Stack(0, pos);

        //(i) insert one random army first (no owning player). The army is an
        //arbitrary army except ships and heroes (this is an infinite loop if
        //the armyset contains only heroes and ships, but with enough impetus
        //you can break anything). Heroes and ships are just too mighty and
        //unusual to put them in ruins
        const Armysetlist* al = Armysetlist::getInstance();
        std::vector<Uint32> sets = al->getArmysets();

        while (!a || (a->getStat(Army::ARMY_BONUS) & (Army::SHIP | Army::LEADER)))
        {
            Uint32 chosenset = sets[rand() % sets.size()];
            a = al->getArmy(chosenset, rand() % al->getSize(chosenset));
        }
            
        s->push_back(new Army(*a));
        a = 0;

        //(ii) with some chance, insert a second army
        if ((rand() % 3) == 0)
        {
            while (!a || (a->getStat(Army::ARMY_BONUS) & (Army::SHIP | Army::LEADER)))
            {
                Uint32 chosenset = sets[rand() % sets.size()];
                a = al->getArmy(chosenset, rand() % al->getSize(chosenset));
            }
            s->push_back(new Army(*a));
            a = 0;
        }

        //now mark this stack as guard
        (*it).setOccupant(s);
    }

    return true;
}

bool CreateScenario::setupSignposts()
{
    int randno;
    int dynamicPercent = static_cast<int>(1.0 / SIGNPOSTS_RATIO * 100);
    debug("CreateScenario::setupSignposts")

    for (Signpostlist::iterator it = Signpostlist::getInstance()->begin();
        it != Signpostlist::getInstance()->end(); it++)
    {
	if (d_signposts.size() == 0)
	    randno = dynamicPercent;
	else
	    randno = rand() % 100;
	if (randno < dynamicPercent)
	{
            // set up a signpost from the list of signposts
            randno = rand() % d_signposts.size();
            (*it).setName(d_signposts[randno]);

            //remove the used name
            d_signposts[randno] = d_signposts[d_signposts.size() - 1];
            d_signposts.pop_back();
	}
	else
	{
	    char *dir = NULL;
	    int xdir, ydir;
	    char buf[101]; buf[100] = '\0';
	    Stack *s;
            PG_Point signpostPos = (*it).getPos();
	    City *nearCity = Citylist::getInstance()->getNearestCity(signpostPos);
            PG_Point cityPos = nearCity->getPos();
	    s  = new Stack(Playerlist::getInstance()->getNeutral() ,cityPos);
	    xdir = cityPos.x - signpostPos.x;
	    ydir = cityPos.y - signpostPos.y;
	    if (xdir >= 1 && ydir >= 1)
		dir = _("southeast");
	    else if (xdir >= 1 && ydir == 0)
		dir = _("east");
	    else if (xdir >= 1 && ydir <= -1)
		dir = _("northeast");
	    else if (xdir == 0 && ydir >= 1)
		dir = _("south");
	    else if (xdir == 0 && ydir <= -1)
		dir = _("north");
	    else if (xdir <= -1 && ydir >= 1)
		dir = _("southwest");
	    else if (xdir <= -1 && ydir == 0)
		dir = _("west");
	    else if (xdir <= -1 && ydir <= -1)
		dir = _("northwest");
	    snprintf(buf,100,_("%s lies to the %s"), 
		             nearCity->getName().c_str(), dir);
            (*it).setName(buf);
	}
    }

    return true;
}

bool CreateScenario::setupPlayers()
{
    debug("CreateScenario::setupPlayers")

    //Up to now, this function only assigns each player a hero.

    //A player may later have more than one city, so loop over the players
    //instead of the cities
    const Playerlist* pl = Playerlist::getInstance();
    const Armysetlist* al = Armysetlist::getInstance();
    Uint32 heroset = al->getHeroId();
    
    for (Playerlist::const_iterator pit = pl->begin(); pit != pl->end(); pit++)
    {
        if ((*pit) == pl->getNeutral())
            continue;

        //find the first city of the player
        City* city = Citylist::getInstance()->getFirstCity(*pit);

        //put a hero in the city
        const Army* herotype = al->getArmy(heroset, rand() % al->getSize(heroset));
        Hero* newhero = new Hero(*herotype, "", (*pit));
        city->addHero(newhero);
    }

    return true;
}

bool CreateScenario::setupEvents()
{
    debug("CreateScenario::setupEvents()")

    // This is the actualized event setup. the idea is as follows:
    //
    // 1. We separate human and AI players (neutral player is ignored)
    //
    // Then, we have two cases:
    // a) AI players only
    //
    // 1. Give a message and end the game in case of "last man standing"
    //
    // b) AI + human players
    //
    // 1. When a player dies, activate NextTurn events for all human players
    //    that tell them that the player has died at the beginning of their
    //    turn.
    // 2. When all human players have died, the game is lost.
    // 3. When all players except one (==human) have died, the game is won.

    // for the beginning, sort all players in AI and human players
    std::list<Player*> ailist, humanlist;
    std::list<Player*>::iterator pit;
    char buf[101]; buf[100] = '\0';
    
    const Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::const_iterator it = pl->begin(); it != pl->end(); it++)
        switch ((*it)->getType())
        {
            case Player::HUMAN:
                humanlist.push_back(*it);
                break;
            case Player::AI_FAST:
            case Player::AI_DUMMY:
            case Player::AI_SMART:
                if ((*it) == Playerlist::getInstance()->getNeutral())
                    continue;
                ailist.push_back(*it);
                break;
        } 

    // case (a)
    if (humanlist.empty())
    {
        RMessage* rmsg;
        RWinGame* rwin;
        EKillAll* ekillall;

        // when all players are dead, show a message and end the game
        ekillall = new EKillAll();
        rmsg = new RMessage("All players have died except one");
        rwin = new RWinGame(1);
        ekillall->addReaction(rmsg);
        ekillall->addReaction(rwin);

        d_scenario->addEvent(ekillall);
    }
    // case (b)
    else
    {
        // 1. message reporting for each player
        for (Playerlist::const_iterator it = pl->begin(); it != pl->end(); it++)
        {
            EPlayerDead* edead;
            ENextTurn* eturn;
            RMessage* rmsg;
            RActEvent* ract;
            CPlayer* cplayer;

            edead = new EPlayerDead((*it)->getId());
            d_scenario->addEvent(edead);
            
            for (pit = humanlist.begin(); pit != humanlist.end(); pit++)
            {
                // create the nextturn event
                eturn = new ENextTurn();
                snprintf(buf, 100, "Player %s died.", (*it)->getName(false).c_str());
                rmsg = new RMessage(buf);
                cplayer = new CPlayer((*pit)->getId());

                eturn->addCondition(cplayer);
                eturn->addReaction(rmsg);
                eturn->setActive(false);
                d_scenario->addEvent(eturn);

                // activate the next turn event when the player dies
                ract = new RActEvent(eturn->getId(), true);
                edead->addReaction(ract);
            }
        }

        // 2. + 3.
        EKillAll* ekillall = new EKillAll();

        // add a message and a win game event for every human player
        for (pit = humanlist.begin(); pit != humanlist.end(); pit++)
        {
            RMessage* rmsg;
            RWinGame* rwin;
            CLiving* clive;

            snprintf(buf, 100, "Player %s has won!", (*pit)->getName(false).c_str());
            rmsg = new RMessage(buf);
            clive = new CLiving((*pit)->getId());
            rmsg->addCondition(clive);
            
            rwin = new RWinGame((*pit)->getId());
            clive = new CLiving((*pit)->getId());
            rwin->addCondition(clive);

            ekillall->addReaction(rmsg);
            ekillall->addReaction(rwin);
        }

        // add a message and a lose game event for every ai player
        for (pit = ailist.begin(); pit != ailist.end(); pit++)
        {
            RMessage* rmsg;
            RLoseGame* rlose;
            CLiving* clive;

            snprintf(buf, 100, "Player %s has won!", (*pit)->getName(false).c_str());
            rmsg = new RMessage(buf);
            clive = new CLiving((*pit)->getId());
            rmsg->addCondition(clive);
            
            rlose = new RLoseGame(2 * (*pit)->getId());
            clive = new CLiving((*pit)->getId());
            rlose->addCondition(clive);

            ekillall->addReaction(rmsg);
            ekillall->addReaction(rlose);
        }

        // to catch potential errors, append a general loose event
        RLoseGame* rend = new RLoseGame(10);

        ekillall->addReaction(rend);

        // add the event
        d_scenario->addEvent(ekillall);
    }
    
    return true;
}

bool CreateScenario::loadNames(std::vector<std::string>& list, std::ifstream& namefile)
{
    debug("CreateScenario::loadNames")

    int counter;
    char buffer[101];
    buffer[100] = '\0';
    
    if (!namefile)
    {
        std::cerr <<_("Critical Error: Couldn't open citynames data file\n");
        return false;
    }

    namefile >> counter;
    list.resize(counter);

    //with getline, the first call will get the first line to the end, i.e.
    //a newline character. Thus, we throw away the result of the first call.
    namefile.getline(buffer, 100);

    for (counter--; counter >= 0; counter--)
    {
        namefile.getline(buffer, 100);
        list[counter] = std::string(buffer);
    }

    return true;
}
