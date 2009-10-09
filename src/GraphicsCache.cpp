// Copyright (C) 2003, 2004, 2005, 2006, 2007 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009 Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
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

#include <assert.h>
#include <string.h>

#include "rectangle.h"

#include "GraphicsCache.h"
#include "GraphicsLoader.h"
#include "armysetlist.h"
#include "shieldsetlist.h"
#include "citylist.h"
#include "army.h"
#include "playerlist.h"
#include "Configuration.h"
#include "File.h"
#include "GameMap.h"
#include "city.h"
#include "stack.h"
#include "rgb_shift.h"
#include "gui/image-helpers.h"
#include "FogMap.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

//some structures first

//this structure is the base class for storing cached objects. It stores the
//(army-, but can be extended) set, the type of army and the player which the
//surface is designed for (and the surface itself, of course).
struct ArmyCacheItem
{
    guint32 armyset;
    guint32 army_id;
    guint32 player_id;
    bool medals[3];
    PixMask* surface;
};
bool operator <(ArmyCacheItem lhs, ArmyCacheItem rhs)
{
  return memcmp(&lhs, &rhs, sizeof (ArmyCacheItem) - sizeof (PixMask*)) < 0;
}

//the structure to store ships in
struct ShipCacheItem
{
    guint32 player_id;
    PixMask* surface;
};

//the structure to store planted standard in
struct PlantedStandardCacheItem
{
    guint32 player_id;
    PixMask* surface;
};

//the structure to store temples in
struct TempleCacheItem
{
    int type;
    PixMask* surface;
};

//the structure to store ruins in
struct RuinCacheItem
{
    int type;
    PixMask* surface;
};

//the structure to store diplomacy icons in
struct DiplomacyCacheItem
{
    int type;
    Player::DiplomaticState state;
    PixMask* surface;
};

//the structure to store roads in
struct RoadCacheItem
{
    int type;
    PixMask* surface;
};

//the structure to store fog patterns in
struct FogCacheItem
{
    int type;
    PixMask* surface;
};
bool operator <(FogCacheItem lhs, FogCacheItem rhs)
{
  return memcmp(&lhs, &rhs, sizeof (FogCacheItem) - sizeof (PixMask*)) < 0;
}

//the structure to store bridges in
struct BridgeCacheItem
{
    int type;
    PixMask* surface;
};

//the structure to store cursors in
struct CursorCacheItem
{
    int type;
    PixMask* surface;
};

//the structure to store buildings in
struct CityCacheItem
{
    int type;
    guint32 player_id;
    PixMask* surface;
};

//the structure to store towers in
struct TowerCacheItem
{
    guint32 player_id;
    PixMask* surface;
};

// the structure to store flags in
struct FlagCacheItem
{
    guint32 size;
    guint32 player_id;
    PixMask* surface;
};

// the structure to store selector images in
struct SelectorCacheItem
{
    guint32 type;
    guint32 frame;
    guint32 player_id;
    PixMask* surface;
};

// the structure to store shield images in
struct ShieldCacheItem
{
    std::string shieldset;
    guint32 type;
    guint32 colour;
    PixMask* surface;
};

// the structure to store production shield images in
struct ProdShieldCacheItem
{
    guint32 type;
    bool prod;
    PixMask* surface;
};

// the structure to store movement bonus images in
struct MoveBonusCacheItem
{
    guint32 type; // 0=empty, 1=trees, 2=foothills, 3=hills+trees, 4=fly, 5=boat
    PixMask* surface;
};

// the structure to store drawn bigmap tiles in
struct TileCacheItem
{
  int tile_style_id;
  int fog_type_id;
  bool has_bag;
  bool has_standard;
  int standard_player_id;
  int stack_size; //flag size
  int stack_player_id;
  int army_type_id;
  bool has_tower;
  bool has_ship;
  Maptile::Building building_type;
  int building_subtype;
  Vector<int> building_tile;
  int building_player_id;
  guint32 tilesize;
  bool has_grid;
  PixMask* surface;
};
bool operator <(TileCacheItem lhs, TileCacheItem rhs)
{
  return memcmp(&lhs, &rhs, sizeof (TileCacheItem) - sizeof (PixMask*)) < 0;
}
//-----------------------------------------------------

GraphicsCache* GraphicsCache::s_instance = 0;

GraphicsCache* GraphicsCache::getInstance()
{
    if (!s_instance)
        s_instance = new GraphicsCache();

    return s_instance;
}

void GraphicsCache::deleteInstance()
{
    if (!s_instance)
        return;

    delete s_instance;
    s_instance =0;
}

GraphicsCache::GraphicsCache()
    :d_cachesize(0)
{
    loadCityPics();
    loadTowerPics();
    loadTemplePics();
    loadRuinPics();
    loadDiplomacyPics();
    loadRoadPics();
    loadFogPics();
    loadBridgePics();
    loadCursorPics();
    loadFlags();
    for (unsigned int i = 0; i < d_selector.size(); i++)
      {
        delete d_selector[i];
        delete d_selectormask[i];
      }

    for (unsigned int i = 0; i < d_smallselector.size(); i++)
      {
        delete d_smallselector[i];
        delete d_smallselectormask[i];
      }
    loadSelectors();
    loadProdShields();
    loadMoveBonusPics();
    loadMedalPics();

    d_smallruinedcity = GraphicsLoader::getMiscPicture("smallruinedcity.png");
    d_smallhero = GraphicsLoader::getMiscPicture("hero.png");
    d_smallinactivehero = GraphicsLoader::getMiscPicture("hero-inactive.png");
    d_small_ruin_unexplored = GraphicsLoader::getMiscPicture("smallunexploredruin.png");
    d_small_stronghold_unexplored = 
      GraphicsLoader::getMiscPicture("smallunexploredstronghold.png");
    d_small_ruin_explored = GraphicsLoader::getMiscPicture("smallexploredruin.png");
    d_small_temple = GraphicsLoader::getMiscPicture("smalltemple.png");
    std::string tileset = GameMap::getInstance()->getTileset()->getSubDir();
    std::string cityset = GameMap::getInstance()->getCityset()->getSubDir();
    d_port = GraphicsLoader::getCitysetPicture(cityset, "port.png");
    d_bag = GraphicsLoader::getMiscPicture("items.png");
    d_explosion = GraphicsLoader::getTilesetPicture(tileset, "misc/explosion.png");
    d_signpost = GraphicsLoader::getCitysetPicture(cityset, "signpost.png");
}

GraphicsCache::~GraphicsCache()
{
    clear();

    for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
    {
        if (d_citypic[i])
	  delete d_citypic[i];

        if (d_razedpic[i])
	  delete d_razedpic[i];
    }

    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
        if (d_towerpic[i])
	  delete d_towerpic[i];
    }

    for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
    {
        delete d_flagpic[i];
        delete d_flagmask[i];
    }

    for (unsigned int i = 0; i < d_selector.size(); i++)
    {
        delete d_selector[i];
        delete d_selectormask[i];
    }

    for (unsigned int i = 0; i < d_smallselector.size(); i++)
    {
        delete d_smallselector[i];
        delete d_smallselectormask[i];
    }

    for (unsigned int i = 0; i < PRODUCTION_SHIELD_TYPES; i++)
    {
        delete d_prodshieldpic[i];
    }

    for (unsigned int i = 0; i < MOVE_BONUS_TYPES; i++)
    {
        delete d_movebonuspic[i];
    }

    for (unsigned int i = 0; i < FOG_TYPES; i++)
    {
        delete d_fogpic[i];
    }

    delete d_smallruinedcity;
    delete d_smallhero;
    delete d_smallinactivehero;
    delete d_small_temple;
    delete d_small_ruin_unexplored;
    delete d_small_stronghold_unexplored;
    delete d_small_ruin_explored;
    delete d_port;
    delete d_bag;
    delete d_explosion;
    delete d_signpost;
}

PixMask* GraphicsCache::getSmallRuinedCityPic()
{
  return d_smallruinedcity;
}

PixMask* GraphicsCache::getSmallHeroPic(bool active)
{
  if (active)
    return d_smallhero;
  else
    return d_smallinactivehero;
}

