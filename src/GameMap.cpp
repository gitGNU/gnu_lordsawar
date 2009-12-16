// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2003, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
// Copyright (C) 2008 Janek Kozicki
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

#include <sstream>
#include <iostream>
#include <iomanip>
#include <assert.h>
#include <sigc++/functors/mem_fun.h>
#include <string.h>

#include "ucompose.hpp"
#include "GameMap.h"
#include "citylist.h"
#include "bridgelist.h"
#include "bridge.h"
#include "portlist.h"
#include "port.h"
#include "roadlist.h"
#include "road.h"
#include "city.h"
#include "ruin.h"
#include "temple.h"
#include "playerlist.h"
#include "stacklist.h"
#include "ruinlist.h"
#include "templelist.h"
#include "signpostlist.h"
#include "xmlhelper.h"
#include "MapGenerator.h"
#include "tilesetlist.h"
#include "shieldsetlist.h"
#include "citysetlist.h"
#include "GraphicsCache.h"
#include "MapBackpack.h"
#include "stacktile.h"
#include "armyprodbase.h"
#include "stack.h"
#include "armyset.h"
#include "armysetlist.h"
#include "CreateScenario.h"

std::string GameMap::d_tag = "map";
std::string GameMap::d_itemstack_tag = "itemstack";
using namespace std;

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<flush;}
#define debug(x)

GameMap* GameMap::s_instance = 0;

int GameMap::s_width = 112;
int GameMap::s_height = 156;

GameMap* GameMap::getInstance()
{
    if (s_instance != 0)
      return s_instance;
    else return 0;
}


GameMap* GameMap::getInstance(std::string TilesetName, 
			      std::string ShieldsetName, 
			      std::string CitysetName)
{
    if (s_instance == 0)
    {
        s_instance = new GameMap(TilesetName, ShieldsetName, CitysetName);

    }
    return s_instance;
}

GameMap* GameMap::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new GameMap(helper);

    return s_instance;
}

void GameMap::deleteInstance()
{
    if (s_instance)
        delete s_instance;
    s_instance = 0;
}

GameMap::GameMap(std::string TilesetName, std::string ShieldsetName,
		 std::string CitysetName)
{
    d_tileSet = Tilesetlist::getInstance()->getTileset(TilesetName);
    d_shieldSet = Shieldsetlist::getInstance()->getShieldset(ShieldsetName);
    d_citySet = Citysetlist::getInstance()->getCityset(CitysetName);

    Vector<int>::setMaximumWidth(s_width);
    d_map = new Maptile*[s_width*s_height];
    for (int j = 0; j < s_height; j++)
        for (int i = 0; i < s_width; i++)
            d_map[j*s_width + i] = 0;

}

bool GameMap::offmap(int x, int y)
{
  if (y<0||y>=GameMap::s_height||x<0||x>=GameMap::s_width)
    return true;
  return false;
}

void GameMap::processStyles(std::string styles, int chars_per_style)
{
  int c = chars_per_style;
    int offset = 0;
    for (int j = 0; j < s_height; j++)
    {
        // remove newline and carriage return lines
        char test = styles[j*s_width*c + offset];
        while (test == '\n' || test == '\r')
        {
            offset++;
            test = styles[j*s_width*c + offset];
        }

        for (int i = 0; i < s_width; i++)
        {
	    char hexstr[15];
            //due to the circumstances, styles is a long stream of
            //hex digit pairs, so read it character for character
	    hexstr[0] = '0';
	    hexstr[1] = 'x';
	    memcpy (&hexstr[2], &styles[j*s_width*c + (i * c) + offset], c);
	    hexstr[2 + c + 1 - 1] = '\0';

	    unsigned long int val = 0;
	    char *end = NULL;
	    val = strtoul (hexstr, &end, 16);
	    guint32 id = (guint32) val;
	    TileStyle *style = d_tileSet->getTileStyle(id);
	    if (!style)
	      style = d_tileSet->getTileStyle(0);
	    d_map[j*s_width + i]->setTileStyle(style);
        }
    }
}
    
int GameMap::determineCharsPerStyle(std::string styles)
{
  return styles.length() / (s_width * s_height);
}

GameMap::GameMap(XML_Helper* helper)
{
    std::string types;
    std::string styles;
    std::string t_dir;
    std::string s_dir;
    std::string c_dir;

    helper->getData(s_width, "width");
    helper->getData(s_height, "height");
    helper->getData(t_dir,"tileset");
    helper->getData(s_dir,"shieldset");
    helper->getData(c_dir,"cityset");
    helper->getData(types, "types");
    helper->getData(styles, "styles");

    d_tileSet = Tilesetlist::getInstance()->getTileset(t_dir);
    d_shieldSet = Shieldsetlist::getInstance()->getShieldset(s_dir);
    d_citySet = Citysetlist::getInstance()->getCityset(c_dir);

    Vector<int>::setMaximumWidth(s_width);
    //create the map
    d_map = new Maptile*[s_width*s_height];

    int offset = 0;
    for (int j = 0; j < s_height; j++)
    {
        // remove newline and carriage return lines
        char test = types[j*s_width + offset];
        while (test == '\n' || test == '\r')
        {
            offset++;
            test = types[j*s_width + offset];
        }

        for (int i = 0; i < s_width; i++)
        {
            //due to the circumstances, types is a long stream of
            //numbers, so read it character for character (no \n's or so)
            char type = types[j*s_width + i + offset];  

            //the chars now hold the ascii representation of the numbers, which
            //we don't want
            type -= '0';
            d_map[j*s_width + i] = new Maptile(d_tileSet, i, j, type, NULL);
        }
    }

    int chars_per_style = determineCharsPerStyle(styles);
    processStyles(styles, chars_per_style);

    //add some callbacks for item loading
    helper->registerTag(GameMap::d_itemstack_tag, 
			sigc::mem_fun(this, &GameMap::loadItems));
    helper->registerTag(Item::d_tag, sigc::mem_fun(this, &GameMap::loadItems));
}


GameMap::~GameMap()
{
    //delete d_tileSet;

    for (int i = 0; i < s_width; i++)
    {
        for (int j = 0; j < s_height; j++)
        {
            if (d_map[j*s_width + i])
                delete d_map[j*s_width + i];
        }
    }

    delete[] d_map;
}

bool GameMap::fill(MapGenerator* generator)
//basically, this does the same as the former random function, but you don't
//need to go the whole way via dumping the map in a file etc.
{
    int width;
    int height;
    const Tile::Type* terrain = generator->getMap(width, height);


    //the sizes should definitely match, else we have a problem here
    if (width != s_width || height != s_height)
    {
        std::cerr << "Error in GameMap::fillMap: sizes don't match!! Exiting.\n";
        exit(-1);
    }

    // create tiles; there is a hack here: The map generator outputs tile types,
    // but we supply the index of the tile types in the tileset to Maptile. Was
    // the easiest version when rewriting this.
    for (int j = 0; j < height; j++)
        for (int i = 0; i < width; i++)
        {
            int index = d_tileSet->getIndex(terrain[j*width + i]);
	    if (index != -1)
	      d_map[j*s_width + i] = new Maptile(d_tileSet, i, j, 
						 (guint32)index, NULL);
        }

    applyTileStyles(0, 0, height, width, true);
    return true;
}

bool GameMap::fill(guint32 type)
{
    for (int i = 0; i < s_width; i++)
        for (int j = 0; j < s_height; j++)
	  {
            d_map[j*s_width + i] = new Maptile(d_tileSet, i, j, type, NULL);
	  }

    applyTileStyles(0, 0, s_height, s_width, false);
    return true;
}

