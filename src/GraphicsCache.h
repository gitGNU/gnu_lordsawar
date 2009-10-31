// Copyright (C) 2003, 2004, 2005, 2006, 2007 Ulf Lorenz
// Copyright (C) 2004, 2006 Andrea Paternesi
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

#ifndef GRAPHICS_CACHE_H
#define GRAPHICS_CACHE_H

#include <string>
#include <list>
#include <map>
#include <vector>
#include <string.h>

#include "player.h"
#include "defs.h"
#include "PixMask.h"
#include "maptile.h"

class Tileset;

struct ArmyCacheItem;
struct ShipCacheItem;
struct CityCacheItem;
struct TowerCacheItem;
struct TempleCacheItem;
struct RuinCacheItem;
struct DiplomacyCacheItem;
struct StoneCacheItem;
struct RoadCacheItem;
struct BridgeCacheItem;
struct CursorCacheItem;
struct FlagCacheItem;
struct SelectorCacheItem;
struct ShieldCacheItem;
struct ProdShieldCacheItem;
struct MoveBonusCacheItem;
struct FogCacheItem;
struct PlantedStandardCacheItem;
struct NewLevelCacheItem;
struct TileCacheItem;
struct PortCacheItem;
struct SignpostCacheItem;
struct BagCacheItem;
struct ExplosionCacheItem;
class City;

/** Soliton class for caching army and map images
  * 
  * With the introduction of player-specific colors, the problem of caching
  * images has popped up. The player colors are implemented by taking an army
  * picture and a mask, with the mask being a 16 color image, substituting
  * the colors in the mask and blitting the mask over the army (or e.g. city)
  * image. This takes several blits (>3, there are also things like medals 
  * to be considered) and is therefore costly.
  *
  * This class approaches this problem by caching formatted images. You get
  * e.g. an army image by querying the cache, which either gives you the cached
  * image or creates a new one. If the size is exceeded, the class will free
  * the oldest (not used for the longest time) images until it gets below the
  * treshold. It maintains some kind of balance between cached city pictures
  * and cached army pictures, but the kind of balance may change, so I'll
  * better not describe it here. :)
  *
  * Recently, it has been improved to cache flag pictures (showing how many
  * armies a stack contains) as well.
  *
  * The maximum cache size can be changed on the fly by modifying the value in 
  * the Configuration class. However, if set too small (around 1 megabyte), this
  * value will silently be ignored.
  *
  * @note For efficiency reasons, the class will not copy the surfaces it has,
  * so DON'T MODIFY THEM unless you know what you do! (i.e. never)
  */

class GraphicsCache
{
    public:
  enum CursorType
    {
      POINTER = 0,
      MAGNIFYING_GLASS,
      SHIP,
      ROOK,
      HAND,
      TARGET,
      FEET,
      RUIN,
      SWORD,
      QUESTION,
      HEART,
      GOTO_ARROW
    };
        //! Function for getting/creating the soliton instance.
        static GraphicsCache* getInstance();

        //! Explicitely deletes the soliton instance
        static void deleteInstance();

        //! Clears the whole cache.
        void clear();

        //! Get the current cache size, the maximum is in Configuration::s_cacheSize
        guint32 getCacheSize() const {return d_cachesize;}

        /** Function for getting the army picture from the cache
          * 
          * This function returns either the cached image of the given type or
          * creates a new one and caches it. Use this function to access
          * army images! And: Don't touch the returned surface!! For performance
          * reasons you get the original surface which is also in the cache.
          *
          * The returned surface contains the correct player colors and some
          * icons displaying the level of the unit.
          *
          * @param armyset      the armyset to be used
          * @param army         the index of the army to be used
          * @param player       the player owning the army
	  * @param greyed       the image is greyed out; deselected/inactive.
          * @return the image of the unit
          */
        PixMask* getArmyPic(guint32 armyset, guint32 army, const Player* p,
                                const bool* medals, bool greyed = false);
	PixMask* getArmyPic(Army *a, bool greyed = false);

