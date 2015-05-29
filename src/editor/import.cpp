// Copyright (C) 2015 Ben Asselstine
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

#include <config.h>

#include <iostream>
#include <algorithm>
#include "Configuration.h"
#include "File.h"
#include "vector.h"
#include "ucompose.hpp"
#include "GameScenario.h"
#include "GameMap.h"
#include "Itemlist.h"
#include "playerlist.h"
#include "counter.h"
#include "armysetlist.h"
#include "armyset.h"
#include "shieldsetlist.h"
#include "shieldset.h"
#include "ai_dummy.h"
#include "CreateScenarioRandomize.h"
#include "tileset.h"
#include "bridge.h"
#include "bridgelist.h"
#include "road.h"
#include "roadlist.h"
#include "player.h"
#include "ruinlist.h"
#include "templelist.h"
#include "temple.h"
#include "ruin.h"
#include "citylist.h"
#include "city.h"
#include "signpostlist.h"
#include "signpost.h"
#include "portlist.h"
#include "port.h"
#include "ItemProto.h"
#include "Itemlist.h"

int max_vector_width;
      
static GameScenario *
setup_new_map (Glib::ustring name)
{
  GameMap::setWidth(112);
  GameMap::setHeight(156);
  GameMap::getInstance("default", "default", "default");
  Itemlist::createStandardInstance();

  GameScenario *g = new GameScenario(name, String::ucompose(_("a scenario called %1 converted by lordsawar-import version %2"), name, VERSION), true);
  for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
    fl_counter->getNextId();

  // fill the map with tile type
  Tileset *tset = GameMap::getTileset();
  for (unsigned int i = 0; i < tset->size(); ++i)
    {
      if ((*tset)[i]->getType() == 0)
        {
          GameMap::getInstance()->fill(i);
          break;
        }
    }
  GameMap::getInstance()->calculateBlockedAvenues();
  return g;
}