bool GameMap::save(XML_Helper* helper) const
{
    bool retval = true;

    std::stringstream types;

    types <<endl;
    for (int i = 0; i < s_height; i++)
    {
        for (int j = 0; j < s_width; j++)
            types << getTile(j, i)->getType();
        types <<endl;
    }

    std::stringstream styles;
    styles <<endl;
	    
    int largest_style_id = d_tileSet->getLargestTileStyleId();
    for (int i = 0; i < s_height; i++)
    {
        for (int j = 0; j < s_width; j++)
	  {
	    Glib::ustring hexstr;
	    TileStyle *style = getTile(j, i)->getTileStyle();
	    assert (style != NULL);
	    if (largest_style_id < 256)
	      hexstr = String::ucompose ("%1", Glib::ustring::format(std::hex, std::setfill(L'0'), std::setw(2), style->getId()));
	    else if (largest_style_id < 4096)
	      hexstr = String::ucompose ("%1", Glib::ustring::format(std::hex, std::setfill(L'0'), std::setw(3), style->getId()));
	    else if (largest_style_id < 65536)
	      hexstr = String::ucompose ("%1", Glib::ustring::format(std::hex, std::setfill(L'0'), std::setw(4), style->getId()));

	    styles << hexstr;
	  }
        styles <<endl;
    }


    retval &= helper->openTag(GameMap::d_tag);
    retval &= helper->saveData("width", s_width);
    retval &= helper->saveData("height", s_height);
    retval &= helper->saveData("tileset", d_tileSet->getSubDir());
    retval &= helper->saveData("shieldset", d_shieldSet->getSubDir());
    retval &= helper->saveData("cityset", d_citySet->getSubDir());
    retval &= helper->saveData("types", types.str());
    retval &= helper->saveData("styles", styles.str());

    // last, save all items lying around somewhere
    for (int i = 0; i < s_width; i++)
      for (int j = 0; j < s_height; j++)
	if (!getTile(i,j)->getBackpack()->empty())
	  retval &= getTile(i,j)->getBackpack()->save(helper);
     
    retval &= helper->closeTag();
    return retval;
}

bool GameMap::loadItems(std::string tag, XML_Helper* helper)
{
    static int x = 0;
    static int y = 0;
    
    if (tag == GameMap::d_itemstack_tag)
    {
        helper->getData(x, "x");
        helper->getData(y, "y");
    }

    if (tag == Item::d_tag)
    {
        Item* item = new Item(helper);
        getTile(x, y)->getBackpack()->addToBackpack(item);
    }

    return true;
}

void GameMap::setTile(int x, int y, Maptile *tile)
{
    delete d_map[y*s_width + x];
    d_map[y*s_width + x] = tile;
    applyTileStyle (y, x);
}

Maptile* GameMap::getTile(int x, int y) const
{
    assert(x >= 0 && x < s_width && y >= 0 && y < s_height);

    return d_map[y*s_width + x];
}

Stack* GameMap::addArmy(Vector<int> pos, Army *a)
{
  City *c = getCity(pos);
  if (c)
    {
      if (c->isBurnt())
        return addArmyAtPos(pos, a);
      else
        return addArmy(c, a);
    }
  Temple *t = getTemple(pos);
  if (t)
    return addArmy(t, a);
  Ruin *r = getRuin(pos);
  if (r)
    return addArmy(r, a);
  return addArmyAtPos(pos, a);
}

Stack* GameMap::addArmyAtPos(Vector<int> pos, Army *a)
{
  Stack *s = NULL;
  bool added_army = false;
  guint32 i, j;
  guint32 d;
  guint32 max;
  int x, y;
  if (s_height > s_width)
    max = s_height;
  else
    max = s_width;
  max--;

  Location l(pos, 1);
  s = l.addArmy(a);
  if (s)
    return s;

  // we couldn't add the army to the square(s) identified by location,
  // so the idea is to go around in ever widening boxes until we find a
  // suitable tile.

  bool land = true;
  if (getTile(pos.x, pos.y)->getType() == Tile::WATER)
    land = false;

  //d is the distance from Pos where our box starts
  for (d = 1; d < max; d++)
    {
      for (i = 0; i < (d * 2) + 1; i++)
        {
          for (j = 0; j < (d * 2) + 1; j++)
            {
              if ((i == 0 || i == (d * 2) + 1) && 
                  (j == 0 || j == (d * 2) + 1))
                {
                  x = pos.x + (i - d);
                  y = pos.y + (j - d);
                  if (x < 0 || y < 0)
                    continue;
		  if (offmap(x, y))
		    continue;
                  //is there somebody else's city here?
                  City *c = getCity(Vector<int>(x, y));
                  if (c && c->getOwner() != a->getOwner())
                    {
                      if (c->isBurnt() == false)
                        continue;
                    }
                  //is this an unsuitable tile?
                  if (land && getTile(x, y)->getType() == Tile::WATER)
                    continue;
                  if (!land && getTile(x, y)->getType() != Tile::WATER)
                    continue;
                  if (land && getTile(x, y)->getType() == Tile::VOID)
		    continue;
                  if (land && getTile(x, y)->getType() == Tile::MOUNTAIN &&
		      (a->getStat(Army::MOVE_BONUS) & Tile::MOUNTAIN) == 0)
                    continue;
		  //do we already have a nifty stack here?
		  s = getFriendlyStack(Vector<int>(x,y));
                  if (s)
                    {
		      if (canAddArmy(Vector<int>(x,y)) == false)
			continue;
                      //is our stack too full?
		      s->add(a);
                    }
                  else 
                    {
                      Vector<int> pos(x, y);
		      //hmm. no nifty stacks here.  anybody else's?
		      s = getEnemyStack(pos);
		      if (s)
			continue;
		      //okay, no stacks here at all.  make one.
                      s = new Stack(a->getOwner(), pos);
		      s->add(a);
                      a->getOwner()->addStack(s);
                    }
                  added_army = true;
                  break;
                }
            }
          if (added_army)
            break;
        }
      if (added_army)
        break;
    }

  if (added_army)
    {
      s->setDefending(false);
      s->setParked(false);
      return s;
    }
  else
    return NULL;
}

Stack* GameMap::addArmy(Location *l, Army *a)
{
  Stack *s;
  s = l->addArmy(a);
  if (s)
    return s;
  return addArmyAtPos(l->getPos(), a);
}

bool GameMap::isDock(Vector<int> pos)
{
  if (getBuilding(pos) == Maptile::CITY)
    return true;
  if (getBuilding(pos) == Maptile::PORT)
    return true;
  if (getBuilding(pos) == Maptile::BRIDGE)
    return true;
  return false;
}

