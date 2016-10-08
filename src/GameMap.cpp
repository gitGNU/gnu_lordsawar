// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2003, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2014, 2015 Ben Asselstine
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
#include <string>
#include <iomanip>
#include <assert.h>
#include <sigc++/functors/mem_fun.h>
#include <string.h>
#include <math.h>

#include "ucompose.hpp"
#include "army.h"
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
#include "MapBackpack.h"
#include "stacktile.h"
#include "armyprodbase.h"
#include "stack.h"
#include "armyset.h"
#include "armysetlist.h"
#include "CreateScenario.h"
#include "SightMap.h"
#include "reward.h"
#include "rewardlist.h"

Glib::ustring GameMap::d_tag = "map";
Glib::ustring GameMap::d_itemstack_tag = "itemstack";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<flush;}
#define debug(x)

GameMap* GameMap::s_instance = 0;

int GameMap::s_width = 112;
int GameMap::s_height = 156;
Tileset* GameMap::s_tileset = 0;
Cityset* GameMap::s_cityset = 0;
Shieldset* GameMap::s_shieldset = 0;


GameMap* GameMap::getInstance(Glib::ustring TilesetName, 
			      Glib::ustring ShieldsetName, 
			      Glib::ustring CitysetName)
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

GameMap::GameMap(Glib::ustring TilesetName, Glib::ustring ShieldsetName,
		 Glib::ustring CitysetName)
{
  s_tileset = 0;
  s_cityset = 0;
  s_shieldset = 0;
  if (TilesetName != "")
    d_tileset = TilesetName;
  if (ShieldsetName != "")
    d_shieldset = ShieldsetName;
  if (CitysetName != "")
    d_cityset = CitysetName;

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

void GameMap::processStyles(Glib::ustring s, int chars_per_style)
{
  std::string styles = s.raw();
  Tileset *tileset = GameMap::getTileset();
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
	    TileStyle *style = tileset->getTileStyle(id);
	    if (!style)
	      style = tileset->getTileStyle(0);
	    d_map[j*s_width + i]->setTileStyle(style);
        }
    }
}

int GameMap::determineCharsPerStyle(Glib::ustring styles)
{
  return styles.length() / (s_width * s_height);
}

GameMap::GameMap(XML_Helper* helper)
{
    s_tileset = 0;
    s_cityset = 0;
    s_shieldset = 0;
    Glib::ustring types;
    Glib::ustring styles;
    Glib::ustring t_dir;
    Glib::ustring s_dir;
    Glib::ustring c_dir;

    helper->getData(s_width, "width");
    helper->getData(s_height, "height");
    helper->getData(t_dir,"tileset");
    helper->getData(s_dir,"shieldset");
    helper->getData(c_dir,"cityset");
    helper->getData(types, "types");
    helper->getData(styles, "styles");

    d_tileset = t_dir;
    d_shieldset = s_dir;
    d_cityset = c_dir;

    Tileset *tileset = Tilesetlist::getInstance()->get(d_tileset);
    s_tileset = tileset;
    Cityset *cityset = Citysetlist::getInstance()->get(d_cityset);
    s_cityset = cityset;
    Shieldset *shieldset = Shieldsetlist::getInstance()->get(d_shieldset);
    s_shieldset = shieldset;
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
            d_map[j*s_width + i] = new Maptile(i, j, type);
        }
    }

    int chars_per_style = determineCharsPerStyle(styles);
    processStyles(styles, chars_per_style);

    //add some callbacks for item loading
    helper->registerTag(MapBackpack::d_tag, sigc::mem_fun(this, &GameMap::loadItems));
}


GameMap::~GameMap()
{
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
    int width = 0;
    int height = 0;
    const Tile::Type* terrain = generator->getMap(width, height);
    Tileset *tileset = GameMap::getTileset();

    //the sizes should definitely match, else we have a problem here
    if (width != s_width || height != s_height)
      {
        std::cerr << String::ucompose(_("Error!  Map Generator and Game Map tile sizes differ.  %1 != %2 || %3 != %4.  Exiting."), width, s_width, height, s_height) << std::endl;
        exit(-1);
      }

    // create tiles; there is a hack here: The map generator outputs tile types,
    // but we supply the index of the tile types in the tileset to Maptile. Was
    // the easiest version when rewriting this.
    for (int j = 0; j < height; j++)
        for (int i = 0; i < width; i++)
        {
            int index = tileset->getIndex(terrain[j*width + i]);
	    if (index != -1)
	      d_map[j*s_width + i] = new Maptile(i, j, (guint32)index);
        }

    applyTileStyles(0, 0, height, width, true);
    return true;
}

bool GameMap::fill(guint32 type)
{
  for (int i = 0; i < s_width; i++)
    for (int j = 0; j < s_height; j++)
      d_map[j*s_width + i] = new Maptile(i, j, type);

  applyTileStyles(0, 0, s_height, s_width, false);
  return true;
}

