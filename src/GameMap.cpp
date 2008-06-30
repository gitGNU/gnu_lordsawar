// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2003, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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

#include <sstream>
#include <assert.h>
#include <sigc++/functors/mem_fun.h>

#include "GameMap.h"
#include "citylist.h"
#include "bridgelist.h"
#include "portlist.h"
#include "roadlist.h"
#include "city.h"
#include "ruin.h"
#include "temple.h"
#include "playerlist.h"
#include "stacklist.h"
#include "ruinlist.h"
#include "templelist.h"
#include "xmlhelper.h"
#include "MapGenerator.h"
#include "tilesetlist.h"
#include "shieldsetlist.h"
#include "citysetlist.h"

using namespace std;

//#include <iostream>
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

    d_map = new Maptile*[s_width*s_height];
    for (int j = 0; j < s_height; j++)
        for (int i = 0; i < s_width; i++)
            d_map[j*s_width + i] = 0;

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

    offset = 0;
    for (int j = 0; j < s_height; j++)
    {
        // remove newline and carriage return lines
        char test = styles[j*s_width*2 + offset];
        while (test == '\n' || test == '\r')
        {
            offset++;
            test = styles[j*s_width*2 + offset];
        }

        for (int i = 0; i < s_width; i++)
        {
	    char hexstr[5];
            //due to the circumstances, styles is a long stream of
            //hex digit pairs, so read it character for character
	    hexstr[0] = '0';
	    hexstr[1] = 'x';
	    hexstr[2] = styles[j*s_width*2 + (i * 2) + offset];  
	    hexstr[3] = styles[j*s_width*2 + (i * 2) + offset + 1];
	    hexstr[4] = '\0';

	    unsigned long int val = 0;
	    char *end = NULL;
	    val = strtoul (hexstr, &end, 16);
	    Uint32 id = (Uint32) val;
	    TileStyle *style = d_tileSet->getTileStyle(id);
	    d_map[j*s_width + i]->setTileStyle(style);
        }
    }

    //add some callbacks for item loading
    helper->registerTag("itemstack", sigc::mem_fun(this, &GameMap::loadItems));
    helper->registerTag("item", sigc::mem_fun(this, &GameMap::loadItems));
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
        std::cerr <<_("Error in GameMap::fillMap: sizes don't match!! Exiting.\n");
        exit(-1);
    }

    // create tiles; there is a hack here: The map generator outputs tile types,
    // but we supply the index of the tile types in the tileset to Maptile. Was
    // the easiest version when rewriting this.
    for (int j = 0; j < height; j++)
        for (int i = 0; i < width; i++)
        {
            Uint32 index = d_tileSet->getIndex(terrain[j*width + i]);
            d_map[j*s_width + i] = new Maptile(d_tileSet, i, j, index, NULL);
        }

    applyTileStyles(0, 0, height, width, true);
    return true;
}

bool GameMap::fill(Uint32 type)
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
    for (int i = 0; i < s_height; i++)
    {
        for (int j = 0; j < s_width; j++)
	  {
	    char *hexstr = NULL;
	    TileStyle *style = getTile(j, i)->getTileStyle();
	    asprintf (&hexstr, "%02x", style->getId());
            styles << hexstr;
	    free (hexstr);
	  }
        styles <<endl;
    }


    retval &= helper->openTag("map");
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
            if (!getTile(i,j)->getItems().empty())
            {
                retval &= helper->openTag("itemstack");
                retval &= helper->saveData("x", i);
                retval &= helper->saveData("y", j);
                
                std::list<Item*> items = getTile(i,j)->getItems();
                
                std::list<Item*>::const_iterator it;
                for (it = items.begin(); it != items.end(); it++)
                    (*it)->save(helper);

                retval &= helper->closeTag();
            }
     
    retval &= helper->closeTag();
    return retval;
}

bool GameMap::loadItems(std::string tag, XML_Helper* helper)
{
    static int x = 0;
    static int y = 0;
    
    if (tag == "itemstack")
    {
        helper->getData(x, "x");
        helper->getData(y, "y");
    }

    if (tag == "item")
    {
        Item* item = new Item(helper);
        getTile(x, y)->addItem(item);
    }

    return true;
}

void GameMap::setTile(int x, int y, Maptile *tile)
{
    delete d_map[y*s_width + x];
    d_map[y*s_width + x] = tile;
}

Maptile* GameMap::getTile(int x, int y) const
{
    assert(x >= 0 && x < s_width && y >= 0 && y < s_height);

    return d_map[y*s_width + x];
}

Stack* GameMap::addArmy(Vector<int> pos, Army *a)
{
  City *c = Citylist::getInstance()->getObjectAt(pos);
  if (c)
    {
      if (c->isBurnt())
        return addArmyAtPos(pos, a);
      else
        return addArmy(c, a);
    }
  Temple *t = Templelist::getInstance()->getObjectAt(pos);
  if (t)
    return addArmy(t, a);
  Ruin *r = Ruinlist::getInstance()->getObjectAt(pos);
  if (r)
    return addArmy(r, a);
  return addArmyAtPos(pos, a);
}

