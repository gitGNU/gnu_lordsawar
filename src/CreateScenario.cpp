// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2004 John Farrell
// Copyright (C) 2004, 2005, 2006 Andrea Patton
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
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
    Itemlist::deleteInstance();

    QuestsManager::deleteInstance();

    fl_counter = new FL_Counter();

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

void CreateScenario::setShieldset(std::string shieldsname)
{
    debug("CreateScenario::setShieldset")
    d_shieldsname = shieldsname;
}

void CreateScenario::setCityset(std::string citysetname)
{
    debug("CreateScenario::setCityset")
    d_citysetname = citysetname;
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

    int number_of_armies_factor;
    getCityDifficulty(g.difficulty, &number_of_armies_factor);
    if (!setupCities(g.quick_start, g.cities_can_produce_allies,
		     number_of_armies_factor))
        return false;

    if (!setupTemples())
        return false;
    
    int sage_factor;
    int no_guardian_factor;
    int stronghold_factor;

    getRuinDifficulty (g.difficulty, &sage_factor, &no_guardian_factor,
		       &stronghold_factor);
    if (!setupRuins(GameScenario::s_play_with_quests, 20, 10, 6))
        return false;

    int signpost_ratio;
    getSignpostDifficulty (g.difficulty, g.hidden_map, &signpost_ratio);
    if (!setupSignposts(signpost_ratio))
        return false;

    int base_gold;
    getBaseGold (g.difficulty, &base_gold);
    if (!setupPlayers(g.diplomacy, g.random_turns, base_gold))
        return false;

    if (!setupItems())
        return false;

    if (!setupRewards())
        return false;

    if (!setupRoads())
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
    d_generator->makeMap(d_width, d_height, true);
    
    //...fill the terrain...
    GameMap::getInstance(d_tilesname, d_shieldsname,
			 d_citysetname)->fill(d_generator);

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
                    Templelist::getInstance()->push_back
		      (Temple(Vector<int>(x,y), popRandomTempleName()));
                    break;
                case Maptile::RUIN:
		    Ruinlist::getInstance()->push_back
		      (Ruin(Vector<int>(x,y), popRandomRuinName()));
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
            (*cit).setOwner(pl->getNeutral());

    }

    return true;
}

bool CreateScenario::setupCities(bool quick_start, 
				 bool cities_can_produce_allies,
				 int number_of_armies_factor)
{
    debug("CreateScenario::setupCities")

    if (quick_start)
      quickStart();

    for (Citylist::iterator it = Citylist::getInstance()->begin();
        it != Citylist::getInstance()->end(); it++)
    {
        //1. set a reasonable cityname
        (*it).setName(popRandomCityName());

        //2. distribute the income a bit (TBD)

        //3. set the city production
        (*it).setRandomArmytypes(cities_can_produce_allies, 
				 number_of_armies_factor);
        if ((*it).getOwner() == Playerlist::getInstance()->getNeutral())
        {
            switch (GameScenario::s_neutral_cities)
              {
              case GameParameters::AVERAGE:
                (*it).produceScout();
                break;
              case GameParameters::STRONG:
                (*it).produceStrongestProductionBase();
                break;
              case GameParameters::ACTIVE:
                if (rand () % 100 >  20)
                  (*it).produceStrongestProductionBase();
                else
                  (*it).produceWeakestProductionBase();
                break;
              }
            (*it).setActiveProductionSlot(-1);
        }
        else
        {
          if ((*it).isCapital())
            (*it).produceStrongestProductionBase();
          else
            (*it).produceWeakestProductionBase();

            (*it).setActiveProductionSlot(0);
        }

	if (rand() % 2 == 0)
	  (*it).raiseDefense();
	(*it).setGold(getRandomCityIncome((*it).isCapital()));
    }

    return true;
}

bool CreateScenario::setupRoads()
{
  Roadlist* rl = Roadlist::getInstance();
  for (Roadlist::iterator it = rl->begin(); it != rl->end(); it++)
    (*it).setType(Roadlist::getInstance()->calculateType((*it).getPos()));
  return true;
}

bool CreateScenario::setupTemples()
{
    Templelist* tl = Templelist::getInstance();
    for (Templelist::iterator it = tl->begin(); it != tl->end(); it++)
    {
        // set a random temple type
        int type= (int) ((TEMPLE_TYPES*1.0) * (rand() / (RAND_MAX + 1.0)));
        (*it).setType(type);

    }

    return true;
}
bool CreateScenario::setupRuins(bool strongholds_invisible, int sage_factor,
				int no_guardian_factor, int stronghold_factor)
{
    debug("CreateScenario::setupRuins")

    //The aim of this function is to put a strong stack as sentinel in all
    //ruins.

    for (Ruinlist::iterator it = Ruinlist::getInstance()->begin();
        it != Ruinlist::getInstance()->end(); it++)
    {
        // set a random ruin type
        if (rand() % stronghold_factor == 0) //one in six ruins is a stronghold
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

        //one in twenty ruins is a sage
        if (rand() % sage_factor == 0 && (*it).getType() == Ruin::RUIN) 
          {
            (*it).setSage (true);
            continue;
          }


        //one in ten ruins doesn't have a guardian
        if (rand() % no_guardian_factor == 0 && (*it).getType() == Ruin::RUIN) 
          continue;

        // and set a guardian
        Stack* s;
        Army* a = 0;
        Vector<int> pos = (*it).getPos();
        
	a = getRandomRuinKeeper(Playerlist::getInstance()->getNeutral());
        if (a)
          {
            //create a stack:
            s = new Stack(0, pos);
            
            s->push_back(a);
            a = 0;

            //now mark this stack as guard
            (*it).setOccupant(s);
          }
    }

    return true;
}

