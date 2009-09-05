//  Copyright (C) 2008, 2009 Ben Asselstine
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

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <assert.h>
#include <libgen.h>

#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>
#include "ucompose.hpp"

#include <gtkmm.h>
#include "tileset-window.h"
#include "tileset-info-dialog.h"
#include "tile-preview-dialog.h"
#include "tileset-selector-editor-dialog.h"
#include "tileset-explosion-picture-editor-dialog.h"

#include "gtksdl.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "error-utils.h"

#include "defs.h"
#include "Configuration.h"
#include "tilesetlist.h"
#include "Tile.h"
#include "File.h"
#include "overviewmap.h"
#include "GraphicsCache.h"
#include "GraphicsLoader.h"

#include "ucompose.hpp"

#include "glade-helpers.h"


TileSetWindow::TileSetWindow()
{
  d_tileset = NULL;
    sdl_inited = false;
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path() + "/tileset-window.ui");

    Gtk::Window *w = 0;
    xml->get_widget("window", w);
    window.reset(w);
    w->set_icon_from_file(File::getMiscFile("various/tileset_icon.png"));

    xml->get_widget("tiles_treeview", tiles_treeview);
    xml->get_widget("tile_name_entry", tile_name_entry);
    tile_name_entry->signal_changed().connect
      (sigc::mem_fun(this, &TileSetWindow::on_tile_name_changed));

    xml->get_widget("tile_type_combobox", tile_type_combobox);
    tile_type_combobox->signal_changed().connect
      (sigc::mem_fun(this, &TileSetWindow::on_tile_type_changed));
    xml->get_widget("tile_moves_spinbutton", tile_moves_spinbutton);
    xml->get_widget("tile_smallmap_pattern_combobox", 
		    tile_smallmap_pattern_combobox);
    tile_smallmap_pattern_combobox->signal_changed().connect
    (sigc::mem_fun(this, &TileSetWindow::on_tile_pattern_changed));
    xml->get_widget("tile_smallmap_first_colorbutton", 
		    tile_smallmap_first_colorbutton);
    tile_smallmap_first_colorbutton->signal_color_set().connect
      (sigc::mem_fun(this, &TileSetWindow::on_tile_first_color_changed));
    xml->get_widget("tile_smallmap_second_colorbutton", 
		    tile_smallmap_second_colorbutton);
    tile_smallmap_second_colorbutton->signal_color_set().connect
      (sigc::mem_fun(this, &TileSetWindow::on_tile_second_color_changed));
    xml->get_widget("tile_smallmap_third_colorbutton", 
		    tile_smallmap_third_colorbutton);
    tile_smallmap_third_colorbutton->signal_color_set().connect
      (sigc::mem_fun(this, &TileSetWindow::on_tile_third_color_changed));
    xml->get_widget("tile_smallmap_image", tile_smallmap_image);

    xml->get_widget("add_tile_button", add_tile_button);
    add_tile_button->signal_clicked().connect
      (sigc::mem_fun(this, &TileSetWindow::on_add_tile_clicked));
    xml->get_widget("remove_tile_button", remove_tile_button);
    remove_tile_button->signal_clicked().connect
      (sigc::mem_fun(this, &TileSetWindow::on_remove_tile_clicked));
    xml->get_widget("tile_vbox", tile_vbox);
    // connect callbacks for the menu
    xml->get_widget("new_tileset_menuitem", new_tileset_menuitem);
    new_tileset_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &TileSetWindow::on_new_tileset_activated));
    xml->get_widget("load_tileset_menuitem", load_tileset_menuitem);
    load_tileset_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &TileSetWindow::on_load_tileset_activated));
    xml->get_widget("save_tileset_menuitem", save_tileset_menuitem);
    save_tileset_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &TileSetWindow::on_save_tileset_activated));
    xml->get_widget("save_tileset_as_menuitem", save_tileset_as_menuitem);
    save_tileset_as_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &TileSetWindow::on_save_tileset_as_activated));
    xml->get_widget("quit_menuitem", quit_menuitem);
    quit_menuitem->signal_activate().connect
       (sigc::mem_fun(this, &TileSetWindow::on_quit_activated));
    xml->get_widget("edit_tileset_info_menuitem", edit_tileset_info_menuitem);
    edit_tileset_info_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &TileSetWindow::on_edit_tileset_info_activated));
    xml->get_widget("army_unit_selector_menuitem", army_unit_selector_menuitem);
    army_unit_selector_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &TileSetWindow::on_army_unit_selector_activated));
    xml->get_widget("explosion_picture_menuitem", explosion_picture_menuitem);
    explosion_picture_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &TileSetWindow::on_explosion_picture_activated));
    xml->get_widget("preview_tile_menuitem", preview_tile_menuitem);
    preview_tile_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &TileSetWindow::on_preview_tile_activated));
    xml->get_widget ("help_about_menuitem", help_about_menuitem);
    help_about_menuitem->signal_activate().connect
       (sigc::mem_fun(this, &TileSetWindow::on_help_about_activated));
    xml->get_widget("tilestyle_image", tilestyle_image);

    w->signal_delete_event().connect(
	sigc::mem_fun(*this, &TileSetWindow::on_delete_event));

    tiles_list = Gtk::ListStore::create(tiles_columns);
    tiles_treeview->set_model(tiles_list);
    tiles_treeview->append_column("", tiles_columns.name);
    tiles_treeview->set_headers_visible(false);
    tiles_treeview->get_selection()->signal_changed().connect
      (sigc::mem_fun(*this, &TileSetWindow::on_tile_selected));

    xml->get_widget("tilestylesets_treeview", tilestylesets_treeview);
    tilestylesets_list = Gtk::ListStore::create(tilestylesets_columns);
    tilestylesets_treeview->set_model(tilestylesets_list);
    tilestylesets_treeview->append_column("", tilestylesets_columns.name);
    tilestylesets_treeview->set_headers_visible(false);
    tilestylesets_treeview->get_selection()->signal_changed().connect
      (sigc::mem_fun(*this, &TileSetWindow::on_tilestyleset_selected));

    xml->get_widget("tilestyles_treeview", tilestyles_treeview);
    tilestyles_list = Gtk::ListStore::create(tilestyles_columns);
    tilestyles_treeview->set_model(tilestyles_list);
    tilestyles_treeview->append_column("", tilestyles_columns.name);
    tilestyles_treeview->set_headers_visible(false);
    tilestyles_treeview->get_selection()->signal_changed().connect
      (sigc::mem_fun(*this, &TileSetWindow::on_tilestyle_selected));

    xml->get_widget("add_tilestyleset_button", add_tilestyleset_button);
    add_tilestyleset_button->signal_clicked().connect
      (sigc::mem_fun(this, &TileSetWindow::on_add_tilestyleset_clicked));
    xml->get_widget("remove_tilestyleset_button", remove_tilestyleset_button);
    remove_tilestyleset_button->signal_clicked().connect
      (sigc::mem_fun(this, &TileSetWindow::on_remove_tilestyleset_clicked));
    xml->get_widget("tilestyleset_frame", tilestyleset_frame);
    xml->get_widget("tilestyle_frame", tilestyle_frame);
    xml->get_widget("tilestyle_combobox", tilestyle_combobox);
    tilestyle_combobox->signal_changed().connect
      (sigc::mem_fun(this, &TileSetWindow::on_tilestyle_changed));
    xml->get_widget("image_filechooser_button", image_filechooser_button);
    image_filechooser_button->signal_selection_changed().connect
      (sigc::mem_fun(*this, &TileSetWindow::on_image_chosen));
    xml->get_widget("refresh_button", refresh_button);
    refresh_button->signal_clicked().connect
      (sigc::mem_fun(this, &TileSetWindow::on_refresh_clicked));

    Gtk::FileFilter sav_filter;
    sav_filter.add_pattern("*.png");
    image_filechooser_button->set_filter(sav_filter);
    image_filechooser_button->set_current_folder
      (Configuration::s_dataPath + "/tilesets/");

    xml->get_widget("tilestyle_standard_image", tilestyle_standard_image);
    tilestyle_standard_images = 
      disassemble_row(File::getMiscFile("various/editor/tilestyles.png"), 17);

    update_tile_panel();
    update_tilestyleset_panel();
    update_tilestyle_panel();
    update_tileset_buttons();
    update_tilestyleset_buttons();

    xml->get_widget("sdl_container", sdl_container);
    update_tileset_buttons();
    update_tilestyleset_buttons();
    update_tileset_menuitems();
    update_tile_preview_menuitem();

    tile_smallmap_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 32, 32, 24,
						 0xFFu, 0xFFu << 8, 
						 0xFFu << 16, 0);

    inhibit_image_change = false;
}

