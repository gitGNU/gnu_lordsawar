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
#include "armysetlist.h"
#include "shieldsetlist.h"
#include "tilesetlist.h"
#include "citysetlist.h"
#include "citylist.h"
#include "army.h"
#include "playerlist.h"
#include "Configuration.h"
#include "File.h"
#include "GameMap.h"
#include "city.h"
#include "stack.h"
#include "gui/image-helpers.h"
#include "FogMap.h"
#include "hero.h"

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
    bool greyed;
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
    guint32 armyset;
    PixMask* surface;
};

//the structure to store planted standard in
struct PlantedStandardCacheItem
{
    guint32 player_id;
    guint32 armyset;
    PixMask* surface;
};

//the structure to store new level pictures in
struct NewLevelCacheItem
{
    guint32 player_id;
    guint32 gender;
    PixMask* surface;
};

//the structure to store temples in
struct TempleCacheItem
{
    guint32 cityset;
    int type;
    PixMask* surface;
};

//the structure to store ruins in
struct RuinCacheItem
{
    guint32 cityset;
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
    guint32 tileset;
    int type;
    PixMask* surface;
};

//the structure to store fog patterns in
struct FogCacheItem
{
    guint32 tileset;
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
    guint32 tileset;
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
    guint32 cityset;
    int type;
    guint32 player_id;
    PixMask* surface;
};

//the structure to store towers in
struct TowerCacheItem
{
    guint32 cityset;
    guint32 player_id;
    PixMask* surface;
};

// the structure to store flags in
struct FlagCacheItem
{
    guint32 tileset;
    guint32 size;
    guint32 player_id;
    PixMask* surface;
};

// the structure to store selector images in
struct SelectorCacheItem
{
    guint32 tileset;
    guint32 type;
    guint32 frame;
    guint32 player_id;
    PixMask* surface;
};

// the structure to store shield images in
struct ShieldCacheItem
{
    guint32 shieldset;
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
  guint32 tileset;
  guint32 cityset;
  PixMask* surface;
};

struct PortCacheItem
{
  guint32 cityset;
  PixMask *surface;
};

struct SignpostCacheItem
{
  guint32 cityset;
  PixMask *surface;
};

struct BagCacheItem
{
  guint32 armyset;
  PixMask *surface;
};

struct ExplosionCacheItem
{
  guint32 tileset;
  PixMask *surface;
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
    loadDiplomacyPics();

    loadCursorPics();
    loadProdShields();
    loadMoveBonusPics();
    loadMedalPics();
    loadNewLevelPics();

    d_smallruinedcity = getMiscPicture("smallruinedcity.png");
    d_smallhero = getMiscPicture("hero.png");
    d_bag = getMiscPicture("bag.png");
    d_smallinactivehero = getMiscPicture("hero-inactive.png");
    d_small_ruin_unexplored = getMiscPicture("smallunexploredruin.png");
    d_small_stronghold_unexplored = 
      getMiscPicture("smallunexploredstronghold.png");
    d_small_ruin_explored = getMiscPicture("smallexploredruin.png");
    d_small_temple = getMiscPicture("smalltemple.png");
}

GraphicsCache::~GraphicsCache()
{
    clear();

    for (unsigned int i = 0; i < PRODUCTION_SHIELD_TYPES; i++)
    {
        delete d_prodshieldpic[i];
    }

    for (unsigned int i = 0; i < MOVE_BONUS_TYPES; i++)
    {
        delete d_movebonuspic[i];
    }

    delete d_smallruinedcity;
    delete d_smallhero;
    delete d_bag;
    delete d_smallinactivehero;
    delete d_small_temple;
    delete d_small_ruin_unexplored;
    delete d_small_stronghold_unexplored;
    delete d_small_ruin_explored;
    delete d_newlevel_male;
    delete d_newlevel_female;
    delete d_newlevelmask_male;
    delete d_newlevelmask_female;
}

PixMask* GraphicsCache::getSmallRuinedCityPic()
{
  return d_smallruinedcity;
}

