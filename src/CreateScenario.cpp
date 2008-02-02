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
#include "rewardlist.h"
#include "Itemlist.h"
#include "templelist.h"
#include "signpostlist.h"
#include "portlist.h"
#include "bridgelist.h"
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
#include "FogMap.h"
#include "history.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

CreateScenario::CreateScenario(int width, int height)
    :d_scenario(0), d_generator(0)
{
    debug("CreateScenario::CreateScenario")
   
    //make sure that objects are deleted
    GameMap::deleteInstance();
    Playerlist::deleteInstance();
    Templelist::deleteInstance();
    Ruinlist::deleteInstance();
    Rewardlist::deleteInstance();
    Portlist::deleteInstance();
    Bridgelist::deleteInstance();
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
    setWidth(width);
    setHeight(height);
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

    Player* p = Player::create(name, armyset, color, d_width, d_height,
			       Player::Type(type));
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

bool CreateScenario::create(const GameParameters &g)
{
    debug("CreateScenario::create")

    d_scenario = new GameScenario("AutoGenerated", "AutoGenerated", d_turnmode);

    GameScenario::s_see_opponents_stacks = g.see_opponents_stacks;
    GameScenario::s_see_opponents_production = g.see_opponents_production;
    GameScenario::s_play_with_quests = g.play_with_quests;
    GameScenario::s_hidden_map = g.hidden_map;
    GameScenario::s_diplomacy = g.diplomacy;
    GameScenario::s_cusp_of_war = g.cusp_of_war;
    GameScenario::s_neutral_cities = g.neutral_cities;
    GameScenario::s_razing_cities = g.razing_cities;
    GameScenario::s_military_advisor= g.military_advisor;
    GameScenario::s_random_turns = g.random_turns;
    GameScenario::s_intense_combat = g.intense_combat;

    if (!createMap())
        return false;

    // fog it up
    if (GameScenario::s_hidden_map)
      {
        Playerlist::iterator pit = Playerlist::getInstance()->begin();
        for (; pit != Playerlist::getInstance()->end(); pit++)
          (*pit)->getFogMap()->fill(FogMap::CLOSED);
      }

    if (!distributePlayers())
        return false;

    if (!setupCities(g.quick_start, g.cities_can_produce_allies))
        return false;

    if (!setupTemples())
        return false;
    
    if (!setupRuins(GameScenario::s_play_with_quests))
        return false;

    if (!setupSignposts())
        return false;

    if (!setupPlayers(g.diplomacy, g.random_turns))
        return false;

    if (!setupRewards())
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
    
    Rewardlist::getInstance();

    //have the generator make the map...
    d_generator->makeMap(d_width, d_height);
    
    //...fill the terrain...
    GameMap::getInstance(d_tilesname)->fill(d_generator);

    //...and create cities, temples, ruins ,signposts
    map = d_generator->getBuildings(d_width, d_height);
    
    for (int y = 0; y < d_height; y++)
        for (int x = 0; x < d_width; x++)
        {
            switch (map[y*d_width + x])
            {
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
                case Maptile::PORT:
                    Portlist::getInstance()->push_back(Port(Vector<int>(x,y)));
                    break;
                case Maptile::BRIDGE:
                    Bridgelist::getInstance()->push_back(Bridge(Vector<int>(x,y)));
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
            (*cit).conquer(*pit);
            (*cit).setCapitalOwner(*pit);
            (*cit).setCapital(true);
            skipping = 0;

	    History_CityWon *item = new History_CityWon();
	    item->fillData(&*cit);
	    (*pit)->getHistorylist()->push_back(item);

            pit++;
            if ((*pit) == pl->getNeutral())
                pit++;
        }
        else
            (*cit).setPlayer(pl->getNeutral());

    }

    return true;
}

bool CreateScenario::setupCities(bool quick_start, 
				 bool cities_can_produce_allies)
{
    debug("CreateScenario::setupCities")

    if (quick_start)
      quickStart();

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
        (*it).setRandomArmytypes(cities_can_produce_allies);
        if ((*it).getPlayer() == Playerlist::getInstance()->getNeutral())
        {
            switch (GameScenario::s_neutral_cities)
              {
              case GameParameters::AVERAGE:
                (*it).produceScout();
                break;
              case GameParameters::STRONG:
                (*it).produceStrongestArmy();
                break;
              case GameParameters::ACTIVE:
                if (rand () % 100 >  20)
                  (*it).produceStrongestArmy();
                else
                  (*it).produceWeakestArmy();
                break;
              }
            (*it).setProduction(-1);
        }
        else
        {
          if ((*it).isCapital())
            (*it).produceStrongestArmy();
          else
            (*it).produceWeakestArmy();

            (*it).setProduction(0);
        }

	if (rand() % 2 == 0)
	  (*it).raiseDefense();
	if ((*it).isCapital())
	  (*it).setGold(33 + (rand() % 8));
	else
	  (*it).setGold(15 + (rand() % 12));
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

bool CreateScenario::setupRuins(bool strongholds_invisible)
{
    debug("CreateScenario::setupRuins")

    //The aim of this function is to put a strong stack as sentinel in all
    //ruins.

    // list all the army types that can be a sentinel.
    std::vector<const Army*> occupants;
    Armysetlist *al = Armysetlist::getInstance();
    Player *p = Playerlist::getInstance()->getNeutral();
    for (unsigned int j = 0; j < al->getSize(p->getArmyset()); j++)
      {
        const Army *a = al->getArmy (p->getArmyset(), j);
        if (a->getDefendsRuins())
          occupants.push_back(a);
      }

    for (Ruinlist::iterator it = Ruinlist::getInstance()->begin();
        it != Ruinlist::getInstance()->end(); it++)
    {
        // set a ruin name
        int randno = rand() % d_ruinnames.size();
        (*it).setName(d_ruinnames[randno]);

        // set a random ruin type
        if (rand() % 6 == 0) //one in six ruins is a stronghold
          {
            (*it).setType(Ruin::STRONGHOLD);
            if (strongholds_invisible == true)
              {
                (*it).setHidden(true);
                (*it).setOwner(NULL);
              }
          }
        else
          (*it).setType(Ruin::RUIN);

        //remove the used name
        d_ruinnames[randno] = d_ruinnames[d_ruinnames.size() - 1];
        d_ruinnames.pop_back();

        //one in twenty ruins is a sage
        if (rand() % 20 == 0 && (*it).getType() == Ruin::RUIN) 
          {
            (*it).setSage (true);
            continue;
          }


        //one in ten ruins doesn't have a guardian
        if (rand() % 10 == 0 && (*it).getType() == Ruin::RUIN) 
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
            s->push_back(new Army(*a, p));
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

bool CreateScenario::setupPlayers(bool diplomacy, bool random_turns)
{
    debug("CreateScenario::setupPlayers")

    Playerlist *pl = Playerlist::getInstance();

    // Give players some gold to start with
    for (Playerlist::iterator pit = pl->begin(); pit != pl->end(); pit++)
      (*pit)->setGold(1000 + ((rand() % 8) * 50));

    // Set up diplomacy
    for (Playerlist::iterator pit = pl->begin(); pit != pl->end(); pit++)
      {
	if (pl->getNeutral() == (*pit))
	  continue;
	for (Playerlist::iterator it = pl->begin(); it != pl->end(); it++)
	  {
	    if (pl->getNeutral() == (*it))
	      continue;
	    if (*pit == *it)
	      continue;
	    if (diplomacy == false)
	      {
		(*pit)->proposeDiplomacy(Player::PROPOSE_WAR, *it);
		(*pit)->declareDiplomacy(Player::AT_WAR, *it);
	      }
	    else 
	      {
		(*pit)->proposeDiplomacy(Player::NO_PROPOSAL, *it);
		(*pit)->declareDiplomacy(Player::AT_PEACE, *it);
	      }
	  }
      }
    if (diplomacy)
      pl->calculateDiplomaticRankings();

    if (random_turns)
      pl->randomizeOrder();
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

bool CreateScenario::setupRewards()
{
  debug("CreateScenario::setupRewards")
  setupItemRewards();
  setupRuinRewards();
  setupMapRewards();
  return true;
}

bool CreateScenario::setupMapRewards()
{
  debug("CreateScenario::setupMapRewards")
  if (GameScenario::s_hidden_map == false)
    return true;
  //okay, let's make some maps
  //split the terrain into a 3x3 grid
  Uint32 h_count = 0;
  Uint32 w_count = 0;
  for (int i = 0; i < GameMap::getHeight(); i += (GameMap::getHeight() / 3))
    {
      for (int j = 0; j < GameMap::getWidth(); j += (GameMap::getWidth() / 3))
	{
	  char *name = NULL;
	  if (h_count == 0 && j == 0)
	    name = _("northwestern map");
	  else if (h_count == 0 && w_count == 1)
	    name = _("northern map");
	  else if (h_count == 0 && w_count == 2)
	    name = _("northeastern map");
	  else if (h_count == 1 && w_count == 0)
	    name = _("western map");
	  else if (h_count == 1 && w_count == 1)
	    name = _("central map");
	  else if (h_count == 1 && w_count == 2)
	    name = _("eastern map");
	  else if (h_count == 2 && w_count == 0)
	    name = _("southwestern map");
	  else if (h_count == 2 && w_count == 1)
	    name = _("southern map");
	  else if (h_count == 2 && w_count == 2)
	    name = _("southeastern map");
	  else
	    continue;

	  Vector<int> pos;
	  pos.x = i;
	  pos.y = j;
	  Location *loc = new Location(name, pos);
	  Reward_Map *reward = new Reward_Map(loc, 
					      GameMap::getHeight() / 3, 
					      GameMap::getWidth() / 3);
	  Rewardlist::getInstance()->push_back(reward); //add it
	  w_count++;
	}
      w_count = 0;
      h_count++;
    }
  return true;
}
bool CreateScenario::setupRuinRewards()
{
  debug("CreateScenario::setupRuinRewards")
    for (Ruinlist::iterator it = Ruinlist::getInstance()->begin();
	 it != Ruinlist::getInstance()->end(); it++)
      {
	if ((*it).isHidden() == true)
	  {
	    //add it to the reward list
	    Reward_Ruin *newReward = new Reward_Ruin(&(*it)); //make a reward
	    Rewardlist::getInstance()->push_back(newReward); //add it
	  }
	if ((*it).hasSage() == false)
	  (*it).populateWithRandomReward();
      }
  return true;
}

bool CreateScenario::setupItemRewards()
{
  debug("CreateScenario::setupItemRewards")
    Itemlist::createInstance();
  Itemlist *il = Itemlist::getInstance();
  Itemlist::iterator iter;
  for (iter = il->begin(); iter != il->end(); iter++)
    {
      Item templateItem = *iter->second;
      Item *newItem = new Item(templateItem); //instantiate it
      Reward_Item *newReward = new Reward_Item(newItem); //make a reward
      Rewardlist::getInstance()->push_back(newReward); //add it
    }

  Itemlist::deleteInstance();
  return true;
}

void CreateScenario::quickStart()
{
  Playerlist *plist = Playerlist::getInstance();
  Citylist *clist = Citylist::getInstance();
  Vector <int> pos;
  // no neutral cities
  // divvy up the neutral cities among other non-neutral players
  int cities_left = clist->size() - plist->size() + 1;
  unsigned int citycount[MAX_PLAYERS];
  memset (citycount, 0, sizeof (citycount));
  Playerlist::iterator pit = plist->begin();
  for (; pit != plist->end(); pit++)
    {
      if (*pit == plist->getNeutral())
	pit = plist->begin();
      citycount[(*pit)->getId()]++;
      cities_left--;
      if (cities_left == 0)
	break;
    }
  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      for (unsigned int j = 0; j < citycount[i]; j++)
	{
	  Player *p = plist->getPlayer(i);
	  pos = clist->getFirstCity(p)->getPos();
	  City *c = clist->getNearestNeutralCity(pos);
	  c->conquer(p);
	  History_CityWon *item = new History_CityWon();
	  item->fillData(c);
	  p->getHistorylist()->push_back(item);
	}
    }
}
