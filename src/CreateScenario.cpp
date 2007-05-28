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
#include "GameScenario.h"
#include "army.h"
#include "GameMap.h"
#include "counter.h"
#include "player.h"
#include "playerlist.h"
#include "stacklist.h"
#include "citylist.h"
#include "ruinlist.h"
#include "templelist.h"
#include "signpostlist.h"
#include "stonelist.h"
#include "roadlist.h"
#include "armysetlist.h"

#include "real_player.h"
#include "ai_fast.h"
#include "ai_smart.h"
#include "ai_dummy.h"
#include "File.h"
#include "MapGenerator.h"
#include "QuestsManager.h"
#include "Configuration.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

CreateScenario::CreateScenario()
    :d_scenario(0), d_generator(0)
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
                                SDL_Color color, int type)
{
    debug("CreateScenario::addPlayer")

    Player* p = Player::create(name, armyset, color, Player::Type(type));
    Playerlist::getInstance()->push_back(p);

    return p;
}

bool CreateScenario::addNeutral(std::string name, Uint32 armyset,
                                SDL_Color color, int type)
{
    // for consistency, we only allow exactly one neutral player
    if (Playerlist::getInstance()->getNeutral() != 0)
        return false;

    Player* p = addPlayer(name, armyset, color, Player::Type(type));
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

    if (!createMap())
        return false;

    if (!distributePlayers())
        return false;

    if (!setupCities())
        return false;

    if (!setupTemples())
        return false;
    
    if (!setupRuins())
        return false;

    if (!setupSignposts())
        return false;

    if (!setupPlayers())
        return false;


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
                    Stonelist::getInstance()->push_back(Stone(Vector<int>(x,y)));
                    break;
                case Maptile::SIGNPOST:
                    Signpostlist::getInstance()->push_back(Signpost(Vector<int>(x,y)));
                    break;
                case Maptile::TEMPLE:
                    Templelist::getInstance()->push_back(Temple(Vector<int>(x,y)));
                    break;
                case Maptile::RUIN:
		    Ruinlist::getInstance()->push_back(Ruin(Vector<int>(x,y)));
		    break;
                case Maptile::CITY:
                    Citylist::getInstance()->push_back(City(Vector<int>(x,y)));
                    break;
                case Maptile::ROAD:
                    Roadlist::getInstance()->push_back(Road(Vector<int>(x,y)));
                    break;
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
    int cityskip = (Citylist::getInstance()->size() - 1)
	/ (Playerlist::getInstance()->size() - 1);
    
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
            (*cit).setCapitalOwner(*pit);
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
        if ((*it).isCapital())
          (*it).produceStrongestArmy();
        else
          (*it).produceWeakestArmy();
        if ((*it).getPlayer() == Playerlist::getInstance()->getNeutral())
            (*it).setProduction(-1);
        else
        {
            (*it).setProduction(0);
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

    // list all the army types that can be a sentinel.
    std::vector<const Army*> occupants;
    Armysetlist *al = Armysetlist::getInstance();
    std::vector<unsigned int> sets = al->getArmysets(true);
    for (unsigned int i = 0; i < sets.size(); i++)
      {
        for (unsigned int j = 0; j < al->getSize(sets[i]); j++)
          {
            const Army *a = al->getArmy (sets[i], j);
            if (a->getDefendsRuins())
              occupants.push_back(a);
          }
      }

    for (Ruinlist::iterator it = Ruinlist::getInstance()->begin();
        it != Ruinlist::getInstance()->end(); it++)
    {
        // set a ruin name
        int randno = rand() % d_ruinnames.size();
        (*it).setName(d_ruinnames[randno]);

        //remove the used name
        d_ruinnames[randno] = d_ruinnames[d_ruinnames.size() - 1];
        d_ruinnames.pop_back();

        if (rand() % 10 == 0) //one in ten doesn't have a guardian
          continue;

        // and set a guardian
        Stack* s;
        const Army* a = 0;
        Vector<int> pos = (*it).getPos();
        
        if (!occupants.empty())
          {
            //create a stack:
            s = new Stack(0, pos);
            
            a = occupants[rand() % occupants.size()];
            s->push_back(new Army(*a));
            a = 0;

            //now mark this stack as guard
            (*it).setOccupant(s);
          }
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
            Vector<int> signpostPos = (*it).getPos();
	    City *nearCity = Citylist::getInstance()->getNearestCity(signpostPos);
            Vector<int> cityPos = nearCity->getPos();
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