	PixMask* getTilePic(int tile_style_id, int fog_type_id, bool has_bag, bool has_standard, int standard_player_id, int stack_size, int stack_player_id, int army_type_id, bool has_tower, bool has_ship, Maptile::Building building_type, int building_subtype, Vector<int> building_tile, int building_player_id, guint32 tilesize, bool has_grid, guint32 tileset, guint32 cityset);
	PixMask* getTilePic(int tile_style_id, int fog_type_id, bool has_bag, bool has_standard, int standard_player_id, int stack_size, int stack_player_id, int army_type_id, bool has_tower, bool has_ship, Maptile::Building building_type, int building_subtype, Vector<int> building_tile, int building_player_id, guint32 tilesize, bool has_grid);

        /** Function for getting the shield picture from the cache
          * 
          * This function returns either the cached image of the given type or
          * creates a new one and caches it. Use this function to access
          * army images! And: Don't touch the returned surface!! For performance
          * reasons you get the original surface which is also in the cache.
          *
          * The returned surface contains the correct player colors and some
          * icons displaying the level of the unit.
          *
          * @param shieldset    the id of the shieldset to be used
	  * @param type         the size of the shield: 0=sm, 1=med, 2=lg
          * @param colour       which player the shield is for
          * @return the image of the shield
          */
        PixMask* getShieldPic(guint32 shieldset, guint32 type, guint32 colour);
        PixMask* getShieldPic(guint32 type, Player *p);

        /** Function for getting a ruin picture
          *
          * @param type         the type of the ruin
          * @return image of the ruin 
          */
        PixMask* getRuinPic(int type);
        PixMask* getRuinPic(int type, guint32 cityset);

        /** Function for getting a diplomacy icon
          *
          * @param type         o = small, or 1 = large.
          * @param state        the diplomatic state.  e.g. peace, war, etc
          * @return image of the icon
          */
        PixMask* getDiplomacyPic(int type, Player::DiplomaticState state);

        /** Function for getting a temple picture
          *
          * @param type         the type of the temple
          * @return image of the temple
          */
        PixMask* getTemplePic(int type);
        PixMask* getTemplePic(int type, guint32 cityset);

        /** Function for getting a road picture
          *
          * @param type         the type of the road
          * @return image of the road
          */
        PixMask* getRoadPic(int type, guint32 tileset);
        PixMask* getRoadPic(int type);

        /** Function for getting a fog picture
          *
          * @param type         the type of the fog
          * @return image of the fog
          */
        PixMask* getFogPic(int type, guint32 tileset);
        PixMask* getFogPic(int type);

        /** Function for getting a bridge picture
          *
          * @param type         the type of the bridge 0=e/w 1=n/s
          * @return image of the bridge
          */
        PixMask* getBridgePic(int type, guint32 tileset);
        PixMask* getBridgePic(int type);

        /** Function for getting a cursor picture
          *
          * @param type         the type of the cursor 
          * @return image of the cursor
          */
        PixMask* getCursorPic(int type);

        /** Function for getting a ship picture.  This is the picture
	  * that appears when the stack goes into the water.
          *
          * @param p            the player to colour the ship as
          * @return image of the ship
          */
        PixMask* getShipPic(const Player* p);

        /** Function for getting a standard picture.  This is the picture
	  * that appears when the hero plants a flag..
          *
          * @param p            the player to colour the flag as
          * @return image of the standard
          */
        PixMask* getPlantedStandardPic(const Player* p);

        /** Function for getting a port picture.  This is the picture
	  * that appears often as an anchor on coastal regions.
          *
          * @return image of the port
          */
        PixMask* getPortPic();
        PixMask* getPortPic(guint32 cityset);

        /** Function for getting a signpost picture.  This is the picture
	  * that appears as a little tiny sign on grassy tiles.
          *
          * @return image of the signpost
          */
        PixMask* getSignpostPic();
        PixMask* getSignpostPic(guint32 cityset);

