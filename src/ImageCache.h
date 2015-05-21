// Copyright (C) 2003, 2004, 2005, 2006, 2007 Ulf Lorenz
// Copyright (C) 2004, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2014 Ben Asselstine
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

#ifndef IMAGE_CACHE_H
#define IMAGE_CACHE_H

#include <list>
#include <map>
#include <vector>
#include <string.h>
#include <cairomm/context.h>
#include "PixMaskCache.h"

#include "player.h"
#include "PixMask.h"
#include "maptile.h"

class SelectorPixMaskCacheItem;
class ArmyPixMaskCacheItem;
class FlagPixMaskCacheItem;
class CircledArmyPixMaskCacheItem;
class TilePixMaskCacheItem;
class CityPixMaskCacheItem;
class TowerPixMaskCacheItem;
class TemplePixMaskCacheItem;
class RuinPixMaskCacheItem;
class DiplomacyPixMaskCacheItem;
class RoadPixMaskCacheItem;
class FogPixMaskCacheItem;
class BridgePixMaskCacheItem;
class CursorPixMaskCacheItem;
class ShieldPixMaskCacheItem;
class ProdShieldPixMaskCacheItem;
class MoveBonusPixMaskCacheItem;
class ShipPixMaskCacheItem;
class PlantedStandardPixMaskCacheItem;
class PortPixMaskCacheItem;
class SignpostPixMaskCacheItem;
class BagPixMaskCacheItem;
class ExplosionPixMaskCacheItem;
class NewLevelPixMaskCacheItem;
class DefaultTileStylePixMaskCacheItem;

//! Cache for generated army and map images.
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
  * so DON'T MODIFY THEM unless you know what you do!
  */
