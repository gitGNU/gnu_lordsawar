// Copyright (C) 2003, 2004, 2005, 2006, 2007 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009 Ben Asselstine
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

#include <assert.h>
#include <string.h>

#include "rectangle.h"

#include "GraphicsCache.h"
#include "GraphicsLoader.h"
#include "armysetlist.h"
#include "shieldsetlist.h"
#include "army.h"
#include "playerlist.h"
#include "Configuration.h"
#include "File.h"
#include "GameMap.h"
#include "city.h"
#include "stack.h"
#include "rgb_shift.h"
#include "gui/image-helpers.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

//some structures first

//this structure is the base class for storing cached objects. It stores the
//(army-, but can be extended) set, the type of army and the player which the
//surface is designed for (and the surface itself, of course).
struct ArmyCacheItem
{
    guint32 armyset;
    guint32 index;
    guint32 player_id;
    bool medals[3];
    Glib::RefPtr<Gdk::Pixbuf> surface;
};

//the structure to store ships in
struct ShipCacheItem
{
    guint32 player_id;
    Glib::RefPtr<Gdk::Pixbuf> surface;
};

//the structure to store planted standard in
struct PlantedStandardCacheItem
{
    guint32 player_id;
    Glib::RefPtr<Gdk::Pixbuf> surface;
};

//the structure to store temples in
struct TempleCacheItem
{
    int type;
    Glib::RefPtr<Gdk::Pixbuf> surface;
};

//the structure to store ruins in
struct RuinCacheItem
{
    int type;
    Glib::RefPtr<Gdk::Pixbuf> surface;
};

//the structure to store diplomacy icons in
struct DiplomacyCacheItem
{
    int type;
    Player::DiplomaticState state;
    Glib::RefPtr<Gdk::Pixbuf> surface;
};

//the structure to store roads in
struct RoadCacheItem
{
    int type;
    Glib::RefPtr<Gdk::Pixbuf> surface;
};

//the structure to store fog patterns in
struct FogCacheItem
{
    int type;
    Glib::RefPtr<Gdk::Pixbuf> surface;
};

//the structure to store bridges in
struct BridgeCacheItem
{
    int type;
    Glib::RefPtr<Gdk::Pixbuf> surface;
};

//the structure to store cursors in
struct CursorCacheItem
{
    int type;
    Glib::RefPtr<Gdk::Pixbuf> surface;
};

//the structure to store buildings in
struct CityCacheItem
{
    int type;
    guint32 player_id;
    Glib::RefPtr<Gdk::Pixbuf> surface;
};

//the structure to store towers in
struct TowerCacheItem
{
    guint32 player_id;
    Glib::RefPtr<Gdk::Pixbuf> surface;
};

// the structure to store flags in
struct FlagCacheItem
{
    guint32 size;
    guint32 player_id;
    Glib::RefPtr<Gdk::Pixbuf> surface;
};

// the structure to store selector images in
struct SelectorCacheItem
{
    guint32 type;
    guint32 frame;
    guint32 player_id;
    Glib::RefPtr<Gdk::Pixbuf> surface;
};

// the structure to store shield images in
struct ShieldCacheItem
{
    std::string shieldset;
    guint32 type;
    guint32 colour;
    Glib::RefPtr<Gdk::Pixbuf> surface;
};

// the structure to store production shield images in
struct ProdShieldCacheItem
{
    guint32 type;
    bool prod;
    Glib::RefPtr<Gdk::Pixbuf> surface;
};

// the structure to store movement bonus images in
struct MoveBonusCacheItem
{
    guint32 type; // 0=empty, 1=trees, 2=foothills, 3=hills+trees, 4=fly, 5=boat
    Glib::RefPtr<Gdk::Pixbuf> surface;
};

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
        d_selector[i].clear();
        d_selectormask[i].clear();
      }

    for (unsigned int i = 0; i < d_smallselector.size(); i++)
      {
        d_smallselector[i].clear();
        d_smallselectormask[i].clear();
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
    d_explosion = GraphicsLoader::getTilesetPicture(tileset, "misc/explosion.png");
    d_signpost = GraphicsLoader::getCitysetPicture(cityset, "signpost.png");
}