        /** Function for getting a bag-of-items picture.  This is the picture
	  * that shows when a hero drops one or more items on the ground.
          *
          * @return image of the sack of items
          */
        PixMask* getBagPic();
        PixMask* getBagPic(guint32 armyset);

        /** Function for getting an explosion picture.  This is the picture
	  * that shows when stacks are fighting.
          *
          * @return image of the explosion.
          */
        PixMask* getExplosionPic();
        PixMask* getExplosionPic(guint32 tileset);

	/** Function for getting a new-level picture.  This is the picture
	 * that appears when a hero gains a new level, and subsequently gets
	 * to increase a stat.
	 *
	 * @param p the player to colour the image as.
	 * @return new-level image.
	 */
        PixMask* getNewLevelPic(const Player* p, guint32 gender);

        /** Function for getting a city picture
          * 
          * For simplicity we have extended the basic_image/mask style to
          * cities as well, since it greatly reduces the number of images.
          * Use this function solely to get city images, and don't touch the
          * images!
          *
          * @param type         the level of the city; -1 returns the pic for
          *                     the razed city
          * @param player       the player owning the city
	  * @param cityset      the cityset that has the city image
          * @return image of the described city
          */
        PixMask* getCityPic(int type, const Player* p, guint32 cityset);
        /** Another function for getting a city picture
          *
          * Most often, we don't need such a sophisticated function. So just
          * supply the city instance and be happy. :)
          *
          * @param city     the city whose picture we want to get
          * @return image of the city
          */
        PixMask* getCityPic(const City* city);
        PixMask* getCityPic(const City* city, guint32 cityset);

        /** Function for getting tower pictures.
          *
          * As with the other functions, use solely this function to get the tower 
          * images. And DON'T modify the images!
          *
          * @param p the player for which we want to get the tower
          * @return image for the tower
          */
        PixMask* getTowerPic(const Player *p);
	PixMask* getTowerPic(const Player* p, guint32 cityset);

        /** Function for getting flag pictures.
          *
          * As with the other functions, use solely this function to get the flag
          * images. And DON'T modify the images!
          *
          * @param stack    the stack for which we want to get the flag
          * @return image for the flag
          */
        PixMask* getFlagPic(const Stack* s);
        PixMask* getFlagPic(const Stack* s, guint32 tileset);
	PixMask* getFlagPic(guint32 stack_size, const Player *p);
        PixMask* getFlagPic(guint32 stack_size, const Player *p, guint32 tileset);

        /** Function for getting selector pictures.
          *
          * As with the other functions, use solely this function to get the 
          * selector images. And DON'T modify the images!
          *
          * @param type the frame of the selector
          * @param p the player to draw it for
          * @return image for the flag
          */
        PixMask* getSelectorPic(guint32 type, guint32 frame, const Player* p, guint32 tileset);

	PixMask* getSelectorPic(guint32 type, guint32 frame, const Player *p);

        /** Function for getting shield pictures.
          *
          * As with the other functions, use solely this function to get the 
          * shield images. And DON'T modify the images!
          *
          * @param type small, medium or large shield size
          * @param p the player to draw it for
          * @return image for the shield
          */
        PixMask* getShieldPic(guint32 type, const Player* p);


        PixMask* getSmallRuinedCityPic();
	//! Return a small hero picture, either white (active==true) or black.
        PixMask* getSmallHeroPic(bool active);
        PixMask* getMoveBonusPic(guint32 bonus, bool has_ship);
        PixMask*getSmallTemplePic();
        PixMask*getSmallRuinExploredPic();
        PixMask* getSmallRuinUnexploredPic();
        PixMask* getSmallStrongholdUnexploredPic();