static bool
convert_terrain_code (unsigned char code, guint32 *t, guint32 *ts)
{
  switch (code)
    {
      //water
      /*
    case 0x10: *t = Tile::WATER;
               *ts = TileStyle::OUTERTOPLEFT;
               break;
    case 0x11: *t = Tile::WATER;
               *ts = TileStyle::OUTERTOPCENTER;
               break;
    case 0x12: *t = Tile::WATER;
               *ts = TileStyle::OUTERTOPRIGHT;
               break;
    case 0x13: *t = Tile::WATER;
               *ts = TileStyle::OUTERBOTTOMLEFT;
               break;
    case 0x14: *t = Tile::WATER;
               *ts = TileStyle::OUTERBOTTOMCENTER;
               break;
    case 0x15: *t = Tile::WATER;
               *ts = TileStyle::OUTERBOTTOMRIGHT;
               break;
    case 0x16: *t = Tile::WATER;
               *ts = TileStyle::OUTERMIDDLERIGHT;
               break;
    case 0x17: *t = Tile::WATER;
               *ts = TileStyle::OTHER;
               break;
    case 0x18: *t = Tile::WATER;
               *ts = TileStyle::OUTERMIDDLELEFT;
               break;
    case 0x19: *t = Tile::WATER;
               *ts = TileStyle::INNERTOPLEFT;
               break;
    case 0x1a: *t = Tile::WATER;
               *ts = TileStyle::INNERTOPRIGHT;
               break;
    case 0x1b: *t = Tile::WATER;
               *ts = TileStyle::INNERBOTTOMLEFT;
               break;
    case 0x1c: *t = Tile::WATER;
               *ts = TileStyle::INNERBOTTOMRIGHT;
               break;
    case 0x20: *t = Tile::WATER;
               *ts = TileStyle::OUTERTOPLEFT;
               break;
    case 0x21: *t = Tile::WATER;
               *ts = TileStyle::OUTERTOPCENTER;
               break;
    case 0x22: *t = Tile::WATER;
               *ts = TileStyle::OUTERTOPRIGHT;
               break;
    case 0x23: *t = Tile::WATER;
               *ts = TileStyle::OUTERBOTTOMLEFT;
               break;
    case 0x24: *t = Tile::WATER;
               *ts = TileStyle::OUTERBOTTOMCENTER;
               break;
    case 0x25: *t = Tile::WATER;
               *ts = TileStyle::OUTERBOTTOMRIGHT;
               break;
    case 0x26: *t = Tile::WATER;
               *ts = TileStyle::OUTERMIDDLERIGHT;
               break;
    case 0x27: *t = Tile::WATER;
               *ts = TileStyle::INNERMIDDLECENTER;
               break;
    case 0x28: *t = Tile::WATER;
               *ts = TileStyle::OUTERMIDDLELEFT;
               break;
    case 0x29: *t = Tile::WATER;
               *ts = TileStyle::INNERTOPLEFT;
               break;
    case 0x2a: *t = Tile::WATER;
               *ts = TileStyle::INNERTOPRIGHT;
               break;
    case 0x2b: *t = Tile::WATER;
               *ts = TileStyle::INNERBOTTOMLEFT;
               break;
    case 0x2c: *t = Tile::WATER;
               *ts = TileStyle::INNERBOTTOMRIGHT;
               break;
    case 0x2d: *t = Tile::WATER;
               *ts = TileStyle::INNERMIDDLECENTER;
               break;
    case 0x2f: *t = Tile::WATER;
               *ts = TileStyle::BOTTOMLEFTTOTOPRIGHTDIAGONAL;
               break;
    case 0x3f: *t = Tile::WATER;
               *ts = TileStyle::BOTTOMLEFTTOTOPRIGHTDIAGONAL;
               break;
               */
    case 0x10: *t = Tile::WATER;
               *ts = TileStyle::INNERTOPLEFT;
               break;
    case 0x11: *t = Tile::WATER;
               *ts = TileStyle::OUTERBOTTOMCENTER;
               break;
    case 0x12: *t = Tile::WATER;
               *ts = TileStyle::INNERTOPRIGHT;
               break;
    case 0x13: *t = Tile::WATER;
               *ts = TileStyle::INNERBOTTOMLEFT;
               break;
    case 0x14: *t = Tile::WATER;
               *ts = TileStyle::OUTERTOPCENTER;
               break;
    case 0x15: *t = Tile::WATER;
               *ts = TileStyle::INNERBOTTOMRIGHT;
               break;
    case 0x16: *t = Tile::WATER;
               *ts = TileStyle::OUTERMIDDLELEFT;
               break;
    case 0x17: *t = Tile::WATER;
               *ts = TileStyle::OTHER;
               break;
    case 0x18: *t = Tile::WATER;
               *ts = TileStyle::OUTERMIDDLERIGHT;
               break;
    case 0x19: *t = Tile::WATER;
               *ts = TileStyle::OUTERTOPLEFT;
               break;
    case 0x1a: *t = Tile::WATER;
               *ts = TileStyle::OUTERTOPRIGHT;
               break;
    case 0x1b: *t = Tile::WATER;
               *ts = TileStyle::OUTERBOTTOMLEFT;
               break;
    case 0x1c: *t = Tile::WATER;
               *ts = TileStyle::OUTERBOTTOMRIGHT;
               break;
    case 0x20: *t = Tile::WATER;
               *ts = TileStyle::INNERTOPLEFT;
               break;
    case 0x21: *t = Tile::WATER;
               *ts = TileStyle::OUTERBOTTOMCENTER;
               break;
    case 0x22: *t = Tile::WATER;
               *ts = TileStyle::INNERTOPRIGHT;
               break;
    case 0x23: *t = Tile::WATER;
               *ts = TileStyle::INNERBOTTOMLEFT;
               break;
    case 0x24: *t = Tile::WATER;
               *ts = TileStyle::OUTERTOPCENTER;
               break;
    case 0x25: *t = Tile::WATER;
               *ts = TileStyle::INNERBOTTOMRIGHT;
               break;
    case 0x26: *t = Tile::WATER;
               *ts = TileStyle::OUTERMIDDLELEFT;
               break;
    case 0x27: *t = Tile::WATER;
               *ts = TileStyle::INNERMIDDLECENTER;
               break;
    case 0x28: *t = Tile::WATER;
               *ts = TileStyle::OUTERMIDDLERIGHT;
               break;
    case 0x29: *t = Tile::WATER;
               *ts = TileStyle::OUTERTOPLEFT;
               break;
    case 0x2a: *t = Tile::WATER;
               *ts = TileStyle::OUTERTOPRIGHT;
               break;
    case 0x2b: *t = Tile::WATER;
               *ts = TileStyle::OUTERBOTTOMLEFT;
               break;
    case 0x2c: *t = Tile::WATER;
               *ts = TileStyle::OUTERBOTTOMRIGHT;
               break;
    case 0x2d: *t = Tile::WATER;
               *ts = TileStyle::INNERMIDDLECENTER;
               break;
    case 0x2f: *t = Tile::WATER;
               *ts = TileStyle::TOPLEFTTOBOTTOMRIGHTDIAGONAL;
               break;
    case 0x3f: *t = Tile::WATER;
               *ts = TileStyle::BOTTOMLEFTTOTOPRIGHTDIAGONAL;
               break;
      //grass
    case 0x00: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x01: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x02: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x03: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x04: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x05: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x06: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x07: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x08: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x09: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x0a: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x0b: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x0c: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x1f: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x60: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x61: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x70: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x71: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
      //forest
    case 0x30: *t = Tile::FOREST;
               *ts = TileStyle::OUTERTOPLEFT;
               break;
    case 0x31: *t = Tile::FOREST;
               *ts = TileStyle::OUTERTOPCENTER;
               break;
    case 0x32: *t = Tile::FOREST;
               *ts = TileStyle::OUTERTOPRIGHT;
               break;
    case 0x33: *t = Tile::FOREST;
               *ts = TileStyle::OUTERBOTTOMLEFT;
               break;
    case 0x34: *t = Tile::FOREST;
               *ts = TileStyle::OUTERBOTTOMCENTER;
               break;
    case 0x35: *t = Tile::FOREST;
               *ts = TileStyle::OUTERBOTTOMRIGHT;
               break;
    case 0x36: *t = Tile::FOREST;
               *ts = TileStyle::OUTERMIDDLELEFT;
               break;
    case 0x37: *t = Tile::FOREST;
               *ts = TileStyle::INNERMIDDLECENTER;
               break;
    case 0x38: *t = Tile::FOREST;
               *ts = TileStyle::OUTERMIDDLERIGHT;
               break;
    case 0x39: *t = Tile::FOREST;
               *ts = TileStyle::INNERTOPLEFT;
               break;
    case 0x3a: *t = Tile::FOREST;
               *ts = TileStyle::INNERMIDDLECENTER;
               break;
    case 0x3b: *t = Tile::FOREST;
               *ts = TileStyle::INNERBOTTOMLEFT;
               break;
    case 0x3c: *t = Tile::FOREST;
               *ts = TileStyle::BOTTOMLEFTTOTOPRIGHTDIAGONAL;
               *ts = TileStyle::INNERBOTTOMRIGHT;
               break;
    case 0x3d: *t = Tile::FOREST;
               *ts = TileStyle::LONE;
               break;
    case 0x3e: *t = Tile::FOREST;
               *ts = TileStyle::LONE;
               break;
    case 0x4e: *t = Tile::FOREST;
               *ts = TileStyle::INNERBOTTOMLEFT;
               break;
    case 0x87: *t = Tile::FOREST;
               *ts = TileStyle::OUTERBOTTOMLEFT;
               break;
    case 0x88: *t = Tile::FOREST;
               *ts = TileStyle::OUTERBOTTOMCENTER;
               break;
    case 0x89: *t = Tile::FOREST;
               *ts = TileStyle::OUTERBOTTOMRIGHT;
               break;
    case 0x8a: *t = Tile::FOREST;
               *ts = TileStyle::OUTERTOPLEFT;
               break;
    case 0x8b: *t = Tile::FOREST;
               *ts = TileStyle::OUTERTOPCENTER;
               break;
    case 0x8c: *t = Tile::FOREST;
               *ts = TileStyle::OUTERTOPRIGHT;
               break;
    case 0x8d: *t = Tile::FOREST;
               *ts = TileStyle::OUTERMIDDLELEFT;
               break;
    case 0x8e: *t = Tile::FOREST;
               *ts = TileStyle::INNERMIDDLECENTER;
               break;
    case 0x8f: *t = Tile::FOREST;
               *ts = TileStyle::OUTERMIDDLERIGHT;
               break;
    case 0x95: *t = Tile::FOREST;
               *ts = TileStyle::INNERTOPLEFT;
               break;
    case 0x96: *t = Tile::FOREST;
               *ts = TileStyle::INNERTOPRIGHT;
               break;
    case 0x97: *t = Tile::FOREST;
               *ts = TileStyle::INNERBOTTOMLEFT;
               break;
    case 0x98: *t = Tile::FOREST;
               *ts = TileStyle::INNERBOTTOMRIGHT;
               break;
    case 0x4f: *t = Tile::FOREST;
               *ts = TileStyle::BOTTOMLEFTTOTOPRIGHTDIAGONAL;
               break;
      //hills
    case 0x40: *t = Tile::HILLS;
               *ts = TileStyle::OUTERTOPLEFT;
               break;
    case 0x41: *t = Tile::HILLS;
               *ts = TileStyle::OUTERTOPCENTER;
               break;
    case 0x42: *t = Tile::HILLS;
               *ts = TileStyle::OUTERTOPRIGHT;
               break;
    case 0x43: *t = Tile::HILLS;
               *ts = TileStyle::OUTERBOTTOMLEFT;
               break;
    case 0x44: *t = Tile::HILLS;
               *ts = TileStyle::OUTERBOTTOMCENTER;
               break;
    case 0x45: *t = Tile::HILLS;
               *ts = TileStyle::OUTERBOTTOMRIGHT;
               break;
    case 0x46: *t = Tile::HILLS;
               *ts = TileStyle::OUTERMIDDLELEFT;
               break;
    case 0x47: *t = Tile::HILLS;
               *ts = TileStyle::INNERMIDDLECENTER;
               break;
    case 0x48: *t = Tile::HILLS;
               *ts = TileStyle::OUTERMIDDLERIGHT;
               break;
    case 0x49: *t = Tile::HILLS;
               *ts = TileStyle::INNERTOPLEFT;
               break;
    case 0x4a: *t = Tile::HILLS;
               *ts = TileStyle::INNERTOPRIGHT;
               break;
    case 0x4b: *t = Tile::HILLS;
               *ts = TileStyle::INNERBOTTOMLEFT;
               break;
    case 0x4c: *t = Tile::HILLS;
               *ts = TileStyle::INNERBOTTOMRIGHT;
               break;
    case 0x4d: *t = Tile::HILLS;
               *ts = TileStyle::LONE;
               break;
      //mountains
    case 0x50: *t = Tile::MOUNTAIN;
               *ts = TileStyle::OUTERTOPLEFT;
               break;
    case 0x51: *t = Tile::MOUNTAIN;
               *ts = TileStyle::OUTERTOPCENTER;
               break;
    case 0x52: *t = Tile::MOUNTAIN;
               *ts = TileStyle::OUTERTOPRIGHT;
               break;
    case 0x53: *t = Tile::MOUNTAIN;
               *ts = TileStyle::OUTERBOTTOMLEFT;
               break;
    case 0x54: *t = Tile::MOUNTAIN;
               *ts = TileStyle::OUTERBOTTOMCENTER;
               break;
    case 0x55: *t = Tile::MOUNTAIN;
               *ts = TileStyle::OUTERBOTTOMRIGHT;
               break;
    case 0x56: *t = Tile::MOUNTAIN;
               *ts = TileStyle::OUTERMIDDLELEFT;
               break;
    case 0x57: *t = Tile::MOUNTAIN;
               *ts = TileStyle::INNERMIDDLECENTER; //really a special
               break;
    case 0x58: *t = Tile::MOUNTAIN;
               *ts = TileStyle::OUTERMIDDLERIGHT;
               break;
    case 0x59: *t = Tile::MOUNTAIN;
               *ts = TileStyle::INNERMIDDLECENTER;
               break;
    case 0x5a: *t = Tile::MOUNTAIN;
               *ts = TileStyle::INNERTOPRIGHT;
               break;
    case 0x5b: *t = Tile::MOUNTAIN;
               *ts = TileStyle::INNERBOTTOMLEFT;
               break;
    case 0x5c: *t = Tile::MOUNTAIN;
               *ts = TileStyle::INNERBOTTOMRIGHT;
               break;
    case 0x5d: *t = Tile::MOUNTAIN;
               *ts = TileStyle::INNERMIDDLECENTER;
               break;
    case 0x5e: *t = Tile::MOUNTAIN;
               *ts = TileStyle::INNERMIDDLECENTER;
               break;
    case 0x5f: *t = Tile::MOUNTAIN;
               *ts = TileStyle::INNERMIDDLECENTER;
               break;
    case 0x7e: *t = Tile::MOUNTAIN;
               *ts = TileStyle::OUTERBOTTOMCENTER;
               break;
    case 0x6e: *t = Tile::MOUNTAIN;
               *ts = TileStyle::OUTERTOPCENTER;
               break;
      //swamp
    case 0x0d: *t = Tile::SWAMP;
               *ts = TileStyle::LONE;
               break;
    case 0x0e: *t = Tile::SWAMP;
               *ts = TileStyle::LONE;
               break;
    case 0x0f: *t = Tile::SWAMP;
               *ts = TileStyle::LONE;
               break;
    case 0x1d: *t = Tile::SWAMP;
               *ts = TileStyle::LONE;
               break;
    case 0x1e: *t = Tile::SWAMP;
               *ts = TileStyle::LONE;
               break;
    case 0x2e: *t = Tile::SWAMP;
               *ts = TileStyle::LONE;
               break;
      //bridges
    case 0x85: *t = Tile::WATER;
               *ts = TileStyle::OUTERMIDDLELEFT;
               break;
    case 0x86: *t = Tile::WATER;
               *ts = TileStyle::OUTERMIDDLERIGHT;
               break;
    case 0x94: *t = Tile::WATER;
               *ts = TileStyle::OUTERBOTTOMCENTER;
               break;
    case 0x84: *t = Tile::WATER;
               *ts = TileStyle::OUTERTOPCENTER;
               break;
    default:
      return false;
      break;
    }
  return true;
}