PixMask* GraphicsCache::getSmallBagPic()
{
  return d_bag;
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
        if ((*it)->player_id == p->getId() && (*it)->armyset == p->getArmyset())
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

PixMask* GraphicsCache::getNewLevelPic(const Player* p, guint32 gender)
{
    debug("getting new level pic " <<p->getName())
    std::list<NewLevelCacheItem*>::iterator it;
    NewLevelCacheItem* myitem;
    for (it = d_newlevellist.begin(); it != d_newlevellist.end(); it++)
    {
        if ((*it)->player_id == p->getId() && (*it)->gender == gender)
        {
            myitem = (*it);
            
            // put the item on the last place (==last touched)
            d_newlevellist.erase(it);
            d_newlevellist.push_back(myitem);
            
            return myitem->surface;
        }
    }
    // We are still here, so the graphic is not in the cache. 
    // addNewLevelPic calls checkPictures on its own, so we can 
    // simply return the surface
    myitem = addNewLevelPic(p, gender);

    return myitem->surface;
}

PixMask* GraphicsCache::getPortPic()
{
  return getPortPic(GameMap::getInstance()->getCityset()->getId());
}

PixMask* GraphicsCache::getPortPic(guint32 cityset)
{
    debug("getting port pic " << cityset)
    std::list<PortCacheItem*>::iterator it;
    PortCacheItem* myitem;
    for (it = d_portlist.begin(); it != d_portlist.end(); it++)
    {
        if ((*it)->cityset == cityset)
        {
            myitem = (*it);
            
            // put the item on the last place (==last touched)
            d_portlist.erase(it);
            d_portlist.push_back(myitem);
            
            return myitem->surface;
        }
    }
    // We are still here, so the graphic is not in the cache. 
    // addPortPic calls checkPictures on its own, so we can 
    // simply return the surface
    myitem = addPortPic(cityset);

    return myitem->surface;
}

PixMask* GraphicsCache::getSignpostPic()
{
  return getSignpostPic(GameMap::getInstance()->getCityset()->getId());
}

PixMask* GraphicsCache::getSignpostPic(guint32 cityset)
{
    debug("getting signpost pic " << cityset)
    std::list<SignpostCacheItem*>::iterator it;
    SignpostCacheItem* myitem;
    for (it = d_signpostlist.begin(); it != d_signpostlist.end(); it++)
    {
        if ((*it)->cityset == cityset)
        {
            myitem = (*it);
            
            // put the item on the last place (==last touched)
            d_signpostlist.erase(it);
            d_signpostlist.push_back(myitem);
            
            return myitem->surface;
        }
    }
    // We are still here, so the graphic is not in the cache. 
    // addSignpostPic calls checkPictures on its own, so we can 
    // simply return the surface
    myitem = addSignpostPic(cityset);

    return myitem->surface;
}

PixMask* GraphicsCache::getBagPic()
{
  guint32 armyset = Playerlist::getActiveplayer()->getArmyset();
  return getBagPic(armyset);
}

PixMask* GraphicsCache::getBagPic(guint32 armyset)
{
    debug("getting bag pic " << armyset)
    std::list<BagCacheItem*>::iterator it;
    BagCacheItem* myitem;
    for (it = d_baglist.begin(); it != d_baglist.end(); it++)
    {
        if ((*it)->armyset == armyset)
        {
            myitem = (*it);
            
            // put the item on the last place (==last touched)
            d_baglist.erase(it);
            d_baglist.push_back(myitem);
            
            return myitem->surface;
        }
    }
    // We are still here, so the graphic is not in the cache. 
    // addBagPic calls checkPictures on its own, so we can 
    // simply return the surface
    myitem = addBagPic(armyset);

    return myitem->surface;
}

PixMask* GraphicsCache::getExplosionPic()
{
  return getExplosionPic(GameMap::getInstance()->getTileset()->getId());
}

PixMask* GraphicsCache::getExplosionPic(guint32 tileset)
{
    debug("getting explosion pic " << tileset)
    std::list<ExplosionCacheItem*>::iterator it;
    ExplosionCacheItem* myitem;
    for (it = d_explosionlist.begin(); it != d_explosionlist.end(); it++)
    {
        if ((*it)->tileset == tileset)
        {
            myitem = (*it);
            
            // put the item on the last place (==last touched)
            d_explosionlist.erase(it);
            d_explosionlist.push_back(myitem);
            
            return myitem->surface;
        }
    }
    // We are still here, so the graphic is not in the cache. 
    // addExplosionPic calls checkPictures on its own, so we can 
    // simply return the surface
    myitem = addExplosionPic(tileset);

    return myitem->surface;
}

PixMask* GraphicsCache::getPlantedStandardPic(const Player* p)
{
    debug("getting planted standard pic " <<p->getName())
    std::list<PlantedStandardCacheItem*>::iterator it;
    PlantedStandardCacheItem* myitem;
    for (it = d_plantedstandardlist.begin(); it != d_plantedstandardlist.end(); it++)
    {
        if ((*it)->player_id == p->getId() && (*it)->armyset == p->getArmyset())
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

PixMask* GraphicsCache::getArmyPic(Army *a, bool greyed)
{
  return getArmyPic(a->getOwner()->getArmyset(), a->getTypeId(), 
		    a->getOwner(), NULL, greyed);
}

PixMask* GraphicsCache::getArmyPic(guint32 armyset, guint32 army_id, 
				   const Player* p, const bool *medals,
				   bool greyed)
{
  debug("getting army pic " <<armyset <<" " <<army <<" " <<p->getName() << 
    " "  << greyed)

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
    item.greyed = greyed;
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

    // We are still here, so the graphic is not in the cache. addArmyPic calls
    // checkPictures on its own, so we can simply return the surface
    debug("getarmypic============= " << my_medals) 
    myitem = addArmyPic(&item);

    return myitem->surface;
}

PixMask* GraphicsCache::getTilePic(int tile_style_id, int fog_type_id, bool has_bag, bool has_standard, int standard_player_id, int stack_size, int stack_player_id, int army_type_id, bool has_tower, bool has_ship, Maptile::Building building_type, int building_subtype, Vector<int> building_tile, int building_player_id, guint32 tilesize, bool has_grid)
{
  guint32 tileset = GameMap::getInstance()->getTileset()->getId();
  guint32 cityset = GameMap::getInstance()->getCityset()->getId();
  return getTilePic(tile_style_id, fog_type_id, has_bag, has_standard, standard_player_id, stack_size, stack_player_id, army_type_id, has_tower, has_ship, building_type, building_subtype, building_tile, building_player_id, tilesize, has_grid, tileset, cityset);
}

PixMask* GraphicsCache::getTilePic(int tile_style_id, int fog_type_id, bool has_bag, bool has_standard, int standard_player_id, int stack_size, int stack_player_id, int army_type_id, bool has_tower, bool has_ship, Maptile::Building building_type, int building_subtype, Vector<int> building_tile, int building_player_id, guint32 tilesize, bool has_grid, guint32 tileset, guint32 cityset)
{
    debug("getting tile pic " << tile_style_id << " " <<
	  fog_type_id << " " << has_bag << " " << has_standard << " " <<
	  standard_player_id << " " << stack_size << " " << stack_player_id <<
	  " " << army_type_id << " " << has_tower << " " << has_ship << " " << 
	  building_type << " " << building_subtype << " " << building_tile.x 
	  << "," << building_tile.y << " " << building_player_id << " " << 
	  tilesize << " " << has_grid << " " <<tileset);

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
    item.tileset = tileset;
    item.cityset = cityset;
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

PixMask* GraphicsCache::getShieldPic(guint32 shieldset, guint32 type, 
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
  guint32 shieldset = GameMap::getInstance()->getShieldset()->getId();
  return getShieldPic(shieldset, type, p->getId());
}

PixMask* GraphicsCache::getTemplePic(int type)
{
  guint32 cityset = GameMap::getInstance()->getCityset()->getId();
  return getTemplePic(type, cityset);
}
PixMask* GraphicsCache::getTemplePic(int type, guint32 cityset)
{
    debug("GraphicsCache::getTemplePic " <<type)

    std::list<TempleCacheItem*>::iterator it;
    TempleCacheItem* myitem;

    for (it = d_templelist.begin(); it != d_templelist.end(); it++)
    {
        if ((*it)->type == type && (*it)->cityset == cityset)
        {
            myitem = (*it);

            //put the item in last place (last touched)
            d_templelist.erase(it);
            d_templelist.push_back(myitem);

            return myitem->surface;
        }
    }

    //no item found -> create a new one
    myitem = addTemplePic(type, cityset);

    return myitem->surface;
}

PixMask* GraphicsCache::getRuinPic(int type)
{
  guint32 cityset = GameMap::getInstance()->getCityset()->getId();
  return getRuinPic(type, cityset);
}
PixMask* GraphicsCache::getRuinPic(int type, guint32 cityset)
{
    debug("GraphicsCache::getRuinPic " <<type)

    std::list<RuinCacheItem*>::iterator it;
    RuinCacheItem* myitem;

    for (it = d_ruinlist.begin(); it != d_ruinlist.end(); it++)
    {
        if ((*it)->type == type && (*it)->cityset == cityset)
        {
            myitem = (*it);

            //put the item in last place (last touched)
            d_ruinlist.erase(it);
            d_ruinlist.push_back(myitem);

            return myitem->surface;
        }
    }

    //no item found -> create a new one
    myitem = addRuinPic(type, cityset);

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
  return getRoadPic(type, GameMap::getInstance()->getTileset()->getId());
}
PixMask* GraphicsCache::getRoadPic(int type, guint32 tileset)
{
    debug("GraphicsCache::getRoadPic " <<type)

    std::list<RoadCacheItem*>::iterator it;
    RoadCacheItem* myitem;

    for (it = d_roadlist.begin(); it != d_roadlist.end(); it++)
    {
        if ((*it)->type == type && (*it)->tileset == tileset)
        {
            myitem = (*it);

            //put the item in last place (last touched)
            d_roadlist.erase(it);
            d_roadlist.push_back(myitem);

            return myitem->surface;
        }
    }

    //no item found -> create a new one
    myitem = addRoadPic(type, tileset);

    return myitem->surface;
}

PixMask* GraphicsCache::getFogPic(int type)
{
  return getFogPic(type, GameMap::getInstance()->getTileset()->getId());
}

PixMask* GraphicsCache::getFogPic(int type, guint32 tileset)
{
    debug("GraphicsCache::getFogPic " <<type)

    std::list<FogCacheItem*>::iterator it;
    FogCacheItem* myitem;

    FogCacheItem item = FogCacheItem();
    item.type = type;
    item.tileset = tileset;
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
  return getBridgePic(type, GameMap::getInstance()->getTileset()->getId());
}
PixMask* GraphicsCache::getBridgePic(int type, guint32 tileset)
{
    debug("GraphicsCache::getBridgePic " <<type)

    std::list<BridgeCacheItem*>::iterator it;
    BridgeCacheItem* myitem;

    for (it = d_bridgelist.begin(); it != d_bridgelist.end(); it++)
    {
        if ((*it)->type == type && (*it)->tileset == tileset)
        {
            myitem = (*it);

            //put the item in last place (last touched)
            d_bridgelist.erase(it);
            d_bridgelist.push_back(myitem);

            return myitem->surface;
        }
    }

    //no item found -> create a new one
    myitem = addBridgePic(type, tileset);

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

PixMask* GraphicsCache::getCityPic(const City* city, guint32 cityset)
{
  if (!city)
    return NULL;
  int type;
  if (city->isBurnt() == true)
    type = -1;
  else
    type = 0;
  return getCityPic(type, city->getOwner(), cityset);
}

PixMask* GraphicsCache::getCityPic(const City* city)
{
  guint32 cityset = GameMap::getInstance()->getCityset()->getId();
  return getCityPic(city, cityset);
}

PixMask* GraphicsCache::getCityPic(int type, const Player* p, guint32 cityset)
{
    debug("GraphicsCache::getCityPic " <<type)

    std::list<CityCacheItem*>::iterator it;
    CityCacheItem* myitem;

    for (it = d_citylist.begin(); it != d_citylist.end(); it++)
    {
        if ((*it)->type == type && (*it)->cityset == cityset &&
	    (*it)->player_id == p->getId())
        {
            myitem = (*it);

            //put the item in last place (last touched)
            d_citylist.erase(it);
            d_citylist.push_back(myitem);

            return myitem->surface;
        }
    }

    //no item found -> create a new one
    myitem = addCityPic(type, p, cityset);

    return myitem->surface;
}

PixMask* GraphicsCache::getTowerPic(const Player* p)
{
  guint32 cityset = GameMap::getInstance()->getCityset()->getId();
  return getTowerPic(p, cityset);
}

PixMask* GraphicsCache::getTowerPic(const Player* p, guint32 cityset)
{
    debug("GraphicsCache::getTowerPic player " <<p->getName())

    std::list<TowerCacheItem*>::iterator it;
    TowerCacheItem* myitem;

    for (it = d_towerlist.begin(); it != d_towerlist.end(); it++)
    {
        if ((*it)->cityset == cityset && (*it)->player_id == p->getId())
        {
            myitem = (*it);

            //put the item in last place (last touched)
            d_towerlist.erase(it);
            d_towerlist.push_back(myitem);

            return myitem->surface;
        }
    }

    //no item found -> create a new one
    myitem = addTowerPic(p, cityset);

    return myitem->surface;
}

PixMask* GraphicsCache::getFlagPic(guint32 stack_size, const Player *p, guint32 tileset)
{
  if (stack_size > MAX_STACK_SIZE || p == NULL || tileset == 0)
    return NULL;
    debug("GraphicsCache::getFlagPic " <<stack_size <<", player" <<p->getId())

    std::list<FlagCacheItem*>::iterator it;
    FlagCacheItem* myitem;

    for (it = d_flaglist.begin(); it != d_flaglist.end(); it++)
    {
        myitem = *it;
        if (myitem->size == stack_size && myitem->player_id == p->getId() &&
	    myitem->tileset == tileset)
        {
            // put the item in last place (last touched)
            d_flaglist.erase(it);
            d_flaglist.push_back(myitem);

            return myitem->surface;
        }
    }

    // no item found => create a new one
    myitem = addFlagPic(stack_size, p, tileset);

    return myitem->surface;
}
PixMask* GraphicsCache::getFlagPic(guint32 stack_size, const Player *p)
{
  return getFlagPic(stack_size, p, 
		    GameMap::getInstance()->getTileset()->getId());
}

PixMask* GraphicsCache::getFlagPic(const Stack* s)
{
  return getFlagPic(s, GameMap::getInstance()->getTileset()->getId());
}
PixMask* GraphicsCache::getFlagPic(const Stack* s, guint32 tileset)
{
    if (!s)
    {
        std::cerr << "GraphicsCache::getFlagPic: no stack supplied! Exiting...\n";
        exit(-1);
    }
    
  return getFlagPic(s->size(), s->getOwner(), tileset);
}

PixMask* GraphicsCache::getSelectorPic(guint32 type, guint32 frame, 
					   const Player *p)
{
  return getSelectorPic(type, frame, p, 
			GameMap::getInstance()->getTileset()->getId());
}
PixMask* GraphicsCache::getSelectorPic(guint32 type, guint32 frame, 
					   const Player *p, guint32 tileset)
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
	    && myitem->frame == frame && myitem->tileset == tileset)
        {
            // put the item in last place (last touched)
            d_selectorlist.erase(it);
            d_selectorlist.push_back(myitem);

            return myitem->surface;
        }
    }

    // no item found => create a new one
    myitem = addSelectorPic(type, frame, p, tileset);

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


PixMask* GraphicsCache::applyMask(PixMask* image, PixMask* mask, Gdk::Color colour, bool isNeutral)
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
	    copy[base+0] = colour.get_red_p() *copy[base+0];
	    copy[base+1] = colour.get_green_p() * copy[base+1];
	    copy[base+2] = colour.get_blue_p() * copy[base+2];
	  }
      }
  Glib::RefPtr<Gdk::Pixbuf> colouredmask = 
    Gdk::Pixbuf::create_from_data(copy, Gdk::COLORSPACE_RGB, true, 8, 
				  width, height, width * 4);
  result->draw_pixbuf(colouredmask, 0, 0, 0, 0, width, height);
  free(copy);

  return result;
}