void
TileSetWindow::update_tileset_menuitems()
{
  if (d_tileset == NULL)
    {
      save_tileset_as_menuitem->set_sensitive(false);
      save_tileset_menuitem->set_sensitive(false);
      army_unit_selector_menuitem->set_sensitive(false);
    }
  else
    {
      save_tileset_as_menuitem->set_sensitive(true);
      save_tileset_menuitem->set_sensitive(true);
      army_unit_selector_menuitem->set_sensitive(true);
    }
}

void
TileSetWindow::update_tileset_buttons()
{
  if (!tiles_treeview->get_selection()->get_selected())
    remove_tile_button->set_sensitive(false);
  else
    remove_tile_button->set_sensitive(true);
  if (d_tileset == NULL)
    add_tile_button->set_sensitive(false);
  else
    add_tile_button->set_sensitive(true);
}

void
TileSetWindow::update_tilestyleset_buttons()
{
  if (!tilestylesets_treeview->get_selection()->get_selected())
    remove_tilestyleset_button->set_sensitive(false);
  else
    remove_tilestyleset_button->set_sensitive(true);
  if (d_tileset == NULL)
    add_tilestyleset_button->set_sensitive(false);
  else
    add_tilestyleset_button->set_sensitive(true);
}

void
TileSetWindow::update_tilestyle_panel()
{
  if (tilestyles_treeview->get_selection()->get_selected() == 0)
    {
      //clear all values
      tilestyle_frame->set_sensitive(false);
      tilestyle_combobox->set_active(0);
      tilestyle_image->clear();
      tilestyle_standard_image->clear();
      tilestyle_image->show_all();
	  
      return;
    }
  tilestyle_frame->set_sensitive(true);
  TileStyle *t = get_selected_tilestyle ();
  if (t)
    {
      tilestyle_combobox->set_active(t->getType());
      SDL_Surface *pixmap = t->getPixmap();
      if (pixmap)
	{
	  tilestyle_image->clear();
	  tilestyle_image->property_pixbuf() = to_pixbuf(pixmap);
	  tilestyle_image->show_all();
	}
      int idx = t->getType();
      tilestyle_standard_image->property_pixbuf() = 
	tilestyle_standard_images[idx];
    }
}