static void
import_terrain (FILE *map)
{
  short tiles[156][112];
  fread (tiles, sizeof (short), 0x4400, map);
  int i, j;
  for (i = 0; i < 156; i++)
    {
      for (j = 0; j < 112; j++)
        {
          short tile = tiles[i][j];
          unsigned char code;
          memcpy (&code, &tile, 1);
          guint32 type = 0;
          guint32 tilestyle = 0;
          bool success = convert_terrain_code (code, &type, &tilestyle);
          if (!success)
            {
              fprintf(stderr, 
                      _("Error: Terrain code %02hhx at %d,%d is unknown\n").c_str(), 
                      code, i, j);
            }
          else if (tilestyle == TileStyle::UNKNOWN)
            fprintf(stderr, 
                    _("Error: Terrain code %02hhx at %d,%d "
                      "is unknown type %s\n").c_str(), 
                    code, i, j, Tile::tileTypeToString(Tile::Type(type)).c_str());
          if (success)
            {
              Tileset *tileset = GameMap::getTileset();
              TileStyle *style = 
                tileset->getRandomTileStyle(tileset->getIndex(Tile::Type(type)), TileStyle::Type(tilestyle));

              Maptile *mtile = GameMap::getInstance()->getTile(Vector<int>(j,i));
              mtile->setIndex(tileset->getIndex(Tile::Type(type)));
              mtile->setTileStyle(style);
            }

          char port;
          memcpy (&port, (((char *)&tile)+1), 1);
          if (port == (char)0x80)
            {
              Port *p = new Port (Vector<int>(j, i));
              Portlist::getInstance()->add(p);
            }
        }
    }
}