        /** Function for getting production shield pictures.
          *
          * As with the other functions, use solely this function to get the 
          * shield images. And DON'T modify the images!
          *
          * @param type home/away/destination/source/invalid.  
	  * one sees home/away
	  * normally, but when "see all" is turned on, one sees source/dest.
          * @param prod city production is going on, true or false
          * @return image for the shield
	  * note that type=source, production=false is impossible
	  * note that type=invalid,production=true is used to show the symbol
	  * that means no more units can be vectored to this city.
          */
        PixMask* getProdShieldPic(guint32 type, bool prod);

        PixMask* getMedalPic(bool large, int type);


        /** Modify an image with player colors.
          * 
          * Take an arbitray surface and mask image, apply the player colors such
          * that the pixels in the mask image reflect this and blit the mask over
          * the original image. This has been made public to allow arbitrary images
          * (in detail the win game image) to have masks.
          *
          * @param image    the original image
          * @param mask     the mask; is only opaque where player colors are wanted
          * @return a surface with player colors applied.
          */

	PixMask* applyMask(PixMask* image, PixMask* mask, const Player* p);

	static PixMask* applyMask(PixMask* image, PixMask* mask, Gdk::Color colour, bool isNeutral);

	static PixMask* greyOut(PixMask* image);

	static bool loadSelectorImages(std::string filename, guint32 tilesize, std::vector<PixMask* > &images, std::vector<PixMask* > &masks);

	static bool loadFlagImages(std::string filename, guint32 size, std::vector<PixMask* > &images, std::vector<PixMask* > &masks);

        /** Load misc pic
          * 
          * @param picname  the name of the image (including the suffix).
          * @param alpha    set this to false if you encounter transparent
          *                 images that should be opaque. :)
          *                 Especially for background images...
          * @return the surface which contains the image
          */
        static PixMask* getMiscPicture(std::string picname, bool alpha=true);

    private:
        GraphicsCache();
        ~GraphicsCache();

        //! Creates a new temple picture with the given parameters.
        TempleCacheItem* addTemplePic(int type, guint32 cityset);

        //! Creates a new ruin picture with the given parameters.
        RuinCacheItem* addRuinPic(int type, guint32 cityset);

        //! Creates a new diplomacy icon with the given parameters.
        DiplomacyCacheItem* addDiplomacyPic(int type, Player::DiplomaticState state);

        //! Creates a new road picture with the given parameters.
        RoadCacheItem* addRoadPic(int type, guint32 tileset);

        //! Creates a new fog picture with the given parameters.
        FogCacheItem* addFogPic(FogCacheItem *item);

        //! Creates a new bridge picture with the given parameters.
        BridgeCacheItem* addBridgePic(int type, guint32 tileset);

        //! Creates a new cursor picture with the given parameters.
        CursorCacheItem* addCursorPic(int type);

        //! Creates a new army picture with the given parameters.
        ArmyCacheItem* addArmyPic(ArmyCacheItem*item);

	//! Creates a new drawn tile with the given parameters.
	TileCacheItem* addTilePic(TileCacheItem*item);

        //! Creates a new shield picture with the given parameters.
        ShieldCacheItem* addShieldPic(guint32 shieldset, guint32 type, guint32 colour);

        //! Creates a new city picture with the given parameters.
        CityCacheItem* addCityPic(int type, const Player* p, guint32 cityset);

        //! Creates a new tower picture with the given parameters.
	TowerCacheItem* addTowerPic(const Player* p, guint32 cityset);

        //! Creates a new ship picture with the given parameters.
        ShipCacheItem* addShipPic(const Player* p);

        //! Creates a new planted standard picture with the given parameters.
        PlantedStandardCacheItem* addPlantedStandardPic(const Player* p);

        //! Creates a new port picture with the given parameters.
        PortCacheItem* addPortPic(guint32 cityset);

        //! Creates a new port picture with the given parameters.
        SignpostCacheItem* addSignpostPic(guint32 cityset);

        //! Creates a new bag-of-items picture with the given parameters.
        BagCacheItem* addBagPic(guint32 armyset);