int get_image_height(std::string filename)
{
  Glib::RefPtr<Gdk::Pixbuf> row = Gdk::Pixbuf::create_from_file(filename);
  if (row)
    return row->get_height();
  else
    return 0;
}

int get_image_width (std::string filename)
{
  Glib::RefPtr<Gdk::Pixbuf> row = Gdk::Pixbuf::create_from_file(filename);
  if (row)
    return row->get_width();
  else
    return 0;
}

void TileSetWindow::fill_tilestyleset_info(TileStyleSet *t)
{
  std::string subdir;
  if (t->getSubDir() == "")
    subdir = d_tileset->getSubDir();
  else
    subdir = t->getSubDir();
  std::string n = Configuration::s_dataPath + "/tilesets/" + subdir + "/" + t->getName() + ".png";
  int height = get_image_height (n);
  if (height)
    {
      d_tileset->setTileSize(height);
      GraphicsLoader::instantiatePixmaps(t, d_tileset->getTileSize());
    }
  inhibit_image_change = true;
  image_filechooser_button->set_filename(n);
  refresh_button->set_sensitive(true);
  //add the tilestyles to the tilestyles_treeview
  tilestyles_list->clear();
  for (unsigned int i = 0; i < t->size(); i++)
    {
      Gtk::TreeIter l = tilestyles_list->append();
      (*l)[tilestyles_columns.name] = 
	String::ucompose("0x%1", Glib::ustring::format(std::hex, std::setfill(L'0'), std::setw(2), (*t)[i]->getId()));
      (*l)[tilestyles_columns.tilestyle] = (*t)[i];
    }
}