static bool
convert_bridge_code (unsigned char code, guint32 *type)
{
  switch (code)
    {
    case 0x85: *type = Bridge::CONNECTS_TO_EAST;
               break;
    case 0x86: *type = Bridge::CONNECTS_TO_WEST;
               break;
    case 0x94: *type = Bridge::CONNECTS_TO_NORTH;
               break;
    case 0x84: *type = Bridge::CONNECTS_TO_SOUTH;
               break;
    default:
               return false;
    }
  return true;
}

static void
import_bridges (FILE *map)
{
  short tiles[156][112];
  fread (tiles, sizeof (short), 0x4400, map);
  int i, j;
  for (i = 0; i < 156; i++)
    {
      for (j = 0; j < 112; j++)
        {
          short tile = tiles[i][j];
          unsigned char code;
          memcpy (&code, &tile, 1);
          guint32 type = 0;
          if (convert_bridge_code(code, &type))
            Bridgelist::getInstance()->add(new Bridge(Vector<int>(j, i), type));
        }
    }
}

static bool
convert_road_code (unsigned char code, guint32 *type)
{
  switch (code)
    {
    case 0x01: *type = Road::CONNECTS_EAST_AND_WEST;
               break;
    case 0x02: *type = Road::CONNECTS_NORTH_AND_SOUTH;
               break;
    case 0x03: *type = Road::CONNECTS_ALL_DIRECTIONS;
               break;
    case 0x04: *type = Road::CONNECTS_EAST_WEST_AND_SOUTH;
               break;
    case 0x05: *type = Road::CONNECTS_NORTH_SOUTH_AND_WEST;
               break;
    case 0x06: *type = Road::CONNECTS_EAST_WEST_AND_NORTH;
               break;
    case 0x07: *type = Road::CONNECTS_NORTH_AND_SOUTH_AND_EAST;
               break;
    case 0x08: *type = Road::CONNECTS_WEST_AND_SOUTH;
               break;
    case 0x09: *type = Road::CONNECTS_NORTH_AND_WEST;
               break;
    case 0x0a: *type = Road::CONNECTS_NORTH_AND_EAST;
               break;
    case 0x0b: *type = Road::CONNECTS_SOUTH_AND_EAST;
               break;
    case 0x0c: *type = Road::CONNECTS_WEST;
               break;
    case 0x0d: *type = Road::CONNECTS_SOUTH;
               break;
    case 0x0e: *type = Road::CONNECTS_EAST;
               break;
    case 0x0f: *type = Road::CONNECTS_NORTH;
               break;
    case 0x10: *type = Road::CONNECTS_EAST_AND_WEST;
               break;
    case 0x11: *type = Road::CONNECTS_NORTH_AND_SOUTH;
               break;
    default:
               return false;
    }
  return true;
}