PixMask* GraphicsCache::getSmallRuinExploredPic()
{
  return d_small_ruin_explored;
}
PixMask* GraphicsCache::getSmallRuinUnexploredPic()
{
  return d_small_ruin_unexplored;
}
PixMask* GraphicsCache::getSmallStrongholdUnexploredPic()
{
  return d_small_stronghold_unexplored;
}
PixMask* GraphicsCache::getSmallTemplePic()
{
  return d_small_temple;
}

PixMask* GraphicsCache::getBagPic()
{
  return d_bag;
}

PixMask* GraphicsCache::getPortPic()
{
  return d_port;
}

PixMask* GraphicsCache::getExplosionPic()
{
  return d_explosion;
}

PixMask* GraphicsCache::getSignpostPic()
{
  return d_signpost;
}

PixMask* GraphicsCache::getMoveBonusPic(guint32 bonus, bool has_ship)
{
  guint32 type;
  if (bonus == Tile::isFlying()) // show fly icon
    type = 4; 
  else if (bonus & Tile::FOREST && bonus & Tile::HILLS) // show trees and hills
    type = 3;
  else if (bonus & Tile::HILLS) // show foothills
    type = 2;
  else if (bonus & Tile::FOREST) // show trees
    type = 1;
  else if (has_ship) // (what a) show boat
    type = 5;
  else // show blank
    type = 0;
  debug("GraphicsCache::getMoveBonusPic " <<bonus <<"," <<type)

  std::list<MoveBonusCacheItem*>::iterator it;
  MoveBonusCacheItem* myitem;

  for (it = d_movebonuslist.begin(); it != d_movebonuslist.end(); it++)
  {
      if ((*it)->type == type)
      {
          myitem = (*it);

          //put the item in last place (last touched)
          d_movebonuslist.erase(it);
          d_movebonuslist.push_back(myitem);

          return myitem->surface;
      }
    }

    //no item found -> create a new one
    myitem = addMoveBonusPic(type);

    return myitem->surface;
}

PixMask* GraphicsCache::getShipPic(const Player* p)
{
    debug("getting ship pic " <<p->getName())
    std::list<ShipCacheItem*>::iterator it;
    ShipCacheItem* myitem;
    for (it = d_shiplist.begin(); it != d_shiplist.end(); it++)
    {
        if ((*it)->player_id == p->getId())
        {
            myitem = (*it);
            
            // put the item on the last place (==last touched)
            d_shiplist.erase(it);
            d_shiplist.push_back(myitem);
            
            return myitem->surface;
        }
    }
    // We are still here, so the graphic is not in the cache. addShipPic calls
    // checkPictures on its own, so we can simply return the surface
    myitem = addShipPic(p);

    return myitem->surface;
}

PixMask* GraphicsCache::getPlantedStandardPic(const Player* p)
{
    debug("getting planted standard pic " <<p->getName())
    std::list<PlantedStandardCacheItem*>::iterator it;
    PlantedStandardCacheItem* myitem;
    for (it = d_plantedstandardlist.begin(); it != d_plantedstandardlist.end(); it++)
    {
        if ((*it)->player_id == p->getId())
        {
            myitem = (*it);
            
            // put the item on the last place (==last touched)
            d_plantedstandardlist.erase(it);
            d_plantedstandardlist.push_back(myitem);
            
            return myitem->surface;
        }
    }
    // We are still here, so the graphic is not in the cache. 
    // addPlantedStandardPic calls checkPictures on its own, so we can 
    // simply return the surface
    myitem = addPlantedStandardPic(p);

    return myitem->surface;
}

PixMask* GraphicsCache::getArmyPic(Army *a)
{
  return getArmyPic(a->getOwner()->getArmyset(), a->getTypeId(), 
		    a->getOwner(), NULL);
}

PixMask* GraphicsCache::getArmyPic(guint32 armyset, guint32 army_id, 
				   const Player* p, const bool *medals)
{
  debug("getting army pic " <<armyset <<" " <<army <<" " <<p->getName())

    std::list<ArmyCacheItem*>::iterator it;
    ArmyCacheItem* myitem;

    // important: medals can be 0 in special cases
    bool my_medals[3] = {0,0,0};
    if (medals)
        for (int i = 0; i < 3; i++)
            my_medals[i] = medals[i];

    // special situation: ruin keepers don't belong to any player
    // we don't actually show them, but what the heck
    if (!p)
        p = Playerlist::getInstance()->getNeutral();
    
    ArmyCacheItem item = ArmyCacheItem();
    item.armyset = armyset;
    item.army_id = army_id;
    item.player_id = p->getId();
    item.medals[0] = my_medals[0];
    item.medals[1] = my_medals[1];
    item.medals[2] = my_medals[2];
    ArmyMap::iterator mit = d_armymap.find(item);
    if (mit != d_armymap.end())
      {
	std::list<ArmyCacheItem*>::iterator it = (*mit).second;
	myitem = *it;
	d_armylist.erase(it);
	d_armylist.push_back(myitem);
	it = d_armylist.end();
	it--;
	d_armymap[*myitem] = it;
	return myitem->surface;
      }

    /*
    for (it =d_armylist.begin(); it != d_armylist.end(); it++)
    {
        if (((*it)->armyset == armyset) && ((*it)->index == army)
            && ((*it)->player_id == p->getId())
            && ((*it)->medals[0]==my_medals[0])
            && ((*it)->medals[1]==my_medals[1])
            && ((*it)->medals[2]==my_medals[2]))
        {
            myitem = (*it);
            
            // put the item on the last place (==last touched)
            d_armylist.erase(it);
            d_armylist.push_back(myitem);
            
            return myitem->surface;
        }
    }
    */

    // We are still here, so the graphic is not in the cache. addArmyPic calls
    // checkPictures on its own, so we can simply return the surface
    debug("getarmypic============= " << my_medals) 
    myitem = addArmyPic(&item);

    return myitem->surface;
}

PixMask* GraphicsCache::getTilePic(int tile_style_id, int fog_type_id, bool has_bag, bool has_standard, int standard_player_id, int stack_size, int stack_player_id, int army_type_id, bool has_tower, bool has_ship, Maptile::Building building_type, int building_subtype, Vector<int> building_tile, int building_player_id, guint32 tilesize, bool has_grid)
{
    debug("getting tile pic " << tile_style_id << " " <<
	  fog_type_id << " " << has_bag << " " << has_standard << " " <<
	  standard_player_id << " " << stack_size << " " << stack_player_id <<
	  " " << army_type_id << " " << has_tower << " " << has_ship << " " << 
	  building_type << " " << building_subtype << " " << building_tile.x 
	  << "," << building_tile.y << " " << building_player_id << " " << 
	  tilesize << " " << has_grid);

    std::list<TileCacheItem*>::iterator it;
    TileCacheItem* myitem;

    TileCacheItem item = TileCacheItem();
    item.tilesize = tilesize;
    item.tile_style_id = tile_style_id;
    item.has_bag = has_bag;
    item.has_standard = has_standard;
    item.standard_player_id = standard_player_id;
    item.stack_size = stack_size;
    item.stack_player_id = stack_player_id;
    item.army_type_id = army_type_id;
    item.has_tower = has_tower;
    item.has_ship = has_ship;
    item.building_type= building_type;
    item.building_subtype= building_subtype;
    item.building_tile = building_tile;
    item.building_player_id = building_player_id;
    item.has_grid = has_grid;
    item.fog_type_id = fog_type_id;
    TileMap::iterator mit = d_tilemap.find(item);
    if (mit != d_tilemap.end())
      {
	std::list<TileCacheItem*>::iterator it = (*mit).second;
	myitem = *it;
	d_tilelist.erase(it);
	d_tilelist.push_back(myitem);
	it = d_tilelist.end();
	it--;
	d_tilemap[*myitem] = it;
	return myitem->surface;
      }

    // We are still here, so the graphic is not in the cache. addTilePic calls
    // checkPictures on its own, so we can simply return the surface
    myitem = addTilePic(&item);

    return myitem->surface;
}

PixMask* GraphicsCache::getShieldPic(std::string shieldset, 
					 guint32 type, 
					 guint32 colour)
{
    debug("getting shield pic " <<shieldset <<" " <<type <<" " <<colour)

    std::list<ShieldCacheItem*>::iterator it;
    ShieldCacheItem* myitem;

    for (it =d_shieldlist.begin(); it != d_shieldlist.end(); it++)
    {
        if ((*it)->shieldset == shieldset && (*it)->type == type
            && (*it)->colour == colour)
        {
            myitem = (*it);
            
            // put the item on the last place (==last touched)
            d_shieldlist.erase(it);
            d_shieldlist.push_back(myitem);
            
            return myitem->surface;
        }
    }

    // We are still here, so the graphic is not in the cache. addShieldPic calls
    // checkPictures on its own, so we can simply return the surface
    myitem = addShieldPic(shieldset, type, colour);

    return myitem->surface;
}
        