bool GameMap::isBlockedAvenue(int x, int y, int destx, int desty)
{
  if (offmap(destx, desty))
    return true;
  if (Citylist::getInstance()->empty())
      return false;
  int diffx = destx - x;
  int diffy = desty - y;
  if (diffx >= -1 && diffx <= 1 && diffy >= -1 && diffy <= 1)
    {
      assert (Citylist::getInstance()->size());
      bool from_dock = isDock(Vector<int>(x,y));
      bool to_dock = isDock(Vector<int>(destx,desty));
      Maptile *from = getTile(x, y);
      Maptile *to = getTile(destx, desty);
      if (from == to)
        return false;
      //am i on land, going towards water that has a port on it?
      //if (from->getMaptileType() != Tile::WATER &&
          //to->getMaptileType() == Tile::WATER &&
          //to_dock)
        //return false;
      //am i on water going towards land from a port?
      //if (from->getMaptileType() == Tile::WATER &&
          //to->getMaptileType() != Tile::WATER &&
          //from_dock)
        //return false;

      //am i on water going towards land that isn't a city,
      //and i'm not coming from a port
      if (from->getMaptileType() == Tile::WATER &&
          to->getMaptileType() != Tile::WATER &&
          !to_dock && !from_dock)
        return true;

      //am i on land, going towards water from a tile that isn't a
      //city, or a port and i'm not going to a port?
      if (from->getMaptileType() != Tile::WATER &&
          to->getMaptileType() == Tile::WATER &&
          !from_dock && !to_dock)
        return true;

      //is the tile i'm going to a mountain that doesn't have a road?
      if (to->getMaptileType() == Tile::MOUNTAIN &&
	  getRoad(Vector<int>(destx, desty)) == NULL)
        return true;

      //am i on a mountain without a road?
      if (from->getMaptileType() == Tile::MOUNTAIN &&
	  getRoad(Vector<int>(x, y)) == NULL)
        return true;

      if (from->getMaptileType() == Tile::VOID)
	return true;
      if (to->getMaptileType() == Tile::VOID)
	return true;
    }
 return false;
}

void GameMap::calculateBlockedAvenue(int i, int j)
{
  int diffx = 0, diffy = 0;
  int destx = 0, desty = 0;
  Maptile *maptile = getTile(i, j);
  for (int k = 0; k < 8; k++)
    {
      switch (k)
	{
	case 0: diffx = -1;  diffy = -1; break;
	case 1: diffx = -1; diffy = 0; break;
	case 2: diffx = -1; diffy = 1; break;
	case 3: diffx = 0; diffy = 1; break;
	case 4: diffx = 0; diffy = -1; break;
	case 5: diffx = 1; diffy = -1; break;
	case 6: diffx = 1; diffy = 0; break;
	case 7: diffx = 1; diffy = 1; break;
	}
      destx = i + diffx;
      desty = j + diffy;
      if (offmap (destx, desty))
	{
	  maptile->d_blocked[k] = true;
	  continue;
	}
      maptile->d_blocked[k] = isBlockedAvenue(i, j, destx, desty);
    }
}
void GameMap::calculateBlockedAvenues()
{
  for (int i = 0; i < s_width; i++)
    for (int j = 0; j < s_height; j++)
      calculateBlockedAvenue(i, j);
}

Vector<int> GameMap::findStack(guint32 id)
{
    bool found = false;
    Vector<int> pos = Vector<int>(-1,-1);
    for (int x = 0; x < getWidth(); x++)
      {
        for (int y = 0; y < getHeight(); y++)
          {
	    StackTile *stile = getTile(x,y)->getStacks();
	    if (stile->contains(id) == true)
	      {
		pos = Vector<int>(x,y);
		found = true;
		break;
	      }
          }
      }
  return pos;
}

Vector<int> GameMap::findPlantedStandard(Player *p)
{
    bool found = false;
    Vector<int> pos;
    pos.x = -1;
    pos.y = -1;
    for (int x = 0; x < getWidth(); x++)
      {
        for (int y = 0; y < getHeight(); y++)
          {
	    MapBackpack *backpack = getTile(x, y)->getBackpack();
	    found = backpack->getPlantedItem(p) != NULL;
	    if (found)
	      {
		pos.x = x;
		pos.y = y;
		break;
	      }
          }
      }
  return pos;
}

TileStyle *GameMap::calculatePreferredStyle(int i, int j)
{
  Tileset *tileset = getTileset();
  Maptile *mtile = getTile(j, i);
  int box[3][3];
  for (int k = -1; k <= +1; k++)
    for (int l = -1; l <= +1; l++)
      {
	box[k+1][l+1] = 1;
	if (offmap(j+l, i+k))
	  continue;
	box[k+1][l+1] = are_those_tiles_similar(getTile(j+l, i+k)->getMaptileType(), mtile->getMaptileType(), false);
      }
  if (box[0][0] && box[0][1] && box[0][2] &&
      box[1][0] && box[1][1] && box[1][2] &&
      box[2][0] && box[2][1] && box[2][2])
    return tileset->getRandomTileStyle(mtile->getType(), 
				       TileStyle::INNERMIDDLECENTER);
  else if (box[0][0] && box[0][1] && !box[0][2] && 
	   box[1][0] && box[1][1] && box[1][2] &&
	   !box[2][0] && box[2][1] && box[2][2])
    return tileset->getRandomTileStyle(mtile->getType(), 
				       TileStyle::TOPLEFTTOBOTTOMRIGHTDIAGONAL);
  else if (!box[0][0] && box[0][1] && box[0][2] && 
	   box[1][0] && box[1][1] && box[1][2] &&
	   box[2][0] && box[2][1] && !box[2][2])
    return tileset->getRandomTileStyle(mtile->getType(), 
				       TileStyle::BOTTOMLEFTTOTOPRIGHTDIAGONAL);
  else if (/*box[0][0] &&*/ !box[0][1] && /*box[0][2] &&*/
	   !box[1][0] && box[1][1] && box[1][2] &&
	   /*!box[2][0] &&*/ box[2][1] && box[2][2])
    return tileset->getRandomTileStyle(mtile->getType(), 
				       TileStyle::OUTERTOPLEFT);
  else if (/*box[0][0] &&*/ !box[0][1] && /*box[0][2] &&*/
	   box[1][0] && box[1][1] && !box[1][2] &&
	   box[2][0] && box[2][1] /*&& !box[2][2] */)
    return tileset->getRandomTileStyle(mtile->getType(), 
				       TileStyle::OUTERTOPRIGHT);
  else if (/*box[0][0] &&*/ box[0][1] && box[0][2] &&
	   !box[1][0] && box[1][1] && box[1][2] &&
	   /*box[2][0] &&*/ !box[2][1] /*&& box[2][2]*/)
    return tileset->getRandomTileStyle(mtile->getType(), 
				       TileStyle::OUTERBOTTOMLEFT);
  else if (box[0][0] && box[0][1] && /*!box[0][2] &&*/
	   box[1][0] && box[1][1] && !box[1][2] && 
	   /*box[2][0] &&*/ !box[2][1] /*&& box[2][2]*/)
    return tileset->getRandomTileStyle(mtile->getType(), 
				       TileStyle::OUTERBOTTOMRIGHT);
  else if (/*box[0][0] &&*/ box[0][1] && /*box[0][2] && */
	   !box[1][0] && box[1][1] && box[1][2] &&
	   /*box[2][0] &&*/ box[2][1] /*&& box[2][2]*/)
    return tileset->getRandomTileStyle(mtile->getType(), 
				       TileStyle::OUTERMIDDLELEFT);
  else if (/*box[0][0] &&*/ box[0][1] && /*box[0][2] && */
	   box[1][0] && box[1][1] && !box[1][2] &&
	   /*box[2][0] &&*/ box[2][1] /*&& box[2][2] */)
    return tileset->getRandomTileStyle(mtile->getType(), 
				       TileStyle::OUTERMIDDLERIGHT);
  else if (box[0][0] && box[0][1] && /*box[0][2] && */
	   box[1][0] && box[1][1] && box[1][2] &&
	   box[2][0] && box[2][1] && !box[2][2])
    return tileset->getRandomTileStyle(mtile->getType(), 
				       TileStyle::INNERTOPLEFT);
  else if (/*box[0][0] &&*/ box[0][1] && box[0][2] && 
	   box[1][0] && box[1][1] && box[1][2] &&
	   !box[2][0] && box[2][1] && box[2][2])
    return tileset->getRandomTileStyle(mtile->getType(), 
				       TileStyle::INNERTOPRIGHT);
  else if (box[0][0] && box[0][1] && !box[0][2] && 
	   box[1][0] && box[1][1] && box[1][2] &&
	   box[2][0] && box[2][1] /*&& box[2][2]*/)
    return tileset->getRandomTileStyle(mtile->getType(), 
				       TileStyle::INNERBOTTOMLEFT);
  else if (!box[0][0] && box[0][1] && box[0][2] && 
	   box[1][0] && box[1][1] && box[1][2] &&
	   /*box[2][0] &&*/ box[2][1] && box[2][2])
    return tileset->getRandomTileStyle(mtile->getType(), 
				       TileStyle::INNERBOTTOMRIGHT);
  else if (/*!box[0][0] &&*/ !box[0][1] && /*!box[0][2] &&*/
	   box[1][0] && box[1][1] && box[1][2] &&
	   /*!box[2][0] &&*/ box[2][1] /*&& box[2][2]*/)
    return tileset->getRandomTileStyle(mtile->getType(), 
				       TileStyle::OUTERTOPCENTER);
  else if (/*box[0][0] &&*/ box[0][1] && /*box[0][2] &&*/
	   box[1][0] && box[1][1] && box[1][2] &&
	   /*!box[2][0] &&*/ !box[2][1] /*&& !box[2][2]*/)
    return tileset->getRandomTileStyle(mtile->getType(), 
				       TileStyle::OUTERBOTTOMCENTER);
  return NULL;
}

