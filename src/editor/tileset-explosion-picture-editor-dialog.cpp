//  Copyright (C) 2009 Ben Asselstine
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

#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>
#include <stdlib.h>

#include "tileset-explosion-picture-editor-dialog.h"

#include "glade-helpers.h"
#include "gui/image-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "GraphicsCache.h"
#include "tile-preview-scene.h"


TilesetExplosionPictureEditorDialog::TilesetExplosionPictureEditorDialog(Tileset *tileset)
{
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/tileset-explosion-picture-editor-dialog.ui");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    d_tileset = tileset;

    xml->get_widget("explosion_filechooserbutton", explosion_filechooserbutton);
    explosion_filechooserbutton->signal_selection_changed().connect
       (sigc::mem_fun(*this, &TilesetExplosionPictureEditorDialog::on_image_chosen));

    xml->get_widget("large_explosion_radiobutton", large_explosion_radiobutton);
    large_explosion_radiobutton->signal_toggled().connect
      (sigc::mem_fun(*this, &TilesetExplosionPictureEditorDialog::on_large_toggled));
    xml->get_widget("small_explosion_radiobutton", small_explosion_radiobutton);
    small_explosion_radiobutton->signal_toggled().connect
      (sigc::mem_fun(*this, &TilesetExplosionPictureEditorDialog::on_small_toggled));

    xml->get_widget("scene_image", scene_image);
    on_large_toggled();
}

void TilesetExplosionPictureEditorDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void TilesetExplosionPictureEditorDialog::run()
{
    dialog->show_all();
    dialog->run();

    return;
}

void TilesetExplosionPictureEditorDialog::on_image_chosen()
{
  std::string selected_filename = explosion_filechooserbutton->get_filename();
  if (selected_filename.empty())
    return;

  std::string str = Configuration::s_dataPath + "/tilesets/" +  
    d_tileset->getSubDir() +"/";
  char mypath[PATH_MAX]; //god i hate path_max.  die die die
  char *tmp = realpath(str.c_str(), mypath);
  std::string path = tmp;
  if (selected_filename.substr(0, path.size()) !=path)
    return;
  std::string filename = &selected_filename.c_str()[path.size() + 1];
  d_tileset->setExplosionFilename(filename);

  show_explosion_image(filename);
}

void TilesetExplosionPictureEditorDialog::on_large_toggled()
{
  update_panel();
}

void TilesetExplosionPictureEditorDialog::on_small_toggled()
{
  update_panel();
}


void TilesetExplosionPictureEditorDialog::update_panel()
{
  std::string filename = d_tileset->getExplosionFilename();
  if (filename.c_str()[0] == '/')
    explosion_filechooserbutton->set_filename (filename);
  else
    explosion_filechooserbutton->set_filename
      (File::getTilesetFile(d_tileset->getSubDir(), filename));

}

void TilesetExplosionPictureEditorDialog::show_explosion_image(std::string filename)
{
  std::vector<PixMask* > tilestyle_images;
  std::vector<PixMask*> base_tilestyles;
  base_tilestyles = 
    disassemble_row(File::getMiscFile("various/editor/tilestyles.png"), 17);

  guint32 size = d_tileset->getTileSize();
  std::vector<PixMask*>::iterator it;
  for (it = base_tilestyles.begin(); it != base_tilestyles.end(); it++)
    {
      PixMask::scale(*it, size, size);
      tilestyle_images.push_back(*it);
    }
  TilePreviewScene *s;
  std::string scene;
  Tile *grass = (*d_tileset)[d_tileset->getIndex(Tile::GRASS)];
  scene.clear();
  if (large_explosion_radiobutton->get_active() == true)
    {
      scene += "aaaaaa";
      scene += "aaaaaa";
      scene += "aaaaaa";
      scene += "aaaaaa";
      scene += "aaaaaa";
      scene += "aaaaaa";
      s = new TilePreviewScene(grass, tilestyle_images, 6, 6, scene);
      update_scene(s, d_tileset->getSubDir() + "/" + filename);
    }
  else if (small_explosion_radiobutton->get_active() == true)
    {
      scene += "aaaaaaa";
      scene += "aaaaaaa";
      scene += "aaaaaaa";
      scene += "aaaaaaa";
      scene += "aaaaaaa";
      scene += "aaaaaaa";
      scene += "aaaaaaa";
      s = new TilePreviewScene(grass, tilestyle_images, 7, 7, scene);
      update_scene(s, d_tileset->getSubDir() + "/" + filename);
    }
}

void TilesetExplosionPictureEditorDialog::update_scene(TilePreviewScene *scene,
						       std::string filename)
{
  if (!scene)
    return;
  if (filename == "")
    return;

  Glib::RefPtr<Gdk::Pixbuf> scene_pixbuf;
  scene_pixbuf = scene->renderScene(d_tileset->getTileSize());
  //center the explosion image on the pixbuf
  //but the large explosion is scaled first

  Glib::RefPtr<Gdk::Pixbuf> explosion;
  if (small_explosion_radiobutton->get_active())
    {
      explosion = Gdk::Pixbuf::create_from_file(filename, d_tileset->getTileSize(), d_tileset->getTileSize(), false);
    }
  else if (large_explosion_radiobutton->get_active())
    {
      explosion = Gdk::Pixbuf::create_from_file(filename, d_tileset->getTileSize() * 2, d_tileset->getTileSize() * 2, false);
    }
  if (explosion == NULL)
    return;

  int i = (scene_pixbuf->get_width() - explosion->get_width()) / 2;
  int j = (scene_pixbuf->get_height() - explosion->get_height()) / 2;
  explosion->composite (scene_pixbuf, i, j, explosion->get_width(), explosion->get_height(), i, j, 1, 1, Gdk::INTERP_NEAREST, 255);
  scene_image->property_pixbuf() = scene_pixbuf;
  scene_image->show_all();
}