PixMask* GraphicsCache::getShieldPic(guint32 type, Player *p)
{
  std::string shieldset = GameMap::getInstance()->getShieldset()->getSubDir();
  return getShieldPic(shieldset, type, p->getId());
}

PixMask* GraphicsCache::getTemplePic(int type)
{
    debug("GraphicsCache::getTemplePic " <<type)

    std::list<TempleCacheItem*>::iterator it;
    TempleCacheItem* myitem;

    for (it = d_templelist.begin(); it != d_templelist.end(); it++)
    {
        if ((*it)->type == type)
        {
            myitem = (*it);

            //put the item in last place (last touched)
            d_templelist.erase(it);
            d_templelist.push_back(myitem);

            return myitem->surface;
        }
    }

    //no item found -> create a new one
    myitem = addTemplePic(type);

    return myitem->surface;
}

PixMask* GraphicsCache::getRuinPic(int type)
{
    debug("GraphicsCache::getRuinPic " <<type)

    std::list<RuinCacheItem*>::iterator it;
    RuinCacheItem* myitem;

    for (it = d_ruinlist.begin(); it != d_ruinlist.end(); it++)
    {
        if ((*it)->type == type)
        {
            myitem = (*it);

            //put the item in last place (last touched)
            d_ruinlist.erase(it);
            d_ruinlist.push_back(myitem);

            return myitem->surface;
        }
    }

    //no item found -> create a new one
    myitem = addRuinPic(type);

    return myitem->surface;
}

PixMask* GraphicsCache::getDiplomacyPic(int type, Player::DiplomaticState state)
{
    debug("GraphicsCache::getDiplomaticPic " <<type << ", " << state)

    std::list<DiplomacyCacheItem*>::iterator it;
    DiplomacyCacheItem* myitem;

    for (it = d_diplomacylist.begin(); it != d_diplomacylist.end(); it++)
    {
        if ((*it)->type == type && (*it)->state == state)
        {
            myitem = (*it);

            //put the item in last place (last touched)
            d_diplomacylist.erase(it);
            d_diplomacylist.push_back(myitem);

            return myitem->surface;
        }
    }

    //no item found -> create a new one
    myitem = addDiplomacyPic(type, state);

    return myitem->surface;
}

PixMask* GraphicsCache::getRoadPic(int type)
{
    debug("GraphicsCache::getRoadPic " <<type)

    std::list<RoadCacheItem*>::iterator it;
    RoadCacheItem* myitem;

    for (it = d_roadlist.begin(); it != d_roadlist.end(); it++)
    {
        if ((*it)->type == type)
        {
            myitem = (*it);

            //put the item in last place (last touched)
            d_roadlist.erase(it);
            d_roadlist.push_back(myitem);

            return myitem->surface;
        }
    }

    //no item found -> create a new one
    myitem = addRoadPic(type);

    return myitem->surface;
}

PixMask* GraphicsCache::getFogPic(int type)
{
    debug("GraphicsCache::getFogPic " <<type)

    std::list<FogCacheItem*>::iterator it;
    FogCacheItem* myitem;

    FogCacheItem item = FogCacheItem();
    item.type = type;
    FogCacheMap::iterator mit = d_fogmap.find(item);
    if (mit != d_fogmap.end())
      {
	std::list<FogCacheItem*>::iterator it = (*mit).second;
	myitem = *it;
	d_foglist.erase(it);
	d_foglist.push_back(myitem);
	it = d_foglist.end();
	it--;
	d_fogmap[*myitem] = it;
	return myitem->surface;
      }

    //no item found -> create a new one
    myitem = addFogPic(&item);

    return myitem->surface;
}

PixMask* GraphicsCache::getBridgePic(int type)
{
    debug("GraphicsCache::getBridgePic " <<type)

    std::list<BridgeCacheItem*>::iterator it;
    BridgeCacheItem* myitem;

    for (it = d_bridgelist.begin(); it != d_bridgelist.end(); it++)
    {
        if ((*it)->type == type)
        {
            myitem = (*it);

            //put the item in last place (last touched)
            d_bridgelist.erase(it);
            d_bridgelist.push_back(myitem);

            return myitem->surface;
        }
    }

    //no item found -> create a new one
    myitem = addBridgePic(type);

    return myitem->surface;
}

PixMask* GraphicsCache::getCursorPic(int type)
{
    debug("GraphicsCache::getCursorPic " <<type)

    std::list<CursorCacheItem*>::iterator it;
    CursorCacheItem* myitem;

    for (it = d_cursorlist.begin(); it != d_cursorlist.end(); it++)
    {
        if ((*it)->type == type)
        {
            myitem = (*it);

            //put the item in last place (last touched)
            d_cursorlist.erase(it);
            d_cursorlist.push_back(myitem);

            return myitem->surface;
        }
    }

    //no item found -> create a new one
    myitem = addCursorPic(type);

    return myitem->surface;
}

PixMask* GraphicsCache::getCityPic(const City* city)
{
    if (!city)
        return NULL;
    if (city->isBurnt())
      return d_razedpic[city->getOwner()->getId()];
    else
      return d_citypic[city->getOwner()->getId()];
}

PixMask* GraphicsCache::getCityPic(int type, const Player* p)
{
    debug("GraphicsCache::getCityPic " <<type <<", player " <<p->getName())

    if (type == -1)
      return d_razedpic[p->getId()];
    else
      return d_citypic[p->getId()];
}

PixMask* GraphicsCache::getTowerPic(const Player* p)
{
    debug("GraphicsCache::getTowerPic player " <<p->getName())
    return d_towerpic[p->getId()];
}

PixMask* GraphicsCache::getFlagPic(guint32 stack_size, const Player *p)
{
    debug("GraphicsCache::getFlagPic " <<stack_size <<", player" <<p->getId())

    std::list<FlagCacheItem*>::iterator it;
    FlagCacheItem* myitem;

    for (it = d_flaglist.begin(); it != d_flaglist.end(); it++)
    {
        myitem = *it;
        if (myitem->size == stack_size && myitem->player_id == p->getId())
        {
            // put the item in last place (last touched)
            d_flaglist.erase(it);
            d_flaglist.push_back(myitem);

            return myitem->surface;
        }
    }

    // no item found => create a new one
    myitem = addFlagPic(stack_size, p);

    return myitem->surface;
}

PixMask* GraphicsCache::getFlagPic(const Stack* s)
{
    if (!s)
    {
        std::cerr << "GraphicsCache::getFlagPic: no stack supplied! Exiting...\n";
        exit(-1);
    }
    
  return getFlagPic(s->size(), s->getOwner());
}

PixMask* GraphicsCache::getSelectorPic(guint32 type, guint32 frame, 
					   const Player *p)
{
    debug("GraphicsCache::getSelectorPic " <<type <<", " << frame << ", player" <<s->getOwner()->getName())

    if (!p)
    {
        std::cerr << "GraphicsCache::getSelectorPic: no player supplied! Exiting...\n";
        exit(-1);
    }
    
    std::list<SelectorCacheItem*>::iterator it;
    SelectorCacheItem* myitem;

    for (it = d_selectorlist.begin(); it != d_selectorlist.end(); it++)
    {
        myitem = *it;
        if ((myitem->type == type) && (myitem->player_id == p->getId()) 
	    && myitem->frame == frame)
        {
            // put the item in last place (last touched)
            d_selectorlist.erase(it);
            d_selectorlist.push_back(myitem);

            return myitem->surface;
        }
    }

    // no item found => create a new one
    myitem = addSelectorPic(type, frame, p);

    return myitem->surface;
}


PixMask* GraphicsCache::getProdShieldPic(guint32 type, bool prod)
{
    debug("GraphicsCache::getProdShieldPic " <<type <<", " << ", prod " <<prod)

    std::list<ProdShieldCacheItem*>::iterator it;
    ProdShieldCacheItem* myitem;

    for (it = d_prodshieldlist.begin(); it != d_prodshieldlist.end(); it++)
    {
        myitem = *it;
        if ((myitem->type == type) && (myitem->prod == prod))
        {
            // put the item in last place (last touched)
            d_prodshieldlist.erase(it);
            d_prodshieldlist.push_back(myitem);

            return myitem->surface;
        }
    }

    // no item found => create a new one
    myitem = addProdShieldPic(type, prod);

    return myitem->surface;
}


