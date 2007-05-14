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

#include "rectangle.h"

#include "GraphicsCache.h"
#include "armysetlist.h"
#include "army.h"
#include "playerlist.h"
#include "Configuration.h"
#include "File.h"
#include "GameMap.h"
#include "city.h"
#include "stack.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

//some structures first

//this structure is the base class for storing cached objects. It stores the
//(army-, but can be extended) set, the type of army and the player which the
//surface is designed for (and the surface itself, of course).
struct ArmyCacheItem
{
    Uint32 armyset;
    Uint32 index;
    const Player* player;
    int level;
    bool medals[3];
    SDL_Surface* surface;
};

//the structure to store temples in
struct TempleCacheItem
{
    int type;
    SDL_Surface* surface;
};

//the structure to store roads in
struct RoadCacheItem
{
    int type;
    SDL_Surface* surface;
};

//the structure to store stones in
struct StoneCacheItem
{
    int type;
    SDL_Surface* surface;
};

//the structure to store buildings in
struct CityCacheItem
{
    int type;
    const Player* player;
    SDL_Surface* surface;
};

// the structure to store flags in
struct FlagCacheItem
{
    Uint32 size;
    const Player* player;
    SDL_Surface* surface;
};

// the structure to store selector images in
struct SelectorCacheItem
{
    Uint32 type;
    Uint32 frame;
    const Player* player;
    SDL_Surface* surface;
};

// the structure to store shield images in
struct ShieldCacheItem
{
    Uint32 type;
    const Player* player;
    SDL_Surface* surface;
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
    loadTemplePics();
    loadStonePics();
    loadRoadPics();
    loadFlags();
    loadSelectors();
    loadShields();

    d_levelmask = File::getMiscPicture("level_mask.png");
    d_medalsmask = File::getMiscPicture("medals_mask.gif");
}

GraphicsCache::~GraphicsCache()
{
    clear();

    for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
    {
        if (d_citypic[i])
            SDL_FreeSurface(d_citypic[i]);

        if (d_razedpic[i])
            SDL_FreeSurface(d_razedpic[i]);
    }

    for (int i = 0; i < 8; i++)
    {
        SDL_FreeSurface(d_flagpic[i]);
        SDL_FreeSurface(d_flagmask[i]);
    }

    for (int i = 0; i < 6; i++)
    {
        SDL_FreeSurface(d_selector[i]);
        SDL_FreeSurface(d_selectormask[i]);
    }

    for (int i = 0; i < 4; i++)
    {
        SDL_FreeSurface(d_smallselector[i]);
        SDL_FreeSurface(d_smallselectormask[i]);
    }

    for (int i = 0; i < 2; i++)
    {
      for (unsigned int j = 0; j < MAX_PLAYERS + 1; j++)
	{
          SDL_FreeSurface(d_shieldpic[i][j]);
          SDL_FreeSurface(d_shieldmask[i][j]);
	}
    }

    SDL_FreeSurface(d_levelmask);
    SDL_FreeSurface(d_medalsmask);
}