class ImageCache
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
  enum GameButtonType
    {
      DIPLOMACY_NO_PROPOSALS = 0,
      STACK_PARK,
      NEXT_MOVABLE_STACK,
      STACK_MOVE,
      MOVE_ALL_STACKS,
      CENTER_ON_STACK,
      STACK_DEFEND,
      STACK_DESELECT,
      DIPLOMACY_NEW_PROPOSALS,
      STACK_SEARCH,
      END_TURN,
    };
  enum ArrowType
    {
      NORTHWEST = 0,
      NORTH,
      NORTHEAST,
      EAST,
      WEST,
      SOUTHWEST,
      SOUTH,
      SOUTHEAST
    };

        //! Method for getting/creating the soliton instance.
        static ImageCache* getInstance();

        //! Explicitly deletes the soliton instance
        static void deleteInstance();

        //! Get the current cache size, the maximum is in Configuration::s_cacheSize
        guint32 getCacheSize() const {return d_cachesize;}

        /** Method for getting the army picture from the cache
          * 
          * This method returns either the cached image of the given type or
          * creates a new one and caches it. Use this method to access
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
        PixMask* getCircledArmyPic(guint32 armyset, guint32 army, 
                                   const Player* p, const bool* medals, 
                                   bool greyed, guint32 circle_colour_id,
                                   bool show_army);
        PixMask *getCircledArmyPic(Army *a, bool greyed, guint32 circle_colour_id, bool show_army);

        /** Method for getting the shield picture from the cache
          * 
          * This method returns either the cached image of the given type or
          * creates a new one and caches it. Use this method to access
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

        /** Method for getting a ruin picture
          *
          * @param type         the type of the ruin
          * @return image of the ruin 
          */
        PixMask* getRuinPic(int type);
        PixMask* getRuinPic(int type, guint32 cityset);

        /** Method for getting a diplomacy icon
          *
          * @param type         o = small, or 1 = large.
          * @param state        the diplomatic state.  e.g. peace, war, etc
          * @return image of the icon
          */
        PixMask* getDiplomacyPic(int type, Player::DiplomaticState state);

        /** Method for getting a temple picture
          *
          * @param type         the type of the temple
          * @return image of the temple
          */
        PixMask* getTemplePic(int type);
        PixMask* getTemplePic(int type, guint32 cityset);

        /** Method for getting a road picture
          *
          * @param type         the type of the road
          * @return image of the road
          */
        PixMask* getRoadPic(int type, guint32 tileset);
        PixMask* getRoadPic(int type);

        /** Method for getting a fog picture
          *
          * @param type         the type of the fog
          * @return image of the fog
          */
        PixMask* getFogPic(int type, guint32 tileset);
        PixMask* getFogPic(int type);

        /** Method for getting a bridge picture
          *
          * @param type         the type of the bridge 0=e/w 1=n/s
          * @return image of the bridge
          */
        PixMask* getBridgePic(int type, guint32 tileset);
        PixMask* getBridgePic(int type);

        /** Method for getting a cursor picture
          *
          * @param type         the type of the cursor 
          * @return image of the cursor
          */
        PixMask* getCursorPic(int type);

        /** Method for getting a ship picture.  This is the picture
	  * that appears when the stack goes into the water.
          *
          * @param p            the player to colour the ship as
          * @return image of the ship
          */
        PixMask* getShipPic(const Player* p);

        /** Method for getting a standard picture.  This is the picture
	  * that appears when the hero plants a flag..
          *
          * @param p            the player to colour the flag as
          * @return image of the standard
          */
        PixMask* getPlantedStandardPic(const Player* p);

        /** Method for getting a port picture.  This is the picture
	  * that appears often as an anchor on coastal regions.
          *
          * @return image of the port
          */
        PixMask* getPortPic();
        PixMask* getPortPic(guint32 cityset);

        /** Method for getting a signpost picture.  This is the picture
	  * that appears as a little tiny sign on grassy tiles.
          *
          * @return image of the signpost
          */
        PixMask* getSignpostPic();
        PixMask* getSignpostPic(guint32 cityset);

        /** Method for getting a bag-of-items picture.  This is the picture
	  * that shows when a hero drops one or more items on the ground.
          *
          * @return image of the sack of items
          */
        PixMask* getBagPic();
        PixMask* getBagPic(guint32 armyset);

        /** Method for getting an explosion picture.  This is the picture
	  * that shows when stacks are fighting.
          *
          * @return image of the explosion.
          */
        PixMask* getExplosionPic();
        PixMask* getExplosionPic(guint32 tileset);

	/** Method for getting a new-level picture.  This is the picture
	 * that appears when a hero gains a new level, and subsequently gets
	 * to increase a stat.
	 *
	 * @param p the player to colour the image as.
	 * @return new-level image.
	 */
        PixMask* getNewLevelPic(const Player* p, guint32 gender);

        /** Method for getting a picture that represents a type of tile style.
         *  The parameter is related to tilestyle.h:TileStyle::Type.
         */
        PixMask* getDefaultTileStylePic(guint32 tilestyle_type, 
                                        guint32 tilesize);

        /** Method for getting a city picture
          * 
          * For simplicity we have extended the basic_image/mask style to
          * cities as well, since it greatly reduces the number of images.
          * Use this method solely to get city images, and don't touch the
          * images!
          *
          * @param type         the level of the city; -1 returns the pic for
          *                     the razed city
          * @param player       the player owning the city
	  * @param cityset      the cityset that has the city image
          * @return image of the described city
          */
        PixMask* getCityPic(int type, const Player* p, guint32 cityset);
        /** Another method for getting a city picture
          *
          * Most often, we don't need such a sophisticated method. So just
          * supply the city instance and be happy. :)
          *
          * @param city     the city whose picture we want to get
          * @return image of the city
          */
        PixMask* getCityPic(const City* city);
        PixMask* getCityPic(const City* city, guint32 cityset);

        /** Method for getting tower pictures.
          *
          * As with the other methods, use solely this method to get the tower 
          * images. And DON'T modify the images!
          *
          * @param p the player for which we want to get the tower
          * @return image for the tower
          */
        PixMask* getTowerPic(const Player *p);
	PixMask* getTowerPic(const Player* p, guint32 cityset);
        /** Method for getting flag pictures.
          *
          * As with the other methods, use solely this method to get the flag
          * images. And DON'T modify the images!
          *
          * @param stack    the stack for which we want to get the flag
          * @return image for the flag
          */
        PixMask* getFlagPic(const Stack* s);
        PixMask* getFlagPic(const Stack* s, guint32 tileset);
	PixMask* getFlagPic(guint32 stack_size, const Player *p);
        PixMask* getFlagPic(guint32 stack_size, const Player *p, guint32 tileset);

        /** Method for getting selector pictures.
          *
          * As with the other methods, use solely this method to get the 
          * selector images. And DON'T modify the images!
          *
          * @param type the frame of the selector
          * @param p the player to draw it for
          * @return image for the flag
          */
        PixMask* getSelectorPic(guint32 type, guint32 frame, const Player* p, guint32 tileset);

	PixMask* getSelectorPic(guint32 type, guint32 frame, const Player *p);

	PixMask* getTilePic(int tile_style_id, int fog_type_id, bool has_bag, bool has_standard, int standard_player_id, int stack_size, int stack_player_id, int army_type_id, bool has_tower, bool has_ship, Maptile::Building building_type, int building_subtype, Vector<int> building_tile, int building_player_id, guint32 tilesize, bool has_grid, guint32 tileset, guint32 cityset, guint32 shieldset);
	PixMask* getTilePic(int tile_style_id, int fog_type_id, bool has_bag, bool has_standard, int standard_player_id, int stack_size, int stack_player_id, int army_type_id, bool has_tower, bool has_ship, Maptile::Building building_type, int building_subtype, Vector<int> building_tile, int building_player_id, guint32 tilesize, bool has_grid);


        PixMask* getMoveBonusPic(guint32 bonus, bool has_ship);
        /** Method for getting production shield pictures.
          *
          * As with the other methods, use solely this method to get the 
          * shield images. And DON'T modify the images!
          *
          * @param size 0 or 1.  0 is small, and 1 is medium sized.
          * @param type home/away/destination/source/invalid.  
	  * one sees home/away
	  * normally, but when "see all" is turned on, one sees source/dest.
          * @param prod city production is going on, true or false
          * @return image for the shield
	  * note that type=source, production=false is impossible
	  * note that type=invalid,production=true is used to show the symbol
	  * that means no more units can be vectored to this city.
          */
        PixMask* getProdShieldPic(int size, guint32 type, bool prod);

        //! Erase cached graphics.
        void reset();

        //these routines get a base image, not a cached image.
        PixMask* getDiplomacyImage(int type, Player::DiplomaticState state);
        PixMask* getCursorImage(int type);
        PixMask *getProdShieldImage(int size, guint32 type);
        PixMask* getMoveBonusImage(guint32 type);
        PixMask* getDefaultTileStyleImage(guint32 type);
        PixMask* getMedalImage(bool large, int type);
        PixMask *getNewLevelImage(bool female, bool mask);
        PixMask* getSmallRuinedCityImage();
	//! Return a small hero picture, either white (active==true) or black.
        PixMask* getSmallHeroImage(bool active);
        PixMask* getSmallBagImage();
        PixMask*getSmallTempleImage();
        PixMask*getSmallRuinExploredImage();
        PixMask* getSmallRuinUnexploredImage();
        PixMask* getSmallStrongholdUnexploredImage();
        //! get an image for one of the buttons on the main game window.
        PixMask* getGameButtonImage(guint32 type, int size);
        //! get an image for one of an arrow, for the main game window.
        PixMask* getArrowImage(guint32 type, int size);
        PixMask* getWaypointImage(guint32 type);

	static PixMask* applyMask(PixMask* image, PixMask* mask, const Player* p);
	static PixMask* applyMask(PixMask* image, PixMask* mask, Gdk::RGBA colour, bool isNeutral);

	static PixMask* greyOut(PixMask* image);

        static PixMask* circled(PixMask* image, Gdk::RGBA colour, bool coloured = true, double width_percent = 75.0);
        static void draw_circle(Cairo::RefPtr<Cairo::Context> cr, double width_percent, int width, int height, Gdk::RGBA colour, bool coloured = true, bool mask = false);
        static PixMask* loadMiscImage(Glib::ustring pngfile);
    private:
        ImageCache();
        ~ImageCache();

        //! Checks if the cache has exceeded the maximum size and reduce it.
        void checkPictures();
        
        bool loadDiplomacyImages();
        bool loadCursorImages();
        bool loadProdShieldImages();
        bool loadMoveBonusImages();
        bool loadNewLevelImages();
        bool loadDefaultTileStyleImages();
        bool loadMedalImages();
        bool loadWaypointImages();
        bool loadGameButtonImages();
        bool loadArrowImages();

        //the data
        static ImageCache* s_instance;

        guint32 d_cachesize;
  
        PixMaskCache<SelectorPixMaskCacheItem> selectorcache;
        PixMaskCache<ArmyPixMaskCacheItem> armycache;
        PixMaskCache<FlagPixMaskCacheItem> flagcache;
        PixMaskCache<CircledArmyPixMaskCacheItem> circledarmycache;
        PixMaskCache<TilePixMaskCacheItem> tilecache;
        PixMaskCache<CityPixMaskCacheItem> citycache;
        PixMaskCache<TowerPixMaskCacheItem> towercache;
        PixMaskCache<TemplePixMaskCacheItem> templecache;
        PixMaskCache<RuinPixMaskCacheItem> ruincache;
        PixMaskCache<DiplomacyPixMaskCacheItem> diplomacycache;
        PixMaskCache<RoadPixMaskCacheItem> roadcache;
        PixMaskCache<FogPixMaskCacheItem> fogcache;
        PixMaskCache<BridgePixMaskCacheItem> bridgecache;
        PixMaskCache<CursorPixMaskCacheItem> cursorcache;
        PixMaskCache<ShieldPixMaskCacheItem> shieldcache;
        PixMaskCache<ProdShieldPixMaskCacheItem> prodshieldcache;
        PixMaskCache<MoveBonusPixMaskCacheItem> movebonuscache;
        PixMaskCache<ShipPixMaskCacheItem> shipcache;
        PixMaskCache<PlantedStandardPixMaskCacheItem> plantedstandardcache;
        PixMaskCache<PortPixMaskCacheItem> portcache;
        PixMaskCache<SignpostPixMaskCacheItem> signpostcache;
        PixMaskCache<BagPixMaskCacheItem> bagcache;
        PixMaskCache<ExplosionPixMaskCacheItem> explosioncache;
        PixMaskCache<NewLevelPixMaskCacheItem> newlevelcache;
        PixMaskCache<DefaultTileStylePixMaskCacheItem> defaulttilestylecache;

        PixMask* d_diplomacy[2][DIPLOMACY_TYPES];
        PixMask* d_cursor[CURSOR_TYPES];
        PixMask* d_prodshield[2][PRODUCTION_SHIELD_TYPES];
        PixMask* d_movebonus[MOVE_BONUS_TYPES];
	PixMask *d_newlevel_male;
	PixMask *d_newlevelmask_male;
	PixMask *d_newlevel_female;
	PixMask *d_newlevelmask_female;
        PixMask *d_default_tilestyles[DEFAULT_TILESTYLE_TYPES];
        PixMask* d_medal[2][MEDAL_TYPES];
	PixMask* d_smallruinedcity;
	PixMask* d_smallhero;
	PixMask* d_smallbag;
	PixMask* d_smallinactivehero;
	PixMask* d_small_ruin_unexplored;
	PixMask* d_small_stronghold_unexplored;
	PixMask* d_small_ruin_explored;
	PixMask* d_small_temple;
        PixMask *d_waypoint[NUM_WAYPOINTS];
        PixMask *d_gamebuttons[2][NUM_GAME_BUTTON_IMAGES];
        PixMask *d_arrow[2][NUM_ARROW_IMAGES];
};