void GameMap::close_circles (int minx, int miny, int maxx, int maxy)
{
  Tileset *tileset = getTileset();
  for (int i = minx; i < maxx; i++)
    {
      for (int j = miny; j < maxy; j++)
	{
	  if (offmap(j, i))
	    continue;
	  Maptile *tile = getTile(j, i);
	  TileStyle *tilestyle = tile->getTileStyle();
	  if (j + 1 < s_width)
	    {
	      Maptile *nexttile = getTile(j + 1, i);
	      TileStyle *nextstyle = nexttile->getTileStyle();
	      if (tilestyle->getType() == TileStyle::OUTERTOPCENTER &&
		  nextstyle->getType() == TileStyle::OUTERBOTTOMCENTER)
		{
		  TileStyle *style;
		  style = tileset->getRandomTileStyle(tile->getType(),
						      TileStyle::OUTERTOPRIGHT);
		  tile->setTileStyle(style);
		  style = tileset->getRandomTileStyle(nexttile->getType(),
						      TileStyle::OUTERBOTTOMLEFT);
		  nexttile->setTileStyle(style);
		}
	      if (tilestyle->getType() == TileStyle::OUTERBOTTOMCENTER &&
		  nextstyle->getType() == TileStyle::OUTERTOPCENTER)
		{
		  TileStyle *style;
		  style = tileset->getRandomTileStyle(tile->getType(),
						      TileStyle::OUTERBOTTOMRIGHT);
		  tile->setTileStyle(style);
		  style = tileset->getRandomTileStyle(nexttile->getType(),
						      TileStyle::OUTERTOPLEFT);
		  nexttile->setTileStyle(style);
		}
	      }
	  if (i + 1 < s_height)
	    {
	      Maptile *nexttile = getTile(j, i + 1);
	      TileStyle *nextstyle = nexttile->getTileStyle();
	      if (tilestyle->getType() == TileStyle::OUTERMIDDLERIGHT&&
		  nextstyle->getType() == TileStyle::OUTERMIDDLELEFT)
		{
		  TileStyle *style;
		  style = tileset->getRandomTileStyle(tile->getType(),
						      TileStyle::OUTERBOTTOMRIGHT);
		  tile->setTileStyle(style);
		  style = tileset->getRandomTileStyle(nexttile->getType(),
						      TileStyle::OUTERTOPLEFT);
		  nexttile->setTileStyle(style);
		}
	      if (tilestyle->getType() == TileStyle::OUTERMIDDLELEFT&&
		  nextstyle->getType() == TileStyle::OUTERMIDDLERIGHT)
		{
		  TileStyle *style;
		  style = tileset->getRandomTileStyle(tile->getType(),
						      TileStyle::OUTERBOTTOMLEFT);
		  tile->setTileStyle(style);
		  style = tileset->getRandomTileStyle(nexttile->getType(),
						      TileStyle::OUTERTOPRIGHT);
		  nexttile->setTileStyle(style);
		}
	      }
	}
    }
}

bool GameMap::are_those_tiles_similar(Tile::Type outer_tile,Tile::Type inner_tile, bool checking_loneliness)
{
    if(checking_loneliness || inner_tile == Tile::HILLS)
    {
        if( (outer_tile == Tile::MOUNTAIN && inner_tile == Tile::HILLS) ||
            (inner_tile == Tile::MOUNTAIN && outer_tile == Tile::HILLS))
            // Mountains and hills are similar, MapGenerator::surroundMountains()
            // makes sure that mountains are surrounded by hills. So a hill tile
            // with only a mountain neighbour is not a lone tile
            //
            // There never should be a lone mountain in grass (not surrounded by hills).
            // Mountain surrounded by hills is perfectly correct.
            return true;
        return outer_tile == inner_tile;
    } 
    else 
    { // to pick correct tile picture for a mountain we treat hills as a tile
      // different than mountain.
        return outer_tile == inner_tile;
    }
}

int GameMap::tile_is_connected_to_other_like_tiles (Tile::Type tile, int i, int j)
{
  int box[3][3];
  memset (box, 0, sizeof (box));
  for (int k = -1; k <= +1; k++)
    for (int l = -1; l <= +1; l++)
      {
	if (offmap(j+l,i+k))
	  continue;
	box[k+1][l+1] = are_those_tiles_similar(getTile(j+l, i+k)->getMaptileType(), tile, true);
      }
  if (box[0][0] && box[0][1] && box[1][0] && box[1][1])
    return 1;
  if (box[0][1] && box[0][2] && box[1][1] && box[1][2])
    return 1;
  if (box[1][0] && box[1][1] && box[2][0] && box[2][1])
    return 1;
  if (box[1][1] && box[1][2] && box[2][1] && box[2][2])
    return 1;
  return 0;
}

void GameMap::demote_lone_tile(int minx, int miny, int maxx, int maxy, 
			       Tile::Type intype, Tile::Type outtype)
{
  int i;
  int j;
  for (i = minx; i < maxx; i++)
    for (j = miny; j < maxy; j++)
      {
	if (offmap(j, i))
	  continue;
	Tile::Type tile = getTile(j, i)->getMaptileType();
	if (tile == intype)
	  {
	    //if we're not connected in a square of
	    //same types, then we're a lone tile.
	    if (tile_is_connected_to_other_like_tiles(tile, i, j) == 0)
	      {
		//okay, this is a lone tile.
		//downgrade it
		int idx = d_tileSet->getIndex(outtype);
		if (idx != -1)
		  setTile(j, i, new Maptile (d_tileSet, j, i, 
					     (guint32)idx, NULL));
	      }
	  }
      }

}

void GameMap::applyTileStyles (Rectangle r, bool smooth_terrain)
{
  applyTileStyles (r.y, r.x, r.y + r.h, r.x + r.w, smooth_terrain);
}

