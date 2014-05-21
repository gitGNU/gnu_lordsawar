//  Copyright (C) 2009, 2010, 2011, 2014 Ben Asselstine
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
#include "tarhelper.h"


TilesetExplosionPictureEditorDialog::TilesetExplosionPictureEditorDialog(Tileset *tileset)
{
  selected_filename = "";
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/tileset-explosion-picture-editor-dialog.ui");

    xml->get_widget("dialog", dialog);
    d_tileset = tileset;

    xml->get_widget("explosion_filechooserbutton", explosion_filechooserbutton);
    explosion_filechooserbutton->signal_file_set().connect
       (sigc::mem_fun(*this, &TilesetExplosionPictureEditorDialog::on_image_chosen));
    explosion_filechooserbutton->set_current_folder(Glib::get_home_dir());

    xml->get_widget("large_explosion_radiobutton", large_explosion_radiobutton);
    large_explosion_radiobutton->signal_toggled().connect
      (sigc::mem_fun(*this, &TilesetExplosionPictureEditorDialog::on_large_toggled));
    xml->get_widget("small_explosion_radiobutton", small_explosion_radiobutton);
    small_explosion_radiobutton->signal_toggled().connect
      (sigc::mem_fun(*this, &TilesetExplosionPictureEditorDialog::on_small_toggled));

    xml->get_widget("scene_image", scene_image);
    
    if (d_tileset->getExplosionFilename().empty() == false)
      {
        selected_filename = d_tileset->getFileFromConfigurationFile(d_tileset->getExplosionFilename() +".png");
        delfiles.push_back(selected_filename);
      }
    on_large_toggled();
}

TilesetExplosionPictureEditorDialog::~TilesetExplosionPictureEditorDialog()
{
  delete dialog;
}
void TilesetExplosionPictureEditorDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

int TilesetExplosionPictureEditorDialog::run()
{
    dialog->show_all();
    int response = dialog->run();

    if (std::find(delfiles.begin(), delfiles.end(), selected_filename)
        == delfiles.end() && response == Gtk::RESPONSE_ACCEPT)
      {
        d_tileset->replaceFileInConfigurationFile(d_tileset->getExplosionFilename()+".png", selected_filename);
        d_tileset->setExplosionFilename(File::get_basename(selected_filename));
      }
    else if (response == Gtk::RESPONSE_ACCEPT)
      response = Gtk::RESPONSE_CANCEL;
    for (std::list<std::string>::iterator it = delfiles.begin(); 
         it != delfiles.end(); it++)
      File::erase(*it);
    return response;
}

void TilesetExplosionPictureEditorDialog::on_image_chosen()
{
  selected_filename = explosion_filechooserbutton->get_filename();
  printf("-%s-\n", selected_filename.c_str());
  if (selected_filename.empty())
    return;

  show_explosion_image(selected_filename);
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
  if (selected_filename.empty() == false)
    {
    explosion_filechooserbutton->set_filename (selected_filename);
    show_explosion_image(selected_filename);
    }
}

void TilesetExplosionPictureEditorDialog::show_explosion_image(std::string filename)
{
  guint32 size = d_tileset->getTileSize();
  TilePreviewScene *s;
  std::string scene;
  guint32 idx = d_tileset->getIndex(Tile::GRASS);
  Tile *grass = NULL;
  if (d_tileset->size() > 0)
    grass = (*d_tileset)[idx];
  scene.clear();
  if (large_explosion_radiobutton->get_active() == true)
    {
      scene += "aaaaaa";
      scene += "aaaaaa";
      scene += "aaaaaa";
      scene += "aaaaaa";
      scene += "aaaaaa";
      scene += "aaaaaa";
      s = new TilePreviewScene(grass, NULL, 6, 6, scene, size);
      update_scene(s, filename);
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
      s = new TilePreviewScene(grass, NULL, 7, 7, scene, size);
      update_scene(s, filename);
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
      try
        {
          explosion = Gdk::Pixbuf::create_from_file(filename, 
                                                    d_tileset->getTileSize(), 
                                                    d_tileset->getTileSize(), 
                                                    false);
        }
      catch (const Glib::Exception &ex)
        {
          return;
        }
    }
  else if (large_explosion_radiobutton->get_active())
    {
      try
        {
          explosion = 
            Gdk::Pixbuf::create_from_file(filename, 
                                          d_tileset->getTileSize() * 2, 
                                          d_tileset->getTileSize() * 2, false);
        }
      catch (const Glib::Exception &ex)
        {
          return;
        }
    }
  if (!explosion)
    return;

  int i = (scene_pixbuf->get_width() - explosion->get_width()) / 2;
  int j = (scene_pixbuf->get_height() - explosion->get_height()) / 2;
  explosion->composite (scene_pixbuf, i, j, explosion->get_width(), explosion->get_height(), i, j, 1, 1, Gdk::INTERP_NEAREST, 255);
  scene_image->property_pixbuf() = scene_pixbuf;
  scene_image->queue_draw();
}