//! Helper class for selector box items in the ImageCache.
/**
 * These selector box images appear around the active stack.
 * It's a set of frames for an animation.
 */
class SelectorPixMaskCacheItem
{
public:
    static PixMask *generate(SelectorPixMaskCacheItem item);
    static bool loadSelectorImages(Glib::ustring filename, guint32 size, std::vector<PixMask* > &images, std::vector<PixMask* > &masks);
    int comp(const SelectorPixMaskCacheItem item) const;
    bool operator == (const SelectorPixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const SelectorPixMaskCacheItem &c) const {return comp(c)<0;};
    guint32 tileset;
    guint32 type;
    guint32 frame;
    guint32 player_id;
};

//! Helper class for army items in the ImageCache.
/**
 * These army images appear on the big map as the leader of a stack.
 */
class ArmyPixMaskCacheItem
{
public:
    static PixMask *generate(ArmyPixMaskCacheItem item);
    int comp(const ArmyPixMaskCacheItem item) const;
    bool operator == (const ArmyPixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const ArmyPixMaskCacheItem &c) const {return comp(c)<0;};
    guint32 armyset;
    guint32 army_id;
    guint32 player_id;
    bool medals[3];
    bool greyed;
};

//! Helper class for stack flag items in the ImageCache.
/**
 * These stack flag images have 8 different sizes, and appear on the big map.
 */