void GameMap::applyTileStyles (int minx, int miny, int maxx, int maxy, 
			       bool smooth_terrain)
{

  if (smooth_terrain)
    {
      demote_lone_tile(minx, miny, maxx, maxy, Tile::FOREST, Tile::GRASS);
      demote_lone_tile(minx, miny, maxx, maxy, Tile::MOUNTAIN, Tile::HILLS);
      demote_lone_tile(minx, miny, maxx, maxy, Tile::HILLS, Tile::GRASS);
      demote_lone_tile(minx, miny, maxx, maxy, Tile::WATER, Tile::SWAMP);
      surroundMountains(minx, miny, maxx, maxy);
    }

  for (int i = minx; i < maxx; i++)
    {
      for (int j = miny; j < maxy; j++)
	{
	  if (offmap(j, i))
	    continue;
	  applyTileStyle(i, j);
	}
    }
  close_circles(minx, miny, maxx, maxy);
}

std::vector<Vector<int> > GameMap::getItems()
{
  std::vector<Vector<int> > items;
  for (int j = 0; j < s_height; j++)
    for (int i = 0; i < s_width; i++)
      {
	if (d_map[j*s_width + i])
	  if (d_map[j*s_width + i]->getBackpack()->empty() == false)
	    items.push_back(Vector<int>(i, j));

      }
  return items;
}


void GameMap::surroundMountains(int minx, int miny, int maxx, int maxy)
{
  for(int j = miny; j < maxy; j++)
    for(int i = minx; i < maxx; i++)
      {
	if (offmap(j, i))
	  continue;
	if(getTile(j, i)->getMaptileType() == Tile::MOUNTAIN)
	  for(int J = -1; J <= +1; ++J)
	    for(int I = -1; I <= +1; ++I)
	      if((!(offmap(j+J,i+I))) &&
		 (getTile((j+J),(i+I))->getMaptileType() != Tile::MOUNTAIN))
		{
		  int idx = d_tileSet->getIndex(Tile::HILLS);
		  if(getTile((j+J), (i+I))->getMaptileType() != Tile::WATER)
		    {
		      if (idx != -1)
			setTile(j+J, i+I, 
				new Maptile (d_tileSet, j+J, i+I, 
					     (guint32)idx, NULL));
		    }
		  else 
		    {
		    // water has priority here, there was some work done to conenct bodies of water
		    // so don't break those connections.
		      setTile(j, i, 
			    new Maptile (d_tileSet, j, i, (guint32)idx, NULL));
		    }
		}
      }
}

void GameMap::applyTileStyle (int i, int j)
{
  Maptile *mtile = getTile(j, i);
  Tileset *tileset = getTileset();
  TileStyle *style = calculatePreferredStyle(i, j);
  if (!style)
    style = tileset->getRandomTileStyle(mtile->getType(), 
					TileStyle::LONE);
  if (!style)
    style = tileset->getRandomTileStyle(mtile->getType(), 
					TileStyle::INNERMIDDLECENTER);
  if (!style)
    printf ("applying null tile style at %d,%d for tile of kind %d\n", i, j,
	    mtile->getMaptileType());
  mtile->setTileStyle(style);
}

Vector<int> GameMap::findNearestObjectInDir(Vector<int> pos, Vector<int> dir)
{
  std::vector<Vector<int> > objects;
  Road *road = Roadlist::getInstance()->getNearestObjectInDir(pos, dir);
  if (road)
    objects.push_back(road->getPos());
  Bridge *bridge = Bridgelist::getInstance()->getNearestObjectInDir(pos, dir);
  if (bridge)
    objects.push_back(bridge->getPos());
  City *city = Citylist::getInstance()->getNearestObjectInDir(pos, dir);
  if (city)
    objects.push_back(city->getPos());
  Temple *temple = Templelist::getInstance()->getNearestObjectInDir(pos, dir);
  if (temple)
    objects.push_back(temple->getPos());
  Ruin *ruin = Ruinlist::getInstance()->getNearestObjectInDir(pos, dir);
  if (ruin && ruin->isHidden() == false)
    objects.push_back(ruin->getPos());
  if (objects.size() == 0)
    return Vector<int>(-1,-1);

  int min_distance = -1;
  Vector<int> closest = Vector<int>(-1,-1);
  for (unsigned int i = 0; i < objects.size(); i++)
    {
      int distance = dist(pos, objects[i]);
      if (min_distance == -1 || distance < min_distance)
	{
	  min_distance = distance;
	  closest = objects[i];
	}
    }
  return closest;
}

Vector<int> GameMap::findNearestObjectToTheNorth(Vector<int> pos)
{
  Vector<int> dir = Vector<int>(0, -1);
  return findNearestObjectInDir(pos, dir);
}

Vector<int> GameMap::findNearestObjectToTheSouth(Vector<int> pos)
{
  Vector<int> dir = Vector<int>(0, 1);
  return findNearestObjectInDir(pos, dir);
}

Vector<int> GameMap::findNearestObjectToTheEast(Vector<int> pos)
{
  Vector<int> dir = Vector<int>(1, 0);
  return findNearestObjectInDir(pos, dir);
}

Vector<int> GameMap::findNearestObjectToTheWest(Vector<int> pos)
{
  Vector<int> dir = Vector<int>(-1, 0);
  return findNearestObjectInDir(pos, dir);
}
City* GameMap::getCity(Vector<int> pos)
{
  return Citylist::getInstance()->getObjectAt(pos);
}
City* GameMap::getEnemyCity(Vector<int> pos)
{
  City *c = Citylist::getInstance()->getObjectAt(pos);
  if (c && c->getOwner() != Playerlist::getActiveplayer())
    return c;
  return NULL;
}
Ruin* GameMap::getRuin(Vector<int> pos)
{
  return Ruinlist::getInstance()->getObjectAt(pos);
}
Temple* GameMap::getTemple(Vector<int> pos)
{
  return Templelist::getInstance()->getObjectAt(pos);
}
Port* GameMap::getPort(Vector<int> pos)
{
  return Portlist::getInstance()->getObjectAt(pos);
}
Road* GameMap::getRoad(Vector<int> pos)
{
  return Roadlist::getInstance()->getObjectAt(pos);
}
Bridge* GameMap::getBridge(Vector<int> pos)
{
  return Bridgelist::getInstance()->getObjectAt(pos);
}
Signpost* GameMap::getSignpost(Vector<int> pos)
{
  return Signpostlist::getInstance()->getObjectAt(pos);
}
Stack* GameMap::getFriendlyStack(Vector<int> pos)
{
  return getStacks(pos)->getFriendlyStack(Playerlist::getActiveplayer());
}

//StackReflist GameMap::getFriendlyStacks(Vector<int> pos, Player *player)
//{
  //if (player == NULL)
    //player = Playerlist::getActiveplayer();
  //return StackReflist(getStacks(pos)->getFriendlyStacks(player));
//}
std::list<Stack*> GameMap::getFriendlyStacks(Vector<int> pos, Player *player)
{
  if (player == NULL)
    player = Playerlist::getActiveplayer();
  return getStacks(pos)->getFriendlyStacks(player);
}
	
Stack* GameMap::getEnemyStack(Vector<int> pos)
{
  return getStacks(pos)->getEnemyStack(Playerlist::getActiveplayer());
}
	
std::list<Stack*> GameMap::getEnemyStacks(std::list<Vector<int> > positions)
{
  std::list<Stack*> enemy_stacks;
  std::list<Vector<int> >::iterator it = positions.begin();
  for (; it != positions.end(); it++)
    {
      Stack *enemy = getEnemyStack(*it);
      if (enemy)
	enemy_stacks.push_back(enemy);
    }
  return enemy_stacks;
}

std::list<Stack*> GameMap::getEnemyStacks(Vector<int> pos, Player *player)
{
  if (player == NULL)
    player = Playerlist::getActiveplayer();
  return getStacks(pos)->getEnemyStacks(player);
}
Stack* GameMap::getStack(Vector<int> pos)
{
  return getStacks(pos)->getStack();
}
	
