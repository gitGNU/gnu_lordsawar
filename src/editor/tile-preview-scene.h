
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
		    std::vector<Glib::RefPtr<Gdk::Pixbuf> > standard_images, 
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
    std::vector<Glib::RefPtr<Gdk::Pixbuf> > d_standard_images;
};

#endif