class FlagPixMaskCacheItem
{
public:
    static PixMask *generate(FlagPixMaskCacheItem item);
    static bool loadFlagImages(Glib::ustring filename, guint32 size, std::vector<PixMask* > &images, std::vector<PixMask* > &masks);
    int comp(const FlagPixMaskCacheItem item) const;
    bool operator == (const FlagPixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const FlagPixMaskCacheItem &c) const {return comp(c)<0;};
    guint32 tileset;
    guint32 size;
    guint32 player_id;
};

//! Helper class for circled army items in the ImageCache.
/**
 * These circled army images appear in various places in the gui.
 * It's just an army unit with a coloured circle behind it.
 */
class CircledArmyPixMaskCacheItem
{
public:
    static PixMask *generate(CircledArmyPixMaskCacheItem item);
    int comp(const CircledArmyPixMaskCacheItem item) const;
    bool operator == (const CircledArmyPixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const CircledArmyPixMaskCacheItem &c) const {return comp(c)<0;};
    guint32 armyset;
    guint32 army_id;
    guint32 player_id;
    bool medals[3];
    bool greyed;
    guint32 circle_colour_id;
    bool show_army;
};

//! Helper class for big map tile items in the ImageCache.
/**
 * These tile images are the almalgmation of all the things on a given tile of
 * the big map.
 */