PixMask* GraphicsCache::applyMask(PixMask* image, PixMask* mask, struct rgb_shift shifts, bool isNeutral)
{
  int width = image->get_width();
  int height = image->get_height();
  PixMask* result = PixMask::create(image->to_pixbuf());
  if (mask->get_width() != width || (mask->get_height()) != height)
    {
      std::cerr <<"Warning: mask and original image do not match\n";
      return NULL;
    }
  if (isNeutral)
    return result;
  
  Glib::RefPtr<Gdk::Pixbuf> maskbuf = mask->to_pixbuf();

  guint8 *data = maskbuf->get_pixels();
  guint8 *copy = (guint8*)  malloc (height * width * 4 * sizeof(guint8));
  memcpy(copy, data, height * width * 4 * sizeof(guint8));
  for (int i = 0; i < width; i++)
    for (int j = 0; j < height; j++)
      {
	const int base = (j * 4) + (i * height * 4);

	if (copy[base+3] != 0)
	  {
	    copy[base+0] >>= (shifts.r);
	    copy[base+1] >>= (shifts.g);
	    copy[base+2] >>= (shifts.b);
	  }
      }
  Glib::RefPtr<Gdk::Pixbuf> colouredmask = 
    Gdk::Pixbuf::create_from_data(copy, Gdk::COLORSPACE_RGB, true, 8, 
				  width, height, width * 4);
  result->draw_pixbuf(colouredmask, 0, 0, 0, 0, width, height);
  free(copy);

  return result;
}

PixMask* GraphicsCache::applyMask(PixMask* image, PixMask* mask, const Player* p)
{
  return applyMask(image, mask, p->getMaskColorShifts(),
		   Playerlist::getInstance()->getNeutral()->getId() == p->getId());
}

void GraphicsCache::checkPictures()
{
  // for security, we always take a minimum cache size of 2MB. This
  // includes (at 4 byte color depth and assuming 64x64 pixel size)
  // - 10 cities (each 64kb => 640kb)
  // - 20 flags (each 16kb => 320kb)
  // - 40 units (each 16kb => 640kb)
  // + a bit more. The problem is that if we have less images than needed
  // for a single rendering, the surfaces will become invalid before actually
  // used. This should not be a problem with the normal map (the surfaces are
  // copied and discarded), but when selecting armies from armyset, where
  // you can have these images assigned to buttons.
  guint32 maxcache = Configuration::s_cacheSize;
  if (maxcache < (1<<21))
    maxcache = (1<<21);

  if (d_cachesize < maxcache)
    return;


  // Now the cache size has been exceeded. We try to guarantee the values
  // given above and reduce the number of images. Let us start with the
  // cities

  while (d_citylist.size() > 10)
    eraseLastCityItem();

  while (d_towerlist.size() > 10)
    eraseLastTowerItem();

  while (d_shiplist.size() > 10)
    eraseLastShipItem();

  while (d_plantedstandardlist.size() > 10)
    eraseLastPlantedStandardItem();

  while (d_templelist.size() > 10)
    eraseLastTempleItem();

  while (d_ruinlist.size() > 10)
    eraseLastRuinItem();

  while (d_diplomacylist.size() > 10)
    eraseLastDiplomacyItem();

  while (d_roadlist.size() > 18)
    eraseLastRoadItem();

  while (d_foglist.size() > 16)
    eraseLastFogItem();

  while (d_bridgelist.size() > 10)
    eraseLastBridgeItem();

  while (d_cursorlist.size() > 10)
    eraseLastCursorItem();

  // was this enough?
  if (d_cachesize < maxcache)
    return;

  // next, kill flag pics
  while (d_flaglist.size() > 20)
    eraseLastFlagItem();

  if (d_cachesize < maxcache)
    return;

  // next, kill selector pics
  while (d_selectorlist.size() > 20)
    eraseLastSelectorItem();

  if (d_cachesize < maxcache)
    return;

  // next, kill shield pics
  while (d_shieldlist.size() > 20)
    eraseLastShieldItem();

  if (d_cachesize < maxcache)
    return;

  // next, kill production shield pics
  while (d_prodshieldlist.size() > 20)
    eraseLastProdShieldItem();

  if (d_cachesize < maxcache)
    return;

  // next, kill movement bonus pics
  while (d_movebonuslist.size() > 20)
    eraseLastMoveBonusItem();

  if (d_cachesize < maxcache)
    return;

  // still not enough? Erase tile images
  while (d_tilelist.size() > 200)
    eraseLastTileItem();

  // still not enough? Erase army images
  while (d_armylist.size() > 40)
    eraseLastArmyItem();

}

void GraphicsCache::drawTilePic(PixMask *surface, int fog_type_id, bool has_bag, bool has_standard, int standard_player_id, int stack_size, int stack_player_id, int army_type_id, bool has_tower, bool has_ship, Maptile::Building building_type, int building_subtype, Vector<int> building_tile, int building_player_id, guint32 ts, bool has_grid)
{
  const Player *player;
  Glib::RefPtr<Gdk::Pixmap> pixmap = surface->get_pixmap();

  switch (building_type)
    {
    case Maptile::CITY:
	{
	  player = Playerlist::getInstance()->getPlayer(building_player_id);
	  getCityPic(building_subtype, player)->blit(building_tile, ts, pixmap);
	}
      break;
    case Maptile::RUIN:
      getRuinPic(building_subtype)->blit(building_tile, ts, pixmap); break;
    case Maptile::TEMPLE:
      getTemplePic(building_subtype)->blit(building_tile, ts, pixmap); break;
    case Maptile::SIGNPOST:
      getSignpostPic()->blit(building_tile, ts, pixmap); break;
    case Maptile::ROAD:
      getRoadPic(building_subtype)->blit(building_tile, ts, pixmap); break;
    case Maptile::PORT:
      getPortPic()->blit(building_tile, ts, pixmap); break;
    case Maptile::BRIDGE:
      getBridgePic(building_subtype)->blit(building_tile, ts, pixmap); break;
      break;
    case Maptile::NONE: default:
      break;
    }

  if (has_standard)
    {
      player = Playerlist::getInstance()->getPlayer(standard_player_id) ;
      getPlantedStandardPic(player)->blit(pixmap);
    }

  if (has_bag)
    getBagPic()->blit(pixmap, Vector<int>(ts - 18, ts - 18));

  if (stack_player_id > -1)
    {
      player = Playerlist::getInstance()->getPlayer(stack_player_id);
      if (has_tower)
	getTowerPic(player)->blit(pixmap);
      else
	{
	  if (stack_size > -1)
	    getFlagPic(stack_size, player)->blit(pixmap);
	  if (has_ship)
	    getShipPic(player)->blit(pixmap);
	  else
	    getArmyPic(player->getArmyset(), army_type_id, player, NULL)->blit(pixmap);
	}
    }
  if (has_grid)
    {
      Glib::RefPtr<Gdk::GC> context = surface->get_gc();
      Gdk::Color line_color = Gdk::Color();
      line_color.set_rgb_p(0,0,0);
      context->set_rgb_fg_color(line_color);
      pixmap->draw_rectangle(context, false, 0, 0, ts, ts);
    }

  if (fog_type_id)
    getFogPic(fog_type_id - 1)->blit(pixmap);
}