SDL_Surface* GraphicsCache::getArmyPic(Uint32 armyset, Uint32 army, const Player* p,
                                       int level, const bool *medals)
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
    if (!p)
        p = Playerlist::getInstance()->getNeutral();
    
    for (it =d_armylist.begin(); it != d_armylist.end(); it++)
    {
        if (((*it)->armyset == armyset) && ((*it)->index == army)
            && ((*it)->player ==p) && ((*it)->level==level) 
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
    myitem = addArmyPic(armyset, army, p, level, my_medals);

    return myitem->surface;
}

SDL_Surface* GraphicsCache::getTemplePic(int type)
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

SDL_Surface* GraphicsCache::getRoadPic(int type)
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

SDL_Surface* GraphicsCache::getStonePic(int type)
{
    debug("GraphicsCache::getStonePic " <<type)

    std::list<StoneCacheItem*>::iterator it;
    StoneCacheItem* myitem;

    for (it = d_stonelist.begin(); it != d_stonelist.end(); it++)
    {
        if ((*it)->type == type)
        {
            myitem = (*it);

            //put the item in last place (last touched)
            d_stonelist.erase(it);
            d_stonelist.push_back(myitem);

            return myitem->surface;
        }
    }

    //no item found -> create a new one
    myitem = addStonePic(type);

    return myitem->surface;
}


SDL_Surface* GraphicsCache::getCityPic(const City* city)
{
    if (!city)
        return 0;
    if (city->isBurnt())
      return d_razedpic[city->getPlayer()->getId()];
    else
      return d_citypic[city->getPlayer()->getId()];
}

SDL_Surface* GraphicsCache::getCityPic(int type, const Player* p)
{
    debug("GraphicsCache::getCityPic " <<type <<", player " <<p->getName())

    if (type == -1)
      return d_razedpic[p->getId()];
    else
      return d_citypic[p->getId()];
}

SDL_Surface* GraphicsCache::getFlagPic(const Stack* s)
{
    debug("GraphicsCache::getFlagPic " <<s->getId() <<", player" <<s->getPlayer()->getName())

    if (!s)
    {
        std::cerr <<_("GraphicsCache::getFlagPic: no stack supplied! Exiting...\n");
        exit(-1);
    }
    
    std::list<FlagCacheItem*>::iterator it;
    FlagCacheItem* myitem;

    for (it = d_flaglist.begin(); it != d_flaglist.end(); it++)
    {
        myitem = *it;
        if ((myitem->size == s->size()) && (myitem->player == s->getPlayer()))
        {
            // put the item in last place (last touched)
            d_flaglist.erase(it);
            d_flaglist.push_back(myitem);

            return myitem->surface;
        }
    }

    // no item found => create a new one
    myitem = addFlagPic(s->size(), s->getPlayer());

    return myitem->surface;
}

SDL_Surface* GraphicsCache::getSelectorPic(Uint32 type, Uint32 frame, 
					   const Player *p)
{
    debug("GraphicsCache::getSelectorPic " <<type <<", " << frame << ", player" <<s->getPlayer()->getName())

    if (!p)
    {
        std::cerr <<_("GraphicsCache::getSelectorPic: no player supplied! Exiting...\n");
        exit(-1);
    }
    
    std::list<SelectorCacheItem*>::iterator it;
    SelectorCacheItem* myitem;

    for (it = d_selectorlist.begin(); it != d_selectorlist.end(); it++)
    {
        myitem = *it;
        if ((myitem->type == type) && (myitem->player == p) 
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

SDL_Surface* GraphicsCache::getShieldPic(Uint32 type, const Player *p)
{
    debug("GraphicsCache::getShieldPic " <<type <<", " << ", player" <<s->getPlayer()->getName())

    if (!p)
    {
        std::cerr <<_("GraphicsCache::getShieldPic: no player supplied! Exiting...\n");
        exit(-1);
    }
    
    std::list<ShieldCacheItem*>::iterator it;
    ShieldCacheItem* myitem;

    for (it = d_shieldlist.begin(); it != d_shieldlist.end(); it++)
    {
        myitem = *it;
        if ((myitem->type == type) && (myitem->player == p))
        {
            // put the item in last place (last touched)
            d_shieldlist.erase(it);
            d_shieldlist.push_back(myitem);

            return myitem->surface;
        }
    }

    // no item found => create a new one
    myitem = addShieldPic(type, p);

    return myitem->surface;
}


SDL_Surface* GraphicsCache::applyMask(SDL_Surface* image, SDL_Surface* mask, const Player* p)
{
    if (!mask || mask->w != image->w || mask->h != image->h)
    {
        // we are expected to produce some output and a missing/wrong mask is no
        // critical error, so just copy the original image.
        std::cerr <<"Warning: mask and original image do not match\n";
        return SDL_DisplayFormatAlpha(image);
    }

    SDL_Surface* s = SDL_DisplayFormatAlpha(image);

    // Now copy the mask. The player's mask colors, that denote how important the
    // corresponding color is, are used to additionally shift the colors for the pixels
    // in the mask (see the code :) ). That way, if the player is e.g. blue, the red
    // and green components of the mask are shifted by an additional 8 bits to the
    // right so that in the end the mask pixels all only consist of different blue
    // colors.
        
    SDL_Color c = p->getMaskColor();

    mask->format->Rshift += c.r;
    mask->format->Gshift += c.g;
    mask->format->Bshift += c.b;

    // copy the mask image over the original image
    SDL_BlitSurface(mask, 0, s, 0);

    // set everything back
    mask->format->Rshift -= c.r;
    mask->format->Gshift -= c.g;
    mask->format->Bshift -= c.b;
 
    return s;
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
    Uint32 maxcache = Configuration::s_cacheSize;
    if (maxcache < (1<<21))
        maxcache = (1<<21);
    
    if (d_cachesize < maxcache)
        return;
    

    // Now the cache size has been exceeded. We try to guarantee the values
    // given above and reduce the number of images. Let us start with the
    // cities
   
    while (d_citylist.size() > 10)
        eraseLastCityItem();

    while (d_templelist.size() > 10)
        eraseLastTempleItem();

    while (d_stonelist.size() > 10)
        eraseLastStoneItem();

    while (d_roadlist.size() > 10)
        eraseLastRoadItem();

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

    // still not enough? Erase army images
    while (d_armylist.size() > 40)
        eraseLastArmyItem();
}

ArmyCacheItem* GraphicsCache::addArmyPic(Uint32 armyset, Uint32 army,
                        const Player* p, int level, const bool *medalsbonus)
{
    debug("ADD army pic: " <<armyset <<"," <<army)

    ArmyCacheItem* myitem = new ArmyCacheItem();
    myitem->armyset = armyset;
    myitem->index = army;
    myitem->player = p;
    myitem->level = level;
    myitem->medals[0] = medalsbonus[0];
    myitem->medals[1] = medalsbonus[1];
    myitem->medals[2] = medalsbonus[2];

    //Now the most important part: load the army picture
    //First, copy the army picture and change it to the display format
    const Army* basearmy = Armysetlist::getInstance()->getArmy(armyset, army);
    
    // copy the pixmap including player colors
    myitem->surface = applyMask(basearmy->getPixmap(), basearmy->getMask(), p);

    if (level > 1 && d_levelmask)
    {
        SDL_Surface* mask = SDL_CreateRGBSurface(SDL_SWSURFACE, 40, 40,
                              d_levelmask->format->BitsPerPixel,0,0,0,0);
        // a little hack while waiting for a complete level picture
        if (level<26) 
        {
	    SDL_Rect r;
	    r.x = 40*(level-2);
	    r.y = 0;
	    r.w = r.h = 40;
            SDL_BlitSurface(d_levelmask, &r, mask, 0);
        }
        else 
        {
	    SDL_Rect r;
	    r.x = 160;
	    r.y = 0;
	    r.w = r.h = 40;
            SDL_BlitSurface(d_levelmask, &r, mask, 0);
        }
      
        //set the first pixel as alpha value
        SDL_SetColorKey(mask, SDL_SRCCOLORKEY, 0);

        //blit mask over the army pic
        SDL_BlitSurface(mask,0, myitem->surface, 0);

        //free the temporary surface
        SDL_FreeSurface(mask);
    }

    if (d_medalsmask && medalsbonus != NULL)
    {
        debug("medalsbonus============= " << medalsbonus); 
        for(int i=0;i<3;i++)
        { 
            if (medalsbonus[i])
            {
                SDL_Surface* mask = SDL_CreateRGBSurface(SDL_SWSURFACE, 40, 40,
                d_medalsmask->format->BitsPerPixel,0,0,0,0);

                // a little hack while waiting for a complete medals picture
		SDL_Rect r;
		r.x = 40*i;
		r.y = 0;
		r.w = r.h = 40;
                SDL_BlitSurface(d_medalsmask, &r, mask, 0);
      
                //set the first pixel as alpha value
                SDL_SetColorKey(mask, SDL_SRCCOLORKEY, 0);

                //blit mask over the army pic
                SDL_BlitSurface(mask,0, myitem->surface, 0);

                //free the temporary surface
                SDL_FreeSurface(mask);
            }
        }
    }

    //now the final preparation steps:
    //a) add the size
    int size = myitem->surface->w * myitem->surface->h;
    d_cachesize += myitem->surface->format->BytesPerPixel * size;

    //b) add the entry to the list
    d_armylist.push_back(myitem);

    //c) check if the cache size is too large
    checkPictures();

    //we are finished, so return the pic
    return myitem;
}

TempleCacheItem* GraphicsCache::addTemplePic(int type)
{
    //    int ts = GameMap::getInstance()->getTileSet()->getTileSize();
    
    SDL_Surface* mysurf = SDL_DisplayFormatAlpha(d_templepic[type]);

    //now create the cache item and add the size
    TempleCacheItem* myitem = new TempleCacheItem();
    myitem->type = type;
    myitem->surface = mysurf;

    d_templelist.push_back(myitem);

    //add the size
    int size = mysurf->w * mysurf->h;
    d_cachesize += size * mysurf->format->BytesPerPixel;

    //and check the size of the cache
    checkPictures();

    return myitem;
}

RoadCacheItem* GraphicsCache::addRoadPic(int type)
{
    //    int ts = GameMap::getInstance()->getTileSet()->getTileSize();
    
    SDL_Surface* mysurf = SDL_DisplayFormatAlpha(d_roadpic[type]);

    //now create the cache item and add the size
    RoadCacheItem* myitem = new RoadCacheItem();
    myitem->type = type;
    myitem->surface = mysurf;

    d_roadlist.push_back(myitem);

    //add the size
    int size = mysurf->w * mysurf->h;
    d_cachesize += size * mysurf->format->BytesPerPixel;

    //and check the size of the cache
    checkPictures();

    return myitem;
}

StoneCacheItem* GraphicsCache::addStonePic(int type)
{
    //    int ts = GameMap::getInstance()->getTileSet()->getTileSize();
    
    SDL_Surface* mysurf = SDL_DisplayFormatAlpha(d_stonepic[type]);

    //now create the cache item and add the size
    StoneCacheItem* myitem = new StoneCacheItem();
    myitem->type = type;
    myitem->surface = mysurf;

    d_stonelist.push_back(myitem);

    //add the size
    int size = mysurf->w * mysurf->h;
    d_cachesize += size * mysurf->format->BytesPerPixel;

    //and check the size of the cache
    checkPictures();

    return myitem;
}

CityCacheItem* GraphicsCache::addCityPic(int type, const Player* p)
{
    //now create the cache item and add the size
    CityCacheItem* myitem = new CityCacheItem();
    myitem->player = p;
    myitem->type = type;
    myitem->surface = d_citypic[p->getId()];

    d_citylist.push_back(myitem);

    //add the size
    int size = d_citypic[p->getId()]->w * d_citypic[p->getId()]->h;
    d_cachesize += size * d_citypic[p->getId()]->format->BytesPerPixel;

    //and check the size of the cache
    checkPictures();

    return myitem;
}

FlagCacheItem* GraphicsCache::addFlagPic(int size, const Player* p)
{
    debug("GraphicsCache::addFlagPic, player="<<p->getName()<<", size="<<size)
    
    // size is the size of the stack, but we need the index, which starts at 0
    size--;
    
    SDL_Surface* mysurf = applyMask(d_flagpic[size], d_flagmask[size], p);
        
    //now create the cache item and add the size
    FlagCacheItem* myitem = new FlagCacheItem();
    myitem->player = p;
    myitem->size = size;
    myitem->surface = mysurf;

    d_flaglist.push_back(myitem);

    //add the size
    int picsize = mysurf->w * mysurf->h;
    d_cachesize += picsize * mysurf->format->BytesPerPixel;

    //and check the size of the cache
    checkPictures();

    return myitem;
}

SelectorCacheItem* GraphicsCache::addSelectorPic(Uint32 type, Uint32 frame, const Player* p)
{
    debug("GraphicsCache::addSelectorPic, player="<<p->getName()<<", type="<<size << ", " << frame)
    
    // frame is the frame of animation we're looking for.  starts at 0.
    // type is 0 for big, 1 for small
    
    SDL_Surface* mysurf;
    if (type == 0)
      mysurf = applyMask(d_selector[frame], d_selectormask[frame], p);
    else
      mysurf = applyMask(d_smallselector[frame], d_smallselectormask[frame], p);
        
    //now create the cache item and add the size
    SelectorCacheItem* myitem = new SelectorCacheItem();
    myitem->player = p;
    myitem->type = type;
    myitem->frame = frame;
    myitem->surface = mysurf;

    d_selectorlist.push_back(myitem);

    //add the size
    int picsize = mysurf->w * mysurf->h;
    d_cachesize += picsize * mysurf->format->BytesPerPixel;

    //and check the size of the cache
    checkPictures();

    return myitem;
}

ShieldCacheItem* GraphicsCache::addShieldPic(Uint32 type, const Player* p)
{
    debug("GraphicsCache::addShieldPic, player="<<p->getName()<<", type="<<size)
    
    // type is 0 for small, 1 for medium
    
    SDL_Surface* mysurf;
    mysurf = applyMask(d_shieldpic[type][p->getId()], 
		       d_shieldmask[type][p->getId()], p);
        
    //now create the cache item and add the size
    ShieldCacheItem* myitem = new ShieldCacheItem();
    myitem->player = p;
    myitem->type = type;
    myitem->surface = mysurf;

    d_shieldlist.push_back(myitem);

    //add the size
    int picsize = mysurf->w * mysurf->h;
    d_cachesize += picsize * mysurf->format->BytesPerPixel;

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

    while (!d_stonelist.empty())
        eraseLastStoneItem();

    while (!d_roadlist.empty())
        eraseLastRoadItem();

    while (!d_citylist.empty())
        eraseLastCityItem();

    while (!d_flaglist.empty())
        eraseLastFlagItem();

    while (!d_selectorlist.empty())
        eraseLastSelectorItem();

    while (!d_shieldlist.empty())
        eraseLastShieldItem();
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
    int size = myitem->surface->w * myitem->surface->h;
    d_cachesize -= myitem->surface->format->BytesPerPixel * size;

    SDL_FreeSurface(myitem->surface);
    delete myitem;
}

void GraphicsCache::eraseLastTempleItem()
{
    if (d_templelist.empty())
        return;

    TempleCacheItem* myitem = *(d_templelist.begin());
    d_templelist.erase(d_templelist.begin());

    int size = myitem->surface->w * myitem->surface->h;
    d_cachesize -= myitem->surface->format->BytesPerPixel * size;

    SDL_FreeSurface(myitem->surface);
    delete myitem;
}

void GraphicsCache::eraseLastRoadItem()
{
    if (d_roadlist.empty())
        return;

    RoadCacheItem* myitem = *(d_roadlist.begin());
    d_roadlist.erase(d_roadlist.begin());

    int size = myitem->surface->w * myitem->surface->h;
    d_cachesize -= myitem->surface->format->BytesPerPixel * size;

    SDL_FreeSurface(myitem->surface);
    delete myitem;
}

void GraphicsCache::eraseLastStoneItem()
{
    if (d_stonelist.empty())
        return;

    StoneCacheItem* myitem = *(d_stonelist.begin());
    d_stonelist.erase(d_stonelist.begin());

    int size = myitem->surface->w * myitem->surface->h;
    d_cachesize -= myitem->surface->format->BytesPerPixel * size;

    SDL_FreeSurface(myitem->surface);
    delete myitem;
}

void GraphicsCache::eraseLastCityItem()
{
    if (d_citylist.empty())
        return;

    CityCacheItem* myitem = *(d_citylist.begin());
    d_citylist.erase(d_citylist.begin());

    int size = myitem->surface->w * myitem->surface->h;
    d_cachesize -= myitem->surface->format->BytesPerPixel * size;

    SDL_FreeSurface(myitem->surface);
    delete myitem;
}

void GraphicsCache::eraseLastFlagItem()
{
    if (d_flaglist.empty())
        return;

    FlagCacheItem* myitem = *(d_flaglist.begin());
    d_flaglist.erase(d_flaglist.begin());

    int size = myitem->surface->w * myitem->surface->h;
    d_cachesize -= myitem->surface->format->BytesPerPixel * size;

    SDL_FreeSurface(myitem->surface);
    delete myitem;
}

void GraphicsCache::eraseLastSelectorItem()
{
    if (d_selectorlist.empty())
        return;

    SelectorCacheItem* myitem = *(d_selectorlist.begin());
    d_selectorlist.erase(d_selectorlist.begin());

    int size = myitem->surface->w * myitem->surface->h;
    d_cachesize -= myitem->surface->format->BytesPerPixel * size;

    SDL_FreeSurface(myitem->surface);
    delete myitem;
}

void GraphicsCache::eraseLastShieldItem()
{
    if (d_shieldlist.empty())
        return;

    ShieldCacheItem* myitem = *(d_shieldlist.begin());
    d_shieldlist.erase(d_shieldlist.begin());

    int size = myitem->surface->w * myitem->surface->h;
    d_cachesize -= myitem->surface->format->BytesPerPixel * size;

    SDL_FreeSurface(myitem->surface);
    delete myitem;
}

void GraphicsCache::loadTemplePics()
{
    // GameMap has the actual tileset stored
    std::string tileset = GameMap::getInstance()->getTileSet()->getName();
    int ts = GameMap::getInstance()->getTileSet()->getTileSize();
   
    // load the city pictures
    SDL_Surface* templepics = File::getMapsetPicture(tileset, "misc/temples.png");

    // copy alpha values, don't use them
    SDL_SetAlpha(templepics, 0, 0);
    
    for (unsigned int i = 0; i < TEMPLE_TYPES ; i++)
    {
        //copy the city image...
        SDL_Surface* tmp;
        SDL_PixelFormat* fmt = templepics->format;
        
        tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, ts, ts, fmt->BitsPerPixel,
                            fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

	SDL_Rect r;
	r.x = i*ts;
	r.y = 0;
	r.w = r.h = ts;
        SDL_BlitSurface(templepics, &r, tmp, 0);

        d_templepic[i] = SDL_DisplayFormatAlpha(tmp);

        SDL_FreeSurface(tmp);
    }

    SDL_FreeSurface(templepics);
}

void GraphicsCache::loadRoadPics()
{
    // GameMap has the actual tileset stored
    std::string tileset = GameMap::getInstance()->getTileSet()->getName();
    int ts = GameMap::getInstance()->getTileSet()->getTileSize();
   
    // load the road pictures
    SDL_Surface* roadpics = File::getMapsetPicture(tileset, "misc/roads.png");

    // copy alpha values, don't use them
    SDL_SetAlpha(roadpics, 0, 0);
    
    for (unsigned int i = 0; i < ROAD_TYPES ; i++)
    {
        //copy the road image...
        SDL_Surface* tmp;
        SDL_PixelFormat* fmt = roadpics->format;
        
        tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, ts, ts, fmt->BitsPerPixel,
                            fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

	SDL_Rect r;
	r.x = i*ts;
	r.y = 0;
	r.w = r.h = ts;
        SDL_BlitSurface(roadpics, &r, tmp, 0);

        d_roadpic[i] = SDL_DisplayFormatAlpha(tmp);

        SDL_FreeSurface(tmp);
    }

    SDL_FreeSurface(roadpics);
}

void GraphicsCache::loadStonePics()
{
    // GameMap has the actual tileset stored
    std::string tileset = GameMap::getInstance()->getTileSet()->getName();
    int ts = GameMap::getInstance()->getTileSet()->getTileSize();
   
    // load the city pictures
    SDL_Surface* stonepics = File::getMapsetPicture(tileset, "misc/stones.png");

    // copy alpha values, don't use them
    SDL_SetAlpha(stonepics, 0, 0);
    
    for (unsigned int i = 0; i < STONE_TYPES ; i++)
    {
        //copy the city image...
        SDL_Surface* tmp;
        SDL_PixelFormat* fmt = stonepics->format;
        
        tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, ts, ts, fmt->BitsPerPixel,
                            fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

	SDL_Rect r;
	r.x = i*ts;
	r.y = 0;
	r.w = r.h = ts;
        SDL_BlitSurface(stonepics, &r, tmp, 0);

        d_stonepic[i] = SDL_DisplayFormatAlpha(tmp);

        SDL_FreeSurface(tmp);
    }

    SDL_FreeSurface(stonepics);
}

void GraphicsCache::loadCityPics()
{
    SDL_Surface *tmp;
    SDL_PixelFormat *fmt;
    // GameMap has the actual tileset stored
    std::string tileset = GameMap::getInstance()->getTileSet()->getName();
    int size = GameMap::getInstance()->getTileSet()->getTileSize() * 2;
   
    // load the image for the razed city
    SDL_Surface* razedpics = File::getMapsetPicture(tileset, "misc/castle_razed.png");
    // copy alpha values, don't use them
    SDL_SetAlpha(razedpics, 0, 0);

    // temporary surface
    fmt = razedpics->format;
    tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, size, size, fmt->BitsPerPixel,
                        fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
    
    for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
    {
        //copy the razed city image...

        SDL_Rect r;
	r.x = i * size;
	r.y = 0;
	r.w = r.h = size;
        SDL_BlitSurface(razedpics, &r, tmp, 0);

        d_razedpic[i] = SDL_DisplayFormatAlpha(tmp);
    }

    SDL_FreeSurface(tmp);
    SDL_FreeSurface(razedpics);
    
    // load the city pictures
    SDL_Surface* citypics = File::getMapsetPicture(tileset, "misc/castles.png");

    // copy alpha values, don't use them
    SDL_SetAlpha(citypics, 0, 0);

    // temporary surface
    fmt = citypics->format;
    tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, size, size, fmt->BitsPerPixel,
                        fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
    
    for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
    {
        //copy the city image...

	SDL_Rect r;
	r.x = i*size;
	r.y = 0;
	r.w = r.h = size;
        SDL_BlitSurface(citypics, &r, tmp, 0);

        d_citypic[i] = SDL_DisplayFormatAlpha(tmp);
#if 0

        //...and copy the mask image
        d_citymask[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, size, size,
                        32, 0xFF000000, 0xFF0000, 0xFF00, 0xFF);

	r.y = size;
        SDL_BlitSurface(citypics, &r, d_citymask[i], 0);
#endif
    }

    SDL_FreeSurface(tmp);
    SDL_FreeSurface(citypics);
}

void GraphicsCache::loadSelectors()
{
    //load the big selector pictures
    int i;
    SDL_Rect selrect;
    SDL_Surface* selpics = File::getMiscPicture("selector.png");
    // copy alpha values, don't use them
    SDL_SetAlpha(selpics, 0, 0);
    SDL_PixelFormat* fmt = selpics->format;
    int size = GameMap::getInstance()->getTileSet()->getTileSize();
    for (i = 0; i < 6; i++)
      {
        SDL_Surface* tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, size, size, 
    						fmt->BitsPerPixel, fmt->Rmask, 
    						fmt->Gmask, fmt->Bmask, 
						fmt->Amask);
	selrect.x = i*size;
	selrect.y = 0;
	selrect.w = selrect.h = size;
	SDL_BlitSurface(selpics, &selrect, tmp, 0);
	d_selector[i] = SDL_DisplayFormatAlpha(tmp);
	SDL_FreeSurface(tmp);

	d_selectormask[i]=  SDL_CreateRGBSurface(SDL_SWSURFACE, size, size, 32,
						 0xFF000000, 0xFF0000, 0xFF00, 
						 0xFF);
	//selrect.SetRect(i*size, size, size, size);
	selrect.x = i*size;
	selrect.y = 0;
	selrect.w = selrect.h = size;
	SDL_BlitSurface(selpics, &selrect, d_selectormask[i], 0);

      }
    SDL_FreeSurface(selpics);
    //load the small selector pictures
    selpics = File::getMiscPicture("small_selector.png");
    // copy alpha values, don't use them
    SDL_SetAlpha(selpics, 0, 0);
    fmt = selpics->format;
    for (i = 0; i < 4; i++)
      {
        SDL_Surface* tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, size, size, 
    						fmt->BitsPerPixel, fmt->Rmask, 
    						fmt->Gmask, fmt->Bmask, 
						fmt->Amask);
	selrect.x = i*size;
	selrect.y = 0;
	selrect.w = selrect.h = size;
	SDL_BlitSurface(selpics, &selrect, tmp, 0);
	d_smallselector[i] = SDL_DisplayFormatAlpha(tmp);
	SDL_FreeSurface(tmp);

	d_smallselectormask[i]=  SDL_CreateRGBSurface(SDL_SWSURFACE, size, size,
						      32, 0xFF000000, 0xFF0000,
						      0xFF00, 0xFF);
	selrect.x = i*size;
	selrect.y = 0;
	selrect.w = selrect.h = size;
	SDL_BlitSurface(selpics, &selrect, d_selectormask[i], 0);

      }
    SDL_FreeSurface(selpics);
}

void GraphicsCache::loadShields()
{
    //load the small shieldset
    unsigned int i;
    SDL_Rect shieldrect;
    SDL_Surface* shieldpics = File::getMiscPicture("shieldsetsmall.png");
    // copy alpha values, don't use them
    SDL_SetAlpha(shieldpics, 0, 0);
    SDL_PixelFormat* fmt = shieldpics->format;
    int size = 8; /*FIXME hardcoded size, should be in .defs? */
    for (i = 0; i < MAX_PLAYERS + 1; i++)
      {
        SDL_Surface* tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, size, size, 
    						fmt->BitsPerPixel, fmt->Rmask, 
    						fmt->Gmask, fmt->Bmask, 
						fmt->Amask);
	shieldrect.x = i*size;
	shieldrect.y = 0;
	shieldrect.w = shieldrect.h = size;
	SDL_BlitSurface(shieldpics, &shieldrect, tmp, 0);
	d_shieldpic[0][i] = SDL_DisplayFormatAlpha(tmp);
	SDL_FreeSurface(tmp);

	d_shieldmask[0][i]=  SDL_CreateRGBSurface(SDL_SWSURFACE, size, size, 32,
						  0xFF000000, 0xFF0000, 0xFF00, 
						  0xFF);
	shieldrect.x = i*size;
	shieldrect.y = 0;
	shieldrect.w = shieldrect.h = size;
	SDL_BlitSurface(shieldpics, &shieldrect, d_shieldmask[0][i], 0);

      }
    SDL_FreeSurface(shieldpics);
    //load the medium shieldset
    shieldpics = File::getMiscPicture("shieldsetmedium.png");
    // copy alpha values, don't use them
    SDL_SetAlpha(shieldpics, 0, 0);
    fmt = shieldpics->format;
    int xsize = 11;
    int ysize = 14;
    for (i = 0; i < MAX_PLAYERS + 1; i++)
      {
        SDL_Surface* tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, xsize, ysize, 
    						fmt->BitsPerPixel, fmt->Rmask, 
    						fmt->Gmask, fmt->Bmask, 
						fmt->Amask);
	shieldrect.x = i*xsize;
	shieldrect.y = 0;
	shieldrect.w = xsize;
	shieldrect.h = ysize;
	SDL_BlitSurface(shieldpics, &shieldrect, tmp, 0);
	d_shieldpic[1][i] = SDL_DisplayFormatAlpha(tmp);
	SDL_FreeSurface(tmp);

	d_shieldmask[1][i]=  SDL_CreateRGBSurface(SDL_SWSURFACE, size, size,
					          32, 0xFF000000, 0xFF0000,
						  0xFF00, 0xFF);
	shieldrect.x = i*xsize;
	shieldrect.y = 0;
	shieldrect.w = xsize;
	shieldrect.h = ysize;
	SDL_BlitSurface(shieldpics, &shieldrect, d_shieldmask[1][i], 0);

      }
    SDL_FreeSurface(shieldpics);
}