class TilePixMaskCacheItem
{
public:
    static PixMask *generate(TilePixMaskCacheItem item);
    int comp(const TilePixMaskCacheItem item) const;
    bool operator == (const TilePixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const TilePixMaskCacheItem &c) const {return comp(c)<0;};
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
  guint32 shieldset;
};

//! Helper class for city items in the ImageCache.
/**
 * These city images appear on the big map.
 */
class CityPixMaskCacheItem
{
public:
    static PixMask *generate(CityPixMaskCacheItem item);
    int comp(const CityPixMaskCacheItem item) const;
    bool operator == (const CityPixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const CityPixMaskCacheItem &c) const {return comp(c)<0;};
    guint32 cityset;
    int type;
    guint32 player_id;
};

//! Helper class for tower items in the ImageCache.
/**
 * These tower images appear on the big map when a stack goes into defend mode.
 */
class TowerPixMaskCacheItem
{
public:
    static PixMask *generate(TowerPixMaskCacheItem item);
    int comp(const TowerPixMaskCacheItem item) const;
    bool operator == (const TowerPixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const TowerPixMaskCacheItem &c) const {return comp(c)<0;};
    guint32 cityset;
    guint32 player_id;
};

//! Helper class for temple items in the ImageCache.
/**
 * These temple images appear on the big map.
 */
class TemplePixMaskCacheItem
{
public:
    static PixMask *generate(TemplePixMaskCacheItem item);
    int comp(const TemplePixMaskCacheItem item) const;
    bool operator == (const TemplePixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const TemplePixMaskCacheItem &c) const {return comp(c)<0;};
    guint32 cityset;
    int type;
};

//! Helper class for ruin items in the ImageCache.
/**
 * These ruin images appear on the big map.
 */