TileCacheItem* GraphicsCache::addTilePic(TileCacheItem *item)
{
    
  debug("ADD tile pic " << " " << item->tile_style_id << " " <<
	item->fog_type_id << " " << item->has_bag << " " << 
	item->has_standard << " " << item->standard_player_id << " " << 
	item->stack_size << " " << item->stack_player_id <<
	" " << item->army_type_id << " " << item->has_tower << " " << 
	item->has_ship << " " << item->building_type << " " << 
	item->building_subtype << " " << item->building_tile.x << 
	"," << item->building_tile.y << " " << item->building_player_id << 
	" " << item->tilesize << " " << item->has_grid);

  TileCacheItem* myitem = new TileCacheItem();
  *myitem = *item;

  //short circuit the drawing sequence if the tile is completely obscured
  if (myitem->fog_type_id == FogMap::ALL)
    myitem->surface = getFogPic(myitem->fog_type_id - 1)->copy();
  else
    {
      Tileset *tileset = GameMap::getInstance()->getTileset();
      myitem->surface = 
	tileset->getTileStyle(myitem->tile_style_id)->getImage()->copy();
    
      drawTilePic(myitem->surface, myitem->fog_type_id, myitem->has_bag, 
		  myitem->has_standard, myitem->standard_player_id, 
		  myitem->stack_size, myitem->stack_player_id, 
		  myitem->army_type_id, myitem->has_tower, myitem->has_ship, 
		  myitem->building_type, myitem->building_subtype, 
		  myitem->building_tile, myitem->building_player_id, 
		  myitem->tilesize, myitem->has_grid);
    }

  //now the final preparation steps:
  //a) add the size
  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize += myitem->surface->get_depth()/8 * size;

  //b) add the entry to the list
  d_tilelist.push_back(myitem);
  std::list<TileCacheItem*>::iterator it = d_tilelist.end();
  it--;
  d_tilemap[*myitem] = it;

  //c) check if the cache size is too large
  checkPictures();

  //we are finished, so return the pic
  return myitem;
}

ArmyCacheItem* GraphicsCache::addArmyPic(ArmyCacheItem *item)
{
  debug("ADD army pic: " <<item->armyset <<"," <<item->army_id)

  ArmyCacheItem* myitem = new ArmyCacheItem();
  *myitem = *item;

  const ArmyProto * basearmy = 
    Armysetlist::getInstance()->getArmy(myitem->armyset, myitem->army_id);

  // copy the pixmap including player colors
  Player *p = Playerlist::getInstance()->getPlayer(myitem->player_id);
  myitem->surface = applyMask(basearmy->getImage(), basearmy->getMask(), p);

  if (myitem->medals != NULL)
    {
      debug("medalsbonus============= " << medalsbonus); 
      for(int i=0;i<3;i++)
	{ 
	  if (myitem->medals[i])
	    {

	      d_medalpic[i]->blit(myitem->surface->get_pixmap(), i * d_medalpic[i]->get_width(), 0);
	      //myitem->surface->draw_drawable(Gdk::GC::create(d_medalpic[i]),
					     //d_medalpic[i], 0, 0, 
					     //0, i * d_medalpic[i]->get_width(), 
					     //d_medalpic[i]->get_width(), 
					     //d_medalpic[i]->get_height());
	    }
	}
    }

  //now the final preparation steps:
  //a) add the size
  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize += myitem->surface->get_depth()/8 * size;

  //b) add the entry to the list
  d_armylist.push_back(myitem);
  std::list<ArmyCacheItem*>::iterator it = d_armylist.end();
  it--;
  d_armymap[*myitem] = it;

  //c) check if the cache size is too large
  checkPictures();

  //we are finished, so return the pic
  return myitem;
}

ShieldCacheItem* GraphicsCache::addShieldPic(std::string shieldset, 
					     guint32 type, guint32 colour)
{
  debug("ADD shield pic: " <<shieldset <<"," <<type <<"," <<colour)

    ShieldCacheItem* myitem = new ShieldCacheItem();
  myitem->shieldset = shieldset;
  myitem->type = type;
  myitem->colour = colour;

  ShieldStyle *sh;
  sh = Shieldsetlist::getInstance()->getShield(shieldset, type, colour);

  // copy the pixmap including player colors
  //lookup the shieldstyle
  myitem->surface = applyMask(sh->getImage(), sh->getMask(), 
			      Playerlist::getInstance()->getPlayer(colour));

  //now the final preparation steps:
  //a) add the size
  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize += myitem->surface->get_depth()/8 * size;

  //b) add the entry to the list
  d_shieldlist.push_back(myitem);

  //c) check if the cache size is too large
  checkPictures();

  //we are finished, so return the pic
  return myitem;
}


ShipCacheItem* GraphicsCache::addShipPic(const Player* p)
{
  debug("ADD ship pic: " <<p->getName())

  ShipCacheItem* myitem = new ShipCacheItem();
  myitem->player_id = p->getId();

  Armysetlist *al = Armysetlist::getInstance();
  PixMask*ship = al->getShipPic(p->getArmyset());
  PixMask*shipmask = al->getShipMask(p->getArmyset());
  // copy the pixmap including player colors
  myitem->surface = applyMask(ship, shipmask, p);

  //now the final preparation steps:
  //a) add the size
  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize += myitem->surface->get_depth()/8 * size;

  //b) add the entry to the list
  d_shiplist.push_back(myitem);

  //c) check if the cache size is too large
  checkPictures();

  //we are finished, so return the pic
  return myitem;
}

PlantedStandardCacheItem* GraphicsCache::addPlantedStandardPic(const Player* p)
{
  debug("ADD planted standard pic: " <<p->getName())

    PlantedStandardCacheItem* myitem = new PlantedStandardCacheItem();
  myitem->player_id = p->getId();

  Armysetlist *al = Armysetlist::getInstance();
  PixMask*standard = al->getStandardPic(p->getArmyset());
  PixMask*standard_mask = al->getStandardMask(p->getArmyset());

  // copy the pixmap including player colors
  myitem->surface = applyMask(standard, standard_mask, p);

  //now the final preparation steps:
  //a) add the size
  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize += myitem->surface->get_depth()/8 * size;

  //b) add the entry to the list
  d_plantedstandardlist.push_back(myitem);

  //c) check if the cache size is too large
  checkPictures();

  //we are finished, so return the pic
  return myitem;
}