static void
import_roads(FILE *rd)
{
  unsigned char roads[156][112];
  fread (roads, sizeof (unsigned char), 0x4400, rd);
  int i, j;
  for (i = 0; i < 156; i++)
    {
      for (j = 0; j < 112; j++)
        {
          unsigned char road = roads[i][j];
          guint32 type = 0;
          if (convert_road_code(road, &type))
            Roadlist::getInstance()->add(new Road(Vector<int>(j, i), type));
        }
    }
}

static int
convert_player_id (int id)
{
  int new_id = -1;
  switch (id)
    {
    case 0: new_id = Shield::WHITE; break;
    case 1: new_id = Shield::YELLOW; break;
    case 2: new_id = Shield::ORANGE; break;
    case 3: new_id = Shield::RED; break;
    case 4: new_id = Shield::GREEN; break;
    case 5: new_id = Shield::DARK_BLUE; break;
    case 6: new_id = Shield::LIGHT_BLUE; break;
    case 7: new_id = Shield::BLACK; break;
    default:
      break;
    }
  return new_id;
}

static void
import_players (FILE *scn)
{
  CreateScenarioRandomize* d_random = new CreateScenarioRandomize();
  char names[8][21];
  memset (names, 0, sizeof (names));
  int i;
  for (i = 0; i < 8; i++)
    {
      fread (&names[i][0], sizeof(char), 20, scn);
      if (strcmp (names[i], "Not Used") == 0)
        continue;
      Player *player = 
        new RealPlayer(Glib::ustring(names[i]),
                       Armysetlist::getInstance()->get("default")->getId(),
                       Shield::get_default_color_for_no(convert_player_id(i)),
                       GameMap::getWidth(), GameMap::getHeight(), Player::HUMAN,
                       convert_player_id(i));
      int gold = 0;
      d_random->getBaseGold(100, &gold);
      gold = d_random->adjustBaseGold(gold);
      player->setGold(gold);
      Playerlist::getInstance()->add(player);
    }
  Glib::ustring neutral_name = d_random->getPlayerName(Shield::NEUTRAL);
  Player* neutral = 
    new AI_Dummy(neutral_name, 
                 Armysetlist::getInstance()->get("default")->getId(), 
                 Shield::get_default_color_for_neutral(), 
                 GameMap::getWidth(), GameMap::getHeight(), MAX_PLAYERS);
  Playerlist::getInstance()->add(neutral);
  Playerlist::getInstance()->setNeutral(neutral);
  delete d_random;
}