        //! Creates a new explosion picture with the given parameters.
        ExplosionCacheItem* addExplosionPic(guint32 tileset);

	//! Creates a new new-level picture in the player's colour.
        NewLevelCacheItem* addNewLevelPic(const Player* p, guint32 gender);

        //! Creates a new flag picture with the given parameters.
        FlagCacheItem* addFlagPic(int size, const Player *p, guint32 tileset);

        //! Creates a new selector picture with the given parameters.
        SelectorCacheItem* addSelectorPic(guint32 type, guint32 frame, 
					  const Player* p, guint32 tileset);

        //! Creates a new production shield picture with the given parameters.
        ProdShieldCacheItem* addProdShieldPic(guint32 type, bool prod);

        //! Creates a new movement bonus picture with the given parameters.
        MoveBonusCacheItem* addMoveBonusPic(guint32 type);


        //! Checks if the cache has exceeded the maximum size and reduce it.
        void checkPictures();
        
        //! Erases the oldest (least recently requested) army cache item.
        void eraseLastArmyItem();

        //! Erases the oldest (least recently requested) army cache item.
        void eraseLastTileItem();

        //! Erases the oldest (least recently requested) temple cache item.
        void eraseLastTempleItem();

        //! Erases the oldest (least recently requested) ruin cache item.
        void eraseLastRuinItem();

        //! Erases the oldest (least recently requested) diplomacy cache item.
        void eraseLastDiplomacyItem();

        //! Erases the oldest (least recently requested) road cache item.
        void eraseLastRoadItem();

        //! Erases the oldest (least recently requested) fog cache item.
        void eraseLastFogItem();

        //! Erases the oldest (least recently requested) bridge cache item.
        void eraseLastBridgeItem();

        //! Erases the oldest (least recently requested) cursor cache item.
        void eraseLastCursorItem();

        //! Erases the oldest (least recently requested) city cache item.
        void eraseLastCityItem();

        //! Erases the oldest (least recently requested) tower cache item.
        void eraseLastTowerItem();

        //! Erases the oldest (least recently requested) ship cache item.
        void eraseLastShipItem();

        //! Erases the oldest planted standard cache item.
        void eraseLastPlantedStandardItem();

        //! Erases the oldest port cache item.
        void eraseLastPortItem();

        //! Erases the oldest bag-of-items cache item.
        void eraseLastBagItem();

        //! Erases the oldest explosion cache item.
        void eraseLastExplosionItem();

        //! Erases the oldest signpost cache item.
        void eraseLastSignpostItem();

	//! Erase the oldest picture of a hero gaining a level.
        void eraseLastNewLevelItem();

        //! Erases the oldest flag cache item
        void eraseLastFlagItem();

        //! Erases the oldest selector cache item
        void eraseLastSelectorItem();

        //! Erases the oldest shield cache item
        void eraseLastShieldItem();

        //! Erases the oldest production shield cache item
        void eraseLastProdShieldItem();

        //! Erases the oldest movement bonus cache item
        void eraseLastMoveBonusItem();

        //! Loads the images for the city pictures
        void loadCityPics();

        //! Loads the images for the tower pictures
        void loadTowerPics();

        //! Loads the images for the temple pictures.
        void loadTemplePics();

        //! Loads the images for the ruin pictures.
        void loadRuinPics();

        //! Loads the images for the diplomacy pictures.
        void loadDiplomacyPics();

        //! Loads the images for the cursor pictures.
        void loadCursorPics();

        //! Loads the images for the medals.
        void loadMedalPics();
        
        //! Loads the images for the production shields
        void loadProdShields();
        
	//! Loads the images associated with heroes gaining a new level.
        void loadNewLevelPics();

        //! Loads the images for the movement bonuses
        void loadMoveBonusPics();

