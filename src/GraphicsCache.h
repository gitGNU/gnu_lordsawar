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

#ifndef GRAPHICS_CACHE_H
#define GRAPHICS_CACHE_H

#include <string>
#include <list>

#include "player.h"
#include "defs.h"

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
        Uint32 getCacheSize() const {return d_cachesize;}

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
          * @return the image of the unit
          */
        SDL_Surface* getArmyPic(Uint32 armyset, Uint32 army, const Player* p,
                                const bool* medals);

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
          * @param shieldset    the shieldset to be used
	  * @param type         the size of the shield: 0=sm, 1=med, 2=lg
          * @param colour       which player the shield is for
          * @return the image of the shield
          */
        SDL_Surface* getShieldPic(std::string shieldset, Uint32 type, Uint32 colour);
        SDL_Surface* getShieldPic(Uint32 type, Player *p);

        /** Function for getting a ruin picture
          *
          * @param type         the type of the ruin
          * @return image of the ruin 
          */
        SDL_Surface* getRuinPic(int type);

        /** Function for getting a diplomacy icon
          *
          * @param type         o = small, or 1 = large.
          * @param state        the diplomatic state.  e.g. peace, war, etc
          * @return image of the icon
          */
        SDL_Surface* getDiplomacyPic(int type, Player::DiplomaticState state);

        /** Function for getting a temple picture
          *
          * @param type         the type of the temple
          * @return image of the temple
          */
        SDL_Surface* getTemplePic(int type);

        /** Function for getting a road picture
          *
          * @param type         the type of the road
          * @return image of the road
          */
        SDL_Surface* getRoadPic(int type);

        /** Function for getting a fog picture
          *
          * @param type         the type of the fog
          * @return image of the fog
          */
        SDL_Surface* getFogPic(int type);

        /** Function for getting a bridge picture
          *
          * @param type         the type of the bridge 0=e/w 1=n/s
          * @return image of the bridge
          */
        SDL_Surface* getBridgePic(int type);

        /** Function for getting a cursor picture
          *
          * @param type         the type of the cursor 
          * @return image of the cursor
          */
        SDL_Surface* getCursorPic(int type);

        /** Function for getting a ship picture.  This is the picture
	  * that appears when the stack goes into the water.
          *
          * @param p            the player to colour the ship as
          * @return image of the ship
          */
        SDL_Surface* getShipPic(const Player* p);

        /** Function for getting a standard picture.  This is the picture
	  * that appears when the hero plants a flag..
          *
          * @param p            the player to colour the flag as
          * @return image of the standard
          */
        SDL_Surface* getPlantedStandardPic(const Player* p);

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
          * @return image of the described city
          */
        SDL_Surface* getCityPic(int type, const Player* p);
        /** Another function for getting a city picture
          *
          * Most often, we don't need such a sophisticated function. So just
          * supply the city instance and be happy. :)
          *
          * @param city     the city whose picture we want to get
          * @return image of the city
          */
        SDL_Surface* getCityPic(const City* city);

        /** Function for getting tower pictures.
          *
          * As with the other functions, use solely this function to get the tower 
          * images. And DON'T modify the images!
          *
          * @param p the player for which we want to get the tower
          * @return image for the tower
          */
        SDL_Surface* getTowerPic(const Player *p);

        /** Function for getting flag pictures.
          *
          * As with the other functions, use solely this function to get the flag
          * images. And DON'T modify the images!
          *
          * @param stack    the stack for which we want to get the flag
          * @return image for the flag
          */
        SDL_Surface* getFlagPic(const Stack* s);

        /** Function for getting selector pictures.
          *
          * As with the other functions, use solely this function to get the 
          * selector images. And DON'T modify the images!
          *
          * @param type the frame of the selector
          * @param p the player to draw it for
          * @return image for the flag
          */
        SDL_Surface* getSelectorPic(Uint32 type, Uint32 frame, const Player* p);

        /** Function for getting shield pictures.
          *
          * As with the other functions, use solely this function to get the 
          * shield images. And DON'T modify the images!
          *
          * @param type small, medium or large shield size
          * @param p the player to draw it for
          * @return image for the shield
          */
        SDL_Surface* getShieldPic(Uint32 type, const Player* p);
        SDL_Surface* getSmallRuinedCityPic();
        SDL_Surface* getSmallHeroPic();
        SDL_Surface* getPortPic();
        SDL_Surface* getExplosionPic();
        SDL_Surface* getSignpostPic();
        SDL_Surface* getMoveBonusPic(Uint32 bonus, bool has_ship);
        SDL_Surface *getSmallTemplePic();
        SDL_Surface *getSmallRuinExploredPic();
        SDL_Surface* getSmallRuinUnexploredPic();
        SDL_Surface* getSmallStrongholdUnexploredPic();

        /** Function for getting production shield pictures.
          *
          * As with the other functions, use solely this function to get the 
          * shield images. And DON'T modify the images!
          *
          * @param type home/away/destination/source.  one sees home/away
	  * normally, but when "see all" is turned on, one sees source/dest.
          * @param prod city production is going on, true or false
          * @return image for the shield
	  * note that type=source, production=false is impossible
          */
        SDL_Surface* getProdShieldPic(Uint32 type, bool prod);


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
        SDL_Surface* applyMask(SDL_Surface* image, SDL_Surface* mask, const Player* p);
        
    private:
        GraphicsCache();
        ~GraphicsCache();

        //! Creates a new temple picture with the given parameters.
        TempleCacheItem* addTemplePic(int type);

        //! Creates a new ruin picture with the given parameters.
        RuinCacheItem* addRuinPic(int type);

        //! Creates a new diplomacy icon with the given parameters.
        DiplomacyCacheItem* addDiplomacyPic(int type, Player::DiplomaticState state);

        //! Creates a new road picture with the given parameters.
        RoadCacheItem* addRoadPic(int type);

        //! Creates a new fog picture with the given parameters.
        FogCacheItem* addFogPic(int type);

        //! Creates a new bridge picture with the given parameters.
        BridgeCacheItem* addBridgePic(int type);

        //! Creates a new cursor picture with the given parameters.
        CursorCacheItem* addCursorPic(int type);

        //! Creates a new army picture with the given parameters.
        ArmyCacheItem* addArmyPic(Uint32 armyset, Uint32 army, const Player* p,
                                  const bool* medalsbonus);

        //! Creates a new shield picture with the given parameters.
        ShieldCacheItem* addShieldPic(std::string shieldset, Uint32 type, Uint32 colour);

        //! Creates a new city picture with the given parameters.
        CityCacheItem* addCityPic(int type, const Player* p);

        //! Creates a new tower picture with the given parameters.
        TowerCacheItem* addTowerPic(const Player* p);

        //! Creates a new ship picture with the given parameters.
        ShipCacheItem* addShipPic(const Player* p);

        //! Creates a new planted standard picture with the given parameters.
        PlantedStandardCacheItem* addPlantedStandardPic(const Player* p);

        //! Creates a new flag picture with the given parameters.
        FlagCacheItem* addFlagPic(int size, const Player* p);

        //! Creates a new selector picture with the given parameters.
        SelectorCacheItem* addSelectorPic(Uint32 type, Uint32 frame, 
					  const Player* p);

        //! Creates a new production shield picture with the given parameters.
        ProdShieldCacheItem* addProdShieldPic(Uint32 type, bool prod);

        //! Creates a new movement bonus picture with the given parameters.
        MoveBonusCacheItem* addMoveBonusPic(Uint32 type);

        //! Checks if the cache has exceeded the maximum size and reduce it.
        void checkPictures();
        
        //! Erases the oldest (least recently requested) army cache item.
        void eraseLastArmyItem();

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

        //! Loads the images for the road pictures.
        void loadRoadPics();

        //! Loads the images for the fog pictures.
        void loadFogPics();

        //! Loads the images for the bridge pictures.
        void loadBridgePics();

        //! Loads the images for the cursor pictures.
        void loadCursorPics();

        //! Loads the images for the flags
        void loadFlags();

        //! Loads the images for the two selectors
        void loadSelectors();
        
        //! Loads the images for the production shields
        void loadProdShields();
        
        //! Loads the images for the movement bonuses
        void loadMoveBonusPics();
        
        //the data
        static GraphicsCache* s_instance;

        Uint32 d_cachesize;
        std::list<ArmyCacheItem*> d_armylist;
        std::list<CityCacheItem*> d_citylist;
        std::list<TowerCacheItem*> d_towerlist;
        std::list<FlagCacheItem*> d_flaglist;
        std::list<TempleCacheItem*> d_templelist;
        std::list<RuinCacheItem*> d_ruinlist;
        std::list<DiplomacyCacheItem*> d_diplomacylist;
        std::list<RoadCacheItem*> d_roadlist;
        std::list<FogCacheItem*> d_foglist;
        std::list<BridgeCacheItem*> d_bridgelist;
        std::list<CursorCacheItem*> d_cursorlist;
        std::list<SelectorCacheItem*> d_selectorlist;
        std::list<ShieldCacheItem*> d_shieldlist;
        std::list<ProdShieldCacheItem*> d_prodshieldlist;
        std::list<MoveBonusCacheItem*> d_movebonuslist;
        std::list<ShipCacheItem*> d_shiplist;
        std::list<PlantedStandardCacheItem*> d_plantedstandardlist;

        //some private surfaces
        SDL_Surface* d_medalsmask;
        SDL_Surface* d_citypic[MAX_PLAYERS + 1]; //+1 for neutral
        SDL_Surface* d_towerpic[MAX_PLAYERS];
        SDL_Surface* d_templepic[TEMPLE_TYPES];
        SDL_Surface* d_ruinpic[RUIN_TYPES];
        SDL_Surface* d_diplomacypic[2][DIPLOMACY_TYPES];
        SDL_Surface* d_roadpic[ROAD_TYPES];
        SDL_Surface* d_bridgepic[BRIDGE_TYPES];
        SDL_Surface* d_cursorpic[CURSOR_TYPES];
        SDL_Surface* d_razedpic[MAX_PLAYERS + 1]; //+1 for neutral
        SDL_Surface* d_flagpic[MAX_STACK_SIZE];
        SDL_Surface* d_flagmask[MAX_STACK_SIZE];
	SDL_Surface* d_selector[6];
	SDL_Surface* d_selectormask[6];
	SDL_Surface* d_smallselector[4];
	SDL_Surface* d_smallselectormask[4];
        SDL_Surface* d_prodshieldpic[7];
	SDL_Surface* d_smallruinedcity;
	SDL_Surface* d_smallhero;
        SDL_Surface* d_movebonuspic[6];
	SDL_Surface* d_port;
	SDL_Surface* d_explosion;
	SDL_Surface* d_signpost;
	SDL_Surface* d_small_ruin_unexplored;
	SDL_Surface* d_small_stronghold_unexplored;
	SDL_Surface* d_small_ruin_explored;
	SDL_Surface* d_small_temple;
	SDL_Surface *d_fogpic[FOG_TYPES];
};

#endif