static void 
import_ruins_and_temples (FILE *scn)
{
  fseek (scn, 0x80f, SEEK_CUR);
  unsigned short num_ruins = 0;
  fread (&num_ruins, sizeof (unsigned short), 1, scn);
  unsigned int i;
  for (i = 0; i < num_ruins; i++)
    {
      short x, y;
      unsigned short type;
      char name[21];
      char unused[5];
      fread (&x, sizeof (short), 1, scn);
      fread (&y, sizeof (short), 1, scn);
      memset (name, 0, sizeof (name));
      fread (name, sizeof (char), 20, scn);
      fread (&type, sizeof (unsigned short), 1, scn);
      fread (&unused, sizeof (char), 5, scn);
      if (type == 1)
        {
          Temple *t = new Temple (Vector<int>(x,y), 1, Glib::ustring(name));
          Templelist::getInstance()->add(t);
        }
      else if (type == 2)
        {
          Ruin *r = new Ruin (Vector<int>(x,y), 1, Glib::ustring(name));
          Ruinlist::getInstance()->add(r);
        }
      else
        std::cerr << String::ucompose(_("Error: We got an unknkown temple/ruin type of %1 for %2 at %3,%4"), type, Glib::ustring(name),x,y) << std::endl;
    }
}

static void 
import_cities (FILE *scn)
{
  fseek (scn, 0x157b, SEEK_CUR);
  unsigned short num_cities = 0;
  fread (&num_cities, sizeof (unsigned short), 1, scn);
  for (unsigned int i = 0; i < num_cities; i++)
    {
      short x, y;
      char name[17];
      char unused1[22];
      short income;
      char unused2[21];
      fread (&x, sizeof (short), 1, scn);
      fread (&y, sizeof (short), 1, scn);
      memset (name, 0, sizeof (name));
      fread (name, sizeof (char), 16, scn);
      fread (&unused1, sizeof (char), 22, scn);
      fread (&income, sizeof (short), 1, scn);
      fread (&unused2, sizeof (char), 21, scn);
      City *city = new City (Vector<int>(x,y), 2, Glib::ustring(name), income);
      city->setOwner(Playerlist::getInstance()->getNeutral());
      Citylist::getInstance()->add(city);
    }
}

static void
set_capital_cities (FILE *scn)
{
  fseek (scn, 0x189, SEEK_CUR);
  for (int i = 0; i < 8; i++)
    {
      short x, y;
      char unused[16];
      fread (&x, sizeof (x), 1, scn);
      fread (&y, sizeof (y), 1, scn);
      fread (unused, sizeof (char), 16, scn);

      Player *player = Playerlist::getInstance()->getPlayer(convert_player_id(i));
      if (player == NULL)
        continue;
      City *c = GameMap::getCity(Vector<int>(x,y));
      c->setOwner(player);
      c->setCapital(true);
      c->setCapitalOwner(player);
    }
}

