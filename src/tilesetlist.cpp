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

#include <iostream>
#include <expat.h>
#include <SDL_image.h>
#include <SDL.h>
#include "rectangle.h"
#include <sigc++/functors/mem_fun.h>

#include "tilesetlist.h"
#include "armyset.h"
#include "File.h"
#include "defs.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

Tilesetlist* Tilesetlist::s_instance = 0;

Tilesetlist* Tilesetlist::getInstance()
{
    if (!s_instance)
        s_instance = new Tilesetlist();

    return s_instance;
}

void Tilesetlist::deleteInstance()
{
    if (s_instance)
      delete s_instance;

    s_instance = 0;
}

Tilesetlist::Tilesetlist()
{
    // load all tilesets
    std::list<std::string> tilesets = File::scanTilesets();

    for (std::list<std::string>::const_iterator i = tilesets.begin(); 
	 i != tilesets.end(); i++)
      {
        loadTileset(*i);
	iterator it = end();
	it--;
	(*it)->setSubDir(*i);
	d_dirs[(*it)->getName()] = *i;
	d_tilesets[*i] = *it;
      }
}

Tilesetlist::~Tilesetlist()
{
  for (iterator it = begin(); it != end(); it++)
    delete (*it);
}

std::list<std::string> Tilesetlist::getNames()
{
  std::list<std::string> names;
  for (iterator it = begin(); it != end(); it++)
    names.push_back((*it)->getName());
  return names;
}

bool Tilesetlist::load(std::string tag, XML_Helper *helper)
{
  if (tag == "tileset")
    {
      TileSet *tileset = new TileSet(helper);
      push_back(tileset); 
    }
  return true;
}

bool Tilesetlist::loadTileset(std::string name)
{
  debug("Loading tileset " <<name);

  XML_Helper helper(File::getTileset(name), ios::in, false);

  helper.registerTag("tileset", sigc::mem_fun((*this), &Tilesetlist::load));

  if (!helper.parse())
    {
      std::cerr <<_("Error, while loading a tileset. TileSet Name: ");
      std::cerr <<name <<std::endl <<std::flush;
      exit(-1);
    }

  return true;
}
        
void Tilesetlist::instantiatePixmaps()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->instantiatePixmaps();
}