class RuinPixMaskCacheItem
{
public:
    static PixMask *generate(RuinPixMaskCacheItem item);
    int comp(const RuinPixMaskCacheItem item) const;
    bool operator == (const RuinPixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const RuinPixMaskCacheItem &c) const {return comp(c)<0;};
    guint32 cityset;
    int type;
};

//! Helper class for diplomacy icon items in the ImageCache.
/**
 * These diplomacy icons appear in the diplomacy dialog.
 */
class DiplomacyPixMaskCacheItem
{
public:
    static PixMask *generate(DiplomacyPixMaskCacheItem item);
    int comp(const DiplomacyPixMaskCacheItem item) const;
    bool operator == (const DiplomacyPixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const DiplomacyPixMaskCacheItem &c) const {return comp(c)<0;};
    int type;
    Player::DiplomaticState state;
};

//! Helper class for road items in the ImageCache.
/**
 * These are the road images that appear on the big map.
 */
class RoadPixMaskCacheItem
{
public:
    static PixMask *generate(RoadPixMaskCacheItem item);
    int comp(const RoadPixMaskCacheItem item) const;
    bool operator == (const RoadPixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const RoadPixMaskCacheItem &c) const {return comp(c)<0;};
    guint32 tileset;
    int type;
};

//! Helper class for fog items in the ImageCache.
/**
 * These are the black fog images that appear on top of the big map.
 * E.g. more of the map gets exposed when army units move around.
 */
class FogPixMaskCacheItem
{
public:
    static PixMask *generate(FogPixMaskCacheItem item);
    int comp(const FogPixMaskCacheItem item) const;
    bool operator == (const FogPixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const FogPixMaskCacheItem &c) const {return comp(c)<0;};
    guint32 tileset;
    int type;
};

//! Helper class for bridge items in the ImageCache.
/**
 * These are the bridge images that appear on the big map.
 */
class BridgePixMaskCacheItem
{
public:
    static PixMask *generate(BridgePixMaskCacheItem item);
    int comp(const BridgePixMaskCacheItem item) const;
    bool operator == (const BridgePixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const BridgePixMaskCacheItem &c) const {return comp(c)<0;};
    guint32 tileset;
    int type;
};

//! Helper class for cursor items in the ImageCache.
/**
 * These are the black and white mouse cursor images.
 */
class CursorPixMaskCacheItem
{
public:
    static PixMask *generate(CursorPixMaskCacheItem item);
    int comp(const CursorPixMaskCacheItem item) const;
    bool operator == (const CursorPixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const CursorPixMaskCacheItem &c) const {return comp(c)<0;};
    int type;
};

//! Helper class for shield items in the ImageCache.
/**
 * These shield images include the small, medium and large shield images.
 */
class ShieldPixMaskCacheItem
{
public:
    static PixMask *generate(ShieldPixMaskCacheItem item);
    int comp(const ShieldPixMaskCacheItem item) const;
    bool operator == (const ShieldPixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const ShieldPixMaskCacheItem &c) const {return comp(c)<0;};
    guint32 shieldset;
    guint32 type;
    guint32 colour;
};

//! Helper class for production icon items in the ImageCache.
/**
 * these icons appear on the smallmap.
 */
class ProdShieldPixMaskCacheItem
{
public:
    static PixMask *generate(ProdShieldPixMaskCacheItem item);
    int comp(const ProdShieldPixMaskCacheItem item) const;
    bool operator == (const ProdShieldPixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const ProdShieldPixMaskCacheItem &c) const {return comp(c)<0;};
    int size;
    guint32 type;
    bool prod;
};

//! Helper class for movement bonus icon items in the ImageCache.
/**
 * These icons appear in the gui, on stack tip infos, or in the stack box.
 */
class MoveBonusPixMaskCacheItem
{
public:
    static PixMask *generate(MoveBonusPixMaskCacheItem item);
    int comp(const MoveBonusPixMaskCacheItem item) const;
    bool operator == (const MoveBonusPixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const MoveBonusPixMaskCacheItem &c) const {return comp(c)<0;};
    guint32 type; // 0=empty, 1=trees, 2=foothills, 3=hills+trees, 4=fly, 5=boat
};