void
TileSetWindow::update_tilestyleset_panel()
{
  if (tilestylesets_treeview->get_selection()->get_selected() == 0)
    {
      //clear all values
      tilestyleset_frame->set_sensitive(false);
      image_filechooser_button->set_current_folder
	(Configuration::s_dataPath + "/tilesets/");
      tilestyles_list->clear();
      refresh_button->set_sensitive(false);
      return;
    }
  tilestyleset_frame->set_sensitive(true);
  TileStyleSet *t = get_selected_tilestyleset ();
  if (t && t->getName() != "")
    {
      fill_tilestyleset_info(t);
    }
}

void
TileSetWindow::update_tile_panel()
{
  Gdk::Color black("black");
  //if nothing selected in the treeview, then we don't show anything in
  //the tile panel
  if (tiles_treeview->get_selection()->get_selected() == 0)
    {
      //clear all values
      tile_smallmap_image->clear();
      tile_vbox->set_sensitive(false);
      tile_type_combobox->set_active(0);
      tile_moves_spinbutton->set_value(0);
      tile_name_entry->set_text("");
      tile_smallmap_pattern_combobox->set_active(0);
      tile_smallmap_first_colorbutton->set_color(black);
      tile_smallmap_second_colorbutton->set_color(black);
      tile_smallmap_third_colorbutton->set_color(black);
      tilestylesets_list->clear();
      return;
    }
  tile_vbox->set_sensitive(true);
  Tile *t = get_selected_tile ();

  if (t)
    {
      tile_smallmap_first_colorbutton->set_color(black);
      tile_smallmap_second_colorbutton->set_color(black);
      tile_smallmap_third_colorbutton->set_color(black);
      fill_tile_info(t);
    }
}
TileSetWindow::~TileSetWindow()
{
  SDL_FreeSurface (tile_smallmap_surface);
}

void TileSetWindow::show()
{
  sdl_container->show_all();
  window->show();
}

void TileSetWindow::hide()
{
  window->hide();
}

namespace 
{
  void surface_attached_helper(GtkSDL *gtksdl, gpointer data)
    {
      static_cast<TileSetWindow *>(data)->on_sdl_surface_changed();
    }
}

void TileSetWindow::init(int width, int height)
{
  sdl_widget
    = Gtk::manage(Glib::wrap(gtk_sdl_new(width, height, 0, SDL_SWSURFACE)));

  sdl_widget->set_flags(Gtk::CAN_FOCUS);

  sdl_widget->grab_focus();
  sdl_widget->add_events(Gdk::KEY_PRESS_MASK |
			 Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
			 Gdk::POINTER_MOTION_MASK | Gdk::LEAVE_NOTIFY_MASK);

  // connect to the special signal that signifies that a new surface has been
  // generated and attached to the widget
  g_signal_connect(G_OBJECT(sdl_widget->gobj()), "surface-attached",
		   G_CALLBACK(surface_attached_helper), this);

  sdl_container->add(*sdl_widget);
}

bool TileSetWindow::on_delete_event(GdkEventAny *e)
{
  hide();

  return true;
}


bool TileSetWindow::load(std::string tag, XML_Helper *helper)
{
  if (tag == "tileset")
    d_tileset = new Tileset(helper);
  return true;
}

void TileSetWindow::on_new_tileset_activated()
{
  bool retval;
  current_save_filename.clear();
  if (d_tileset)
    {
      tiles_list->clear();
      delete d_tileset;
    }
  std::string name = "";
  d_tileset = new Tileset(name);
  TileSetInfoDialog d(d_tileset);
  d.set_parent_window(*window.get());
  retval = d.run();
  if (retval == false)
    {
      delete d_tileset;
      d_tileset = NULL;
    }


  update_tileset_buttons();
  update_tilestyleset_buttons();
  update_tileset_menuitems();
  update_tile_preview_menuitem();
}

