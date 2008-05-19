//  Copyright (C) 2008 Ben Asselstine
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

#include <config.h>

#include <libglademm/xml.h>
#include <sigc++/functors/mem_fun.h>

#include "tile-preview-scene.h"

#include "glade-helpers.h"
#include "../gui/image-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../File.h"


TilePreviewScene::TilePreviewScene (Tile *tile, 
				    std::vector<Glib::RefPtr<Gdk::Pixbuf> > 
				      standard_images, 
				    Uint32 height, Uint32 width, 
				    std::string scene)
{
  std::list<TileStyle::Type> tilescene;
  for (const char *letter = scene.c_str(); *letter != '\0'; letter++)
    if (*letter - 'a' >=0 && *letter - 'a' <= TileStyle::OTHER)
      tilescene.push_back(TileStyle::Type(*letter - 'a'));

  if (height * width != tilescene.size())
    return;

  d_tile = tile;
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
  for (std::list<TileStyle::Type>::iterator it = d_model.begin(); 
       it != d_model.end(); it++)
    {
      TileStyle::Type type = *it;
      TileStyle *tilestyle = d_tile->getRandomTileStyle(type);
      d_tilestyles.push_back(tilestyle);
      if (tilestyle)
	d_view.push_back(to_pixbuf(tilestyle->getPixmap()));
      else
	d_view.push_back(d_standard_images[type]);
    }
}