GraphicsCache::~GraphicsCache()
{
    clear();

    for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
    {
        if (d_citypic[i])
	  d_citypic[i].clear();

        if (d_razedpic[i])
	  d_razedpic[i].clear();
    }

    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
        if (d_towerpic[i])
	  d_towerpic[i].clear();
    }

    for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
    {
        d_flagpic[i].clear();
        d_flagmask[i].clear();
    }

    for (unsigned int i = 0; i < d_selector.size(); i++)
    {
        d_selector[i].clear();
        d_selectormask[i].clear();
    }

    for (unsigned int i = 0; i < d_smallselector.size(); i++)
    {
        d_smallselector[i].clear();
        d_smallselectormask[i].clear();
    }

    for (unsigned int i = 0; i < PRODUCTION_SHIELD_TYPES; i++)
    {
        d_prodshieldpic[i].clear();
    }

    for (unsigned int i = 0; i < MOVE_BONUS_TYPES; i++)
    {
        d_movebonuspic[i].clear();
    }

    for (unsigned int i = 0; i < FOG_TYPES; i++)
    {
        d_fogpic[i].clear();
    }

    d_smallruinedcity.clear();
    d_smallhero.clear();
    d_smallinactivehero.clear();
    d_small_temple.clear();
    d_small_ruin_unexplored.clear();
    d_small_stronghold_unexplored.clear();
    d_small_ruin_explored.clear();
    d_port.clear();
    d_explosion.clear();
    d_signpost.clear();
}

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getSmallRuinedCityPic()
{
  return d_smallruinedcity;
}

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getSmallHeroPic(bool active)
{
  if (active)
    return d_smallhero;
  else
    return d_smallinactivehero;
}

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getSmallRuinExploredPic()
{
  return d_small_ruin_explored;
}
Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getSmallRuinUnexploredPic()
{
  return d_small_ruin_unexplored;
}
Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getSmallStrongholdUnexploredPic()
{
  return d_small_stronghold_unexplored;
}
Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getSmallTemplePic()
{
  return d_small_temple;
}

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getPortPic()
{
  return d_port;
}

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getExplosionPic()
{
  return d_explosion;
}

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getSignpostPic()
{
  return d_signpost;
}

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getMoveBonusPic(guint32 bonus, bool has_ship)
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

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getShipPic(const Player* p)
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

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getPlantedStandardPic(const Player* p)
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

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getArmyPic(Army *a)
{
  return getArmyPic(a->getOwner()->getArmyset(), a->getTypeId(), 
		    a->getOwner(), NULL);
}

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getArmyPic(guint32 armyset, guint32 army, const Player* p,
                                       const bool *medals)
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

    // We are still here, so the graphic is not in the cache. addArmyPic calls
    // checkPictures on its own, so we can simply return the surface
    debug("getarmypic============= " << my_medals) 
    myitem = addArmyPic(armyset, army, p, my_medals);

    return myitem->surface;
}

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getShieldPic(std::string shieldset, 
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
        
Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getShieldPic(guint32 type, Player *p)
{
  std::string shieldset = GameMap::getInstance()->getShieldset()->getSubDir();
  return getShieldPic(shieldset, type, p->getId());
}

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getTemplePic(int type)
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

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getRuinPic(int type)
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

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getDiplomacyPic(int type, Player::DiplomaticState state)
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

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getRoadPic(int type)
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

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getFogPic(int type)
{
    debug("GraphicsCache::getFogPic " <<type)

    std::list<FogCacheItem*>::iterator it;
    FogCacheItem* myitem;

    for (it = d_foglist.begin(); it != d_foglist.end(); it++)
    {
        if ((*it)->type == type)
        {
            myitem = (*it);

            //put the item in last place (last touched)
            d_foglist.erase(it);
            d_foglist.push_back(myitem);

            return myitem->surface;
        }
    }

    //no item found -> create a new one
    myitem = addFogPic(type);

    return myitem->surface;
}

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getBridgePic(int type)
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

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getCursorPic(int type)
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

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getCityPic(const City* city)
{
    if (!city)
        return Glib::RefPtr<Gdk::Pixbuf>(0);
    if (city->isBurnt())
      return d_razedpic[city->getOwner()->getId()];
    else
      return d_citypic[city->getOwner()->getId()];
}

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getCityPic(int type, const Player* p)
{
    debug("GraphicsCache::getCityPic " <<type <<", player " <<p->getName())

    if (type == -1)
      return d_razedpic[p->getId()];
    else
      return d_citypic[p->getId()];
}

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getTowerPic(const Player* p)
{
    debug("GraphicsCache::getTowerPic player " <<p->getName())
    return d_towerpic[p->getId()];
}

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getFlagPic(const Stack* s)
{
    debug("GraphicsCache::getFlagPic " <<s->getId() <<", player" <<s->getOwner()->getName())

    if (!s)
    {
        std::cerr << "GraphicsCache::getFlagPic: no stack supplied! Exiting...\n";
        exit(-1);
    }
    
    std::list<FlagCacheItem*>::iterator it;
    FlagCacheItem* myitem;

    for (it = d_flaglist.begin(); it != d_flaglist.end(); it++)
    {
        myitem = *it;
        if (myitem->size == s->size() - 1 && myitem->player_id == s->getOwner()->getId())
        {
            // put the item in last place (last touched)
            d_flaglist.erase(it);
            d_flaglist.push_back(myitem);

            return myitem->surface;
        }
    }

    // no item found => create a new one
    myitem = addFlagPic(s->size(), s->getOwner());

    return myitem->surface;
}

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getSelectorPic(guint32 type, guint32 frame, 
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


Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::getProdShieldPic(guint32 type, bool prod)
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


Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::applyMask(Glib::RefPtr<Gdk::Pixbuf> image, Glib::RefPtr<Gdk::Pixbuf> mask, struct rgb_shift shifts, bool isNeutral)
{
  Glib::RefPtr<Gdk::Pixbuf> result;
  result = image->copy();
  int width = image->get_width();
  int height = image->get_height();
  if (mask->get_width() != width || mask->get_height() != height)
    {
      std::cerr <<"Warning: mask and original image do not match\n";
      return Glib::RefPtr<Gdk::Pixbuf>(0);
    }
  if (isNeutral)
    return image->copy();
  
  guint8 *data = mask->get_pixels();
  guint8 *copy = (guint8*)  malloc (height * width * 4 * sizeof(guint8));
  memcpy(copy, data, height * width * 4 * sizeof(guint8));
  for (int i = 0; i < width; i++)
    for (int j = 0; j < height; j++)
      {

	const int base = (j * 4) + (i * height * 4);

	if (data[base+3] != 0)
	  {
	    copy[base+0] >>= (shifts.r);
	    copy[base+1] >>= (shifts.g);
	    copy[base+2] >>= (shifts.b);
	  }
      }
  Glib::RefPtr<Gdk::Pixbuf> colouredmask = Gdk::Pixbuf::create_from_data(copy, Gdk::COLORSPACE_RGB, true, 8, width, height, width * 4);
  free (copy);
      
  //result = Gdk::Pixbuf::create(Glib::RefPtr<Gdk::Drawable>(colouredmask), 0, 0, width, height);
  colouredmask->composite(result, 0, 0, width, height, 0, 0, 1, 1, 
			  Gdk::INTERP_NEAREST, 255);
  //image->save("/tmp/image.png", "png");
  //mask->save("/tmp/mask.png", "png");
  //colouredmask->save("/tmp/colouredmask.png", "png");
  //result->save("/tmp/result.png", "png");

  return result;
}

Glib::RefPtr<Gdk::Pixbuf> GraphicsCache::applyMask(Glib::RefPtr<Gdk::Pixbuf> image, Glib::RefPtr<Gdk::Pixbuf> mask, const Player* p)
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

  while (d_roadlist.size() > 10)
    eraseLastRoadItem();

  while (d_foglist.size() > 10)
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

  // still not enough? Erase army images
  while (d_armylist.size() > 40)
    eraseLastArmyItem();

}

ArmyCacheItem* GraphicsCache::addArmyPic(guint32 armyset, guint32 army,
					 const Player* p, 
					 const bool *medalsbonus)
{
  debug("ADD army pic: " <<armyset <<"," <<army)

    ArmyCacheItem* myitem = new ArmyCacheItem();
  myitem->armyset = armyset;
  myitem->index = army;
  myitem->player_id = p->getId();
  myitem->medals[0] = medalsbonus[0];
  myitem->medals[1] = medalsbonus[1];
  myitem->medals[2] = medalsbonus[2];

  const ArmyProto * basearmy = Armysetlist::getInstance()->getArmy(armyset, army);

  // copy the pixmap including player colors
  myitem->surface = applyMask(basearmy->getImage(), basearmy->getMask(), p);

  if (medalsbonus != NULL)
    {
      debug("medalsbonus============= " << medalsbonus); 
      for(int i=0;i<3;i++)
	{ 
	  if (medalsbonus[i])
	    {
	      d_medalpic[i]->composite(myitem->surface, 
				       i * d_medalpic[i]->get_width(), 0, 
				       d_medalpic[i]->get_width(), 
				       d_medalpic[i]->get_height(), 
				       i * d_medalpic[i]->get_width(), 0, 
				       1, 1, Gdk::INTERP_BILINEAR, 255);
	    }
	}
    }

  //now the final preparation steps:
  //a) add the size
  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize += myitem->surface->get_bits_per_sample()/8 * size;

  //b) add the entry to the list
  d_armylist.push_back(myitem);

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
  d_cachesize += myitem->surface->get_bits_per_sample()/8 * size;

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
  Glib::RefPtr<Gdk::Pixbuf>ship = al->getShipPic(p->getArmyset());
  Glib::RefPtr<Gdk::Pixbuf>shipmask = al->getShipMask(p->getArmyset());
  // copy the pixmap including player colors
  myitem->surface = applyMask(ship, shipmask, p);

  //now the final preparation steps:
  //a) add the size
  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize += myitem->surface->get_bits_per_sample()/8 * size;

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
  Glib::RefPtr<Gdk::Pixbuf>standard = al->getStandardPic(p->getArmyset());
  Glib::RefPtr<Gdk::Pixbuf>standard_mask = al->getStandardMask(p->getArmyset());

  // copy the pixmap including player colors
  myitem->surface = applyMask(standard, standard_mask, p);

  //now the final preparation steps:
  //a) add the size
  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize += myitem->surface->get_bits_per_sample()/8 * size;

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

  Glib::RefPtr<Gdk::Pixbuf> mysurf = d_templepic[type]->copy();

  //now create the cache item and add the size
  TempleCacheItem* myitem = new TempleCacheItem();
  myitem->type = type;
  myitem->surface = mysurf;

  d_templelist.push_back(myitem);

  //add the size
  int size = mysurf->get_width() * mysurf->get_height();
  d_cachesize += size * mysurf->get_bits_per_sample()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

RuinCacheItem* GraphicsCache::addRuinPic(int type)
{
  Glib::RefPtr<Gdk::Pixbuf> mysurf = d_ruinpic[type]->copy();

  //now create the cache item and add the size
  RuinCacheItem* myitem = new RuinCacheItem();
  myitem->type = type;
  myitem->surface = mysurf;

  d_ruinlist.push_back(myitem);

  //add the size
  int size = mysurf->get_width() * mysurf->get_height();
  d_cachesize += size * mysurf->get_bits_per_sample()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

DiplomacyCacheItem* GraphicsCache::addDiplomacyPic(int type, Player::DiplomaticState state)
{
  Glib::RefPtr<Gdk::Pixbuf> mysurf = 
    d_diplomacypic[type][state - Player::AT_PEACE]->copy();

  //now create the cache item and add the size
  DiplomacyCacheItem* myitem = new DiplomacyCacheItem();
  myitem->type = type;
  myitem->state = state;
  myitem->surface = mysurf;

  d_diplomacylist.push_back(myitem);

  //add the size
  int size = mysurf->get_width() * mysurf->get_height();
  d_cachesize += size * mysurf->get_bits_per_sample()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

RoadCacheItem* GraphicsCache::addRoadPic(int type)
{
  //    int ts = GameMap::getInstance()->getTileset()->getTileSize();

  Glib::RefPtr<Gdk::Pixbuf> mysurf = d_roadpic[type]->copy();

  //now create the cache item and add the size
  RoadCacheItem* myitem = new RoadCacheItem();
  myitem->type = type;
  myitem->surface = mysurf;

  d_roadlist.push_back(myitem);

  //add the size
  int size = mysurf->get_width() * mysurf->get_height();
  d_cachesize += size * mysurf->get_bits_per_sample()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

FogCacheItem* GraphicsCache::addFogPic(int type)
{

  Glib::RefPtr<Gdk::Pixbuf> mysurf = d_fogpic[type]->copy();

  //now create the cache item and add the size
  FogCacheItem* myitem = new FogCacheItem();
  myitem->type = type;
  myitem->surface = mysurf;

  d_foglist.push_back(myitem);

  //add the size
  int size = mysurf->get_width() * mysurf->get_height();
  d_cachesize += size * mysurf->get_bits_per_sample()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

BridgeCacheItem* GraphicsCache::addBridgePic(int type)
{
  //    int ts = GameMap::getInstance()->getTileset()->getTileSize();

  Glib::RefPtr<Gdk::Pixbuf> mysurf = d_bridgepic[type]->copy();

  //now create the cache item and add the size
  BridgeCacheItem* myitem = new BridgeCacheItem();
  myitem->type = type;
  myitem->surface = mysurf;

  d_bridgelist.push_back(myitem);

  //add the size
  int size = mysurf->get_width() * mysurf->get_height();
  d_cachesize += size * mysurf->get_bits_per_sample()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

CursorCacheItem* GraphicsCache::addCursorPic(int type)
{
  Glib::RefPtr<Gdk::Pixbuf> mysurf = d_cursorpic[type]->copy();

  //now create the cache item and add the size
  CursorCacheItem* myitem = new CursorCacheItem();
  myitem->type = type;
  myitem->surface = mysurf;

  d_cursorlist.push_back(myitem);

  //add the size
  int size = mysurf->get_width() * mysurf->get_height();
  d_cachesize += size * mysurf->get_bits_per_sample()/8;

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
  myitem->surface = d_citypic[p->getId()]->copy();

  d_citylist.push_back(myitem);

  //add the size
  int size = d_citypic[p->getId()]->get_width() * d_citypic[p->getId()]->get_height();
  d_cachesize += size * d_citypic[p->getId()]->get_bits_per_sample()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

TowerCacheItem* GraphicsCache::addTowerPic(const Player* p)
{
  //now create the cache item and add the size
  TowerCacheItem* myitem = new TowerCacheItem();
  myitem->player_id = p->getId();
  myitem->surface = d_towerpic[p->getId()]->copy();

  d_towerlist.push_back(myitem);

  //add the size
  int size = d_towerpic[p->getId()]->get_width() * d_towerpic[p->getId()]->get_height();
  d_cachesize += size * d_towerpic[p->getId()]->get_bits_per_sample()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

FlagCacheItem* GraphicsCache::addFlagPic(int size, const Player* p)
{
  debug("GraphicsCache::addFlagPic, player="<<p->getName()<<", size="<<size)

    // size is the size of the stack, but we need the index, which starts at 0
    size--;

  Glib::RefPtr<Gdk::Pixbuf> mysurf = applyMask(d_flagpic[size], d_flagmask[size], p);

  //now create the cache item and add the size
  FlagCacheItem* myitem = new FlagCacheItem();
  myitem->player_id = p->getId();
  myitem->size = size;
  myitem->surface = mysurf;

  d_flaglist.push_back(myitem);

  //add the size
  int picsize = mysurf->get_width() * mysurf->get_height();
  d_cachesize += picsize * mysurf->get_bits_per_sample()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

SelectorCacheItem* GraphicsCache::addSelectorPic(guint32 type, guint32 frame, const Player* p)
{
  debug("GraphicsCache::addSelectorPic, player="<<p->getName()<<", type="<<type<< ", " << frame)

    // frame is the frame of animation we're looking for.  starts at 0.
    // type is 0 for big, 1 for small

    Glib::RefPtr<Gdk::Pixbuf> mysurf;
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
  d_cachesize += picsize * mysurf->get_bits_per_sample()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

ProdShieldCacheItem* GraphicsCache::addProdShieldPic(guint32 type, bool prod)
{
  debug("GraphicsCache::addProdShieldPic, prod="<<prod<<", type="<<type)

    // type is 0 for home, 1 for away, 2 for destination, 3 for source,
    // 4 for invalid

    Glib::RefPtr<Gdk::Pixbuf> mysurf;
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
  d_cachesize += picsize * mysurf->get_bits_per_sample()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}

MoveBonusCacheItem* GraphicsCache::addMoveBonusPic(guint32 type)
{
  debug("GraphicsCache::addMoveBonusPic, type="<<type)

    //type is 0=empty, 1=trees, 2=foothills, 3=hills+trees, 4=fly, 5=boat

    Glib::RefPtr<Gdk::Pixbuf> mysurf;
  mysurf = d_movebonuspic[type]->copy();

  //now create the cache item and add the size
  MoveBonusCacheItem* myitem = new MoveBonusCacheItem();
  myitem->type = type;
  myitem->surface = mysurf;

  d_movebonuslist.push_back(myitem);

  //add the size
  int picsize = mysurf->get_width() * mysurf->get_height();
  d_cachesize += picsize * mysurf->get_bits_per_sample()/8;

  //and check the size of the cache
  checkPictures();

  return myitem;
}


void GraphicsCache::clear()
{
  while (!d_armylist.empty())
    eraseLastArmyItem();

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
  d_armylist.erase(d_armylist.begin());

  //don't forget to subtract the size from the size entry
  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_bits_per_sample()/8 * size;

  myitem->surface.clear();
  delete myitem;
}

void GraphicsCache::eraseLastTempleItem()
{
  if (d_templelist.empty())
    return;

  TempleCacheItem* myitem = *(d_templelist.begin());
  d_templelist.erase(d_templelist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_bits_per_sample()/8 * size;

  myitem->surface.clear();
  delete myitem;
}

void GraphicsCache::eraseLastRuinItem()
{
  if (d_ruinlist.empty())
    return;

  RuinCacheItem* myitem = *(d_ruinlist.begin());
  d_ruinlist.erase(d_ruinlist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_bits_per_sample()/8 * size;

  myitem->surface.clear();
  delete myitem;
}

void GraphicsCache::eraseLastDiplomacyItem()
{
  if (d_diplomacylist.empty())
    return;

  DiplomacyCacheItem* myitem = *(d_diplomacylist.begin());
  d_diplomacylist.erase(d_diplomacylist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_bits_per_sample()/8 * size;

  myitem->surface.clear();
  delete myitem;
}

void GraphicsCache::eraseLastRoadItem()
{
  if (d_roadlist.empty())
    return;

  RoadCacheItem* myitem = *(d_roadlist.begin());
  d_roadlist.erase(d_roadlist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_bits_per_sample()/8 * size;

  myitem->surface.clear();
  delete myitem;
}

void GraphicsCache::eraseLastFogItem()
{
  if (d_foglist.empty())
    return;

  FogCacheItem* myitem = *(d_foglist.begin());
  d_foglist.erase(d_foglist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_bits_per_sample()/8 * size;

  myitem->surface.clear();
  delete myitem;
}

void GraphicsCache::eraseLastBridgeItem()
{
  if (d_bridgelist.empty())
    return;

  BridgeCacheItem* myitem = *(d_bridgelist.begin());
  d_bridgelist.erase(d_bridgelist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_bits_per_sample()/8 * size;

  myitem->surface.clear();
  delete myitem;
}

void GraphicsCache::eraseLastCursorItem()
{
  if (d_cursorlist.empty())
    return;

  CursorCacheItem* myitem = *(d_cursorlist.begin());
  d_cursorlist.erase(d_cursorlist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_bits_per_sample()/8 * size;

  myitem->surface.clear();
  delete myitem;
}

void GraphicsCache::eraseLastCityItem()
{
  if (d_citylist.empty())
    return;

  CityCacheItem* myitem = *(d_citylist.begin());
  d_citylist.erase(d_citylist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_bits_per_sample()/8 * size;

  myitem->surface.clear();
  delete myitem;
}

void GraphicsCache::eraseLastTowerItem()
{
  if (d_towerlist.empty())
    return;

  TowerCacheItem* myitem = *(d_towerlist.begin());
  d_towerlist.erase(d_towerlist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_bits_per_sample()/8 * size;

  myitem->surface.clear();
  delete myitem;
}

void GraphicsCache::eraseLastShipItem()
{
  if (d_shiplist.empty())
    return;

  ShipCacheItem* myitem = *(d_shiplist.begin());
  d_shiplist.erase(d_shiplist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_bits_per_sample()/8 * size;

  myitem->surface.clear();
  delete myitem;
}

void GraphicsCache::eraseLastPlantedStandardItem()
{
  if (d_plantedstandardlist.empty())
    return;

  PlantedStandardCacheItem* myitem = *(d_plantedstandardlist.begin());
  d_plantedstandardlist.erase(d_plantedstandardlist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_bits_per_sample()/8 * size;

  myitem->surface.clear();
  delete myitem;
}

void GraphicsCache::eraseLastFlagItem()
{
  if (d_flaglist.empty())
    return;

  FlagCacheItem* myitem = *(d_flaglist.begin());
  d_flaglist.erase(d_flaglist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_bits_per_sample()/8 * size;

  myitem->surface.clear();
  delete myitem;
}

void GraphicsCache::eraseLastSelectorItem()
{
  if (d_selectorlist.empty())
    return;

  SelectorCacheItem* myitem = *(d_selectorlist.begin());
  d_selectorlist.erase(d_selectorlist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_bits_per_sample()/8 * size;

  myitem->surface.clear();
  delete myitem;
}

void GraphicsCache::eraseLastShieldItem()
{
  if (d_shieldlist.empty())
    return;

  ShieldCacheItem* myitem = *(d_shieldlist.begin());
  d_shieldlist.erase(d_shieldlist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_bits_per_sample()/8 * size;

  myitem->surface.clear();
  delete myitem;
}

void GraphicsCache::eraseLastProdShieldItem()
{
  if (d_prodshieldlist.empty())
    return;

  ProdShieldCacheItem* myitem = *(d_prodshieldlist.begin());
  d_prodshieldlist.erase(d_prodshieldlist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_bits_per_sample()/8 * size;

  myitem->surface.clear();
  delete myitem;
}

void GraphicsCache::eraseLastMoveBonusItem()
{
  if (d_movebonuslist.empty())
    return;

  MoveBonusCacheItem* myitem = *(d_movebonuslist.begin());
  d_movebonuslist.erase(d_movebonuslist.begin());

  int size = myitem->surface->get_width() * myitem->surface->get_height();
  d_cachesize -= myitem->surface->get_bits_per_sample()/8 * size;

  myitem->surface.clear();
  delete myitem;
}

void GraphicsCache::loadTemplePics()
{
  // GameMap has the actual cityset stored
  std::string cityset = GameMap::getInstance()->getCityset()->getSubDir();
  int ts = GameMap::getInstance()->getCityset()->getTileSize();

  // load the temple pictures
  std::vector<Glib::RefPtr<Gdk::Pixbuf> > templepics;
  templepics = disassemble_row(File::getCitysetFile(cityset, "temples.png"), 
			       TEMPLE_TYPES);
  for (unsigned int i = 0; i < TEMPLE_TYPES; i++)
    {
      if (templepics[i]->get_width() != ts)
	templepics[i] = templepics[i]->scale_simple(ts, ts, Gdk::INTERP_BILINEAR);
      d_templepic[i] = templepics[i];
    }
}

void GraphicsCache::loadRuinPics()
{
  // GameMap has the actual cityset stored
  std::string cityset = GameMap::getInstance()->getCityset()->getSubDir();
  int ts = GameMap::getInstance()->getCityset()->getTileSize();

  // load the ruin pictures
  std::vector<Glib::RefPtr<Gdk::Pixbuf> > ruinpics;
  ruinpics = disassemble_row(File::getCitysetFile(cityset, "ruin.png"), 
			     RUIN_TYPES);

  for (unsigned int i = 0; i < RUIN_TYPES ; i++)
    {
      if (ruinpics[i]->get_width() != ts)
	ruinpics[i] = ruinpics[i]->scale_simple(ts, ts, Gdk::INTERP_BILINEAR);
      d_ruinpic[i] = ruinpics[i];
    }
}

void GraphicsCache::loadDiplomacyPics()
{
  int ts = 30;
  std::vector<Glib::RefPtr<Gdk::Pixbuf> > diplomacypics;
  diplomacypics = disassemble_row(File::getMiscFile("various/diplomacy-small.png"), 
			     DIPLOMACY_TYPES);
  for (unsigned int i = 0; i < DIPLOMACY_TYPES ; i++)
    {
      if (diplomacypics[i]->get_width() != ts)
	diplomacypics[i] = diplomacypics[i]->scale_simple(ts,ts,Gdk::INTERP_BILINEAR);
      d_diplomacypic[0][i] = diplomacypics[i];

    }

  ts = 50;
  diplomacypics = disassemble_row(File::getMiscFile("various/diplomacy-large.png"), 
			     DIPLOMACY_TYPES);
  for (unsigned int i = 0; i < DIPLOMACY_TYPES ; i++)
    {
      if (diplomacypics[i]->get_width() != ts)
	diplomacypics[i] = diplomacypics[i]->scale_simple(ts,ts,Gdk::INTERP_BILINEAR);
      d_diplomacypic[1][i] = diplomacypics[i];

    }
}

void GraphicsCache::loadRoadPics()
{
  // GameMap has the actual tileset stored
  std::string tileset = GameMap::getInstance()->getTileset()->getSubDir();
  int ts = GameMap::getInstance()->getTileset()->getTileSize();

  std::vector<Glib::RefPtr<Gdk::Pixbuf> > roadpics;
  roadpics = disassemble_row(File::getTilesetFile(tileset, "misc/roads.png"), 
			     ROAD_TYPES);
  for (unsigned int i = 0; i < ROAD_TYPES ; i++)
    {
      if (roadpics[i]->get_width() != ts)
	roadpics[i] = roadpics[i]->scale_simple(ts, ts, Gdk::INTERP_BILINEAR);
      d_roadpic[i] = roadpics[i];
    }
}

void GraphicsCache::loadFogPics()
{
  // GameMap has the actual tileset stored
  int ts = GameMap::getInstance()->getTileset()->getTileSize();
  std::string tileset = GameMap::getInstance()->getTileset()->getSubDir();

  // load the fog pictures
  std::vector<Glib::RefPtr<Gdk::Pixbuf> > fogpics;
  fogpics = disassemble_row(File::getTilesetFile(tileset, "misc/fog.png"),
			     FOG_TYPES);
  for (unsigned int i = 0; i < FOG_TYPES ; i++)
    {
      if (fogpics[i]->get_width() != ts)
	fogpics[i] = fogpics[i]->scale_simple(ts, ts, Gdk::INTERP_BILINEAR);
      d_fogpic[i] = fogpics[i];
    }

}

void GraphicsCache::loadBridgePics()
{
  // GameMap has the actual tileset stored
  std::string tileset = GameMap::getInstance()->getTileset()->getSubDir();
  int ts = GameMap::getInstance()->getTileset()->getTileSize();

  // load the bridge pictures
  std::vector<Glib::RefPtr<Gdk::Pixbuf> > bridgepics;
  bridgepics = disassemble_row(File::getTilesetFile(tileset, 
						    "misc/bridges.png"),
			       BRIDGE_TYPES);
  for (unsigned int i = 0; i < BRIDGE_TYPES ; i++)
    {
      if (bridgepics[i]->get_width() != ts)
	bridgepics[i] = bridgepics[i]->scale_simple(ts, ts, Gdk::INTERP_BILINEAR);
      d_bridgepic[i] = bridgepics[i];
    }
}

void GraphicsCache::loadCursorPics()
{
  int ts = 16;

  // load the cursor pictures
  std::vector<Glib::RefPtr<Gdk::Pixbuf> > cursorpics;
  cursorpics = disassemble_row(File::getMiscFile("various/cursors.png"),
			       CURSOR_TYPES);
  for (unsigned int i = 0; i < CURSOR_TYPES ; i++)
    {
      if (cursorpics[i]->get_width() != ts)
	cursorpics[i] = cursorpics[i]->scale_simple(ts, ts, Gdk::INTERP_BILINEAR);
      d_cursorpic[i] = cursorpics[i];
    }
}

void GraphicsCache::loadCityPics()
{
  // GameMap has the actual cityset stored
  std::string cityset = GameMap::getInstance()->getCityset()->getSubDir();
  int ts = GameMap::getInstance()->getTileset()->getTileSize();

  std::vector<Glib::RefPtr<Gdk::Pixbuf> > razedpics;
  razedpics = disassemble_row(File::getCitysetFile(cityset, "castle_razed.png"),
			      MAX_PLAYERS + 1);
  for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
    {
      if (razedpics[i]->get_width() != ts)
	razedpics[i] = razedpics[i]->scale_simple(ts * CITY_TILE_WIDTH, ts * CITY_TILE_WIDTH, Gdk::INTERP_BILINEAR);
      d_razedpic[i] = razedpics[i];
    }

  // load the city pictures
  std::vector<Glib::RefPtr<Gdk::Pixbuf> > citypics;
  citypics = disassemble_row(File::getCitysetFile(cityset, "castles.png"),
			      MAX_PLAYERS + 1);
  for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
    {
      if (citypics[i]->get_width() != ts)
	citypics[i] = citypics[i]->scale_simple(ts * CITY_TILE_WIDTH, ts * CITY_TILE_WIDTH, Gdk::INTERP_BILINEAR);
      d_citypic[i] = citypics[i];
    }
}

void GraphicsCache::loadTowerPics()
{
  // GameMap has the actual cityset stored
  std::string cityset = GameMap::getInstance()->getCityset()->getSubDir();
  int ts = GameMap::getInstance()->getCityset()->getTileSize();

  std::vector<Glib::RefPtr<Gdk::Pixbuf> > towerpics;
  towerpics = disassemble_row(File::getCitysetFile(cityset, "towers.png"),
			      MAX_PLAYERS);
  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      if (towerpics[i]->get_width() != ts)
	towerpics[i] = towerpics[i]->scale_simple(ts, ts, Gdk::INTERP_BILINEAR);
      d_towerpic[i] = towerpics[i];
    }
}

bool GraphicsCache::loadSelectorImages(std::string tileset, std::string filename, guint32 size, std::vector<Glib::RefPtr<Gdk::Pixbuf> > &images, std::vector<Glib::RefPtr<Gdk::Pixbuf> > &masks)
{
  int num_frames;
  num_frames = Gdk::Pixbuf::create_from_file 
    (File::getTilesetFile(tileset, filename))->get_width() / size;
  images = disassemble_row(File::getTilesetFile(tileset, filename),
			      num_frames, true);
  for (int i = 0; i < num_frames; i++)
    {
      if (images[i]->get_width() != (int)size)
	images[i] = images[i]->scale_simple (size, size, Gdk::INTERP_BILINEAR);
    }

  masks = disassemble_row(File::getTilesetFile(tileset, filename),
			      num_frames, false);
  for (int i = 0; i < num_frames; i++)
    {
      if (masks[i]->get_width() != (int)size)
	masks[i] = masks[i]->scale_simple (size, size, Gdk::INTERP_BILINEAR);
    }

  return true;
}

void GraphicsCache::loadSelectors()
{
  std::string tileset = GameMap::getInstance()->getTileset()->getSubDir();
  std::string small = GameMap::getInstance()->getTileset()->getSmallSelectorFilename();
  std::string large = GameMap::getInstance()->getTileset()->getLargeSelectorFilename();

  int size = GameMap::getInstance()->getTileset()->getTileSize();
  std::vector<Glib::RefPtr<Gdk::Pixbuf> > images;
  std::vector<Glib::RefPtr<Gdk::Pixbuf> > masks;
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
  std::vector<Glib::RefPtr<Gdk::Pixbuf> > prodshieldpics;
  prodshieldpics = disassemble_row
    (File::getMiscFile("various/prodshieldset.png"), PRODUCTION_SHIELD_TYPES);
  for (unsigned int i = 0; i < PRODUCTION_SHIELD_TYPES; i++)
    {
      if (prodshieldpics[i]->get_width() != xsize)
	prodshieldpics[i] = prodshieldpics[i]->scale_simple(xsize, ysize, 
							    Gdk::INTERP_BILINEAR);
      d_prodshieldpic[i] = prodshieldpics[i];
    }
}

void GraphicsCache::loadMedalPics()
{
  //load the medal icons
  int ts = 40;
  std::vector<Glib::RefPtr<Gdk::Pixbuf> > medalpics;
  medalpics = disassemble_row(File::getMiscFile("various/medals_mask.png"),
				  MEDAL_TYPES);
  for (unsigned int i = 0; i < MEDAL_TYPES; i++)
    {
      if (medalpics[i]->get_width() != ts)
	medalpics[i] = medalpics[i]->scale_simple(ts, ts, Gdk::INTERP_BILINEAR);
      d_medalpic[i] = medalpics[i];
    }
}

void GraphicsCache::loadMoveBonusPics()
{
  //load the movement bonus icons
  int xsize = MOVE_BONUS_WIDTH;
  int ysize = MOVE_BONUS_HEIGHT;
  std::vector<Glib::RefPtr<Gdk::Pixbuf> > movebonuspics;
  movebonuspics = disassemble_row(File::getMiscFile("various/movebonus.png"),
				  MOVE_BONUS_TYPES);
  for (unsigned int i = 0; i < MOVE_BONUS_TYPES; i++)
    {
      if (movebonuspics[i]->get_width() != xsize)
	movebonuspics[i] = movebonuspics[i]->scale_simple(xsize, ysize, 
							 Gdk::INTERP_BILINEAR);
      d_movebonuspic[i] = movebonuspics[i];
    }
}

void GraphicsCache::loadFlags()
{
  //GameMap has the actual tileset stored
  std::string tileset = GameMap::getInstance()->getTileset()->getSubDir();
  int ts = GameMap::getInstance()->getTileset()->getTileSize();

  std::vector<Glib::RefPtr<Gdk::Pixbuf> > flagpics;
  flagpics = disassemble_row(File::getTilesetFile(tileset, "misc/flags.png"),
				  MAX_STACK_SIZE, true);
  for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
    {
      if (flagpics[i]->get_width() != ts)
	flagpics[i] = flagpics[i]->scale_simple(ts, ts, Gdk::INTERP_BILINEAR);
      d_flagpic[i] = flagpics[i];

    }
  std::vector<Glib::RefPtr<Gdk::Pixbuf> > maskpics;
  maskpics = disassemble_row(File::getTilesetFile(tileset, "misc/flags.png"),
				  MAX_STACK_SIZE, false);
  for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
    {
      if (maskpics[i]->get_width() != ts)
	maskpics[i] = maskpics[i]->scale_simple(ts, ts, Gdk::INTERP_BILINEAR);
      d_flagmask[i] = maskpics[i];
    }
}
