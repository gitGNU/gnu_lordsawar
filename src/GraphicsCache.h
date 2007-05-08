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
struct CityCacheItem;
struct TempleCacheItem;
struct StoneCacheItem;
struct RoadCacheItem;
struct FlagCacheItem;
struct SelectorCacheItem;
class City;

/** Soliton class for caching army and map images
  * 
  * With the introduction of player-specific colors, the problem of caching
  * images has popped up. The player colors are implemented by taking an army
  * picture and a mask, with the mask being a 16 color image, substituting
  * the colors in the mask and blitting the mask over the army (or e.g. city)
  * image. This takes several blits (>3, there are also things like medals and
  * level of the unit to be considered) and is therefore costly.
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
          * @param level        the level of the unit
          * @return the image of the unit
          */
        SDL_Surface* getArmyPic(Uint32 armyset, Uint32 army, const Player* p,
                                int level, const bool* medals);

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
        SDL_Surface* getTemplePic(int type);
        /** Function for getting a temple picture
          *
          * Most often, we don't need such a sophisticated function. So just
          * supply the city instance and be happy. :)
          *
          * @param type         the type of the temple
          * @return image of the temple
          */
        SDL_Surface* getRoadPic(int type);
        /** Function for getting a road picture
          *
          * @param type         the type of the road
          * @return image of the road
          */
        SDL_Surface* getStonePic(int type);
        /** Function for getting a stone picture
          *
          * Most often, we don't need such a sophisticated function. So just
          * supply the stone type and be happy. :)
          *
          * @param type         the type of the stone
          * @return image of the stone
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
        SDL_Surface* getSelectorPic(Uint32 type, const Player* p);

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

        //! Creates a new temple picture with the given parametres.
        TempleCacheItem* addTemplePic(int type);

        //! Creates a new stone picture with the given parametres.
        StoneCacheItem* addStonePic(int type);

        //! Creates a new road picture with the given parametres.
        RoadCacheItem* addRoadPic(int type);

        //! Creates a new army picture with the given parametres.
        ArmyCacheItem* addArmyPic(Uint32 armyset, Uint32 army, const Player* p,
                                  int level, const bool* medalsbonus);

        //! Creates a new city picture with the given parametres.
        CityCacheItem* addCityPic(int type, const Player* p);

        //! Creates a new flag picture with the given parametres.
        FlagCacheItem* addFlagPic(int size, const Player* p);

        //! Creates a new selector picture with the given parameters.
        SelectorCacheItem* addSelectorPic(Uint32 type, const Player* p);

        //! Checks if the cache has exceeded the maximum size and reduce it.
        void checkPictures();
        
        //! Erases the oldest (least recently requested) army cache item.
        void eraseLastArmyItem();

        //! Erases the oldest (least recently requested) temple cache item.
        void eraseLastTempleItem();

        //! Erases the oldest (least recently requested) stone cache item.
        void eraseLastStoneItem();

        //! Erases the oldest (least recently requested) road cache item.
        void eraseLastRoadItem();

        //! Erases the oldest (least recently requested) city cache item.
        void eraseLastCityItem();

        //! Erases the oldest flag cache item
        void eraseLastFlagItem();

        //! Erases the oldest selector cache item
        void eraseLastSelectorItem();

        //! Loads the images for the city pictures and their masks.
        void loadCityPics();

        //! Loads the images for the city pictures and their masks.
        void loadTemplePics();

        //! Loads the images for the stone pictures and their masks.
        void loadStonePics();

        //! Loads the images for the stone pictures and their masks.
        void loadRoadPics();

        //! Loads the images for the flags
        void loadFlags();

        //! Loads the images for the selector
        void loadSelector();
        
        //the data
        static GraphicsCache* s_instance;

        Uint32 d_cachesize;
        std::list<ArmyCacheItem*> d_armylist;
        std::list<CityCacheItem*> d_citylist;
        std::list<FlagCacheItem*> d_flaglist;
        std::list<TempleCacheItem*> d_templelist;
        std::list<StoneCacheItem*> d_stonelist;
        std::list<RoadCacheItem*> d_roadlist;
        std::list<SelectorCacheItem*> d_selectorlist;

        //some private surfaces
        SDL_Surface* d_levelmask;
        SDL_Surface* d_medalsmask;
        SDL_Surface* d_citypic[MAX_PLAYERS + 1]; //+1 for neutral
        SDL_Surface* d_templepic[TEMPLE_TYPES];
        SDL_Surface* d_stonepic[STONE_TYPES];
        SDL_Surface* d_roadpic[ROAD_TYPES];
        SDL_Surface* d_razedpic[MAX_PLAYERS + 1]; //+1 for neutral
        SDL_Surface* d_flagpic[8];
        SDL_Surface* d_flagmask[8];
	SDL_Surface* d_selector[6];
	SDL_Surface* d_selectormask[6];
};

#endif