	void drawTilePic(PixMask *surface, int fog_type_id, bool has_bag, bool has_standard, int standard_player_id, int stack_size, int stack_player_id, int army_type_id, bool has_tower, bool has_ship, Maptile::Building building_type, int building_subtype, Vector<int> building_tile, int building_player_id, guint32 ts, bool has_grid, guint32 tileset, guint32 cityset);

        /** Loads an image
          * 
          * This function loads an image, adjusts it to the current resolution etc.
          * to improve blitting performance.
          *
          * @note Some of the images (.jpg??) become transparent if the alpha
          * channel is copied (they don't have transparent bits anyway), so they
          * should be loaded with alpha set to false
          *
          * @param filename     full filename (with path) of the image to load
          * @param alpha        if set to true, copy the alpha channel as well
          *                     (i.e. if the file has transparent pixels, they
          *                     will be marked as transparent in the returned image)
          * @return converted image or 0 if anything failed.
          */
        static PixMask* loadImage(std::string filename, bool alpha = true);
        //the data
        static GraphicsCache* s_instance;

        guint32 d_cachesize;
        std::list<ArmyCacheItem*> d_armylist;
	typedef std::map<ArmyCacheItem, std::list<ArmyCacheItem*>::iterator > ArmyMap;

	ArmyMap	d_armymap;
        std::list<TileCacheItem*> d_tilelist;
	typedef std::map<TileCacheItem, std::list<TileCacheItem*>::iterator > TileMap;
	TileMap	d_tilemap;
        std::list<CityCacheItem*> d_citylist;
        std::list<TowerCacheItem*> d_towerlist;
        std::list<FlagCacheItem*> d_flaglist;
        std::list<TempleCacheItem*> d_templelist;
        std::list<RuinCacheItem*> d_ruinlist;
        std::list<DiplomacyCacheItem*> d_diplomacylist;
        std::list<RoadCacheItem*> d_roadlist;
        std::list<FogCacheItem*> d_foglist;
	typedef std::map<FogCacheItem, std::list<FogCacheItem*>::iterator > FogCacheMap;

	FogCacheMap d_fogmap;
        std::list<BridgeCacheItem*> d_bridgelist;
        std::list<CursorCacheItem*> d_cursorlist;
        std::list<SelectorCacheItem*> d_selectorlist;
        std::list<ShieldCacheItem*> d_shieldlist;
        std::list<ProdShieldCacheItem*> d_prodshieldlist;
        std::list<MoveBonusCacheItem*> d_movebonuslist;
        std::list<ShipCacheItem*> d_shiplist;
        std::list<PlantedStandardCacheItem*> d_plantedstandardlist;
        std::list<PortCacheItem*> d_portlist;
        std::list<SignpostCacheItem*> d_signpostlist;
        std::list<BagCacheItem*> d_baglist;
        std::list<ExplosionCacheItem*> d_explosionlist;
        std::list<NewLevelCacheItem*> d_newlevellist;

        //some private surfaces
        PixMask* d_diplomacypic[2][DIPLOMACY_TYPES];

        PixMask* d_cursorpic[CURSOR_TYPES];
        PixMask* d_prodshieldpic[PRODUCTION_SHIELD_TYPES];
	PixMask* d_smallruinedcity;
	PixMask* d_smallhero;
	PixMask* d_smallinactivehero;
        PixMask* d_movebonuspic[MOVE_BONUS_TYPES];
        PixMask* d_medalpic[2][MEDAL_TYPES];
	PixMask* d_small_ruin_unexplored;
	PixMask* d_small_stronghold_unexplored;
	PixMask* d_small_ruin_explored;
	PixMask* d_small_temple;
	PixMask *d_newlevel_male;
	PixMask *d_newlevelmask_male;
	PixMask *d_newlevel_female;
	PixMask *d_newlevelmask_female;
};

bool operator <(ArmyCacheItem lhs, ArmyCacheItem rhs);
bool operator <(TileCacheItem lhs, TileCacheItem rhs);
bool operator <(FogCacheItem lhs, FogCacheItem rhs);
#endif