PixMask* GraphicsCache::greyOut(PixMask* image)
{
  int width = image->get_width();
  int height = image->get_height();
  PixMask* result = PixMask::create(image->to_pixbuf());
  
  guint8 *data = result->to_pixbuf()->get_pixels();
  guint8 *copy = (guint8*)  malloc (height * width * 4 * sizeof(guint8));
  memcpy(copy, data, height * width * 4 * sizeof(guint8));
  for (int i = 0; i < width; i++)
    for (int j = 0; j < height; j++)
      {
	const int base = (j * 4) + (i * height * 4);

	if (data[base+3] != 0)
	  {
	    guint32 max = 0;
	    if (copy[base+0] > max)
	      max = copy[base+0];
	    else if (copy[base+1] > max)
	      max = copy[base+1];
	    else if (copy[base+2] > max)
	      max = copy[base+2];
	    int x =  i % 2;
	    int y = j % 2;
	    if ((x == 0 && y == 0) || (x == 1 && y == 1))
	      max = 88;
	    copy[base+0] = max;
	    copy[base+1] = max;
	    copy[base+2] = max;
	  }
      }
  Glib::RefPtr<Gdk::Pixbuf> greyed_out =  
    Gdk::Pixbuf::create_from_data(copy, Gdk::COLORSPACE_RGB, true, 8, 
				  width, height, width * 4);

  result->draw_pixbuf(greyed_out, 0, 0, 0, 0, width, height);
  free(copy);

  return result;
}