//! Helper class for boat items in the ImageCache.
/**
 * ship images are for when a stack is in a boat.
 * one ship image per army set, and drawn in the player's colour.
 */
class ShipPixMaskCacheItem
{
public:
    static PixMask *generate(ShipPixMaskCacheItem item);
    int comp(const ShipPixMaskCacheItem item) const;
    bool operator == (const ShipPixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const ShipPixMaskCacheItem &c) const {return comp(c)<0;};
    guint32 player_id;
    guint32 armyset;
};

//! Helper class for planted standard items in the ImageCache.
/**
 * planted standard images are for when the hero plants a flag on the big map.
 */
class PlantedStandardPixMaskCacheItem
{
public:
    static PixMask *generate(PlantedStandardPixMaskCacheItem item);
    int comp(const PlantedStandardPixMaskCacheItem item) const;
    bool operator == (const PlantedStandardPixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const PlantedStandardPixMaskCacheItem &c) const {return comp(c)<0;};
    guint32 player_id;
    guint32 armyset;
};

//! Helper class for port items in the ImageCache.
/**
 * port images are for the ship loading/unloading points on the big map.
 */
class PortPixMaskCacheItem
{
public:
    static PixMask *generate(PortPixMaskCacheItem item);
    int comp(const PortPixMaskCacheItem item) const;
    bool operator == (const PortPixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const PortPixMaskCacheItem &c) const {return comp(c)<0;};
  guint32 cityset;
};

//! Helper class for signpost items in the ImageCache.
/**
 * signpost images are for the signs on the big map.
 */
class SignpostPixMaskCacheItem
{
public:
    static PixMask *generate(SignpostPixMaskCacheItem item);
    int comp(const SignpostPixMaskCacheItem item) const;
    bool operator == (const SignpostPixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const SignpostPixMaskCacheItem &c) const {return comp(c)<0;};
  guint32 cityset;
};

//! Helper class for bag items in the ImageCache.
/**
 * Bags are the things that hold item objects on the big map.
 * There is one bag image per army set.
 */
class BagPixMaskCacheItem
{
public:
    static PixMask *generate(BagPixMaskCacheItem item);
    int comp(const BagPixMaskCacheItem item) const;
    bool operator == (const BagPixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const BagPixMaskCacheItem &c) const {return comp(c)<0;};
    guint32 armyset;
};

//! Helper class for explosion items in the ImageCache.
/**
 * Explosion images appear on the big map and in the fight window.
 * Sometimes they appear in a 2x2 tile size, and sometimes in a 1x1 tile size.
 */
class ExplosionPixMaskCacheItem
{
public:
    static PixMask *generate(ExplosionPixMaskCacheItem item);
    int comp(const ExplosionPixMaskCacheItem item) const;
    bool operator == (const ExplosionPixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const ExplosionPixMaskCacheItem &c) const {return comp(c)<0;};
    guint32 tileset;
};

//! Helper class for "new level" items in the ImageCache.
/**
 * New Level images are full-body images of the hero who is levelling up.  There
 * is a male image and a female image.
 */
class NewLevelPixMaskCacheItem
{
public:
    static PixMask *generate(NewLevelPixMaskCacheItem item);
    int comp(const NewLevelPixMaskCacheItem item) const;
    bool operator == (const NewLevelPixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const NewLevelPixMaskCacheItem &c) const {return comp(c)<0;};
    guint32 player_id;
    guint32 gender;
};

//! Helper class for default tile style items in the ImageCache.
/**
 * "default tile style" images are the black and white representations of TileStyle::Type.
 */
class DefaultTileStylePixMaskCacheItem
{
public:
    static PixMask *generate(DefaultTileStylePixMaskCacheItem item);
    int comp(const DefaultTileStylePixMaskCacheItem item) const;
    bool operator == (const DefaultTileStylePixMaskCacheItem &c) {return !comp(c);};
    bool operator < (const DefaultTileStylePixMaskCacheItem &c) const {return comp(c)<0;};
public:
    guint32 tilestyle_type;
    guint32 tilesize;
};
#endif