void TileSetWindow::on_load_tileset_activated()
{
  Gtk::FileChooserDialog chooser(*window.get(), 
				 _("Choose a Tileset to Load"));
  Gtk::FileFilter sav_filter;
  sav_filter.add_pattern("*.xml");
  chooser.set_filter(sav_filter);
  chooser.set_current_folder(Configuration::s_savePath);

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);

  chooser.set_current_folder(Configuration::s_dataPath + "/tilesets/");

  chooser.show_all();
  int res = chooser.run();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      current_save_filename = chooser.get_filename();
      chooser.hide();

      tiles_list->clear();
      if (d_tileset)
	delete d_tileset;
      XML_Helper helper(current_save_filename, std::ios::in, false);

      helper.registerTag("tileset", 
			 sigc::mem_fun((*this), &TileSetWindow::load));


      if (!helper.parse())
	{
	  std::cerr <<_("Error, while loading an tileset. Tileset Name: ");
	  std::cerr <<current_save_filename <<std::endl <<std::flush;
	  exit(-1);
	}

      char *dir = g_strdup(current_save_filename.c_str());
      char *tmp = strrchr (dir, '/');
      if (tmp)
	tmp[0] = '\0';
      //hackus horribilium
      std::string back = "../../../../../../../../../../../../../../../../";
      d_tileset->setSubDir(back + dir);
      GraphicsLoader::instantiatePixmaps(d_tileset);
      for (Tileset::iterator i = d_tileset->begin(); i != d_tileset->end(); ++i)
	{
	  Gtk::TreeIter l = tiles_list->append();
	  (*l)[tiles_columns.name] = (*i)->getName();
	  (*l)[tiles_columns.tile] = *i;
	}
      if (d_tileset->size())
	tiles_treeview->set_cursor (Gtk::TreePath ("0"));

      update_tileset_buttons();
      update_tilestyleset_buttons();
      update_tileset_menuitems();
      update_tile_panel();
      update_tilestyleset_panel();
      update_tilestyle_panel();
      update_tile_preview_menuitem();
    }
}

void TileSetWindow::on_save_tileset_activated()
{
  if (current_save_filename.empty())
    on_save_tileset_as_activated();
  else
    {
      XML_Helper helper(current_save_filename, std::ios::out, false);
      helper.openTag("tileset");
      d_tileset->save(&helper);
      helper.closeTag();
      helper.close();
    }
}

void TileSetWindow::on_save_tileset_as_activated()
{
  Gtk::FileChooserDialog chooser(*window.get(), _("Choose a Name"),
				 Gtk::FILE_CHOOSER_ACTION_SAVE);
  Gtk::FileFilter sav_filter;
  sav_filter.add_pattern("*.xml");
  chooser.set_filter(sav_filter);
  chooser.set_current_folder(Configuration::s_dataPath + "/tilesets/");

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);

  chooser.show_all();
  int res = chooser.run();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      current_save_filename = chooser.get_filename();
      chooser.hide();
      on_save_tileset_activated();
    }
}

void TileSetWindow::on_quit_activated()
{
  // FIXME: ask
  bool end = true;

  if (end) {
  }
  window->hide();
}

void TileSetWindow::on_edit_tileset_info_activated()
{
  TileSetInfoDialog d(d_tileset);
  d.set_parent_window(*window.get());
  d.run();
}

void TileSetWindow::on_help_about_activated()
{
  std::auto_ptr<Gtk::AboutDialog> dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/../about-dialog.ui");

  Gtk::AboutDialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  dialog->set_transient_for(*window.get());
  d->set_icon_from_file(File::getMiscFile("various/tileset_icon.png"));

  dialog->set_version(PACKAGE_VERSION);
  SDL_Surface *logo = GraphicsLoader::getMiscPicture("tileset_icon.png");
  dialog->set_logo(to_pixbuf(logo));
  dialog->show_all();
  dialog->run();

  return;
}

void TileSetWindow::on_sdl_surface_changed()
{
  if (!sdl_inited) {
    GraphicsLoader::instantiatePixmaps(Tilesetlist::getInstance());
    sdl_inited = true;
    sdl_initialized.emit();
  }
}

void TileSetWindow::update_tile_preview_menuitem()
{
  if (get_selected_tile())
    preview_tile_menuitem->set_sensitive(true);
  else
    preview_tile_menuitem->set_sensitive(false);
}

void TileSetWindow::on_tile_selected()
{
  update_tile_panel();
  update_tilestyleset_panel();
  update_tilestyle_panel();
  update_tileset_buttons();
  update_tilestyleset_buttons();
  update_tile_preview_menuitem();
}

void TileSetWindow::on_tilestyleset_selected()
{
  update_tilestyleset_panel();
  update_tilestyle_panel();
  update_tilestyleset_buttons();
}

void TileSetWindow::on_tilestyle_selected()
{
  update_tilestyle_panel();
}

