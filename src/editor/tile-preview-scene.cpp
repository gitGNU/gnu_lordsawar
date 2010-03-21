//  Copyright (C) 2008, 2009 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>

#include "tile-preview-scene.h"

#include "glade-helpers.h"
#include "gui/image-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"


TilePreviewScene::TilePreviewScene (Tile *tile, Tile *secondary_tile,
				    std::vector<PixMask* > 
				      standard_images, 
				    guint32 height, guint32 width, 
				    std::string scene)
{
  struct tile_model model;
  std::list<struct tile_model> tilescene;
  for (const char *letter = scene.c_str(); *letter != '\0'; letter++)
    if (*letter - 'a' >=0 && *letter - 'a' <= TileStyle::OTHER)
      {
        model.tile = tile;
        model.type = TileStyle::Type(*letter - 'a');
        tilescene.push_back(model);
      }
    else if (*letter - 'A' >=0 && *letter - 'A' <= TileStyle::OTHER)
      {
        model.tile = secondary_tile;
        model.type = TileStyle::Type(*letter - 'A');
        tilescene.push_back(model);
      }

  if (height * width != tilescene.size())
    return;

  d_tile = tile;
  d_secondary_tile = secondary_tile;
  d_standard_images = standard_images;
  d_height = height;
  d_width = width;
  d_model = tilescene;
  regenerate();
}

Glib::RefPtr<Gdk::Pixbuf> TilePreviewScene::getTileStylePixbuf(int x, int y)
{
  return d_view[x * d_width + y];
}
  
TileStyle* TilePreviewScene::getTileStyle(int x, int y)
{
  return d_tilestyles[x * d_width + y];
}
  
void TilePreviewScene::regenerate()
{
  //populate d_view
  d_view.clear();
  for (std::list<struct tile_model>::iterator it = d_model.begin(); 
       it != d_model.end(); it++)
    {
      struct tile_model model = *it;
      TileStyle *tilestyle = NULL;
      if (model.tile)
	{
	  tilestyle = model.tile->getRandomTileStyle(model.type);
	  d_tilestyles.push_back(tilestyle);
	}

      if (tilestyle)
	d_view.push_back(tilestyle->getImage()->to_pixbuf());
      else
	d_view.push_back(d_standard_images[model.type]->to_pixbuf());
    }
}
  
Glib::RefPtr<Gdk::Pixbuf> TilePreviewScene::renderScene(guint32 tilesize)
{
  Glib::RefPtr<Gdk::Pixbuf> dest;
  dest = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB,true, 8, (int)(d_height * tilesize), (int)(d_width * tilesize));
  for (unsigned int i = 0; i < d_width; i++)
    for (unsigned int j = 0; j < d_height; j++)
      {
	getTileStylePixbuf(i,j)->copy_area (0, 0, tilesize, tilesize, dest, j * tilesize, i *tilesize);
      }
  return dest;
}