Stack* GameMap::addArmyAtPos(Vector<int> pos, Army *a)
{
  Stack *s = NULL;
  bool added_army = false;
  Uint32 i, j;
  Uint32 d;
  Uint32 max;
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
                  if (x >= s_width || y >= s_height)
                    continue;
                  //is there somebody else's city here?
                  City *c = Citylist::getInstance()->getObjectAt(x, y);
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
		      (a->getStat(Army::MOVE_BONUS) & Tile::MOUNTAIN) == 
		      Tile::MOUNTAIN)
                    continue;
                  //is there somebody else's stack here?
                  s = Stacklist::getObjectAt(x, y);
                  if (s)
                    { 
                      if (s->getOwner() != a->getOwner())
                        continue;
                      //is it our stack, but too full?
                      if (s->size() >= MAX_STACK_SIZE)
                        continue;
                    }
                  //hey this looks like a good place for a stack
                  else  //but there isn't a stack here
                    {
                      Vector<int> pos(x, y);
                      s = new Stack(a->getOwner(), pos);
                      a->getOwner()->addStack(s);
                    }
                  s->push_front(a);
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
    return s;
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

bool GameMap::isDock(int x, int y)
{
  if (Citylist::getInstance()->getObjectAt(x,y))
    return true;
  if (Portlist::getInstance()->getObjectAt(x,y))
    return true;
  if (Bridgelist::getInstance()->getObjectAt(x,y))
    return true;
  return false;
}

bool GameMap::isBlockedAvenue(int x, int y, int destx, int desty)
{
  if (destx < 0 || destx >= s_width)
    return true;
  if (desty < 0 || desty >= s_height)
    return true;
  if (Citylist::getInstance()->empty())
      return false;
  int diffx = destx - x;
  int diffy = desty - y;
  if (diffx >= -1 && diffx <= 1 && diffy >= -1 && diffy <= 1)
    {
      assert (Citylist::getInstance()->size());
      bool from_dock = isDock(x,y);
      bool to_dock = isDock(destx,desty);
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
	  Roadlist::getInstance()->getObjectAt(destx, desty) == NULL)
        return true;

      //am i on a mountain without a road?
      if (from->getMaptileType() == Tile::MOUNTAIN &&
	  Roadlist::getInstance()->getObjectAt(x, y) == NULL)
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
      if (destx < 0 || destx >= s_width)
	{
	  maptile->d_blocked[k] = true;
	  continue;
	}
      if (desty < 0 || desty >= s_height)
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
            std::list<Item*> items = getTile(x, y)->getItems();
            for (std::list<Item*>::iterator it = items.begin(); 
                 it != items.end(); it++)
              {
                if ((*it)->isPlantable() && (*it)->getPlantableOwner() == p &&
		    (*it)->getPlanted() == true)
                  {
                    pos.x = x;
                    pos.y = y;
                    found = true;
                    break;
                  }
              }
            if (found)
              break;
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
	if ((i+k) >= s_height || (i+k) < 0)
	  continue;
	if ((j+l) >= s_width || (j+l) < 0)
	  continue;
	box[k+1][l+1] = (getTile(j+l, i+k)->getType() == mtile->getType());
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
	  if (i < 0 || i >= s_height)
	    continue;
	  if (j < 0 || j >= s_width)
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

int GameMap::tile_is_connected_to_other_like_tiles (Tile::Type tile, int i, int j)
{
  int box[3][3];
  memset (box, 0, sizeof (box));
  for (int k = -1; k <= +1; k++)
    for (int l = -1; l <= +1; l++)
      {
	if ((i+k) >= s_height || (i+k) < 0)
	  continue;
	if ((j+l) >= s_width || (j+l) < 0)
	  continue;
	box[k+1][l+1] = (getTile(j+l, i+k)->getMaptileType() == tile);
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
	if (i < 0 || i >= s_height)
	  continue;
	if (j < 0 || j >= s_width)
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
		setTile(j, i, new Maptile
			(d_tileSet, j, i, d_tileSet->getIndex(outtype), NULL));
	      }
	  }
      }

}

void GameMap::applyTileStyles (int minx, int miny, int maxx, int maxy, 
			       bool smooth_terrain)
{

  if (smooth_terrain)
    {
      demote_lone_tile(0, 0, s_height, s_width, Tile::FOREST, Tile::GRASS);
      demote_lone_tile(0, 0, s_height, s_width, Tile::MOUNTAIN, Tile::HILLS);
      demote_lone_tile(0, 0, s_height, s_width, Tile::HILLS, Tile::GRASS);
      demote_lone_tile(0, 0, s_height, s_width, Tile::WATER, Tile::SWAMP);
    }

  for (int i = minx; i < maxx; i++)
    {
      for (int j = miny; j < maxy; j++)
	{
	  if (i < 0 || i >= s_height)
	    continue;
	  if (j < 0 || j >= s_width)
	    continue;
	  Maptile *mtile = getTile(j, i);
	  Tileset *tileset = getTileset();
	  TileStyle *style = calculatePreferredStyle(i, j);
	  if (!style)
	    style = tileset->getRandomTileStyle(mtile->getType(), 
						TileStyle::LONE);
	  if (!style)
	    style = tileset->getRandomTileStyle(mtile->getType(), 
						TileStyle::INNERMIDDLECENTER);
	  mtile->setTileStyle(style);
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
	  if (d_map[j*s_width + i]->getItems().empty() == false)
	    items.push_back(Vector<int>(i, j));

      }
  return items;
}