void GraphicsCache::loadFlags()
{
    //GameMap has the actual tileset stored
    std::string tileset = GameMap::getInstance()->getTileSet()->getName();
    
    // to build flags, we need these three images as basic blocks
    SDL_Surface* flag = File::getMapsetPicture(tileset, "misc/flags.png");
    SDL_PixelFormat* fmt = flag->format;
    int tilesize = GameMap::getInstance()->getTileSet()->getTileSize();
    
    // copy alpha values, don't use them!
    SDL_SetAlpha(flag, 0, 0);
    
    for (int i = 0; i < 8; i++)
    {
        // first, create the flag image; create a temporary surface...
        SDL_Surface* tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, tilesize, tilesize, 
                                    fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask,
                                    fmt->Bmask, fmt->Amask);
        
        // blit the correct flag on the top of the image
	SDL_Rect flagrect;
	flagrect.x = i*tilesize;
	flagrect.y = 0;
	flagrect.w = flagrect.h = tilesize;
        SDL_BlitSurface(flag, &flagrect, tmp, 0);

        // convert the surface to screen resolution
        d_flagpic[i] = SDL_DisplayFormatAlpha(tmp);

        // free the temporary surface
        SDL_FreeSurface(tmp);


        // now create the masks
        d_flagmask[i]=  SDL_CreateRGBSurface(SDL_SWSURFACE, tilesize, tilesize,
                            32, 0xFF000000, 0xFF0000, 0xFF00, 0xFF);
        flagrect.y = tilesize;
        SDL_BlitSurface(flag, &flagrect, d_flagmask[i], 0);
    }

    // free the temporary surfaces
    SDL_FreeSurface(flag);
}