PixMask* GraphicsCache::applyMask(PixMask* image, PixMask* mask, const Player* p)
{
  return applyMask(image, mask, p->getColor(),
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

  while (d_newlevellist.size() > 10)
    eraseLastNewLevelItem();

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

  // next, kill port pics
  while (d_portlist.size() > 1)
    eraseLastPortItem();

  if (d_cachesize < maxcache)
    return;

  // next, kill signpost pics
  while (d_signpostlist.size() > 1)
    eraseLastSignpostItem();

  if (d_cachesize < maxcache)
    return;

  // next, kill bag pics
  while (d_baglist.size() > 1)
    eraseLastBagItem();

  if (d_cachesize < maxcache)
    return;

  // next, kill explosion pics
  while (d_explosionlist.size() > 1)
    eraseLastExplosionItem();

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

void GraphicsCache::drawTilePic(PixMask *surface, int fog_type_id, bool has_bag, bool has_standard, int standard_player_id, int stack_size, int stack_player_id, int army_type_id, bool has_tower, bool has_ship, Maptile::Building building_type, int building_subtype, Vector<int> building_tile, int building_player_id, guint32 ts, bool has_grid, guint32 tileset, guint32 cityset)
{
  const Player *player;
  Glib::RefPtr<Gdk::Pixmap> pixmap = surface->get_pixmap();

  switch (building_type)
    {
    case Maptile::CITY:
	{
	  player = Playerlist::getInstance()->getPlayer(building_player_id);
	  getCityPic(building_subtype, player, cityset)->blit(building_tile, ts, pixmap);
	}
      break;
    case Maptile::RUIN:
      getRuinPic(building_subtype, cityset)->blit(building_tile, ts, pixmap); break;
    case Maptile::TEMPLE:
      getTemplePic(building_subtype, cityset)->blit(building_tile, ts, pixmap); break;
    case Maptile::SIGNPOST:
      getSignpostPic(cityset)->blit(building_tile, ts, pixmap); break;
    case Maptile::ROAD:
      getRoadPic(building_subtype)->blit(building_tile, ts, pixmap); break;
    case Maptile::PORT:
      getPortPic(cityset)->blit(building_tile, ts, pixmap); break;
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
    {
      PixMask *pic = getBagPic();
      Vector<int>bagsize = Vector<int>(pic->get_width(), pic->get_height());
      pic->blit(pixmap, Vector<int>(ts,ts)-bagsize);
    }

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
    {
      Tileset *t= 
	Tilesetlist::getInstance()->getTileset(tileset);
      t->getFogImage(fog_type_id - 1)->blit(pixmap);
    }
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
	" " << item->tilesize << " " << item->has_grid << " " << item->tileset
	" " << item->cityset);

  TileCacheItem* myitem = new TileCacheItem();
  *myitem = *item;

  //short circuit the drawing sequence if the tile is completely obscured
  Tileset *t = Tilesetlist::getInstance()->getTileset(item->tileset);
  if (myitem->fog_type_id == FogMap::ALL)
    myitem->surface = t->getFogImage(myitem->fog_type_id - 1)->copy();
  else
    {
      myitem->surface = 
	t->getTileStyle(myitem->tile_style_id)->getImage()->copy();
    
      drawTilePic(myitem->surface, myitem->fog_type_id, myitem->has_bag, 
		  myitem->has_standard, myitem->standard_player_id, 
		  myitem->stack_size, myitem->stack_player_id, 
		  myitem->army_type_id, myitem->has_tower, myitem->has_ship, 
		  myitem->building_type, myitem->building_subtype, 
		  myitem->building_tile, myitem->building_player_id, 
		  myitem->tilesize, myitem->has_grid, myitem->tileset,
		  myitem->cityset);
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
  Shield::Colour c = Shield::Colour(p->getId());
  PixMask *coloured = applyMask(basearmy->getImage(c), basearmy->getMask(c), p);
  if (myitem->greyed)
    {
      PixMask *greyed_out = greyOut(coloured);
      myitem->surface = greyed_out;
      delete coloured;
    }
  else
      myitem->surface = coloured;

  if (myitem->medals != NULL)
    {
      debug("medalsbonus============= " << medalsbonus); 
      for(int i=0;i<3;i++)
	{ 
	  if (myitem->medals[i])
	    d_medalpic[0][i]->blit(myitem->surface->get_pixmap());
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

ShieldCacheItem* GraphicsCache::addShieldPic(guint32 shieldset, guint32 type, 
					     guint32 colour)
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
  myitem->armyset = p->getArmyset();

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

NewLevelCacheItem* GraphicsCache::addNewLevelPic(const Player* p, guint32 gender)
{
  debug("ADD new level pic: " <<p->getName())

  NewLevelCacheItem* myitem = new NewLevelCacheItem();
  myitem->player_id = p->getId();
  myitem->gender = gender;

  // copy the pixmap including player colors
  if (gender == Hero::FEMALE)
    myitem->surface = applyMask(d_newlevel_female, d_newlevelmask_female, p);
  else
    myitem->surface = applyMask(d_newlevel_male, d_newlevelmask_male, p);

  //now the final preparation steps:
  //a) add the size
  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize += myitem->surface->get_depth()/8 * size;

  //b) add the entry to the list
  d_newlevellist.push_back(myitem);

  //c) check if the cache size is too large
  checkPictures();

  //we are finished, so return the pic
  return myitem;
}

PortCacheItem* GraphicsCache::addPortPic(guint32 cityset)
{
  debug("ADD port pic: " << cityset);

  PortCacheItem* myitem = new PortCacheItem();
  myitem->cityset = cityset;

  Citysetlist *csl = Citysetlist::getInstance();
  Cityset *cs = csl->getCityset(cityset);

  // copy the pixmap
  myitem->surface = cs->getPortImage()->copy();

  //now the final preparation steps:
  //a) add the size
  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize += myitem->surface->get_depth()/8 * size;

  //b) add the entry to the list
  d_portlist.push_back(myitem);

  //c) check if the cache size is too large
  checkPictures();

  //we are finished, so return the pic
  return myitem;
}

ExplosionCacheItem* GraphicsCache::addExplosionPic(guint32 tileset)
{
  debug("ADD explosion pic: " << tileset);

  ExplosionCacheItem* myitem = new ExplosionCacheItem();
  myitem->tileset = tileset;

  // copy the pixmap
  myitem->surface = 
    Tilesetlist::getInstance()->getTileset(tileset)->getExplosionImage()->copy();

  //now the final preparation steps:
  //a) add the size
  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize += myitem->surface->get_depth()/8 * size;

  //b) add the entry to the list
  d_explosionlist.push_back(myitem);

  //c) check if the cache size is too large
  checkPictures();

  //we are finished, so return the pic
  return myitem;
}

BagCacheItem* GraphicsCache::addBagPic(guint32 armyset)
{
  debug("ADD bad pic: " << armyset);

  BagCacheItem* myitem = new BagCacheItem();
  myitem->armyset = armyset;

  // copy the pixmap
  myitem->surface = Armysetlist::getInstance()->getBagPic(armyset)->copy();

  //now the final preparation steps:
  //a) add the size
  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize += myitem->surface->get_depth()/8 * size;

  //b) add the entry to the list
  d_baglist.push_back(myitem);

  //c) check if the cache size is too large
  checkPictures();

  //we are finished, so return the pic
  return myitem;
}

SignpostCacheItem* GraphicsCache::addSignpostPic(guint32 cityset)
{
  debug("ADD signpost pic: " << cityset);

  SignpostCacheItem* myitem = new SignpostCacheItem();
  myitem->cityset = cityset;

  Citysetlist *csl = Citysetlist::getInstance();
  Cityset *cs = csl->getCityset(cityset);

  // copy the pixmap
  myitem->surface = cs->getSignpostImage()->copy();

  //now the final preparation steps:
  //a) add the size
  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize += myitem->surface->get_depth()/8 * size;

  //b) add the entry to the list
  d_signpostlist.push_back(myitem);

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
  myitem->armyset = p->getArmyset();

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


TempleCacheItem* GraphicsCache::addTemplePic(int type, guint32 cityset)
{
  Cityset *cs = Citysetlist::getInstance()->getCityset(cityset);
  PixMask* mysurf = cs->getTempleImage(type)->copy();

  //now create the cache item and add the size
  TempleCacheItem* myitem = new TempleCacheItem();
  myitem->type = type;
  myitem->cityset = cityset;
  myitem->surface = mysurf;

  d_templelist.push_back(myitem);

  //add the size
  int size = mysurf->get_width() * mysurf->get_height();
  d_cachesize += size * mysurf->get_depth()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

RuinCacheItem* GraphicsCache::addRuinPic(int type, guint32 cityset)
{
  Cityset *cs = Citysetlist::getInstance()->getCityset(cityset);
  PixMask* mysurf = cs->getRuinImage(type)->copy();

  //now create the cache item and add the size
  RuinCacheItem* myitem = new RuinCacheItem();
  myitem->type = type;
  myitem->cityset = cityset;
  myitem->surface = mysurf;
  myitem->cityset = cityset;

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

RoadCacheItem* GraphicsCache::addRoadPic(int type, guint32 tileset)
{
  Tileset *ts = Tilesetlist::getInstance()->getTileset(tileset);
  PixMask* mysurf = ts->getRoadImage(type)->copy();

  //now create the cache item and add the size
  RoadCacheItem* myitem = new RoadCacheItem();
  myitem->type = type;
  myitem->tileset = tileset;
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
  Tileset *ts = Tilesetlist::getInstance()->getTileset(item->tileset);
  PixMask* mysurf = ts->getFogImage(item->type - 1)->copy();

  //now create the cache item and add the size
  FogCacheItem* myitem = new FogCacheItem();
  *myitem = *item;
  myitem->surface = mysurf;

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

BridgeCacheItem* GraphicsCache::addBridgePic(int type, guint32 tileset)
{
  Tileset *ts = Tilesetlist::getInstance()->getTileset(tileset);
  PixMask* mysurf = ts->getBridgeImage(type)->copy();

  //now create the cache item and add the size
  BridgeCacheItem* myitem = new BridgeCacheItem();
  myitem->type = type;
  myitem->tileset = tileset;
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

CityCacheItem* GraphicsCache::addCityPic(int type, const Player* p, guint32 cityset)
{
  //now create the cache item and add the size
  CityCacheItem* myitem = new CityCacheItem();
  myitem->cityset = cityset;
  myitem->player_id = p->getId();
  myitem->type = type;
  Cityset *cs = Citysetlist::getInstance()->getCityset(cityset);
  if (type == -1)
    myitem->surface = cs->getRazedCityImage(p->getId())->copy();
  else
    myitem->surface = cs->getCityImage(p->getId())->copy();

  d_citylist.push_back(myitem);

  //add the size
  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize += size * myitem->surface->get_depth()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

TowerCacheItem* GraphicsCache::addTowerPic(const Player* p, guint32 cityset)
{
  //now create the cache item and add the size
  TowerCacheItem* myitem = new TowerCacheItem();
  myitem->player_id = p->getId();
  myitem->cityset = cityset;
  Cityset *cs = Citysetlist::getInstance()->getCityset(cityset);
  myitem->surface = cs->getTowerImage(p->getId());

  d_towerlist.push_back(myitem);

  //add the size
  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize += size * myitem->surface->get_depth()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

FlagCacheItem* GraphicsCache::addFlagPic(int size, const Player *p, guint32 tileset)
{
  Tileset *ts = Tilesetlist::getInstance()->getTileset(tileset);
  debug("GraphicsCache::addFlagPic, player="<<p->getId()<<", size="<<size)

  // size of the stack starts at 1, but we need the index, which starts at 0

  PixMask* mysurf = applyMask (ts->getFlagImage(size-1), 
			       ts->getFlagMask(size-1), p);

  //now create the cache item and add the size
  FlagCacheItem* myitem = new FlagCacheItem();
  myitem->player_id = p->getId();
  myitem->size = size;
  myitem->tileset = tileset;
  myitem->surface = mysurf;

  d_flaglist.push_back(myitem);

  //add the size
  int picsize = mysurf->get_width() * mysurf->get_height();
  d_cachesize += picsize * mysurf->get_depth()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

SelectorCacheItem* GraphicsCache::addSelectorPic(guint32 type, guint32 frame, const Player* p, guint32 tileset)
{
  Tileset *ts = Tilesetlist::getInstance()->getTileset(tileset);
  debug("GraphicsCache::addSelectorPic, player="<<p->getName()<<", type="<<type<< ", " << frame)

    // frame is the frame of animation we're looking for.  starts at 0.
    // type is 0 for big, 1 for small

  PixMask* mysurf;
  if (type == 0)
    mysurf = applyMask(ts->getSelectorImage(frame), ts->getSelectorMask(frame), p);
  else
    mysurf = applyMask(ts->getSmallSelectorImage(frame), 
		       ts->getSmallSelectorMask(frame), p);

  //now create the cache item and add the size
  SelectorCacheItem* myitem = new SelectorCacheItem();
  myitem->player_id = p->getId();
  myitem->type = type;
  myitem->tileset = tileset;
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

  while (!d_portlist.empty())
    eraseLastPortItem();

  while (!d_signpostlist.empty())
    eraseLastSignpostItem();

  while (!d_baglist.empty())
    eraseLastBagItem();

  while (!d_explosionlist.empty())
    eraseLastExplosionItem();

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

void GraphicsCache::eraseLastNewLevelItem()
{
  if (d_newlevellist.empty())
    return;

  NewLevelCacheItem* myitem = *(d_newlevellist.begin());
  d_newlevellist.erase(d_newlevellist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_depth()/8 * size;

  delete myitem->surface;
  delete myitem;
}

void GraphicsCache::eraseLastExplosionItem()
{
  if (d_explosionlist.empty())
    return;

  ExplosionCacheItem* myitem = *(d_explosionlist.begin());
  d_explosionlist.erase(d_explosionlist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_depth()/8 * size;

  delete myitem->surface;
  delete myitem;
}

void GraphicsCache::eraseLastBagItem()
{
  if (d_baglist.empty())
    return;

  BagCacheItem* myitem = *(d_baglist.begin());
  d_baglist.erase(d_baglist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_depth()/8 * size;

  delete myitem->surface;
  delete myitem;
}

void GraphicsCache::eraseLastSignpostItem()
{
  if (d_signpostlist.empty())
    return;

  SignpostCacheItem* myitem = *(d_signpostlist.begin());
  d_signpostlist.erase(d_signpostlist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_depth()/8 * size;

  delete myitem->surface;
  delete myitem;
}

void GraphicsCache::eraseLastPortItem()
{
  if (d_portlist.empty())
    return;

  PortCacheItem* myitem = *(d_portlist.begin());
  d_portlist.erase(d_portlist.begin());

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

bool GraphicsCache::loadSelectorImages(std::string filename, guint32 size, std::vector<PixMask* > &images, std::vector<PixMask* > &masks)
{
  int num_frames;
  num_frames = Gdk::Pixbuf::create_from_file (filename)->get_width() / size;
  images = disassemble_row(filename, num_frames, true);
  for (int i = 0; i < num_frames; i++)
    {
      if (images[i]->get_width() != (int)size)
	PixMask::scale(images[i], size, size);
    }

  masks = disassemble_row(filename, num_frames, false);
  for (int i = 0; i < num_frames; i++)
    {
      if (masks[i]->get_width() != (int)size)
	PixMask::scale(masks[i], size, size);
    }

  return true;
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
      d_medalpic[0][i] = medalpics[i];
    }
  medalpics = disassemble_row(File::getMiscFile("various/bigmedals.png"),
			      MEDAL_TYPES);
  for (unsigned int i = 0; i < MEDAL_TYPES; i++)
    d_medalpic[1][i] = medalpics[i];
}

void GraphicsCache::loadNewLevelPics()
{
  std::vector<PixMask* > half;
  half = disassemble_row(File::getMiscFile("various/hero-newlevel-male.png"), 
			 2);
  d_newlevel_male = half[0];
  d_newlevelmask_male = half[1];
  half = disassemble_row(File::getMiscFile("various/hero-newlevel-female.png"), 
			 2);
  d_newlevel_female = half[0];
  d_newlevelmask_female = half[1];
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

bool GraphicsCache::loadFlagImages(std::string filename, guint32 size, std::vector<PixMask* > &images, std::vector<PixMask* > &masks)
{
  images = disassemble_row(filename, FLAG_TYPES, true);
  for (unsigned int i = 0; i < FLAG_TYPES; i++)
    {
      if (images[i]->get_width() != (int)size)
	PixMask::scale(images[i], size, size);

    }
  masks = disassemble_row(filename, FLAG_TYPES, false);
  for (unsigned int i = 0; i < FLAG_TYPES; i++)
    {
      if (masks[i]->get_width() !=(int) size)
	PixMask::scale(masks[i], size, size);
    }
  return true;
}

PixMask* GraphicsCache::getMedalPic(bool large, int type)
{
  if (large)
    return d_medalpic[1][type];
  else
    return d_medalpic[0][type];
}

PixMask* GraphicsCache::loadImage(std::string filename, bool alpha)
{
  return PixMask::create(filename);
}

PixMask* GraphicsCache::getMiscPicture(std::string picname, bool alpha)
{
  return loadImage(File::getMiscFile("/various/" + picname), alpha);
}

