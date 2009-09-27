//  Copyright (C) 2008 Ben Asselstine
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
#ifndef TILE_PREVIEW_SCENE_H
#define TILE_PREVIEW_SCENE_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>
#include "Tile.h"
#include "tilestyle.h"
#include <list>
#include <vector>
#include <string>

class TilePreviewScene: public sigc::trackable
{
public:
  TilePreviewScene (Tile *tile, 
		    std::vector<PixMask*> standard_images, 
		    guint32 height, guint32 width, 
		    std::string scene);
  void regenerate();
  Glib::RefPtr<Gdk::Pixbuf> getTileStylePixbuf(int x, int y);
  TileStyle* getTileStyle(int x, int y);
  int getWidth() {return d_width;}
  int getHeight() {return d_height;}
  Tile *getTile() {return d_tile;}
  Glib::RefPtr<Gdk::Pixbuf> renderScene(guint32 tilesize);
private:
  //data:
    std::list<TileStyle::Type> d_model;
    std::vector<Glib::RefPtr<Gdk::Pixbuf> > d_view;
    std::vector<TileStyle*> d_tilestyles;
    guint32 d_height;
    guint32 d_width;
    Tile *d_tile;
    std::vector<PixMask*> d_standard_images;
};

#endif