void TileSetWindow::fill_tilestylesets()
{
  Tile *t = get_selected_tile();
  if (!t)
    return;
  tilestylesets_list->clear();
  for (std::list<TileStyleSet*>::iterator it = t->begin(); it != t->end(); it++)
    {
      Gtk::TreeIter l = tilestylesets_list->append();
      (*l)[tilestylesets_columns.name] = (*it)->getName();
      (*l)[tilestylesets_columns.subdir] = (*it)->getSubDir();
      (*l)[tilestylesets_columns.tilestyleset] = *it;
    }
}

void TileSetWindow::fill_tile_info(Tile *tile)
{
  tile_name_entry->set_text(tile->getName());
  tile_type_combobox->set_active(tile->getTypeIndex());
  tile_moves_spinbutton->set_value(tile->getMoves());
  tile_smallmap_pattern_combobox->set_active(tile->getSmallTile()->getPattern());
  tile_smallmap_first_colorbutton->set_sensitive(true);
  fill_tilestylesets();
  fill_colours(tile);
  fill_tile_smallmap(tile);
}

void TileSetWindow::on_add_tile_clicked()
{
  //add a new empty tile to the tileset
  Tile *t = new Tile();
  //add it to the treeview
  Gtk::TreeIter i = tiles_list->append();
  t->setName("Untitled");
  (*i)[tiles_columns.name] = t->getName();
  (*i)[tiles_columns.tile] = t;
  d_tileset->push_back(t);
  tiles_treeview->set_cursor (Gtk::TreePath (String::ucompose("%1", d_tileset->size() - 1)));
  update_tile_preview_menuitem();
}

void TileSetWindow::on_remove_tile_clicked()
{
  //erase the selected row from the treeview
  //remove the tile from the tileset
  Glib::RefPtr<Gtk::TreeSelection> selection = tiles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Tile *a = row[tiles_columns.tile];
      tiles_list->erase(iterrow);

      for (std::vector<Tile*>::iterator it = d_tileset->begin(); it != d_tileset->end(); it++)
	{
	  if (*it == a)
	    {
	      d_tileset->erase(it);
	      break;
	    }
	}
    }
  update_tile_preview_menuitem();
}

void TileSetWindow::on_tile_first_color_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = tiles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Tile *t = row[tiles_columns.tile];
      Gdk::Color c = tile_smallmap_first_colorbutton->get_color();
      SDL_Color sdl;
      memset (&sdl, 0, sizeof (sdl));
      sdl.r = c.get_red() / 255;
      sdl.g = c.get_green() / 255;
      sdl.b = c.get_blue() / 255;
      t->getSmallTile()->setColor(sdl);
      fill_tile_smallmap(t);
    }
}

void TileSetWindow::on_tile_second_color_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = tiles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Tile *t = row[tiles_columns.tile];
      Gdk::Color c = tile_smallmap_second_colorbutton->get_color();
      SDL_Color sdl;
      memset (&sdl, 0, sizeof (sdl));
      sdl.r = c.get_red() / 255;
      sdl.g = c.get_green() / 255;
      sdl.b = c.get_blue() / 255;
      t->getSmallTile()->setSecondColor(sdl);
      fill_tile_smallmap(t);
    }
}

void TileSetWindow::on_tile_third_color_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = tiles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Tile *t = row[tiles_columns.tile];
      Gdk::Color c = tile_smallmap_third_colorbutton->get_color();
      SDL_Color sdl;
      memset (&sdl, 0, sizeof (sdl));
      sdl.r = c.get_red() / 255;
      sdl.g = c.get_green() / 255;
      sdl.b = c.get_blue() / 255;
      t->getSmallTile()->setThirdColor(sdl);
      fill_tile_smallmap(t);
    }
}