StackTile* GameMap::getStacks(Vector<int> pos)
{
  return getInstance()->getTile(pos)->getStacks();
}
Stack *GameMap::groupStacks(Vector<int> pos)
{
  return getStacks(pos)->group(Playerlist::getActiveplayer());
}
  
void GameMap::groupStacks(Stack *stack)
{
  getStacks(stack->getPos())->group(Playerlist::getActiveplayer(), stack);
}
  
void GameMap::updateStackPositions()
{
  Playerlist *plist = Playerlist::getInstance();
  for (Playerlist::iterator i = plist->begin(); i != plist->end(); i++)
    {
      Stacklist *sl = (*i)->getStacklist();
      for (Stacklist::iterator s = sl->begin(); s != sl->end(); s++)
	getStacks((*s)->getPos())->add(*s);
    }
}

bool GameMap::canJoin(const Stack *src, Vector<int> dest)
{
  return getStacks(dest)->canAdd(src);
}
bool GameMap::canJoin(const Stack *src, Stack *dest)
{
  return canJoin (src, dest->getPos());
}

bool GameMap::canAddArmy(Vector<int> dest)
{
  if (countArmyUnits(dest) < MAX_ARMIES_ON_A_SINGLE_TILE)
    return true;
  return false;
}
bool GameMap::canAddArmies(Vector<int> dest, guint32 stackSize)
{
  if (countArmyUnits(dest) + stackSize <= MAX_ARMIES_ON_A_SINGLE_TILE)
    return true;
  return false;
}

void GameMap::switchTileset(Tileset *tileset)
{
  d_tileSet = tileset;
  applyTileStyles (0, 0, s_width, s_height,  false);
}

void GameMap::switchShieldset(Shieldset *shieldset)
{
  d_shieldSet = shieldset;
}

void GameMap::switchCityset(Cityset *cityset)
{
  d_citySet = cityset;
}

void GameMap::switchArmysets(Armyset *armyset)
{
  Playerlist *pl= Playerlist::getInstance();
  Ruinlist *rl= Ruinlist::getInstance();
  Citylist *cl= Citylist::getInstance();
  //change the keepers in ruins
  for (Ruinlist::iterator i = rl->begin(); i != rl->end(); i++)
    {
      Stack *s = (*i)->getOccupant();
      if (s == NULL)
	continue;
      for (Stack::iterator j = s->begin(); j != s->end(); j++)
	Armyset::switchArmysetForRuinKeeper(*j, armyset);
    }
  for (Playerlist::iterator i = pl->begin(); i != pl->end(); i++)
    {
      Armyset *a = 
	Armysetlist::getInstance()->getArmyset((*i)->getArmyset());
      if (armyset == a)
	continue;

      //change the armyprodbases in cities.
      for (Citylist::iterator j = cl->begin(); j != cl->end(); j++)
	{
	  City *c = *j;
	  for (unsigned int k = 0; c->getSize(); k++)
	    {
	      ArmyProdBase *prodbase = (*c)[k]->getArmyProdBase();
	      if (prodbase)
		Armyset::switchArmyset(prodbase, armyset);
	    }
	}

      //change the armies in the stacklist
      Stacklist *sl = (*i)->getStacklist();
      for (Stacklist::iterator j = sl->begin(); j != sl->end(); j++)
	{
	  Stack *s = (*j);
	  for (Stack::iterator k = s->begin(); k != s->end(); k++)
	    Armyset::switchArmyset(*k,armyset);
	}

      //finally, change the player's armyset.
      (*i)->setArmyset(armyset->getId());
      //where else are armyset ids hanging around?
    }
}

bool GameMap::canPutBuilding(Maptile::Building bldg, guint32 size, Vector<int> to, bool making_islands)
{
  bool can_move = true;
  //gotta have a building to move
  if (bldg == Maptile::NONE)
    return false;
  //there can't be another building in the way.
  bool found = false;
  for (unsigned int i = 0; i < size; i++)
    for (unsigned int j = 0; j < size; j++)
      {
	Vector<int> pos = to + Vector<int>(i,j);
	if (offmap(pos.x, pos.y))
	  return false;
	if (getBuilding(pos) != Maptile::NONE)
	  found = true;
      }
  if (found)
    return false;
  //ok different objects have different rules wrt the kinds of tiles they 
  //can be on.
  switch (bldg)
    {
      case Maptile::CITY: 
      case Maptile::RUIN: 
      case Maptile::TEMPLE: 
      case Maptile::SIGNPOST:
	//gotta be on grass.
	  {
	    if (making_islands)
	      return true;
	    for (unsigned int i = 0; i < size; i++)
	      for (unsigned int j = 0; j < size; j++)
		{
		  Vector<int> pos = to + Vector<int>(i, j);
		  if (getTerrainType(pos) != Tile::GRASS)
		    return false;
		}
	  }
	break;
      case Maptile::ROAD: 
	//can't be in the water
	if (getTerrainType(to) == Tile::WATER ||
	    getTerrainType(to) == Tile::VOID)
	  return false;
	break;
      case Maptile::PORT: 
	if (getTerrainType(to) == Tile::WATER &&
	    getTile(to)->getTileStyle()->getType() != 
	    TileStyle::INNERMIDDLECENTER)
	  return can_move;
	else
	  return false;
	break;
      case Maptile::BRIDGE: 
	if (getTerrainType(to) == Tile::WATER &&
	    (getTile(to)->getTileStyle()->getType() == 
	     TileStyle::OUTERTOPCENTER || 
	    getTile(to)->getTileStyle()->getType() == 
	    TileStyle::OUTERBOTTOMCENTER || 
	    getTile(to)->getTileStyle()->getType() == 
	    TileStyle::OUTERMIDDLELEFT || 
	    getTile(to)->getTileStyle()->getType() == 
	    TileStyle::OUTERMIDDLERIGHT ))
	  return can_move;
	else
	  return false;
	break;
      case Maptile::NONE: break;
    }
  return can_move;
}

bool GameMap::moveBuilding(Vector<int> from, Vector<int> to)
{
  //move a game object located at FROM, and move it to TO.
  //watch out for overlaps.
  //return true if we moved something.
  bool moved = true;
    
  guint32 size = getBuildingSize(from);
  if (size == 0)
    return false;

  if (canPutBuilding(getBuilding(from), size, to) == false)
    {
      if (getLocation(from)->contains(to) == false &&
	  LocationBox(to, size).contains(from) == false)
	return false;
    }
	  
  switch (getBuilding(from))
    {
    case Maptile::NONE:
      break;
    case Maptile::SIGNPOST:
	{
	  Signpost *old_signpost = getSignpost(getSignpost(from)->getPos());
	  Signpost *new_signpost = new Signpost(*old_signpost, to);
	  removeSignpost(old_signpost->getPos());
	  putSignpost(new_signpost);
	  break;
	}
    case Maptile::PORT:
	{
	  Port *old_port = getPort(getPort(from)->getPos());
	  Port *new_port = new Port(*old_port, to);
	  removePort(old_port->getPos());
	  putPort(new_port);
	  break;
	}
    case Maptile::BRIDGE:
	{
	  Bridge *old_bridge = getBridge(getBridge(from)->getPos());
	  Bridge *new_bridge = new Bridge(*old_bridge, to);
	  removeBridge(old_bridge->getPos());
	  putBridge(new_bridge);
	  break;
	}
    case Maptile::ROAD:
	{
	  Road *old_road = getRoad(getRoad(from)->getPos());
	  Road *new_road = new Road(*old_road, to);
	  removeRoad(old_road->getPos());
	  putRoad(new_road);
	  break;
	}
    case Maptile::RUIN:
	{
	  Ruin* old_ruin = getRuin(getRuin(from)->getPos());
	  Ruin *new_ruin = new Ruin(*old_ruin, to);
	  removeRuin(old_ruin->getPos());
	  putRuin(new_ruin);
	  break;
	}
    case Maptile::TEMPLE:
	{
	  Temple* old_temple = getTemple(getTemple(from)->getPos());
	  Temple* new_temple = new Temple(*old_temple, to);
	  removeTemple(old_temple->getPos());
	  putTemple(new_temple);
	  break;
	}
    case Maptile::CITY:
	{
	  City* old_city = getCity(getCity(from)->getPos());
	  City* new_city = new City(*old_city, to);
	  removeCity(old_city->getPos());
	  putCity(new_city);
	  break;
	}
    }
  return moved;
}

