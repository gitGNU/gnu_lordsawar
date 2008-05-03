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

#include <SDL_image.h>
#include <sigc++/functors/mem_fun.h>

#include "tileset.h"

#include "File.h"
#include "xmlhelper.h"

using namespace std;

#include <iostream>
//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

Tileset::Tileset(XML_Helper *helper)
{
    helper->getData(d_name, "name"); 
    helper->getData(d_info, "info");
    helper->getData(d_tileSize, "tilesize");
    helper->registerTag("tile", sigc::mem_fun((*this), &Tileset::loadTile));
    helper->registerTag("smallmap", sigc::mem_fun((*this), &Tileset::loadTile));
    helper->registerTag("tilestyle", sigc::mem_fun((*this), &Tileset::loadTile));
    helper->registerTag("tilestyleset", sigc::mem_fun((*this), &Tileset::loadTile));
}

Tileset::~Tileset()
{
    for (unsigned int i=0; i < size(); i++)
        delete (*this)[i];
}

void Tileset::instantiatePixmaps()
{
    for (Uint32 i = 0; i < size(); i++)
      (*this)[i]->instantiatePixmaps(d_dir, d_tileSize);
}

Uint32 Tileset::getIndex(Tile::Type type) const
{
    for (Uint32 i = 0; i < size(); i++)
        if (type == (*this)[i]->getType())
            return i;

    // catch errors?
    return 0;
}

bool Tileset::loadTile(string tag, XML_Helper* helper)
{
    debug("loadTile()")

    if (tag == "tile")
      {
	// create a new tile with the information we got
	Tile* tile = new Tile(helper);
	this->push_back(tile);

	return true;
      }

    if (tag == "smallmap")
      {
	Uint32 i;
	SDL_Color color;
	color.unused = 0;
	Tile *tile = this->back();
	helper->getData(i, "red");      color.r = i;
	helper->getData(i, "green");    color.g = i;
	helper->getData(i, "blue");     color.b = i;
	tile->setColor(color);

	helper->getData(i, "pattern");
	Tile::Pattern pattern = static_cast<Tile::Pattern>(i);
	tile->setPattern(pattern);
    
	if (pattern != Tile::SOLID)
	  {
	    helper->getData(i, "2nd_red");      color.r = i;
	    helper->getData(i, "2nd_green");    color.g = i;
	    helper->getData(i, "2nd_blue");     color.b = i;
	    tile->setSecondColor(color);
	    if (pattern != Tile::STIPPLED && pattern != Tile::SUNKEN)
	      {
		helper->getData(i, "3rd_red");      color.r = i;
		helper->getData(i, "3rd_green");    color.g = i;
		helper->getData(i, "3rd_blue");     color.b = i;
		tile->setThirdColor(color);
	      }
	  }
	return true;
      }

    if (tag == "tilestyle")
      {
	Tile *tile = this->back();
	TileStyleSet *tilestyleset = tile->back();
	// create a new tile style with the information we got
	// put it on the latest tilestyleset
	TileStyle* tilestyle = new TileStyle(helper);
	tilestyleset->push_back(tilestyle);
	d_tilestyles[tilestyle->getId()] = tilestyle;

	return true;
      }

    if (tag == "tilestyleset")
      {
	Tile *tile = this->back();
	// create a new tile style set with the information we got
	// put it on the latest tile
	TileStyleSet* tilestyleset = new TileStyleSet(helper);
	tile->push_back(tilestyleset);
	return true;
      }

    return false;
}

TileStyle *Tileset::getRandomTileStyle(Uint32 index, TileStyle::Type style)
{
  Tile *tile = (*this)[index];
  std::vector<TileStyle*> tilestyles;
  for (Uint32 j = 0; j < tile->size(); j++)
    {
      TileStyleSet *tilestyleset = (*tile)[j];
      for (Uint32 k = 0; k < tilestyleset->size(); k++)
	{
	  TileStyle *tilestyle = (*tilestyleset)[k];
	  if (tilestyle->getType() == style)
	    tilestyles.push_back(tilestyle);
	}
    }

  if (tilestyles.empty() == true)
    return NULL;
  return tilestyles[rand() % tilestyles.size()];
}

bool Tileset::save(XML_Helper *helper)
{
  bool retval = true;

  retval &= helper->openTag("tileset");
  retval &= helper->saveData("name", d_name);
  retval &= helper->saveData("info", d_info);
  retval &= helper->saveData("tilesize", d_tileSize);
  for (Tileset::iterator i = begin(); i != end(); ++i)
    retval &= (*i)->save(helper);
  retval &= helper->closeTag();

  return retval;
}
// End of file