static void
import_signposts (FILE *sg)
{
  unsigned short num_signs = 0;
  fread (&num_signs, sizeof (unsigned short), 1, sg);
  for (unsigned int i = 0; i < num_signs; i++)
    {
      short x, y;
      char line1[51];
      char line2[51];
      fread (&x, sizeof (short), 1, sg);
      fread (&y, sizeof (short), 1, sg);
      memset (line1, 0, sizeof (line1));
      fread (line1, sizeof (char), 50, sg);
      memset (line2, 0, sizeof (line2));
      fread (line2, sizeof (char), 50, sg);
      Signpost *s = 
        new Signpost (Vector<int>(x, y), 
                      Glib::ustring(line1) + "\n" + Glib::ustring(line2));
      Signpostlist::getInstance()->add(s);
    }
}

long get_sgn_offset (Glib::ustring filename)
{
  std::ifstream ifs(filename.c_str(), std::ios::binary);
  std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
  size_t pos = str.find(".SGN");

  if (pos != std::string::npos)
    return long(pos) + 8 +4;
  return 0;
}

static bool 
convert_item_code (int code, int value, ItemProto::Bonus &bonus)
{
  switch (code)
    {
    case 1: //battle
      switch (value)
        {
        case 1:
          bonus = ItemProto::ADD1STR; break;
        case 2:
          bonus = ItemProto::ADD2STR; break;
        case 3:
          bonus = ItemProto::ADD3STR; break;
        default:
          return false;
        }
      break;
    case 2: //command
      switch (value)
        {
        case 1:
          bonus = ItemProto::ADD1STACK; break;
        case 2:
          bonus = ItemProto::ADD2STACK; break;
        case 3:
          bonus = ItemProto::ADD3STACK; break;
        default:
          return false;
        }
      break;
    case 5:
      bonus = ItemProto::FLYSTACK;
      break;
    case 6:
      bonus = ItemProto::DOUBLEMOVESTACK;
      break;
    case 7:
      switch (value)
        {
        case 1:
          // whoopsie, better than nothing
          bonus = ItemProto::ADD2GOLDPERCITY; break;
        case 2:
          bonus = ItemProto::ADD2GOLDPERCITY; break;
        case 3:
          bonus = ItemProto::ADD3GOLDPERCITY; break;
        case 4:
          bonus = ItemProto::ADD4GOLDPERCITY; break;
        case 5:
          bonus = ItemProto::ADD5GOLDPERCITY; break;
        case 6:
          bonus = ItemProto::Bonus(ItemProto::ADD4GOLDPERCITY |
                                   ItemProto::ADD2GOLDPERCITY);
          break;
        default:
          return false;
        }
      break;
    default:
      return false;
    }
  return true;
}

static void
import_items (FILE *it)
{
  Itemlist::create();
  char *line = NULL;
  size_t len = 0;
  getline (&line, &len, it);
  int num_items = atoi (line);
  for (int i = 0; i < num_items; i++)
    {
      getdelim (&line, &len, ' ', it);
      std::string item_name = std::string(line);
      std::replace (item_name.begin(), item_name.end(), ' ', '\0');
      std::replace (item_name.begin(), item_name.end(), '_', ' ');
      getline (&line, &len, it);
      int code, value;
      sscanf (line, "%d %d", &code, &value);
      ItemProto::Bonus bonus;
      if (convert_item_code (code, value, bonus))
        {
          ItemProto *item = new ItemProto(Glib::ustring(item_name));
          item->addBonus(bonus);
          Itemlist::getInstance()->add(item);
        }
      else
        std::cerr << String::ucompose(_("Error: couldn't convert item %1 code %2, %3"), item_name, code, value) << std::endl;
    }
  free (line);
}

static void 
import (FILE *map, FILE *scn, FILE *rd, FILE *sg, FILE *it, Glib::ustring name)
{
  GameScenario *g = setup_new_map (name);

  long at = ftell (map);
  import_terrain (map);
  fseek (map, at, SEEK_SET);
  import_bridges (map);
  import_roads(rd);
  at = ftell (scn);
  import_players (scn);
  fseek (scn, at, SEEK_SET);
  import_ruins_and_temples (scn);
  fseek (scn, at, SEEK_SET);
  import_cities (scn);
  fseek (scn, at, SEEK_SET);
  set_capital_cities (scn);
  fseek (scn, at, SEEK_SET);
  import_signposts (sg);
  import_items (it);

  bool success = g->saveGame(name, MAP_EXT);
  if (!success)
    std::cerr << String::ucompose(_("Error: Could not save `%1%2'"), name, MAP_EXT) << std::endl;
  else
    std::cout << String::ucompose(_("Saved to %1.map"), name) << std::endl;
  delete g;
  return;
}

