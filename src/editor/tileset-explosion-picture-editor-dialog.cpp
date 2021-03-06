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

#include "tileset-explosion-picture-editor-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "ImageCache.h"
#include "tarhelper.h"
#include "tile-preview-scene.h"
#include "tileset-window.h"
#include "past-chooser.h"

#define method(x) sigc::mem_fun(*this, &TilesetExplosionPictureEditorDialog::x)

TilesetExplosionPictureEditorDialog::TilesetExplosionPictureEditorDialog(Gtk::Window &parent, Tileset *tileset)
 : LwEditorDialog(parent, "tileset-explosion-picture-editor-dialog.ui")
{
  selected_filename = "";
    d_tileset = tileset;

    xml->get_widget("explosion_filechooserbutton", explosion_filechooserbutton);
    explosion_filechooserbutton->signal_file_set().connect (method(on_image_chosen));
    explosion_filechooserbutton->set_current_folder(Glib::get_home_dir());

    xml->get_widget("large_explosion_radiobutton", large_explosion_radiobutton);
    large_explosion_radiobutton->signal_toggled().connect (method(on_large_toggled));
    xml->get_widget("small_explosion_radiobutton", small_explosion_radiobutton);
    small_explosion_radiobutton->signal_toggled().connect (method(on_small_toggled));

    xml->get_widget("scene_image", scene_image);
    
    if (d_tileset->getExplosionFilename().empty() == false)
      {
        selected_filename = d_tileset->getFileFromConfigurationFile(d_tileset->getExplosionFilename() +".png");
        delfiles.push_back(selected_filename);
      }
    on_large_toggled();
}

int TilesetExplosionPictureEditorDialog::run()
{
    dialog->show_all();
    int response = dialog->run();

    if (response == Gtk::RESPONSE_ACCEPT)
      PastChooser::getInstance()->set_dir(explosion_filechooserbutton);

    if (std::find(delfiles.begin(), delfiles.end(), selected_filename)
        == delfiles.end() && response == Gtk::RESPONSE_ACCEPT)
      {
        Glib::ustring file = File::get_basename(selected_filename);
        if (d_tileset->replaceFileInConfigurationFile(d_tileset->getExplosionFilename()+".png", selected_filename))
          d_tileset->setExplosionFilename(file);
        else
          {
            TileSetWindow::show_add_file_error (d_tileset, *dialog, file);
            response =Gtk::RESPONSE_CANCEL;
          }
      }
    else if (response == Gtk::RESPONSE_ACCEPT)
      response = Gtk::RESPONSE_CANCEL;
    for (std::list<Glib::ustring>::iterator it = delfiles.begin(); 
         it != delfiles.end(); it++)
      File::erase(*it);
    return response;
}

void TilesetExplosionPictureEditorDialog::on_image_chosen()
{
  selected_filename = explosion_filechooserbutton->get_filename();
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

void TilesetExplosionPictureEditorDialog::show_explosion_image(Glib::ustring filename)
{
  guint32 size = d_tileset->getTileSize();
  TilePreviewScene *s;
  Glib::ustring scene;
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
						       Glib::ustring filename)
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

void TilesetExplosionPictureEditorDialog::on_add(Gtk::Widget *widget)
{
  if (widget)
    {
      Gtk::Button *button = dynamic_cast<Gtk::Button*>(widget);
      button->signal_clicked().connect (method(on_button_pressed));
    }
}

void TilesetExplosionPictureEditorDialog::on_button_pressed()
{
  Glib::ustring d = 
    PastChooser::getInstance()->get_dir(explosion_filechooserbutton);
  if (d.empty() == false)
    explosion_filechooserbutton->set_current_folder(d);
}
