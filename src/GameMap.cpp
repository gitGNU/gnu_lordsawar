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

#include <sstream>
#include <assert.h>
#include <sigc++/functors/mem_fun.h>

#include "GameMap.h"
#include "citylist.h"
#include "city.h"
#include "ruin.h"
#include "temple.h"
#include "playerlist.h"
#include "stacklist.h"
#include "ruinlist.h"
#include "templelist.h"
#include "xmlhelper.h"
#include "MapGenerator.h"

using namespace std;

//#include <iostream>
//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<flush;}
#define debug(x)

GameMap* GameMap::s_instance = 0;

int GameMap::s_width = 100;
int GameMap::s_height = 100;

GameMap* GameMap::getInstance()
{
    if (s_instance != 0)
      return s_instance;
    else return 0;
}


GameMap* GameMap::getInstance(std::string TilesetName)
{
    if (s_instance == 0)
    {
        s_instance = new GameMap(TilesetName);

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

GameMap::GameMap(std::string TilesetName)
{
    d_tileSet = new TileSet(TilesetName);

    d_map = new Maptile*[s_width*s_height];
    for (int j = 0; j < s_height; j++)
        for (int i = 0; i < s_width; i++)
            d_map[j*s_width + i] = 0;

}

GameMap::GameMap(XML_Helper* helper)
{
    std::string types;
    std::string t_name;

    helper->getData(s_width, "width");
    helper->getData(s_height, "height");
    helper->getData(t_name,"tileset");
    helper->getData(types, "types");

    d_tileSet = new TileSet(t_name);

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
            d_map[j*s_width + i] = new Maptile(d_tileSet, i, j, type);
        }
    }

    //add some callbacks for item loading
    helper->registerTag("itemstack", sigc::mem_fun(this, &GameMap::loadItems));
    helper->registerTag("item", sigc::mem_fun(this, &GameMap::loadItems));
}


GameMap::~GameMap()
{
    delete d_tileSet;

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
            d_map[j*s_width + i] = new Maptile(d_tileSet, i, j, index);
        }

    return true;
}

bool GameMap::fill(Uint32 type)
{
    for (int i = 0; i < s_width; i++)
        for (int j = 0; j < s_height; j++)
            d_map[j*s_width + i] = new Maptile(d_tileSet, i, j, type);

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


    retval &= helper->openTag("map");
    retval &= helper->saveData("width", s_width);
    retval &= helper->saveData("height", s_height);
    retval &= helper->saveData("tileset", d_tileSet->getName());
    retval &= helper->saveData("types", types.str());

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
    return addArmy(t, a);
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

  Location l("", pos, 1);
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
                  if (x > s_width || y > s_height)
                    continue;
                  //is there somebody else's city here?
                  City *c = Citylist::getInstance()->getObjectAt(x, y);
                  if (c && c->getPlayer() != a->getPlayer())
                    {
                      if (c->isBurnt() == false)
                        continue;
                    }
                  //is this an unsuitable tile?
                  if (land && getTile(x, y)->getType() == Tile::WATER)
                    continue;
                  if (!land && getTile(x, y)->getType() != Tile::WATER)
                    continue;
                  //is there somebody else's stack here?
                  s = Stacklist::getObjectAt(x, y);
                  if (s)
                    { 
                      if (s->getPlayer() != a->getPlayer())
                        continue;
                      //is it our stack, but too full?
                      if (s->size() >= 8)
                        continue;
                    }
                  //hey this looks like a good place for a stack
                  else  //but there isn't a stack here
                    {
                      Vector<int> pos(x, y);
                      s = new Stack(a->getPlayer(), pos);
                      a->getPlayer()->addStack(s);
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
  return addArmy(l->getPos(), a);
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
      bool from_city = Citylist::getInstance()->getObjectAt(x,y) != NULL;
      bool to_city = Citylist::getInstance()->getObjectAt(destx,desty) != NULL;
      Maptile *from = getTile(x, y);
      Maptile *to = getTile(destx, desty);
      if (from == to)
        return false;
      //am i on water going towards land that isn't a city?
      if (from->getMaptileType() == Tile::WATER &&
          to->getMaptileType() != Tile::WATER &&
          !to_city)
        return true;
      //am i on land, going towards water from a tile that isn't a city?
      if (from->getMaptileType() != Tile::WATER &&
          to->getMaptileType() == Tile::WATER &&
          !from_city)
        return true;
      //is the tile i'm going to a mountain?
      if (to->getMaptileType() == Tile::MOUNTAIN)
        return true;
    }
 return false;
}

void GameMap::calculateBlockedAvenues()
{
  int diffx = 0, diffy = 0;
  int destx = 0, desty = 0;
  for (int i = 0; i < s_width; i++)
    for (int j = 0; j < s_height; j++)
      {
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
}