int main(int argc, char* argv[])
{
  Glib::ustring filename;
  initialize_configuration();
  Vector<int>::setMaximumWidth(1000);

  Glib::init();
  #if ENABLE_NLS
  setlocale(LC_ALL, Configuration::s_lang.c_str());
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
  #endif

  if (argc > 1)
    {
      for (int i = 2; i <= argc; i++)
	{
          Glib::ustring parameter(argv[i-1]); 
	  if (parameter == "--help" || parameter == "-?")
	    {
              std::cout << String::ucompose(_("Usage: %1 [OPTION]... FILE"), Glib::ustring(argv[0])) << std::endl;
              std::cout << String::ucompose("  or:  %1 [OPTION]... DIRECTORY", Glib::ustring(argv[0])) << std::endl << std::endl;
              std::cout << "LordsAWar! Warlords 2 Scenario Importing Tool " << _("version") << 
                " " << VERSION << std::endl << std::endl;
              std::cout << _("Options:") << std::endl << std::endl; 
              std::cout << "  -?, --help                 " << _("Display this help and exit") <<std::endl;
              std::cout << std::endl;
              std::cout << _("Report bugs to") << " <" << PACKAGE_BUGREPORT ">." << std::endl;
	      exit(0);
	    }
	  else
	    filename = parameter;
	}
    }

  if (File::directory_exists (filename) == true)
    {
      //look for the files we need.
      std::list<Glib::ustring> map = File::scanForFiles(filename, ".MAP");
      if (map.size() == 0)
        {
          std::cerr << String::ucompose (_("Error, Could not find a .MAP file in %1"), filename) << std::endl;
          exit (EXIT_FAILURE);
        }
      std::list<Glib::ustring> scn = File::scanForFiles(filename, ".SCN");
      if (scn.size() == 0)
        {
          std::cerr << String::ucompose (_("Error: Could not find a .SCN file in `%1'"), filename) << std::endl;
          exit (EXIT_FAILURE);
        }
      std::list<Glib::ustring> rd = File::scanForFiles(filename, ".RD");
      if (rd.size() == 0)
        {
          std::cerr << String::ucompose (_("Error: Could not find a .RD file in `%1'"), filename) << std::endl;
          exit (EXIT_FAILURE);
        }
      std::list<Glib::ustring> signs = File::scanForFiles(filename, ".SGN");
      if (signs.size() == 0)
        {
          std::cerr << String::ucompose (_("Error: Could not find a .SGN file in `%1'"), filename) << std::endl;
          exit (EXIT_FAILURE);
        }
      std::list<Glib::ustring> items = File::scanForFiles(filename, ".ITM");
      if (items.size() == 0)
        {
          std::cerr << String::ucompose (_("Error: Could not find a .ITM file in `%1'"), filename) << std::endl;
          exit (EXIT_FAILURE);
        }
      FILE *m = fopen (map.front().c_str(), "rb");
      FILE *s = fopen (scn.front().c_str(), "rb");
      FILE *r = fopen (rd.front().c_str(), "rb");
      FILE *sg = fopen (signs.front().c_str(), "rb");
      FILE *it = fopen (items.front().c_str(), "rb");
      Glib::ustring name = File::get_basename(map.front());
      import(m, s, r, sg, it, name);
      fclose (m);
      fclose (s);
      fclose (r);
      fclose (sg);
    }
  else if (File::exists (filename) == true)
    {
      Glib::ustring name = File::get_basename (filename);
      FILE *m = fopen (filename.c_str(), "rb");
      fseek (m, 0x02F69, SEEK_SET);
      FILE *s = fopen (filename.c_str(), "rb");
      fseek (s, 0x00074, SEEK_SET);
      FILE *r = fopen (filename.c_str(), "rb");
      fseek (r, 0x10DED, SEEK_SET);
      FILE *sg = fopen (filename.c_str(), "rb");
      fseek (sg, get_sgn_offset (filename), SEEK_SET);
      FILE *it = fopen (filename.c_str(), "rb");
      fseek (r, 0x15241, SEEK_SET);
      import (m, s, r, sg, it, name);
      fclose (m);
      fclose (s);
      fclose (r);
      fclose (sg);
    }
  else
    {
      std::cerr << String::ucompose (_("Error: Could not open `%1'"), filename) << std::endl;
      exit (EXIT_FAILURE);
    }
  return EXIT_SUCCESS;
}