void TileSetWindow::fill_colours(Tile *tile)
{
  Gdk::Color c;
  SDL_Color sdl;
  switch (tile->getSmallTile()->getPattern())
    {
    case SmallTile::SOLID:
      tile_smallmap_second_colorbutton->set_sensitive(false);
      tile_smallmap_third_colorbutton->set_sensitive(false);
      sdl = tile->getSmallTile()->getColor();
      c.set_red(sdl.r * 255); c.set_green(sdl.g * 255); c.set_blue(sdl.b * 255);
      tile_smallmap_first_colorbutton->set_color(c);
      break;
    case SmallTile::STIPPLED: case SmallTile::SUNKEN:
      tile_smallmap_second_colorbutton->set_sensitive(true);
      tile_smallmap_third_colorbutton->set_sensitive(false);
      sdl = tile->getSmallTile()->getColor();
      c.set_red(sdl.r * 255); c.set_green(sdl.g * 255); c.set_blue(sdl.b * 255);
      tile_smallmap_first_colorbutton->set_color(c);
      sdl = tile->getSmallTile()->getSecondColor();
      c.set_red(sdl.r * 255); c.set_green(sdl.g * 255); c.set_blue(sdl.b * 255);
      tile_smallmap_second_colorbutton->set_color(c);
      break;
    case SmallTile::RANDOMIZED: case SmallTile::TABLECLOTH: case SmallTile::DIAGONAL: case SmallTile::CROSSHATCH: case SmallTile::SUNKEN_STRIPED:
      tile_smallmap_second_colorbutton->set_sensitive(true);
      tile_smallmap_third_colorbutton->set_sensitive(true);
      sdl = tile->getSmallTile()->getColor();
      c.set_red(sdl.r * 255); c.set_green(sdl.g * 255); c.set_blue(sdl.b * 255);
      tile_smallmap_first_colorbutton->set_color(c);
      sdl = tile->getSmallTile()->getSecondColor();
      c.set_red(sdl.r * 255); c.set_green(sdl.g * 255); c.set_blue(sdl.b * 255);
      tile_smallmap_second_colorbutton->set_color(c);
      sdl = tile->getSmallTile()->getThirdColor();
      c.set_red(sdl.r * 255); c.set_green(sdl.g * 255); c.set_blue(sdl.b * 255);
      tile_smallmap_third_colorbutton->set_color(c);
      break;
    }
}

void TileSetWindow::on_tile_pattern_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = tiles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Tile *t = row[tiles_columns.tile];
      int idx = tile_smallmap_pattern_combobox->get_active_row_number();
      SmallTile::Pattern pattern = SmallTile::Pattern(idx);
      t->getSmallTile()->setPattern(pattern);
      fill_colours(t);
      fill_tile_smallmap(t);
    }
}

void TileSetWindow::on_tile_type_changed()
{
  int idx = tile_type_combobox->get_active_row_number();
  Glib::RefPtr<Gtk::TreeSelection> selection = tiles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Tile *t = row[tiles_columns.tile];
      t->setTypeByIndex(idx);
    }
}

void TileSetWindow::on_tile_name_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = tiles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      row[tiles_columns.name] = tile_name_entry->get_text();
      Tile *t = row[tiles_columns.tile];
      t->setName(tile_name_entry->get_text());
    }
}

void TileSetWindow::fill_tile_smallmap(Tile *tile)
{
  for (int i = 0; i < 32; i++)
    {
      for (int j = 0; j < 32; j++)
	{
	  bool shadowed = false;
	  if (i == 32 - 1)
	    shadowed = true;
	  else if (j == 32 - 1)
	    shadowed = true;
	  OverviewMap::draw_tile_pixel (tile_smallmap_surface, 
					tile->getSmallTile()->getPattern(),
					tile->getSmallTile()->getColor(),
					tile->getSmallTile()->getSecondColor(),
					tile->getSmallTile()->getThirdColor(),
					i, j, shadowed);
	}
    }

  tile_smallmap_image->property_pixbuf() = to_pixbuf(tile_smallmap_surface);
}

void TileSetWindow::on_add_tilestyleset_clicked()
{

  Gtk::FileChooserDialog chooser(*window.get(), _("Choose a Name"),
				 Gtk::FILE_CHOOSER_ACTION_OPEN);
  Gtk::FileFilter sav_filter;
  sav_filter.add_pattern("*.png");
  chooser.set_filter(sav_filter);
  chooser.set_current_folder(Configuration::s_dataPath + "/tilesets/");

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);

  chooser.show_all();
  int res = chooser.run();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      std::string filename = chooser.get_filename();
      chooser.hide();
      //add a new empty tile to the tileset
      TileStyleSet *t = new TileStyleSet();
      //add it to the treeview
      Gtk::TreeIter i = tilestylesets_list->append();
      t->setName("");
      (*i)[tilestylesets_columns.name] = t->getName();
      (*i)[tilestylesets_columns.tilestyleset] = t;

      Tile *tile = get_selected_tile ();
      tile->push_back(t);
      tilestylesets_treeview->set_cursor (Gtk::TreePath (String::ucompose("%1", tile->size() - 1)));
      image_filechooser_button->set_filename(filename);
    }
}