Tile::Type GameMap::getTerrainType(Vector<int> tile)
{
  guint32 idx = getTile(tile)->getType();
  return (*d_tileSet)[idx]->getType();
}

Maptile::Building GameMap::getBuilding(Vector<int> tile)
{
  return getTile(tile)->getBuilding();
}

void GameMap::setBuilding(Vector<int> tile, Maptile::Building building)
{
  getTile(tile)->setBuilding(building);
}

guint32 GameMap::getBuildingSize(Vector<int> tile)
{
  switch (getTile(tile)->getBuilding())
    {
    case Maptile::CITY: return getCity(tile)->getSize(); break;
    case Maptile::RUIN: return getRuin(tile)->getSize(); break;
    case Maptile::TEMPLE: return getTemple(tile)->getSize(); break;
    case Maptile::ROAD: return getRoad(tile)->getSize(); break;
    case Maptile::BRIDGE: return getBridge(tile)->getSize(); break;
    case Maptile::SIGNPOST: return getSignpost(tile)->getSize(); break;
    case Maptile::PORT: return getPort(tile)->getSize(); break;
    case Maptile::NONE: break;
    }

  return 0;
}
	
bool GameMap::canPutStack(guint32 size, Player *p, Vector<int> to)
{
  StackTile *stile = GameMap::getInstance()->getStacks(to);
  if (!stile)
    return true;
  if (stile->canAdd(size, p) == true)
    return true;
  return false;

}

bool GameMap::moveStack(Stack *stack, Vector<int> to)
{
  bool moved = true;
  if (stack->getPos() == to)
    return true;
  if (canPutStack(stack->size(), stack->getOwner(), to) == false)
    return false;


  getStacks(stack->getPos())->leaving(stack);
  stack->setPos(to);
  //Stack *new_stack = new Stack(*stack);
  //new_stack->setPos(to);
  //getStacks(stack->getPos())->leaving(stack);
  //delete stack;
  //if we dropped it on a city, then change the ownership.
  City *c = GameMap::getCity(to);
  if (c != NULL && stack->getOwner() != c->getOwner())
    Stacklist::changeOwnership(stack, c->getOwner());
  getStacks(stack->getPos())->arriving(stack);
  bool ship = false;
  if (getTerrainType(stack->getPos()) == Tile::WATER &&
      getBuilding(stack->getPos()) != Maptile::PORT &&
      getBuilding(stack->getPos()) != Maptile::BRIDGE)
    ship = true;
  updateShips(stack->getPos());

  return moved;
}
	
MapBackpack *GameMap::getBackpack(Vector<int> pos)
{
  return getInstance()->getTile(pos)->getBackpack();
}
		    
void GameMap::moveBackpack(Vector<int> from, Vector<int> to)
{
  getBackpack(to)->add(getBackpack(from));
  getBackpack(from)->clear();
}

bool GameMap::removeRuin(Vector<int> pos)
{
  Ruin *r = GameMap::getRuin(pos);
  if (r)
    {
      removeBuilding(r);
      Ruinlist::getInstance()->subtract(r);
      return true;
    }
  return false;
}
bool GameMap::putRuin(Ruin *r)
{
  Ruinlist::getInstance()->add(r);
  putTerrain(r->getArea(), Tile::GRASS);
  putBuilding(r, Maptile::RUIN);
  return true;
}

bool GameMap::removeTemple(Vector<int> pos)
{
  Temple *t = GameMap::getTemple(pos);
  if (t)
    {
      removeBuilding(t);
      Templelist::getInstance()->subtract(t);
      return true;
    }
  return false;
}

bool GameMap::putTemple(Temple *t)
{
  Templelist::getInstance()->add(t);
  putTerrain(t->getArea(), Tile::GRASS);
  putBuilding(t, Maptile::TEMPLE);
  return true;
}

bool GameMap::removePort(Vector<int> pos)
{
  Port *p = GameMap::getPort(pos);
  if (p)
    {
      removeBuilding(p);
      Portlist::getInstance()->subtract(p);
      return true;
    }
  return false;
}

bool GameMap::putPort(Port *p)
{
  Portlist::getInstance()->add(p);
  putBuilding(p, Maptile::PORT);
  //is there a stack here?
  if (GameMap::getStack(p->getPos()) != NULL)
    updateShips(p->getPos());
  return true;
}

bool GameMap::removeSignpost(Vector<int> pos)
{
  Signpost *s = GameMap::getSignpost(pos);
  if (s)
    {
      removeBuilding(s);
      Signpostlist::getInstance()->subtract(s);
      return true;
    }
  return false;
}

bool GameMap::putSignpost(Signpost *s)
{
  Signpostlist::getInstance()->add(s);
  putTerrain(s->getArea(), Tile::GRASS);
  putBuilding(s, Maptile::SIGNPOST);
  return true;
}

bool GameMap::removeRoad(Vector<int> pos)
{
  Road *r = GameMap::getRoad(pos);
  if (r)
    {
      removeBuilding(r);
      Roadlist::getInstance()->subtract(r);
      return true;
    }
  return false;
}

bool GameMap::putRoad(Road *r)
{
  Roadlist::getInstance()->add(r);
  setBuilding(r->getPos(), Maptile::ROAD);

  // now reconfigure all roads in the surroundings
  Vector<int> tile = r->getPos();
  for (int x = tile.x - 1; x <= tile.x + 1; ++x)
    for (int y = tile.y - 1; y <= tile.y + 1; ++y)
      {
	if ((x < 0 || x >= GameMap::getWidth()) &&
	    (y < 0 || y >= GameMap::getHeight()))
	  continue;

	Vector<int> pos(x, y);
	if (Road *r = Roadlist::getInstance()->getObjectAt(pos))
	  {
	    int newtype = CreateScenario::calculateRoadType(pos);
	    r->setType(newtype);
	  }
      }
  return true;
}

bool GameMap::removeBridge(Vector<int> pos)
{
  Bridge *b = GameMap::getBridge(pos);
  if (b)
    {
      removeBuilding(b);
      Bridgelist::getInstance()->subtract(b);
      return true;
    }
  return false;
}

bool GameMap::putBridge(Bridge *b)
{
  Bridgelist::getInstance()->add(b);
  setBuilding(b->getPos(), Maptile::BRIDGE);
  if (GameMap::getStack(b->getPos()) != NULL)
    updateShips(b->getPos());
  return true;
}