TempleCacheItem* GraphicsCache::addTemplePic(int type)
{
  //    int ts = GameMap::getInstance()->getTileset()->getTileSize();

  PixMask* mysurf = d_templepic[type];

  //now create the cache item and add the size
  TempleCacheItem* myitem = new TempleCacheItem();
  myitem->type = type;
  myitem->surface = mysurf;

  d_templelist.push_back(myitem);

  //add the size
  int size = mysurf->get_width() * mysurf->get_height();
  d_cachesize += size * mysurf->get_depth()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

RuinCacheItem* GraphicsCache::addRuinPic(int type)
{
  PixMask* mysurf = d_ruinpic[type];

  //now create the cache item and add the size
  RuinCacheItem* myitem = new RuinCacheItem();
  myitem->type = type;
  myitem->surface = mysurf;

  d_ruinlist.push_back(myitem);

  //add the size
  int size = mysurf->get_width() * mysurf->get_height();
  d_cachesize += size * mysurf->get_depth()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

DiplomacyCacheItem* GraphicsCache::addDiplomacyPic(int type, Player::DiplomaticState state)
{
  PixMask* mysurf = 
    d_diplomacypic[type][state - Player::AT_PEACE];

  //now create the cache item and add the size
  DiplomacyCacheItem* myitem = new DiplomacyCacheItem();
  myitem->type = type;
  myitem->state = state;
  myitem->surface = mysurf;

  d_diplomacylist.push_back(myitem);

  //add the size
  int size = mysurf->get_width() * mysurf->get_height();
  d_cachesize += size * mysurf->get_depth()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

RoadCacheItem* GraphicsCache::addRoadPic(int type)
{
  //    int ts = GameMap::getInstance()->getTileset()->getTileSize();

  PixMask* mysurf = d_roadpic[type];

  //now create the cache item and add the size
  RoadCacheItem* myitem = new RoadCacheItem();
  myitem->type = type;
  myitem->surface = mysurf;

  d_roadlist.push_back(myitem);

  //add the size
  int size = mysurf->get_width() * mysurf->get_height();
  d_cachesize += size * mysurf->get_depth()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

FogCacheItem* GraphicsCache::addFogPic(FogCacheItem *item)
{

  PixMask* mysurf = d_fogpic[item->type]->copy();

  //now create the cache item and add the size
  FogCacheItem* myitem = new FogCacheItem();
  *myitem = *item;

  d_foglist.push_back(myitem);
  std::list<FogCacheItem*>::iterator it = d_foglist.end();
  it--;
  d_fogmap[*myitem] = it;

  //add the size
  int size = mysurf->get_width() * mysurf->get_height();
  d_cachesize += size * mysurf->get_depth()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

BridgeCacheItem* GraphicsCache::addBridgePic(int type)
{
  //    int ts = GameMap::getInstance()->getTileset()->getTileSize();

  PixMask* mysurf = d_bridgepic[type];

  //now create the cache item and add the size
  BridgeCacheItem* myitem = new BridgeCacheItem();
  myitem->type = type;
  myitem->surface = mysurf;

  d_bridgelist.push_back(myitem);

  //add the size
  int size = mysurf->get_width() * mysurf->get_height();
  d_cachesize += size * mysurf->get_depth()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

CursorCacheItem* GraphicsCache::addCursorPic(int type)
{
  PixMask* mysurf = d_cursorpic[type];

  //now create the cache item and add the size
  CursorCacheItem* myitem = new CursorCacheItem();
  myitem->type = type;
  myitem->surface = mysurf;

  d_cursorlist.push_back(myitem);

  //add the size
  int size = mysurf->get_width() * mysurf->get_height();
  d_cachesize += size * mysurf->get_depth()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

CityCacheItem* GraphicsCache::addCityPic(int type, const Player* p)
{
  //now create the cache item and add the size
  CityCacheItem* myitem = new CityCacheItem();
  myitem->player_id = p->getId();
  myitem->type = type;
  myitem->surface = d_citypic[p->getId()];

  d_citylist.push_back(myitem);

  //add the size
  int size = d_citypic[p->getId()]->get_width() * d_citypic[p->getId()]->get_height();
  d_cachesize += size * d_citypic[p->getId()]->get_depth()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

TowerCacheItem* GraphicsCache::addTowerPic(const Player* p)
{
  //now create the cache item and add the size
  TowerCacheItem* myitem = new TowerCacheItem();
  myitem->player_id = p->getId();
  myitem->surface = d_towerpic[p->getId()];

  d_towerlist.push_back(myitem);

  //add the size
  int size = d_towerpic[p->getId()]->get_width() * d_towerpic[p->getId()]->get_height();
  d_cachesize += size * d_towerpic[p->getId()]->get_depth()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

FlagCacheItem* GraphicsCache::addFlagPic(int size, const Player *p)
{
  debug("GraphicsCache::addFlagPic, player="<<p->getId()<<", size="<<size)

    // size of the stack starts at 1, but we need the index, which starts at 0
    size--;

  PixMask* mysurf = applyMask(d_flagpic[size], d_flagmask[size], p);

  //now create the cache item and add the size
  FlagCacheItem* myitem = new FlagCacheItem();
  myitem->player_id = p->getId();
  myitem->size = size;
  myitem->surface = mysurf;

  d_flaglist.push_back(myitem);

  //add the size
  int picsize = mysurf->get_width() * mysurf->get_height();
  d_cachesize += picsize * mysurf->get_depth()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

SelectorCacheItem* GraphicsCache::addSelectorPic(guint32 type, guint32 frame, const Player* p)
{
  debug("GraphicsCache::addSelectorPic, player="<<p->getName()<<", type="<<type<< ", " << frame)

    // frame is the frame of animation we're looking for.  starts at 0.
    // type is 0 for big, 1 for small

    PixMask* mysurf;
  if (type == 0)
    mysurf = applyMask(d_selector[frame], d_selectormask[frame], p);
  else
    mysurf = applyMask(d_smallselector[frame], d_smallselectormask[frame], p);

  //now create the cache item and add the size
  SelectorCacheItem* myitem = new SelectorCacheItem();
  myitem->player_id = p->getId();
  myitem->type = type;
  myitem->frame = frame;
  myitem->surface = mysurf;

  d_selectorlist.push_back(myitem);

  //add the size
  int picsize = mysurf->get_width() * mysurf->get_height();
  d_cachesize += picsize * mysurf->get_depth()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

ProdShieldCacheItem* GraphicsCache::addProdShieldPic(guint32 type, bool prod)
{
  debug("GraphicsCache::addProdShieldPic, prod="<<prod<<", type="<<type)

    // type is 0 for home, 1 for away, 2 for destination, 3 for source,
    // 4 for invalid

    PixMask* mysurf = NULL;
  switch (type)
    {
    case 0: //home city
      if (prod) //production
	mysurf = d_prodshieldpic[1]->copy();
      else //no production
	mysurf = d_prodshieldpic[0]->copy();
      break;
    case 1: //away city
      if (prod) //production
	mysurf = d_prodshieldpic[3]->copy();
      else //no production
	mysurf = d_prodshieldpic[2]->copy();
      break;
    case 2: //destination city
      if (prod) //production
	mysurf = d_prodshieldpic[5]->copy();
      else //no production
	mysurf = d_prodshieldpic[4]->copy();
      break;
    case 3: //source city
      mysurf = d_prodshieldpic[6]->copy();
      break;
    case 4: //invalid
      mysurf = d_prodshieldpic[7]->copy();
      break;
    }

  //now create the cache item and add the size
  ProdShieldCacheItem* myitem = new ProdShieldCacheItem();
  myitem->prod = prod;
  myitem->type = type;
  myitem->surface = mysurf;

  d_prodshieldlist.push_back(myitem);

  //add the size
  int picsize = mysurf->get_width() * mysurf->get_height();
  d_cachesize += picsize * mysurf->get_depth()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

MoveBonusCacheItem* GraphicsCache::addMoveBonusPic(guint32 type)
{
  debug("GraphicsCache::addMoveBonusPic, type="<<type)

    //type is 0=empty, 1=trees, 2=foothills, 3=hills+trees, 4=fly, 5=boat

    PixMask* mysurf;
  mysurf = d_movebonuspic[type]->copy();

  //now create the cache item and add the size
  MoveBonusCacheItem* myitem = new MoveBonusCacheItem();
  myitem->type = type;
  myitem->surface = mysurf;

  d_movebonuslist.push_back(myitem);

  //add the size
  int picsize = mysurf->get_width() * mysurf->get_height();
  d_cachesize += picsize * mysurf->get_depth()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}


void GraphicsCache::clear()
{
  while (!d_armylist.empty())
    eraseLastArmyItem();

  while (!d_tilelist.empty())
    eraseLastTileItem();

  while (!d_templelist.empty())
    eraseLastTempleItem();

  while (!d_ruinlist.empty())
    eraseLastRuinItem();

  while (!d_diplomacylist.empty())
    eraseLastDiplomacyItem();

  while (!d_roadlist.empty())
    eraseLastRoadItem();

  while (!d_foglist.empty())
    eraseLastFogItem();

  while (!d_bridgelist.empty())
    eraseLastBridgeItem();

  while (!d_cursorlist.empty())
    eraseLastCursorItem();

  while (!d_citylist.empty())
    eraseLastCityItem();

  while (!d_towerlist.empty())
    eraseLastTowerItem();

  while (!d_shiplist.empty())
    eraseLastShipItem();

  while (!d_plantedstandardlist.empty())
    eraseLastPlantedStandardItem();

  while (!d_flaglist.empty())
    eraseLastFlagItem();

  while (!d_selectorlist.empty())
    eraseLastSelectorItem();

  while (!d_shieldlist.empty())
    eraseLastShieldItem();

  while (!d_prodshieldlist.empty())
    eraseLastProdShieldItem();

  while (!d_movebonuslist.empty())
    eraseLastMoveBonusItem();
}

void GraphicsCache::eraseLastArmyItem()
{
  if (d_armylist.empty())
    return;

  //As the name suggests, this function erases the last item in the list.
  //Whenever an item is requested, it moves to the first position, so the
  //last item is the oldest and therefore propably most useless in the list.
  ArmyCacheItem* myitem = *(d_armylist.begin());
  ArmyMap::iterator it = d_armymap.find(*myitem);
  if (it != d_armymap.end())
    d_armymap.erase(it);
  d_armylist.erase(d_armylist.begin());

  //don't forget to subtract the size from the size entry
  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_depth()/8 * size;

  delete myitem->surface;
  delete myitem;
}

void GraphicsCache::eraseLastTileItem()
{
  if (d_tilelist.empty())
    return;

  //As the name suggests, this function erases the last item in the list.
  //Whenever an item is requested, it moves to the first position, so the
  //last item is the oldest and therefore propably most useless in the list.
  TileCacheItem* myitem = *(d_tilelist.begin());
  TileMap::iterator it = d_tilemap.find(*myitem);
  if (it != d_tilemap.end()) //fixme, find out why this check is necessary.
    d_tilemap.erase(it);
  d_tilelist.erase(d_tilelist.begin());

  //don't forget to subtract the size from the size entry
  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_depth()/8 * size;

  delete myitem->surface;
  delete myitem;
}

void GraphicsCache::eraseLastTempleItem()
{
  if (d_templelist.empty())
    return;

  TempleCacheItem* myitem = *(d_templelist.begin());
  d_templelist.erase(d_templelist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_depth()/8 * size;

  delete myitem->surface;
  delete myitem;
}

void GraphicsCache::eraseLastRuinItem()
{
  if (d_ruinlist.empty())
    return;

  RuinCacheItem* myitem = *(d_ruinlist.begin());
  d_ruinlist.erase(d_ruinlist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_depth()/8 * size;

  delete myitem->surface;
  delete myitem;
}

void GraphicsCache::eraseLastDiplomacyItem()
{
  if (d_diplomacylist.empty())
    return;

  DiplomacyCacheItem* myitem = *(d_diplomacylist.begin());
  d_diplomacylist.erase(d_diplomacylist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_depth()/8 * size;

  delete myitem->surface;
  delete myitem;
}

void GraphicsCache::eraseLastRoadItem()
{
  if (d_roadlist.empty())
    return;

  RoadCacheItem* myitem = *(d_roadlist.begin());
  d_roadlist.erase(d_roadlist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_depth()/8 * size;

  delete myitem->surface;
  delete myitem;
}

void GraphicsCache::eraseLastFogItem()
{
  if (d_foglist.empty())
    return;

  FogCacheItem* myitem = *(d_foglist.begin());
  FogCacheMap::iterator it = d_fogmap.find(*myitem);
  if (it != d_fogmap.end())
    d_fogmap.erase(it);
  d_foglist.erase(d_foglist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_depth()/8 * size;

  delete myitem->surface;
  delete myitem;
}

void GraphicsCache::eraseLastBridgeItem()
{
  if (d_bridgelist.empty())
    return;

  BridgeCacheItem* myitem = *(d_bridgelist.begin());
  d_bridgelist.erase(d_bridgelist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_depth()/8 * size;

  delete myitem->surface;
  delete myitem;
}

void GraphicsCache::eraseLastCursorItem()
{
  if (d_cursorlist.empty())
    return;

  CursorCacheItem* myitem = *(d_cursorlist.begin());
  d_cursorlist.erase(d_cursorlist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_depth()/8 * size;

  delete myitem->surface;
  delete myitem;
}

void GraphicsCache::eraseLastCityItem()
{
  if (d_citylist.empty())
    return;

  CityCacheItem* myitem = *(d_citylist.begin());
  d_citylist.erase(d_citylist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_depth()/8 * size;

  delete myitem->surface;
  delete myitem;
}

void GraphicsCache::eraseLastTowerItem()
{
  if (d_towerlist.empty())
    return;

  TowerCacheItem* myitem = *(d_towerlist.begin());
  d_towerlist.erase(d_towerlist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_depth()/8 * size;

  delete myitem->surface;
  delete myitem;
}

void GraphicsCache::eraseLastShipItem()
{
  if (d_shiplist.empty())
    return;

  ShipCacheItem* myitem = *(d_shiplist.begin());
  d_shiplist.erase(d_shiplist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_depth()/8 * size;

  delete myitem->surface;
  delete myitem;
}

void GraphicsCache::eraseLastPlantedStandardItem()
{
  if (d_plantedstandardlist.empty())
    return;

  PlantedStandardCacheItem* myitem = *(d_plantedstandardlist.begin());
  d_plantedstandardlist.erase(d_plantedstandardlist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_depth()/8 * size;

  delete myitem->surface;
  delete myitem;
}

void GraphicsCache::eraseLastFlagItem()
{
  if (d_flaglist.empty())
    return;

  FlagCacheItem* myitem = *(d_flaglist.begin());
  d_flaglist.erase(d_flaglist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_depth()/8 * size;

  delete myitem->surface;
  delete myitem;
}

void GraphicsCache::eraseLastSelectorItem()
{
  if (d_selectorlist.empty())
    return;

  SelectorCacheItem* myitem = *(d_selectorlist.begin());
  d_selectorlist.erase(d_selectorlist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_depth()/8 * size;

  delete myitem->surface;
  delete myitem;
}

void GraphicsCache::eraseLastShieldItem()
{
  if (d_shieldlist.empty())
    return;

  ShieldCacheItem* myitem = *(d_shieldlist.begin());
  d_shieldlist.erase(d_shieldlist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_depth()/8 * size;

  delete myitem->surface;
  delete myitem;
}

void GraphicsCache::eraseLastProdShieldItem()
{
  if (d_prodshieldlist.empty())
    return;

  ProdShieldCacheItem* myitem = *(d_prodshieldlist.begin());
  d_prodshieldlist.erase(d_prodshieldlist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_depth()/8 * size;

  delete myitem->surface;
  delete myitem;
}

void GraphicsCache::eraseLastMoveBonusItem()
{
  if (d_movebonuslist.empty())
    return;

  MoveBonusCacheItem* myitem = *(d_movebonuslist.begin());
  d_movebonuslist.erase(d_movebonuslist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_depth()/8 * size;

  delete myitem->surface;
  delete myitem;
}

void GraphicsCache::loadTemplePics()
{
  // GameMap has the actual cityset stored
  std::string cityset = GameMap::getInstance()->getCityset()->getSubDir();
  int ts = GameMap::getInstance()->getCityset()->getTileSize();

  // load the temple pictures
  std::vector<PixMask* > templepics;
  templepics = disassemble_row(File::getCitysetFile(cityset, "temples.png"), 
			       TEMPLE_TYPES);
  for (unsigned int i = 0; i < TEMPLE_TYPES; i++)
    {
      if (templepics[i]->get_width() != ts)
	PixMask::scale(templepics[i], ts, ts);
      d_templepic[i] = templepics[i];
    }
}

void GraphicsCache::loadRuinPics()
{
  // GameMap has the actual cityset stored
  std::string cityset = GameMap::getInstance()->getCityset()->getSubDir();
  int ts = GameMap::getInstance()->getCityset()->getTileSize();

  // load the ruin pictures
  std::vector<PixMask* > ruinpics;
  ruinpics = disassemble_row(File::getCitysetFile(cityset, "ruin.png"), 
			     RUIN_TYPES);

  for (unsigned int i = 0; i < RUIN_TYPES ; i++)
    {
      if (ruinpics[i]->get_width() != ts)
	PixMask::scale(ruinpics[i], ts, ts);
      d_ruinpic[i] = ruinpics[i];
    }
}

void GraphicsCache::loadDiplomacyPics()
{
  int ts = 30;
  std::vector<PixMask* > diplomacypics;
  diplomacypics = disassemble_row(File::getMiscFile("various/diplomacy-small.png"), 
			     DIPLOMACY_TYPES);
  for (unsigned int i = 0; i < DIPLOMACY_TYPES ; i++)
    {
      if (diplomacypics[i]->get_width() != ts)
	PixMask::scale(diplomacypics[i], ts, ts);
      d_diplomacypic[0][i] = diplomacypics[i];

    }

  ts = 50;
  diplomacypics = disassemble_row(File::getMiscFile("various/diplomacy-large.png"), 
			     DIPLOMACY_TYPES);
  for (unsigned int i = 0; i < DIPLOMACY_TYPES ; i++)
    {
      if (diplomacypics[i]->get_width() != ts)
	PixMask::scale(diplomacypics[i], ts, ts);
      d_diplomacypic[1][i] = diplomacypics[i];

    }
}

void GraphicsCache::loadRoadPics()
{
  // GameMap has the actual tileset stored
  std::string tileset = GameMap::getInstance()->getTileset()->getSubDir();
  int ts = GameMap::getInstance()->getTileset()->getTileSize();

  std::vector<PixMask* > roadpics;
  roadpics = disassemble_row(File::getTilesetFile(tileset, "misc/roads.png"), 
			     ROAD_TYPES);
  for (unsigned int i = 0; i < ROAD_TYPES ; i++)
    {
      if (roadpics[i]->get_width() != ts)
	PixMask::scale(roadpics[i], ts, ts);
      d_roadpic[i] = roadpics[i];
    }
}

void GraphicsCache::loadFogPics()
{
  // GameMap has the actual tileset stored
  int ts = GameMap::getInstance()->getTileset()->getTileSize();
  std::string tileset = GameMap::getInstance()->getTileset()->getSubDir();

  // load the fog pictures
  std::vector<PixMask* > fogpics;
  fogpics = disassemble_row(File::getTilesetFile(tileset, "misc/fog.png"),
			     FOG_TYPES);
  for (unsigned int i = 0; i < FOG_TYPES ; i++)
    {
      if (fogpics[i]->get_width() != ts)
	PixMask::scale(fogpics[i], ts, ts);
      d_fogpic[i] = fogpics[i];
    }

}

void GraphicsCache::loadBridgePics()
{
  // GameMap has the actual tileset stored
  std::string tileset = GameMap::getInstance()->getTileset()->getSubDir();
  int ts = GameMap::getInstance()->getTileset()->getTileSize();

  // load the bridge pictures
  std::vector<PixMask* > bridgepics;
  bridgepics = disassemble_row(File::getTilesetFile(tileset, 
						    "misc/bridges.png"),
			       BRIDGE_TYPES);
  for (unsigned int i = 0; i < BRIDGE_TYPES ; i++)
    {
      if (bridgepics[i]->get_width() != ts)
	PixMask::scale(bridgepics[i], ts, ts);
      d_bridgepic[i] = bridgepics[i];
    }
}

void GraphicsCache::loadCursorPics()
{
  int ts = 16;

  // load the cursor pictures
  std::vector<PixMask* > cursorpics;
  cursorpics = disassemble_row(File::getMiscFile("various/cursors.png"),
			       CURSOR_TYPES);
  for (unsigned int i = 0; i < CURSOR_TYPES ; i++)
    {
      if (cursorpics[i]->get_width() != ts)
	PixMask::scale(cursorpics[i], ts, ts);
      d_cursorpic[i] = cursorpics[i];
    }
}

void GraphicsCache::loadCityPics()
{
  // GameMap has the actual cityset stored
  std::string cityset = GameMap::getInstance()->getCityset()->getSubDir();
  int ts = GameMap::getInstance()->getTileset()->getTileSize();

  std::vector<PixMask* > razedpics;
  razedpics = disassemble_row(File::getCitysetFile(cityset, "castle_razed.png"),
			      MAX_PLAYERS + 1);
  for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
    {
      if (razedpics[i]->get_width() != ts)
	PixMask::scale(razedpics[i], ts *City::getWidth() , ts * City::getWidth());
      d_razedpic[i] = razedpics[i];
    }

  // load the city pictures
  std::vector<PixMask* > citypics;
  citypics = disassemble_row(File::getCitysetFile(cityset, "castles.png"),
			      MAX_PLAYERS + 1);
  for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
    {
      if (citypics[i]->get_width() != ts)
	PixMask::scale(citypics[i], ts *City::getWidth(), ts * City::getWidth());
      d_citypic[i] = citypics[i];
    }
}

void GraphicsCache::loadTowerPics()
{
  // GameMap has the actual cityset stored
  std::string cityset = GameMap::getInstance()->getCityset()->getSubDir();
  int ts = GameMap::getInstance()->getCityset()->getTileSize();

  std::vector<PixMask* > towerpics;
  towerpics = disassemble_row(File::getCitysetFile(cityset, "towers.png"),
			      MAX_PLAYERS);
  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      if (towerpics[i]->get_width() != ts)
	PixMask::scale(towerpics[i], ts, ts);
      d_towerpic[i] = towerpics[i];
    }
}

bool GraphicsCache::loadSelectorImages(std::string tileset, std::string filename, guint32 size, std::vector<PixMask* > &images, std::vector<PixMask* > &masks)
{
  int num_frames;
  num_frames = Gdk::Pixbuf::create_from_file 
    (File::getTilesetFile(tileset, filename))->get_width() / size;
  images = disassemble_row(File::getTilesetFile(tileset, filename),
			      num_frames, true);
  for (int i = 0; i < num_frames; i++)
    {
      if (images[i]->get_width() != (int)size)
	PixMask::scale(images[i], size, size);
    }

  masks = disassemble_row(File::getTilesetFile(tileset, filename),
			      num_frames, false);
  for (int i = 0; i < num_frames; i++)
    {
      if (masks[i]->get_width() != (int)size)
	PixMask::scale(masks[i], size, size);
    }

  return true;
}

void GraphicsCache::loadSelectors()
{
  std::string tileset = GameMap::getInstance()->getTileset()->getSubDir();
  std::string small = GameMap::getInstance()->getTileset()->getSmallSelectorFilename();
  std::string large = GameMap::getInstance()->getTileset()->getLargeSelectorFilename();

  int size = GameMap::getInstance()->getTileset()->getTileSize();
  std::vector<PixMask* > images;
  std::vector<PixMask* > masks;
  bool success = loadSelectorImages(tileset, large, size, images, masks);
  if (!success)
    {
      fprintf (stderr,"Selector file %s is malformed\n", large.c_str());
      exit(1);
    }
  for (unsigned int i = 0; i < images.size(); i++)
    {
      d_selector.push_back(images[i]);
      d_selectormask.push_back(masks[i]);
    }

  images.clear();
  masks.clear();
  success = loadSelectorImages(tileset, small, size, images, masks);
  if (!success)
    {
      fprintf (stderr,"Selector file %s is malformed\n", small.c_str());
      exit(1);
    }
  for (unsigned int i = 0; i < images.size(); i++)
    {
      d_smallselector.push_back(images[i]);
      d_smallselectormask.push_back(masks[i]);
    }
}

void GraphicsCache::loadProdShields()
{
  //load the production shieldset
  int xsize = PRODUCTION_SHIELD_WIDTH;
  int ysize = PRODUCTION_SHIELD_HEIGHT;
  std::vector<PixMask* > prodshieldpics;
  prodshieldpics = disassemble_row
    (File::getMiscFile("various/prodshieldset.png"), PRODUCTION_SHIELD_TYPES);
  for (unsigned int i = 0; i < PRODUCTION_SHIELD_TYPES; i++)
    {
      if (prodshieldpics[i]->get_width() != xsize)
	PixMask::scale(prodshieldpics[i], xsize, ysize);
      d_prodshieldpic[i] = prodshieldpics[i];
    }
}

void GraphicsCache::loadMedalPics()
{
  //load the medal icons
  int ts = 40;
  std::vector<PixMask* > medalpics;
  medalpics = disassemble_row(File::getMiscFile("various/medals_mask.png"),
				  MEDAL_TYPES);
  for (unsigned int i = 0; i < MEDAL_TYPES; i++)
    {
      if (medalpics[i]->get_width() != ts)
	PixMask::scale(medalpics[i], ts, ts);
      d_medalpic[i] = medalpics[i];
    }
}

void GraphicsCache::loadMoveBonusPics()
{
  //load the movement bonus icons
  int xsize = MOVE_BONUS_WIDTH;
  int ysize = MOVE_BONUS_HEIGHT;
  std::vector<PixMask* > movebonuspics;
  movebonuspics = disassemble_row(File::getMiscFile("various/movebonus.png"),
				  MOVE_BONUS_TYPES);
  for (unsigned int i = 0; i < MOVE_BONUS_TYPES; i++)
    {
      if (movebonuspics[i]->get_width() != xsize)
	PixMask::scale(movebonuspics[i], xsize, ysize);
      d_movebonuspic[i] = movebonuspics[i];
    }
}

void GraphicsCache::loadFlags()
{
  //GameMap has the actual tileset stored
  std::string tileset = GameMap::getInstance()->getTileset()->getSubDir();
  int ts = GameMap::getInstance()->getTileset()->getTileSize();

  std::vector<PixMask* > flagpics;
  flagpics = disassemble_row(File::getTilesetFile(tileset, "misc/flags.png"),
				  MAX_STACK_SIZE, true);
  for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
    {
      if (flagpics[i]->get_width() != ts)
	PixMask::scale(flagpics[i], ts, ts);
      d_flagpic[i] = flagpics[i];

    }
  std::vector<PixMask* > maskpics;
  maskpics = disassemble_row(File::getTilesetFile(tileset, "misc/flags.png"),
				  MAX_STACK_SIZE, false);
  for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
    {
      if (maskpics[i]->get_width() != ts)
	PixMask::scale(maskpics[i], ts, ts);
      d_flagmask[i] = maskpics[i];
    }
}