TileStyle * TileSetWindow::get_selected_tilestyle ()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = tilestyles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      return row[tilestyles_columns.tilestyle];
    }
  return NULL;
}

TileStyleSet * TileSetWindow::get_selected_tilestyleset ()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = tilestylesets_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      return row[tilestylesets_columns.tilestyleset];
    }
  return NULL;
}

Tile * TileSetWindow::get_selected_tile ()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = tiles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      return row[tiles_columns.tile];
    }
  return NULL;
}

void TileSetWindow::on_remove_tilestyleset_clicked()
{
  //erase the selected row from the treeview
  //remove the tile from the tileset
  Glib::RefPtr<Gtk::TreeSelection> selection = tilestylesets_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Tile *t = get_selected_tile();
      Gtk::TreeModel::Row row = *iterrow;
      TileStyleSet *s = row[tilestylesets_columns.tilestyleset];
      tilestylesets_list->erase(iterrow);

      for (std::list<TileStyleSet*>::iterator it = t->begin(); it != t->end(); it++)
	{
	  if (*it == s)
	    {
	      t->erase(it);
	      break;
	    }
	}
    }
}

void TileSetWindow::on_tilestyle_changed()
{
  TileStyle *t = get_selected_tilestyle ();
  if (t)
    {
      t->setType(TileStyle::Type(tilestyle_combobox->get_active_row_number()));
      int idx = t->getType();
      tilestyle_standard_image->property_pixbuf() = 
	tilestyle_standard_images[idx];
    }
}

void TileSetWindow::on_image_chosen()
{
  std::string selected_filename = image_filechooser_button->get_filename();
  if (selected_filename.empty())
    return;
  if (inhibit_image_change == true)
    {
      inhibit_image_change = false;
      return;
    }

  unsigned int height = get_image_height (selected_filename);
  if (height != d_tileset->getTileSize() && d_tileset->getTileSize() > 0)
    return;

  unsigned int width = get_image_width (selected_filename);
  if ((width % height) != 0)
    return;

  d_tileset->setTileSize(height);

  on_remove_tilestyleset_clicked();

  //now make a new one
  TileStyleSet *set = new TileStyleSet();
  Gtk::TreeIter i = tilestylesets_list->append();
  char *dir = g_strdup (selected_filename.c_str());
  char *tmp = strrchr (dir, '/');
  if (!tmp)
    return;
  tmp[0] = '\0';

  //hackus horribilium
  std::string back = "../../../../../../../../../../../../../../../../";

  set->setSubDir(back + dir + "/");
  (*i)[tilestylesets_columns.subdir] = set->getSubDir();

  tmp++;
  char *name = g_strdup (tmp);
  tmp = strrchr (name, '.');
  if (tmp)
    tmp[0] = '\0';

  set->setName(name);
  free (name);
  free (dir);
  (*i)[tilestylesets_columns.name] = set->getName();
  (*i)[tilestylesets_columns.tilestyleset] = set;
  Tile *tile = get_selected_tile ();
  tile->push_back(set);

  int no = width / height;
  for (int j = 0; j < no; j++)
    {
      int id = d_tileset->getFreeTileStyleId();
      if (id >= 0)
	{
	  TileStyle *style = new TileStyle();
	  style->setId(id);
	  style->setType(TileStyle::LONE);
	  set->push_back(style);
	}
    }
}

void TileSetWindow::on_refresh_clicked()
{
  TileStyleSet *set = get_selected_tilestyleset ();
  GraphicsLoader::instantiatePixmaps(set, d_tileset->getTileSize());
}

void TileSetWindow::on_preview_tile_activated()
{
  Tile *tile = get_selected_tile();
  if (tile)
    {
      TilePreviewDialog d(tile, d_tileset->getTileSize());
      d.run();
    }
}
      
void TileSetWindow::on_army_unit_selector_activated()
{
  TilesetSelectorEditorDialog d(d_tileset);
  d.run();
}

void TileSetWindow::on_explosion_picture_activated()
{
  TilesetExplosionPictureEditorDialog d(d_tileset);
  d.run();
}