Rectangle GameMap::putTerrain(Rectangle r, Tile::Type type, int tile_style_id, bool always_alter_tilestyles)
{
  bool replaced = false;
  int index = getTileset()->getIndex(type);
  if (index == -1)
    return r;
  for (int x = r.x; x < r.x + r.w; ++x)
    for (int y = r.y; y < r.y + r.h; ++y)
      {
	if (offmap(x,y))
	  continue;
	Maptile* t = getTile(Vector<int>(x, y));
	if (t->getMaptileType() != type)
          {
            t->setType(index);
            calculateBlockedAvenue(x, y);
            updateShips(Vector<int>(x,y));
            replaced = true;
          }
      }
  if (tile_style_id == -1)
    {
      if (replaced || always_alter_tilestyles)
        {
          guint32 border = 1;
          r.pos -= Vector<int>(border, border);
          r.dim += Vector<int>(border * 2, border * 2);
          applyTileStyles(r, true);
        }
    }
  else
    {
      for (int x = r.x; x < r.x + r.w; ++x)
        for (int y = r.y; y < r.y + r.h; ++y)
          {
            if (offmap(x,y))
              continue;
	    TileStyle *style = getTileset()->getTileStyle(tile_style_id);
	    getTile(x, y)->setTileStyle(style);
          }
    }

  return r;
}

void GameMap::putBuilding(LocationBox *b, Maptile::Building building)
{
  Rectangle r = b->getArea();
  for (int x = r.x; x < r.x + r.w; ++x)
    for (int y = r.y; y < r.y + r.h; ++y)
      {
	Maptile* t = getTile(Vector<int>(x, y));
	t->setBuilding(building);
        if (building == Maptile::CITY || building == Maptile::PORT || 
            building == Maptile::BRIDGE)
          GameMap::getInstance()->calculateBlockedAvenue(x, y);
      }
}

void GameMap::removeBuilding(LocationBox *b)
{
  Rectangle r = b->getArea();
  for (int x = r.x; x < r.x + r.w; ++x)
    for (int y = r.y; y < r.y + r.h; ++y)
      {
	Maptile* t = getTile(Vector<int>(x, y));
	t->setBuilding(Maptile::NONE);
      }
}

bool GameMap::removeCity(Vector<int> pos)
{
  City *c = GameMap::getCity(pos);
  if (c)
    {
      removeBuilding(c);
      Citylist::getInstance()->subtract(c);
      return true;
    }
  return false;
}
bool GameMap::putCity(City *c)
{
  Player *active = Playerlist::getActiveplayer();

  // create the city
  c->setOwner(active);
  Citylist::getInstance()->add(c);

  putTerrain(c->getArea(), Tile::GRASS);
  // notify the maptiles that a city has been placed here
  putBuilding(c, Maptile::CITY);

  //change allegiance of stacks under this city
  for (unsigned int x = 0; x < c->getSize(); x++)
    {
      for (unsigned int y = 0; y < c->getSize(); y++)
	{
	  Stack *s = getStack(c->getPos() + Vector<int>(x,y));
	  if (s)
	    {
	      if (c->getOwner() == active && s->getFortified() == true)
		s->setFortified(false);
	      if (s->getOwner() != c->getOwner())
		Stacklist::changeOwnership(s, c->getOwner());
	    }
	}
    }
  return true;
}

//the ground changed, and now we need all stacks on a tile to react.
void GameMap::updateShips(Vector<int> pos)
{
  std::list<Stack*> stks = getStacks(pos)->getStacks();
  for (std::list<Stack *>::iterator it = stks.begin(); it != stks.end(); it++)
    {
      for (Stack::iterator sit = (*it)->begin(); sit != (*it)->end(); sit++)
	{
	  if (((*sit)->getStat(Army::MOVE_BONUS) & Tile::WATER) == 0 &&
	      getTerrainType(pos) == Tile::WATER)
	    {
	      if (getBridge(pos) || getPort(pos))
		(*sit)->setInShip(false);
	      else
		(*sit)->setInShip(true);
	    }
	  else
	    (*sit)->setInShip(false);

	}
    }
}
Location *GameMap::getLocation(Vector<int> tile)
{
  switch (getBuilding(tile))
    {
    case Maptile::CITY: return getCity(tile);
    case Maptile::RUIN: return getRuin(tile);
    case Maptile::TEMPLE: return getTemple(tile);
    case Maptile::ROAD: return getRoad(tile);
    case Maptile::BRIDGE: return getBridge(tile);
    case Maptile::SIGNPOST: return getSignpost(tile);
    case Maptile::PORT: return getPort(tile);
    case Maptile::NONE: break;
    }
  return NULL;
}
	
bool GameMap::putStack(Stack *s)
{
  Playerlist::getActiveplayer()->addStack(s);
  getStacks(s->getPos())->add(s);
  updateShips(s->getPos());
  return true;
}

void GameMap::removeStack(Stack *s)
{
  getStacks(s->getPos())->leaving(s);
  Playerlist::getActiveplayer()->deleteStack(s);
}
	
guint32 GameMap::countArmyUnits(Vector<int> pos)
{
  return getStacks(pos)->countNumberOfArmies(Playerlist::getActiveplayer());
}

std::list<Stack*> GameMap::getNearbyFriendlyStacks(Vector<int> pos, int dist)
{
  return getNearbyStacks(pos, dist, true);
}

std::list<Stack*> GameMap::getNearbyEnemyStacks(Vector<int> pos, int dist)
{
  return getNearbyStacks(pos, dist, false);
}

std::list<Stack*> GameMap::getNearbyStacks(Vector<int> pos, int dist, bool friendly)
{
  std::list<Stack *> stacks;
  guint32 i, j;
  guint32 d;
  guint32 max = dist;
  int x, y;

  std::list<Stack*> stks;
  if (friendly)
    stks = GameMap::getFriendlyStacks(pos);
  else
    stks = GameMap::getEnemyStacks(pos);
  stacks.merge(stks);

  //d is the distance from Pos where our box starts
  //instead of a regular loop around a box of dist large, we're going to add
  //the nearer stacks first.
  for (d = 1; d < max; d++)
    {
      for (i = 0; i < (d * 2) + 1; i++)
        {
          for (j = 0; j < (d * 2) + 1; j++)
            {
              if ((i == 0 || i == (d * 2) + 1) && 
                  (j == 0 || j == (d * 2) + 1))
                {
                  x = pos.x + (i - d);
                  y = pos.y + (j - d);
                  if (x < 0 || y < 0)
                    continue;
		  if (offmap(x, y))
		    continue;
		  //are there any stacks here?
		  if (friendly)
		    stks = GameMap::getFriendlyStacks(Vector<int>(x,y));
		  else
		    stks = GameMap::getEnemyStacks(Vector<int>(x,y));
		  stacks.merge(stks);
                }
            }
        }
    }

  return stacks;
}

bool GameMap::checkCityAccessibility()
{
  //check to see if all cities are accessible
  //check if all cities are accessible
  Citylist *cl = Citylist::getInstance();
  if (cl->size() <= 1)
    return true;
  Vector<int> pos = GameMap::getCenterOfMap();
  City *center = Citylist::getInstance()->getNearestCity(pos);
  Stack s(NULL, center->getPos());
  ArmyProto *basearmy = ArmyProto::createScout();
  Army *a = Army::createNonUniqueArmy(*basearmy);
  delete basearmy;
  s.push_back(a);
  PathCalculator pc(&s, true, 10, 10);

  for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
    {
      if (center == *it)
	continue;

      int mp = pc.calculate((*it)->getPos());
      if (mp <= 0)
	{
	  printf("we made a map that has an inaccessible city (%d)\n", mp);
	  printf("can't get from %s to %s\n", (*it)->getName().c_str(), center->getName().c_str());
	  return false;
	}
    }
  return true;
}

Vector<int> GameMap::getCenterOfMap()
{
  return Vector<int>(GameMap::s_width/2, GameMap::s_height/2);
}