bool CreateScenario::setupSignposts(int ratio)
{
    int randno;
    int dynamicPercent = static_cast<int>(1.0 / ratio * 100);
    debug("CreateScenario::setupSignposts")
    Signpostlist *sl = Signpostlist::getInstance();

    for (Signpostlist::iterator it = sl->begin(); it != sl->end(); it++)
    {
	if (randomSignpostsEmpty())
	    randno = dynamicPercent;
	else
	    randno = rand() % 100;
	if (randno < dynamicPercent)
	{
            // set up a signpost from the list of signposts
	    (*it).setName(popRandomSignpost());
	}
	else
	{
            (*it).setName(getDynamicSignpost(&*it));
	}
    }

    return true;
}

bool CreateScenario::setupPlayers(bool diplomacy, bool random_turns, 
				  int base_gold)
{
    debug("CreateScenario::setupPlayers")

    Playerlist *pl = Playerlist::getInstance();

    // Give players some gold to start with
    for (Playerlist::iterator pit = pl->begin(); pit != pl->end(); pit++)
      (*pit)->setGold(base_gold + ((rand() % 8) * 50));

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

	  Reward_Map *reward = new Reward_Map(Vector<int>(i, j), name,
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
	    newReward->setName(newReward->getDescription());
	    Rewardlist::getInstance()->push_back(newReward); //add it
	  }
	if ((*it).hasSage() == false)
	  (*it).populateWithRandomReward();
      }
  return true;
}

bool CreateScenario::setupItems()
{
  Itemlist::createStandardInstance();
  return true;
}

bool CreateScenario::setupItemRewards()
{
  debug("CreateScenario::setupItemRewards")
  Itemlist *il = Itemlist::getInstance();
  Itemlist::iterator iter;
  for (iter = il->begin(); iter != il->end(); iter++)
    {
      Item templateItem = *iter->second;
      Item *newItem = new Item(templateItem); //instantiate it
      Reward_Item *newReward = new Reward_Item(newItem); //make a reward
      newReward->setName(newReward->getDescription());
      Rewardlist::getInstance()->push_back(newReward); //add it
    }

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
void CreateScenario::getRuinDifficulty (int difficulty, int *sage_factor, 
					int *no_guardian_factor, 
					int *stronghold_factor)
{
  if (difficulty < 50)
    {
      *sage_factor = 3;
      *no_guardian_factor = 5;
      *stronghold_factor = 12;
    }
  else if (difficulty < 60)
    {
      *sage_factor = 9;
      *no_guardian_factor = 6;
      *stronghold_factor = 10;
    }
  else if (difficulty < 70)
    {
      *sage_factor = 14;
      *no_guardian_factor = 8;
      *stronghold_factor = 9;
    }
  else if (difficulty < 80)
    {
      *sage_factor = 20;
      *no_guardian_factor = 10;
      *stronghold_factor = 6;
    }
  else if (difficulty < 90)
    {
      *sage_factor = 22;
      *no_guardian_factor = 12;
      *stronghold_factor = 4;
    }
  else 
    {
      *sage_factor = 24;
      *no_guardian_factor = 15;
      *stronghold_factor = 3;
    }
}
    
void CreateScenario::getSignpostDifficulty (int difficulty, bool hidden_map, 
					    int *signpost_ratio)
{
  //the idea here is that we're on a hidden map, and if it's harder
  //difficulty, then we don't get as many signs directing us to cities.
  if (hidden_map)
    {
      if (difficulty < 60)
	*signpost_ratio = 2; //50% of signs point to cities
      else if (difficulty < 70)
	*signpost_ratio = 3; //33% of signs point to cities
      else if (difficulty < 80)
	*signpost_ratio = 6; //16% of signs point to cities
      else if (difficulty < 90)
	*signpost_ratio = 9; //11% of signs point to cities
      else
	*signpost_ratio = 15; //6% of signs point to cities
    }
  else
    *signpost_ratio = 6;
}

    
void CreateScenario::getCityDifficulty(int difficulty, 
				       int *number_of_armies_factor)
{
  if (difficulty < 50)
    *number_of_armies_factor = 3;
  else if (difficulty < 60)
    *number_of_armies_factor = 2;
  else if (difficulty < 70)
    *number_of_armies_factor = 1;
  else 
    *number_of_armies_factor = 0;
}
    
void CreateScenario::getBaseGold (int difficulty, int *base_gold)
{
  if (difficulty < 50)
    *base_gold = 1600;
  else if (difficulty < 60)
    *base_gold = 1200;
  else if (difficulty < 70)
    *base_gold = 1000;
  else if (difficulty < 80)
    *base_gold = 900;
  else if (difficulty < 90)
    *base_gold = 800;
  else
    *base_gold = 700;
}