bool GameMap::save(XML_Helper* helper) const
{
    Tileset *tileset = GameMap::getTileset();
    bool retval = true;

    std::stringstream types;

    types <<std::endl;
    for (int i = 0; i < s_height; i++)
    {
        for (int j = 0; j < s_width; j++)
          {
            guint32 tile_type = getTile(j, i)->getIndex();
            types << (guint32) tile_type;
          }
        types <<std::endl;
    }

    std::stringstream styles;
    styles <<std::endl;
	    
    int largest_style_id = tileset->getLargestTileStyleId();
    guint32 num_digits = TileStyle::calculateHexDigits(largest_style_id);
    for (int i = 0; i < s_height; i++)
    {
        for (int j = 0; j < s_width; j++)
	  {
	    Glib::ustring hexstr;
	    TileStyle *style = getTile(j, i)->getTileStyle();
	    assert (style != NULL);
            hexstr = TileStyle::idToString(style->getId(), num_digits);
	    styles << hexstr;
	  }
        styles <<std::endl;
    }


    retval &= helper->openTag(GameMap::d_tag);
    retval &= helper->saveData("width", s_width);
    retval &= helper->saveData("height", s_height);
    retval &= helper->saveData("tileset", d_tileset);
    retval &= helper->saveData("shieldset", d_shieldset);
    retval &= helper->saveData("cityset", d_cityset);
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

bool GameMap::loadItems(Glib::ustring tag, XML_Helper* helper)
{
    if (tag == MapBackpack::d_tag)
      {
        MapBackpack* backpack = new MapBackpack(helper);
        Vector<int> pos = backpack->getPos();
        getTile(pos)->setBackpack(backpack);
      }

    return true;
}

void GameMap::setTile(int x, int y, Maptile *tile)
{
    delete d_map[y*s_width + x];
    d_map[y*s_width + x] = tile;
    applyTileStyle (y, x);
}

Stack* GameMap::addArmy(Vector<int> pos, Army *a)
{
  City *c = getCity(pos);
  if (c)
    {
      if (c->isBurnt() || c->getOwner() != a->getOwner())
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

  if (getBuilding(pos) == Maptile::NONE)
    {
      Location l(pos, 1);
      s = l.addArmy(a);
      if (s)
        return s;
    }

  // we couldn't add the army to the square(s) identified by location,
  // so the idea is to go around in ever widening boxes until we find a
  // suitable tile.

  bool land = true;
  if (getTile(pos.x, pos.y)->getType() == Tile::WATER)
    land = false;

  //d is the distance from Pos where our box starts
  for (d = 1; d < max; d++)
    {
      guint32 imax = (d * 2) + 1;
      guint32 jmax = (d * 2) + 1;
      for (i = 0; i < imax; i++)
        {
          for (j = 0; j < jmax; j++)
            {
              if ((i == 0 || i == imax - 1) && 
                  (j == 0 || j == jmax - 1))
                {
                  x = pos.x + (i - d);
                  y = pos.y + (j - d);
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
                      Vector<int> p(x, y);
		      //hmm. no nifty stacks here.  anybody else's?
		      s = getEnemyStack(p);
		      if (s)
			continue;
		      //okay, no stacks here at all.  make one.
                      s = new Stack(a->getOwner(), p);
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
  //if (Citylist::getInstance()->empty())
      //return false;
  int diffx = destx - x;
  int diffy = desty - y;
  if (diffx >= -1 && diffx <= 1 && diffy >= -1 && diffy <= 1)
    {
      //assert (Citylist::getInstance()->size());
      bool from_dock = isDock(Vector<int>(x,y));
      bool to_dock = isDock(Vector<int>(destx,desty));
      Maptile *from = getTile(x, y);
      Maptile *to = getTile(destx, desty);
      if (from == to)
        return false;
      //am i on land, going towards water that has a port on it?
      //if (from->getType() != Tile::WATER &&
          //to->getType() == Tile::WATER &&
          //to_dock)
        //return false;
      //am i on water going towards land from a port?
      //if (from->getType() == Tile::WATER &&
          //to->getType() != Tile::WATER &&
          //from_dock)
        //return false;

      //am i on water going towards land that isn't a city,
      //and i'm not coming from a port
      if (from->getType() == Tile::WATER && to->getType() != Tile::WATER &&
          !to_dock && !from_dock)
        return true;

      //am i on land, going towards water from a tile that isn't a
      //city, or a port and i'm not going to a port?
      if (from->getType() != Tile::WATER && to->getType() == Tile::WATER &&
          !from_dock && !to_dock)
        return true;

      //is the tile i'm going to a mountain that doesn't have a road?
      if (to->getType() == Tile::MOUNTAIN && 
          getRoad(Vector<int>(destx, desty)) == NULL)
        return true;

      //am i on a mountain without a road?
      if (from->getType() == Tile::MOUNTAIN &&
	  getRoad(Vector<int>(x, y)) == NULL)
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

std::list<MapBackpack*> GameMap::getBackpacks() const
{
  std::list<MapBackpack*> bags;
  for (int x = 0; x < getWidth(); x++)
    {
      for (int y = 0; y < getHeight(); y++)
        {
          MapBackpack *backpack = getTile(x, y)->getBackpack();
          if (backpack->size() > 0)
            bags.push_back(backpack);
        }
    }
  return bags;
}


TileStyle *GameMap::calculatePreferredStyle(int i, int j)
{
  Tileset *tileset = GameMap::getTileset();
  Maptile *mtile = getTile(j, i);
  int box[3][3];
  for (int k = -1; k <= +1; k++)
    for (int l = -1; l <= +1; l++)
      {
	box[k+1][l+1] = 1;
	if (offmap(j+l, i+k))
	  continue;
	box[k+1][l+1] = 
          are_those_tiles_similar(getTile(j+l, i+k)->getType(),
                                  mtile->getType(), false);
      }
  if (box[0][0] && box[0][1] && box[0][2] &&
      box[1][0] && box[1][1] && box[1][2] &&
      box[2][0] && box[2][1] && box[2][2])
    return tileset->getRandomTileStyle(mtile->getIndex(), 
				       TileStyle::INNERMIDDLECENTER);
  else if (box[0][0] && box[0][1] && !box[0][2] && 
	   box[1][0] && box[1][1] && box[1][2] &&
	   !box[2][0] && box[2][1] && box[2][2])
    return tileset->getRandomTileStyle(mtile->getIndex(), 
				       TileStyle::TOPLEFTTOBOTTOMRIGHTDIAGONAL);
  else if (!box[0][0] && box[0][1] && box[0][2] && 
	   box[1][0] && box[1][1] && box[1][2] &&
	   box[2][0] && box[2][1] && !box[2][2])
    return tileset->getRandomTileStyle(mtile->getIndex(), 
				       TileStyle::BOTTOMLEFTTOTOPRIGHTDIAGONAL);
  else if (/*box[0][0] &&*/ !box[0][1] && /*box[0][2] &&*/
	   !box[1][0] && box[1][1] && box[1][2] &&
	   /*!box[2][0] &&*/ box[2][1] && box[2][2])
    return tileset->getRandomTileStyle(mtile->getIndex(), 
				       TileStyle::OUTERTOPLEFT);
  else if (/*box[0][0] &&*/ !box[0][1] && /*box[0][2] &&*/
	   box[1][0] && box[1][1] && !box[1][2] &&
	   box[2][0] && box[2][1] /*&& !box[2][2] */)
    return tileset->getRandomTileStyle(mtile->getIndex(), 
				       TileStyle::OUTERTOPRIGHT);
  else if (/*box[0][0] &&*/ box[0][1] && box[0][2] &&
	   !box[1][0] && box[1][1] && box[1][2] &&
	   /*box[2][0] &&*/ !box[2][1] /*&& box[2][2]*/)
    return tileset->getRandomTileStyle(mtile->getIndex(), 
				       TileStyle::OUTERBOTTOMLEFT);
  else if (box[0][0] && box[0][1] && /*!box[0][2] &&*/
	   box[1][0] && box[1][1] && !box[1][2] && 
	   /*box[2][0] &&*/ !box[2][1] /*&& box[2][2]*/)
    return tileset->getRandomTileStyle(mtile->getIndex(), 
				       TileStyle::OUTERBOTTOMRIGHT);
  else if (/*box[0][0] &&*/ box[0][1] && /*box[0][2] && */
	   !box[1][0] && box[1][1] && box[1][2] &&
	   /*box[2][0] &&*/ box[2][1] /*&& box[2][2]*/)
    return tileset->getRandomTileStyle(mtile->getIndex(), 
				       TileStyle::OUTERMIDDLELEFT);
  else if (/*box[0][0] &&*/ box[0][1] && /*box[0][2] && */
	   box[1][0] && box[1][1] && !box[1][2] &&
	   /*box[2][0] &&*/ box[2][1] /*&& box[2][2] */)
    return tileset->getRandomTileStyle(mtile->getIndex(), 
				       TileStyle::OUTERMIDDLERIGHT);
  else if (box[0][0] && box[0][1] && /*box[0][2] && */
	   box[1][0] && box[1][1] && box[1][2] &&
	   box[2][0] && box[2][1] && !box[2][2])
    return tileset->getRandomTileStyle(mtile->getIndex(), 
				       TileStyle::INNERTOPLEFT);
  else if (/*box[0][0] &&*/ box[0][1] && box[0][2] && 
	   box[1][0] && box[1][1] && box[1][2] &&
	   !box[2][0] && box[2][1] && box[2][2])
    return tileset->getRandomTileStyle(mtile->getIndex(), 
				       TileStyle::INNERTOPRIGHT);
  else if (box[0][0] && box[0][1] && !box[0][2] && 
	   box[1][0] && box[1][1] && box[1][2] &&
	   box[2][0] && box[2][1] /*&& box[2][2]*/)
    return tileset->getRandomTileStyle(mtile->getIndex(), 
				       TileStyle::INNERBOTTOMLEFT);
  else if (!box[0][0] && box[0][1] && box[0][2] && 
	   box[1][0] && box[1][1] && box[1][2] &&
	   /*box[2][0] &&*/ box[2][1] && box[2][2])
    return tileset->getRandomTileStyle(mtile->getIndex(), 
				       TileStyle::INNERBOTTOMRIGHT);
  else if (/*!box[0][0] &&*/ !box[0][1] && /*!box[0][2] &&*/
	   box[1][0] && box[1][1] && box[1][2] &&
	   /*!box[2][0] &&*/ box[2][1] /*&& box[2][2]*/)
    return tileset->getRandomTileStyle(mtile->getIndex(), 
				       TileStyle::OUTERTOPCENTER);
  else if (/*box[0][0] &&*/ box[0][1] && /*box[0][2] &&*/
	   box[1][0] && box[1][1] && box[1][2] &&
	   /*!box[2][0] &&*/ !box[2][1] /*&& !box[2][2]*/)
    return tileset->getRandomTileStyle(mtile->getIndex(), 
				       TileStyle::OUTERBOTTOMCENTER);
  return NULL;
}

void GameMap::close_circles (int minx, int miny, int maxx, int maxy)
{
  Tileset *tileset = GameMap::getTileset();
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
		  style = tileset->getRandomTileStyle(tile->getIndex(),
						      TileStyle::OUTERTOPRIGHT);
		  tile->setTileStyle(style);
		  style = tileset->getRandomTileStyle(nexttile->getIndex(),
						      TileStyle::OUTERBOTTOMLEFT);
		  nexttile->setTileStyle(style);
		}
	      if (tilestyle->getType() == TileStyle::OUTERBOTTOMCENTER &&
		  nextstyle->getType() == TileStyle::OUTERTOPCENTER)
		{
		  TileStyle *style;
		  style = tileset->getRandomTileStyle(tile->getIndex(),
						      TileStyle::OUTERBOTTOMRIGHT);
		  tile->setTileStyle(style);
		  style = tileset->getRandomTileStyle(nexttile->getIndex(),
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
		  style = tileset->getRandomTileStyle(tile->getIndex(),
						      TileStyle::OUTERBOTTOMRIGHT);
		  tile->setTileStyle(style);
		  style = tileset->getRandomTileStyle(nexttile->getIndex(),
						      TileStyle::OUTERTOPLEFT);
		  nexttile->setTileStyle(style);
		}
	      if (tilestyle->getType() == TileStyle::OUTERMIDDLELEFT&&
		  nextstyle->getType() == TileStyle::OUTERMIDDLERIGHT)
		{
		  TileStyle *style;
		  style = tileset->getRandomTileStyle(tile->getIndex(),
						      TileStyle::OUTERBOTTOMLEFT);
		  tile->setTileStyle(style);
		  style = tileset->getRandomTileStyle(nexttile->getIndex(),
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
	box[k+1][l+1] = are_those_tiles_similar(getTile(j+l, i+k)->getType(), 
                                                tile, true);
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
  Tileset *tileset = GameMap::getTileset();
  int i;
  int j;
  for (i = minx; i < maxx; i++)
    for (j = miny; j < maxy; j++)
      {
	if (offmap(j, i))
	  continue;
	Tile::Type tile = getTile(j, i)->getType();
	if (tile == intype)
	  {
	    //if we're not connected in a square of
	    //same types, then we're a lone tile.
            if (getTile(j,i)->getBuilding())
              continue;
	    if (tile_is_connected_to_other_like_tiles(tile, i, j) == 0)
	      {
		//okay, this is a lone tile.
		//downgrade it
		int idx = tileset->getIndex(outtype);
		if (idx != -1)
		  setTile(j, i, new Maptile (j, i, (guint32)idx));
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
  Tileset *tileset = GameMap::getTileset();
  for(int j = miny; j < maxy; j++)
    for(int i = minx; i < maxx; i++)
      {
	if (offmap(j, i))
	  continue;
	if(getTile(j, i)->getType() == Tile::MOUNTAIN)
	  for(int J = -1; J <= +1; ++J)
	    for(int I = -1; I <= +1; ++I)
	      if((!(offmap(j+J,i+I))) &&
		 (getTile((j+J),(i+I))->getType() != Tile::MOUNTAIN))
		{
		  int idx = tileset->getIndex(Tile::HILLS);
		  if(getTile((j+J), (i+I))->getType() != Tile::WATER)
		    {
		      if (idx != -1)
                        {
                          Maptile::Building b = getTile(j+J, i+I)->getBuilding();
                          setTile(j+J, i+I,
                                  new Maptile (j+J, i+I, (guint32)idx));
                          if (b)
                            setBuilding(Vector<int>(j+J,i+I), b);
                        }
		    }
		  else 
		    {
		    // water has priority here, there was some work done to conenct bodies of water
		    // so don't break those connections.
		      setTile(j, i, new Maptile (j, i, (guint32)idx));
		    }
		}
      }
}

void GameMap::applyTileStyle (int i, int j)
{
  Maptile *mtile = getTile(j, i);
  Tileset *tileset = GameMap::getTileset();

  TileStyle *style = calculatePreferredStyle(i, j);
  if (!style)
    style = tileset->getRandomTileStyle(mtile->getIndex(), 
					TileStyle::LONE);
  if (!style)
    style = tileset->getRandomTileStyle(mtile->getIndex(), 
					TileStyle::INNERMIDDLECENTER);
  if (!style)
    printf ("applying null tile style at %d,%d for tile of kind %d\n", i, j,
	    mtile->getType());
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
  if (getInstance()->getBuilding(pos) != Maptile::CITY)
    return NULL;
  return Citylist::getInstance()->getObjectAt(pos);
}

City* GameMap::getEnemyCity(Vector<int> pos)
{
  if (getInstance()->getBuilding(pos) != Maptile::CITY)
    return NULL;
  City *c = Citylist::getInstance()->getObjectAt(pos);
  if (c && c->getOwner() != Playerlist::getActiveplayer())
    return c;
  return NULL;
}

Ruin* GameMap::getRuin(Vector<int> pos)
{
  if (getInstance()->getBuilding(pos) != Maptile::RUIN)
    return NULL;
  return Ruinlist::getInstance()->getObjectAt(pos);
}

Temple* GameMap::getTemple(Vector<int> pos)
{
  if (getInstance()->getBuilding(pos) != Maptile::TEMPLE)
    return NULL;
  return Templelist::getInstance()->getObjectAt(pos);
}

Port* GameMap::getPort(Vector<int> pos)
{
  if (getInstance()->getBuilding(pos) != Maptile::PORT)
    return NULL;
  return Portlist::getInstance()->getObjectAt(pos);
}

Road* GameMap::getRoad(Vector<int> pos)
{
  if (getInstance()->getBuilding(pos) != Maptile::ROAD)
    return NULL;
  return Roadlist::getInstance()->getObjectAt(pos);
}

Bridge* GameMap::getBridge(Vector<int> pos)
{
  if (getInstance()->getBuilding(pos) != Maptile::BRIDGE)
    return NULL;
  return Bridgelist::getInstance()->getObjectAt(pos);
}

Signpost* GameMap::getSignpost(Vector<int> pos)
{
  if (getInstance()->getBuilding(pos) != Maptile::SIGNPOST)
    return NULL;
  return Signpostlist::getInstance()->getObjectAt(pos);
}

Stack* GameMap::getFriendlyStack(Vector<int> pos)
{
  return getStacks(pos)->getFriendlyStack(Playerlist::getActiveplayer());
}

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
  if (!getStacks(pos))
    {
      std::list<Stack*> empty;
      return empty;
    }

  if (player == NULL)
    player = Playerlist::getActiveplayer();
  return getStacks(pos)->getEnemyStacks(player);
}


bool GameMap::compareStackStrength(Stack *lhs, Stack *rhs)
{
  Army *lhero = lhs->getStrongestHero();
  Army *rhero = rhs->getStrongestHero();
  if (lhero && rhero)
    return 
      lhero->getStat(Army::STRENGTH) > rhero->getStat(Army::STRENGTH);
  else if (!lhero && rhero)
    return false;
  else if (lhero && !rhero)
    return true;

  Army *larmy = lhs->getStrongestArmy();
  Army *rarmy = rhs->getStrongestArmy();
  return larmy->getStat(Army::STRENGTH) > rarmy->getStat(Army::STRENGTH);
}

Stack* GameMap::getStrongestStack(Vector<int> pos)
{
  StackTile *s = getStacks(pos);
  std::list<Stack*> stacks = s->getStacks();
  if (stacks.empty())
    return NULL;
  stacks.sort(compareStackStrength);
  return stacks.front();
}

Stack* GameMap::getStack(Vector<int> pos)
{
  if (getStacks(pos))
    return getStacks(pos)->getStack();
  else
    return NULL;
}
	
StackTile* GameMap::getStacks(Vector<int> pos)
{
  if (getInstance()->getTile(pos))
    return getInstance()->getTile(pos)->getStacks();
  else
    return NULL;
}

Stack *GameMap::groupStacks(Vector<int> pos)
{
  return getInstance()->groupStacks(pos, Playerlist::getActiveplayer());
}

Stack *GameMap::groupStacks(Vector<int> pos, Player *player)
{
  if (getStacks(pos))
    return getStacks(pos)->group(player);
  else
    return NULL;
}
  
void GameMap::groupStacks(Stack *stack)
{
  if (getStacks(stack->getPos()))
    return getStacks(stack->getPos())->group(Playerlist::getActiveplayer(), stack);
}
  
void GameMap::clearStackPositions()
{
  Playerlist *plist = Playerlist::getInstance();
  for (Playerlist::iterator i = plist->begin(); i != plist->end(); i++)
    {
      Stacklist *sl = (*i)->getStacklist();
      for (Stacklist::iterator s = sl->begin(); s != sl->end(); s++)
        {
          Vector<int> pos = (*s)->getPos();
          StackTile *st = getStacks(pos);
          st->clear();
        }
    }
}
void GameMap::updateStackPositions()
{
  Playerlist *plist = Playerlist::getInstance();
  for (Playerlist::iterator i = plist->begin(); i != plist->end(); i++)
    {
      Stacklist *sl = (*i)->getStacklist();
      for (Stacklist::iterator s = sl->begin(); s != sl->end(); s++)
        {
          Vector<int> pos = (*s)->getPos();
          getStacks(pos)->add(*s);
        }
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
  d_tileset = tileset->getBaseName();
  s_tileset = Tilesetlist::getInstance()->get(d_tileset);
  for (int i = 0; i < s_width; i++)
    for (int j = 0; j < s_height; j++)
      {
        //there is also the problem of the index being kept in the maptile.
        //perhaps we need to get a new index, right? e.g. when we switch
        //the tileset for another one in the editor.
        //because "Grass" won't always be in the 0th spot in the tileset.
        d_map[j*s_width + i]->setIndex(d_map[j*s_width + i]->getIndex());
      }
  applyTileStyles (0, 0, s_width, s_height,  false);
}

void GameMap::reloadTileset()
{
  Tileset *tileset = GameMap::getTileset();
  if (tileset)
    Tilesetlist::getInstance()->reload(tileset->getId());
}

void GameMap::reloadShieldset()
{
  Shieldset *shieldset = GameMap::getShieldset();
  if (shieldset)
    {
      Shieldsetlist::getInstance()->reload(shieldset->getId());
      Playerlist::getInstance()->setNewColours(shieldset);
    }
}

void GameMap::switchShieldset(Shieldset *shieldset)
{
  Playerlist::getInstance()->setNewColours(shieldset);
  d_shieldset = shieldset->getBaseName();
  s_shieldset = Shieldsetlist::getInstance()->get(d_shieldset);
}

Vector<int> GameMap::findNearestAreaForBuilding(Maptile::Building building_type, Vector<int> pos, guint32 width)
{
  std::list<Vector<int> > points = getNearbyPoints(pos, -1);
  std::list<Vector<int> >::iterator it = points.begin();
  for (;it != points.end(); it++)
    {
      if (canPutBuilding (building_type, width, *it, true))
        return *it;
    }
  return Vector<int>(-1,-1);
}

void GameMap::switchCityset(Cityset *cityset)
{
  setCityset(cityset->getBaseName());

  if (Templelist::getInstance()->size())
    {
      guint32 tiles = 
        GameMap::getInstance()->countBuildings(Maptile::TEMPLE) / 
        Templelist::getInstance()->size();
      double old_tile_width = sqrt ((double)tiles);
      if (old_tile_width != cityset->getTempleTileWidth())
        Templelist::getInstance()->resizeLocations
          (Maptile::TEMPLE, cityset->getTempleTileWidth(), old_tile_width, 
           (void (*)(Location*, Maptile::Building, guint32)) changeFootprintToSmallerCityset, 
           (void (*)(Location*, Maptile::Building, guint32)) relocateLocation);
    }
    
  if (Ruinlist::getInstance()->size())
    {
      guint32 tiles = GameMap::getInstance()->countBuildings(Maptile::RUIN) / 
        Ruinlist::getInstance()->size();
      double old_tile_width = sqrt ((double)tiles);
      if (old_tile_width != cityset->getRuinTileWidth())
        Ruinlist::getInstance()->resizeLocations
          (Maptile::RUIN, cityset->getRuinTileWidth(), old_tile_width, 
           (void (*)(Location*, Maptile::Building, guint32)) changeFootprintToSmallerCityset, 
           (void (*)(Location*, Maptile::Building, guint32)) relocateLocation);
    }
  if (Citylist::getInstance()->size())
    {
      guint32 tiles = GameMap::getInstance()->countBuildings(Maptile::CITY) / 
        Citylist::getInstance()->size();
      double old_tile_width = sqrt ((double)tiles);
      if (old_tile_width != cityset->getCityTileWidth())
        Citylist::getInstance()->resizeLocations
          (Maptile::CITY, cityset->getCityTileWidth(), old_tile_width, 
           (void (*)(Location*, Maptile::Building, guint32)) changeFootprintToSmallerCityset, 
           (void (*)(Location*, Maptile::Building, guint32)) relocateLocation);
    }
}

guint32 GameMap::countBuildings(Maptile::Building building_type)
{
  guint32 count = 0;
  for (int x = 0; x < getWidth(); x++)
    {
      for (int y = 0; y < getHeight(); y++)
        {
          Vector<int> pos = Vector<int>(x, y);
          if (getBuilding(pos) == building_type)
            count++;
        }
    }
  return count;
}

void GameMap::reloadCityset()
{
  Cityset *cityset = GameMap::getCityset();
  if (cityset)
    {
      Citysetlist::getInstance()->reload(cityset->getId());
      switchCityset(cityset);  //is this still needed?
    }
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
      s->removeArmiesWithoutArmyType(armyset->getId());
      for (Stack::iterator j = s->begin(); j != s->end(); j++)
	Armyset::switchArmysetForRuinKeeper(*j, armyset);
    }
  for (Playerlist::iterator i = pl->begin(); i != pl->end(); i++)
    {
      //change the armyprodbases in cities.
      for (Citylist::iterator j = cl->begin(); j != cl->end(); j++)
	{
	  City *c = *j;
          c->removeArmyProdBasesWithoutAType(armyset->getId());
	  for (unsigned int k = 0; k < c->getSize(); k++)
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
          s->removeArmiesWithoutArmyType(armyset->getId());
          if (s->size() == 0)
            {
              GameMap::getInstance()->getStacks(s->getPos())->leaving(s);
              j=sl->flErase(j);//this doesn't remove the stack from the map of id->stack pointer in stacklist. XXX XXX XXX
              if (sl->size() > 0)
                j--;
              continue;
            }
          for (Stack::iterator k = s->begin(); k != s->end(); k++)
            Armyset::switchArmyset(*k,armyset);
	}

      //finally, change the player's armyset.
      (*i)->setArmyset(armyset->getId());
      //where else are armyset ids hanging around?
    }
}

void GameMap::reloadArmyset(Armyset *armyset)
{
  Armysetlist::getInstance()->reload(armyset->getId());
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
      case Maptile::RUIN: 
      case Maptile::TEMPLE: 
      case Maptile::SIGNPOST:
	  {
	    if (making_islands)
	      return true;
	    for (unsigned int i = 0; i < size; i++)
	      for (unsigned int j = 0; j < size; j++)
		{
		  Vector<int> pos = to + Vector<int>(i, j);
		  if (getTerrainType(pos) == Tile::WATER)
		    return false;
		}
	  }
	break;
      case Maptile::ROAD: 
	//can't be in the water
	if (getTerrainType(to) == Tile::WATER)
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

bool GameMap::moveBuilding(Vector<int> from, Vector<int> to, guint32 new_width)
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
          if (new_width)
            new_signpost->setSize(new_width);
	  putSignpost(new_signpost);
	  break;
	}
    case Maptile::PORT:
	{
	  Port *old_port = getPort(getPort(from)->getPos());
	  Port *new_port = new Port(*old_port, to);
	  removePort(old_port->getPos());
          if (new_width)
            new_port->setSize(new_width);
	  putPort(new_port);
	  break;
	}
    case Maptile::BRIDGE:
	{
	  Bridge *old_bridge = getBridge(getBridge(from)->getPos());
	  Bridge *new_bridge = new Bridge(*old_bridge, to);
	  removeBridge(old_bridge->getPos());
          if (new_width)
            new_bridge->setSize(new_width);
	  putBridge(new_bridge);
	  break;
	}
    case Maptile::ROAD:
	{
	  Road *old_road = getRoad(getRoad(from)->getPos());
	  Road *new_road = new Road(*old_road, to);
	  removeRoad(old_road->getPos());
          if (new_width)
            new_road->setSize(new_width);
	  putRoad(new_road);
	  break;
	}
    case Maptile::RUIN:
	{
	  Ruin* old_ruin = getRuin(getRuin(from)->getPos());
	  Ruin *new_ruin = new Ruin(*old_ruin, to);
	  removeRuin(old_ruin->getPos());
          if (new_width)
            new_ruin->setSize(new_width);
	  putRuin(new_ruin);
	  break;
	}
    case Maptile::TEMPLE:
	{
	  Temple* old_temple = getTemple(getTemple(from)->getPos());
	  Temple* new_temple = new Temple(*old_temple, to);
	  removeTemple(old_temple->getPos());
          if (new_width)
            new_temple->setSize(new_width);
	  putTemple(new_temple);
	  break;
	}
    case Maptile::CITY:
	{
	  City* old_city = getCity(getCity(from)->getPos());
	  City* new_city = new City(*old_city, to);
	  removeCity(old_city->getPos());
          if (new_width)
            new_city->setSize(new_width);
	  putCity(new_city, true);
	  break;
	}
    }
  return moved;
}

void GameMap::setBuilding(Vector<int> tile, Maptile::Building building)
{
  if (getTile(tile))
    getTile(tile)->setBuilding(building);
}

guint32 GameMap::getBuildingSize(Vector<int> tile)
{
  if (getTile(tile) == NULL)
    return 0;
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
  City *c = GameMap::getCity(to);
  if (c != NULL && stack->getOwner() != c->getOwner())
    stack = Stacklist::changeOwnership(stack, c->getOwner());
  getStacks(stack->getPos())->arriving(stack);
  updateShips(stack->getPos());

  return moved;
}
	
MapBackpack *GameMap::getBackpack(Vector<int> pos)
{
  if (getInstance()->getTile(pos))
    return getInstance()->getTile(pos)->getBackpack();
  else
    return NULL;
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

bool GameMap::containsWater(Rectangle rect)
{
  for (int y = rect.y; y < rect.y + rect.h; y++)
    for (int x = rect.x; x < rect.x + rect.w; x++)
      if (getTile(x, y)->getType() == Tile::WATER)
        return true;
  return false;
}

bool GameMap::putRuin(Ruin *r)
{
  Ruinlist::getInstance()->add(r);
  if (containsWater(r->getArea()))
    putTerrain(r->getArea(), Tile::GRASS);
  putBuilding(r, Maptile::RUIN);
  return true;
}

bool GameMap::removeLocation (Vector<int> pos)
{
  switch (getBuilding(pos))
    {
    case Maptile::CITY: return removeCity(pos);
    case Maptile::RUIN: return removeRuin(pos);
    case Maptile::TEMPLE: return removeTemple(pos);
    case Maptile::ROAD: return removeRoad(pos);
    case Maptile::BRIDGE: return removeBridge(pos);
    case Maptile::SIGNPOST: return removeSignpost(pos);
    case Maptile::PORT: return removePort(pos);
    case Maptile::NONE: break;
    }
  return false;
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
  if (containsWater(t->getArea()))
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
  if (containsWater(s->getArea()))
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

bool GameMap::putNewRoad(Vector<int> tile)
{
  Road *r = new Road(tile);
  if (r)
    return putRoad(r);
  else
    return false;
}

bool GameMap::putRoad(Road *r, bool smooth)
{
  if (containsWater(r->getArea()))
    putTerrain(r->getArea(), Tile::GRASS);
  Roadlist::getInstance()->add(r);
  setBuilding(r->getPos(), Maptile::ROAD);

  if (smooth == false)
    return true;
  // now reconfigure all roads in the surroundings
  Vector<int> tile = r->getPos();
  for (int x = tile.x - 1; x <= tile.x + 1; ++x)
    for (int y = tile.y - 1; y <= tile.y + 1; ++y)
      {
        if (offmap(x,y))
          continue;

	Vector<int> pos(x, y);
	if (Roadlist::getInstance()->getObjectAt(pos))
	  {
            r = Roadlist::getInstance()->getObjectAt(pos);
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
      updateShips(pos);
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
  Tileset *tileset = GameMap::getTileset();
  int index = tileset->getIndex(type);
  if (index == -1)
    return r;
  for (int x = r.x; x < r.x + r.w; ++x)
    for (int y = r.y; y < r.y + r.h; ++y)
      {
	if (offmap(x,y))
	  continue;
        
	Maptile* t = getTile(Vector<int>(x, y));
        if (t->hasLandBuilding() && type == Tile::WATER)
          continue;
        if (t->hasWaterBuilding() && type != Tile::WATER)
          continue;
	if (t->getType() != type)
          {
            //it's always grass under cities.
            if (t->getBuilding() == Maptile::CITY)
              t->setIndex(tileset->getIndex(Tile::GRASS));
            else
              t->setIndex(index);
            updateShips(Vector<int>(x,y));
            replaced = true;
          }
      }
  if (replaced)
    {
      for (int x = r.x - 2; x < r.x + r.w + 2; ++x)
        for (int y = r.y - 2; y < r.y + r.h + 2; ++y)
          {
            if (offmap(x,y))
              continue;
            calculateBlockedAvenue(x, y);
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
	    TileStyle *style = tileset->getTileStyle(tile_style_id);
	    getTile(x, y)->setTileStyle(style);
          }
    }

  return r;
}

void GameMap::clearBuilding(Vector<int> pos, guint32 width)
{
  for (unsigned int x = pos.x; x < pos.x + width; ++x)
    for (unsigned int y = pos.y; y < pos.y + width; ++y)
      {
        if (offmap(x,y))
          continue;
	Maptile* t = getTile(Vector<int>(x, y));
	t->setBuilding(Maptile::NONE);
      }
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

bool GameMap::putNewCity(Vector<int> tile)
{
  Cityset *cs = GameMap::getCityset();
  // check if we can place the city
  bool city_placeable =
    canPutBuilding (Maptile::CITY, cs->getCityTileWidth(), tile);

  if (!city_placeable)
    return false;

  City *c = new City(tile, cs->getCityTileWidth());
  return putCity(c);
}

bool GameMap::putNewRuin(Vector<int> tile)
{
  Cityset *cs = GameMap::getCityset();
  // check if we can place the city
  bool ruin_placeable =
    canPutBuilding (Maptile::RUIN, cs->getRuinTileWidth(), tile);

  if (!ruin_placeable)
    return false;

  Ruin *r = new Ruin(tile, cs->getRuinTileWidth());
  return putRuin(r);
}

bool GameMap::putNewTemple(Vector<int> tile)
{
  Cityset *cs = GameMap::getCityset();
  // check if we can place the city
  bool temple_placeable =
    canPutBuilding (Maptile::TEMPLE, cs->getTempleTileWidth(), tile);

  if (!temple_placeable)
    return false;

  Temple *t = new Temple(tile, cs->getTempleTileWidth());
  return putTemple(t);
}

bool GameMap::putCity(City *c, bool keep_owner)
{
  Player *active = Playerlist::getActiveplayer();

  // create the city
  if (keep_owner == false)
    c->setOwner(active);
  else
    active = c->getOwner();
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
  s->getOwner()->deleteStack(s);
}
	
guint32 GameMap::countArmyUnits(Vector<int> pos)
{
  if (getStacks(pos))
    return getStacks(pos)->countNumberOfArmies(Playerlist::getActiveplayer());
  return 0;
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
  std::list<Vector<int> > points = getNearbyPoints(pos, dist);
  std::list<Stack*> stks;
  std::list<Stack *> stacks;
  for (std::list<Vector<int> >::iterator it = points.begin(); 
       it != points.end(); it++)
    {
      if (friendly)
        stks = GameMap::getFriendlyStacks(*it);
      else
        stks = GameMap::getEnemyStacks(*it);
      stacks.merge(stks);
    }

  return stacks;
}

std::list<Vector<int> > GameMap::getNearbyPoints(Vector<int> pos, int dist)
{
  std::list<Vector<int> > points;
  guint32 i, j;
  guint32 d;
  guint32 max = dist;
  int x, y;

  points.push_back(pos);

  if (dist == -1)
    {
      if (getWidth() > getHeight())
        max = getWidth();
      else
        max = getHeight();
    }
  //d is the distance from Pos where our box starts
  //instead of a regular loop around a box of dist large, we're going to add
  //the nearer stacks first.
  for (d = 1; d <= max; d++)
    {
      for (i = 0; i < (d * 2) + 1; i++)
        {
          for (j = 0; j < (d * 2) + 1; j++)
            {
              if ((i == 0 || i == (d * 2)) ||
                  (j == 0 || j == (d * 2)))
                {
                  x = pos.x + (i - d);
                  y = pos.y + (j - d);
		  if (offmap(x, y))
		    continue;
                  points.push_back(Vector<int>(x,y));
                }
            }
        }
    }

  return points;
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

int GameMap::calculateTilesPerOverviewMapTile(int width, int height)
{
  if (width <= (int)MAP_SIZE_NORMAL_WIDTH && 
      height <= (int)MAP_SIZE_NORMAL_HEIGHT)
    return 1;
  else if (width <= (int)(MAP_SIZE_NORMAL_WIDTH  * 2) && 
      height <= (int)(MAP_SIZE_NORMAL_HEIGHT * 2))
    return 2;
  else if (width <= (int)(MAP_SIZE_NORMAL_WIDTH  * 3) && 
      height <= (int)(MAP_SIZE_NORMAL_HEIGHT * 3))
    return 3;
  else if (width <= (int)(MAP_SIZE_NORMAL_WIDTH  * 4) && 
      height <= (int)(MAP_SIZE_NORMAL_HEIGHT * 4))
    return 4;
  return 1;
}

int GameMap::calculateTilesPerOverviewMapTile()
{
  return calculateTilesPerOverviewMapTile(GameMap::getWidth(), GameMap::getHeight());
}

void GameMap::changeFootprintToSmallerCityset(Location *location, Maptile::Building building_type, guint32 old_tile_width)
{
  GameMap::getInstance()->clearBuilding(location->getPos(), (guint32)old_tile_width);

  GameMap::getInstance()->putBuilding (location, building_type);
}

void GameMap::relocateLocation(Location *location, Maptile::Building building_type, guint32 tile_width)
{
  //look for a suitable place for this building
  //remove our buildingness so it can find where we are now.
  GameMap::getInstance()->removeBuilding(location);
  Vector<int> dest = 
    GameMap::getInstance()->findNearestAreaForBuilding(building_type, location->getPos(), tile_width);
  GameMap::getInstance()->putBuilding (location, building_type);
  if (dest == Vector<int>(-1, -1))
    GameMap::getInstance()->removeLocation (location->getPos());
  else
    GameMap::getInstance()->moveBuilding (location->getPos(), dest, tile_width);
}
        
guint32 GameMap::getTileSize() const
{
  Tileset *ts = GameMap::getTileset();
  return ts->getTileSize();
}

guint32 GameMap::getTilesetId() const
{
  if (GameMap::getTileset())
    return GameMap::getTileset()->getId();
  else
    return 0;
}

guint32 GameMap::getCitysetId() const
{
  if (GameMap::getCityset())
    return GameMap::getCityset()->getId();
  else
    return 0;
}

guint32 GameMap::getShieldsetId() const
{
  return Shieldsetlist::getInstance()->getSetId(d_shieldset);
}

Glib::ustring GameMap::getTilesetBaseName() const
{
  return d_tileset;
}

Glib::ustring GameMap::getCitysetBaseName() const
{
  return d_cityset;
}

Glib::ustring GameMap::getShieldsetBaseName() const
{
  return d_shieldset;
}

bool GameMap::eraseTiles(Rectangle r)
{
  bool erased = false;
  for (int x = r.x; x < r.x + r.w; ++x)
    for (int y = r.y; y < r.y + r.h; ++y)
      {
	if (offmap(x,y))
	  continue;
        erased |= eraseTile(Vector<int>(x,y));
      }
  return erased;
}

bool GameMap::eraseTile(Vector<int> tile) 
{
  bool erased = false;
  // first stack, it's above everything else
  while  (getStack(tile) != NULL)
    {
      Stack *s = getStack(tile);
      removeStack(s);
      erased = true;
    }

  // ... or a temple ...
  erased |= removeTemple(tile);

  // ... or a port ...
  erased |= removePort(tile);

  // ... or a ruin ...
  if (getRuin(tile) != NULL)
    {
      Rewardlist *rl = Rewardlist::getInstance();
      for (Rewardlist::iterator i = rl->begin(); i != rl->end(); i++)
        {
          if ((*i)->getType() == Reward::RUIN)
            {
              Reward_Ruin *rr = static_cast<Reward_Ruin*>(*i);
              if (rr->getRuin()->getPos() == tile)
                rl->remove(*i);
            }
        }
    }
  erased |= removeRuin(tile);

  // ... or a road ...
  erased |= removeRoad(tile);

  // ... or a bridge...
  erased |= removeBridge(tile);

  // ... or a signpost ...
  erased |= removeSignpost(tile);

  // ... or a city
  erased |= removeCity(tile);

  // ... or a bag
  if (getTile(tile)->getBackpack()->size() > 0)
    {
      getTile(tile)->getBackpack()->removeAllFromBackpack();
      erased = true;
    }
  return erased;
}

Tileset* GameMap::getTileset()
{
  if (s_tileset == 0)
    s_tileset = Tilesetlist::getInstance()->get(GameMap::getInstance()->getTilesetBaseName());
    
  return s_tileset;
}

Cityset* GameMap::getCityset()
{
  if (s_cityset == 0)
    s_cityset = Citysetlist::getInstance()->get(GameMap::getInstance()->getCitysetBaseName());
    
  return s_cityset;
}

Shieldset* GameMap::getShieldset()
{
  if (s_shieldset == 0)
    s_shieldset = Shieldsetlist::getInstance()->get(GameMap::getInstance()->getShieldsetBaseName());
    
  return s_shieldset;
}

void GameMap::setTileset(Glib::ustring tileset)
{
  d_tileset = tileset;
  s_tileset = Tilesetlist::getInstance()->get(tileset);
}

void GameMap::setCityset(Glib::ustring cityset)
{
  d_cityset = cityset;
  s_cityset = Citysetlist::getInstance()->get(cityset);
}

void GameMap::setShieldset(Glib::ustring shieldset)
{
  d_shieldset = shieldset;
  s_shieldset = Shieldsetlist::getInstance()->get(shieldset);
}

bool GameMap::can_search(Stack *stack)
{
  /*
   * a note about searching.
   * ruins can be searched by stacks that have a hero, and when the
   * hero has moves left.  also the ruin must be unexplored.
   * temples can be searched by any stack, when the stack has 
   * movement left.
   */
  if (!stack)
    return false;
  if (stack->getMoves() < 1)
    return false;
  bool temple_searchable = false;
  Temple *temple = GameMap::getTemple(stack->getPos());
  if (temple)
    temple_searchable = true;
  bool ruin_searchable = true;
  Ruin *ruin = GameMap::getRuin(stack->getPos());
  if (!ruin)
    ruin_searchable = false;
  else
    {
      if (ruin->isSearched() == true)
        ruin_searchable = false;
      if (ruin->isHidden() == true &&
          ruin->getOwner() != Playerlist::getActiveplayer())
        ruin_searchable = false;
      if (stack->hasHero() == false)
        ruin_searchable = false;
    }
  if (ruin_searchable || temple_searchable)
    return true;
  return false;
}

bool GameMap::can_plant_flag(Stack *stack)
{
  Player *player = Playerlist::getActiveplayer();
  if (stack->hasHero())
    {
      //does the hero have the player's standard?
      for (Stack::iterator it = stack->begin(); it != stack->end(); it++)
        {
          if ((*it)->isHero())
            {
              Hero *hero = dynamic_cast<Hero*>((*it));
              if (hero->getBackpack()->getPlantableItem(player))
                {
                  //can't plant on city/ruin/temple/signpost
                  City *city = getCity(stack->getPos());
                  Temple *temple = getTemple(stack);
                  Ruin *ruin = getRuin(stack);
                  Signpost *sign = getSignpost(stack);
                  if (!city && !temple && !ruin && !sign)
                    {
                      MapBackpack *backpack;
                      Vector<int> pos = stack->getPos();
                      backpack = getInstance()->getTile(pos)->getBackpack();
                      bool standard_already_planted = 
                        backpack->getFirstPlantedItem() != NULL;
                      //are there any other standards here?
                      if (standard_already_planted == false)
                        return true;
                    }
                }
            }
        }
    }
  return false;
}

bool GameMap::burnBridge(Vector<int> pos)
{
  bool burned = false;
  Bridge *bridge = GameMap::getBridge(pos);
  if (bridge)
    {
      Bridge *other = Bridgelist::getInstance()->getOtherSide(bridge);
      Vector<int> src = bridge->getPos();
      GameMap::getInstance()->removeBridge(src);
      Vector<int> dest = Vector<int>(-1, -1);
      std::list<Stack*> stacks;
      if (other)
        {
          dest = other->getPos();
          GameMap::getInstance()->removeBridge(dest);
          std::list<Stack*> s = GameMap::getStacks(src)->getStacks();
          stacks.merge(s);
        }
      std::list<Stack*> s = 
        GameMap::getFriendlyStacks(src, Playerlist::getActiveplayer());
      stacks.merge(s);
      for (std::list<Stack*>::iterator i = stacks.begin(); 
           i != stacks.end(); i++)
        {
          (*i)->setDefending(false);
          (*i)->setParked(false);
          (*i)->clearPath();
          (*i)->drainMovement();
        }
      std::list<Vector<int> > r = 
        Bridgelist::getInstance()->getRoadEntryPoints(bridge);
      for (std::list<Vector<int> >::iterator i = r.begin(); i != r.end(); i++)
        {
          Road *rd = GameMap::getInstance()->getRoad(*i);
          if (rd)
            rd->setType(Roadlist::getInstance()->calculateType(rd->getPos()));
        }
      burned = true;
    }
  return burned;
}

bool GameMap::friendlyCitiesPresent()
{
  return Citylist::getInstance()->countCities(Playerlist::getActiveplayer());
}

bool GameMap::enemyCitiesPresent()
{
  Playerlist *plist = Playerlist::getInstance();
  for (Playerlist::iterator i = plist->begin(); i != plist->end(); i++)
    {
      if ((*i) == plist->getNeutral())
        continue;
      if ((*i) == plist->getActiveplayer())
        continue;
      if ((*i)->isDead())
        continue;
      if (Citylist::getInstance()->countCities(*i) > 0)
        return true;
    }
  return false;
}

bool GameMap::neutralCitiesPresent()
{
  return Citylist::getInstance()->countCities
    (Playerlist::getInstance()->getNeutral());
}

void GameMap::addArmies(const ArmyProto *a, guint32 num_allies, Vector<int> pos)
{
  for (unsigned int i = 0; i < num_allies; i++)
    {
      Army *army = new Army(*a, Playerlist::getActiveplayer());
      if (army)
        addArmy(pos, army);
    }
}
      
//we can't defend on cities, ruins, temples, ports, or water.
bool GameMap::can_defend(Stack *stack)
{
  Tile::Type type = getInstance()->getTile(stack->getPos())->getType();
  Maptile::Building building = getInstance()->getBuilding(stack->getPos());
  if (type == Tile::WATER && building == Maptile::NONE)
    return false;
  if (building == Maptile::CITY || building == Maptile::RUIN ||
      building == Maptile::TEMPLE || building == Maptile::PORT)
    return false;
  return true;
}
        
bool GameMap::checkBuildingTerrain(Maptile::Building b, bool land)
{
  bool found = false;
  GameMap *gm = GameMap::getInstance();
  for (int i = 0; i < s_width; i++)
    {
      for (int j = 0; j < s_height; j++)
        {
          if (gm->getBuilding(Vector<int>(i, j)) == b)
            {
              if (land)
                {
                  if (gm->getTerrainType(Vector<int>(i, j)) != Tile::WATER)
                    found = true;
                }
              else
                {
                  if (gm->getTerrainType(Vector<int>(i, j)) == Tile::WATER)
                    found = true;
                }
              if (found)
                break;
            }
        }
    }

  return found;
}
        
City* GameMap::getCity(Movable *m)
{
  return getCity(m->getPos());
}
        
Ruin* GameMap::getRuin(Movable *m)
{
  return getRuin(m->getPos());
}
        
Temple* GameMap::getTemple(Movable *m)
{
  return getTemple(m->getPos());
}
        
Signpost* GameMap::getSignpost(Movable *m)
{
  return getSignpost(m->getPos());
}
