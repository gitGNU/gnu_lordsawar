//  Copyright (C) 2008, 2009, 2010 Ben Asselstine
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

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <assert.h>
#include <libgen.h>
#include <string.h>

#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>
#include "ucompose.hpp"

#include <gtkmm.h>
#include "tileset-window.h"
#include "tileset-info-dialog.h"
#include "tile-preview-dialog.h"
#include "tileset-selector-editor-dialog.h"
#include "tileset-flag-editor-dialog.h"
#include "tileset-explosion-picture-editor-dialog.h"
#include "image-editor-dialog.h"

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
#include "recently-edited-file.h"
#include "recently-edited-file-list.h"
#include "tile-size-editor-dialog.h"
#include "editor-quit-dialog.h"
#include "editor-recover-dialog.h"
#include "tilestyle-organizer-dialog.h"
#include "tileset-smallmap-building-colors-dialog.h"

#include "ucompose.hpp"

#include "glade-helpers.h"


TileSetWindow::TileSetWindow(std::string load_filename)
{
  autosave = File::getSavePath() + "autosave" + Tileset::file_extension;
  needs_saving = false;
  inhibit_needs_saving = false;
  d_tileset = NULL;
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path() + "/tileset-window.ui");

    xml->get_widget("window", window);
    window->set_icon_from_file(File::getMiscFile("various/tileset_icon.png"));
    window->signal_delete_event().connect
      (sigc::mem_fun(*this, &TileSetWindow::on_window_closed));

    xml->get_widget("tiles_treeview", tiles_treeview);
    xml->get_widget("tile_name_entry", tile_name_entry);
    tile_name_entry->signal_changed().connect
      (sigc::mem_fun(this, &TileSetWindow::on_tile_name_changed));

    Gtk::HBox *type_combo_container;
    xml->get_widget("type_combo_container", type_combo_container);
    tile_type_combobox = new Gtk::ComboBoxText();
    tile_type_combobox->append_text(_("Grass"));
    tile_type_combobox->append_text(_("Water"));
    tile_type_combobox->append_text(_("Forest"));
    tile_type_combobox->append_text(_("Hills"));
    tile_type_combobox->append_text(_("Mountains"));
    tile_type_combobox->append_text(_("Swamp"));
    tile_type_combobox->append_text(_("Void"));
    type_combo_container->add(*manage(tile_type_combobox));
    type_combo_container->show_all();
    tile_type_combobox->signal_changed().connect
      (sigc::mem_fun(this, &TileSetWindow::on_tile_type_changed));

    Gtk::HBox *tilestyle_combo_container;
    xml->get_widget("tilestyle_combo_container", tilestyle_combo_container);
    tilestyle_combobox = new Gtk::ComboBoxText();
    tilestyle_combobox->append_text(_("Lone"));
    tilestyle_combobox->append_text(_("Outer Top-Left"));
    tilestyle_combobox->append_text(_("Outer Top-Center"));
    tilestyle_combobox->append_text(_("Outer Top-Right"));
    tilestyle_combobox->append_text(_("Outer Bottom-Left"));
    tilestyle_combobox->append_text(_("Outer Bottom-Center"));
    tilestyle_combobox->append_text(_("Outer Bottom-Right"));
    tilestyle_combobox->append_text(_("Outer Middle-Left"));
    tilestyle_combobox->append_text(_("Inner Middle-Center"));
    tilestyle_combobox->append_text(_("Outer Middle-Right"));
    tilestyle_combobox->append_text(_("Inner Top-Left"));
    tilestyle_combobox->append_text(_("Inner Top-Right"));
    tilestyle_combobox->append_text(_("Inner Bottom-Left"));
    tilestyle_combobox->append_text(_("Inner Bottom-Right"));
    tilestyle_combobox->append_text(_("Top-Left To Bottom-Right"));
    tilestyle_combobox->append_text(_("Bottom-Left To Top-Right"));
    tilestyle_combobox->append_text(_("Other"));
    tilestyle_combobox->append_text(_("Unknown"));
    tilestyle_combo_container->add(*manage(tilestyle_combobox));
    tilestyle_combo_container->show_all();
    tilestyle_combobox->signal_changed().connect
      (sigc::mem_fun(this, &TileSetWindow::on_tilestyle_changed));

    Gtk::HBox *pattern_container;
    xml->get_widget("pattern_container", pattern_container);
    tile_smallmap_pattern_combobox = new Gtk::ComboBoxText();
    tile_smallmap_pattern_combobox->append_text(_("Solid"));
    tile_smallmap_pattern_combobox->append_text(_("Stippled"));
    tile_smallmap_pattern_combobox->append_text(_("Randomized"));
    tile_smallmap_pattern_combobox->append_text(_("Sunken"));
    tile_smallmap_pattern_combobox->append_text(_("Tablecloth"));
    tile_smallmap_pattern_combobox->append_text(_("Diagonal"));
    tile_smallmap_pattern_combobox->append_text(_("Crosshatched"));
    tile_smallmap_pattern_combobox->append_text(_("Sunken Striped"));
    pattern_container->add(*manage(tile_smallmap_pattern_combobox));
    pattern_container->show_all();
    tile_smallmap_pattern_combobox->signal_changed().connect
      (sigc::mem_fun(this, &TileSetWindow::on_tile_pattern_changed));

    xml->get_widget("tile_moves_spinbutton", tile_moves_spinbutton);
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
    xml->get_widget("save_as_menuitem", save_as_menuitem);
    save_as_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &TileSetWindow::on_save_as_activated));
    xml->get_widget("quit_menuitem", quit_menuitem);
    quit_menuitem->signal_activate().connect
       (sigc::mem_fun(this, &TileSetWindow::on_quit_activated));
    xml->get_widget("edit_tileset_info_menuitem", edit_tileset_info_menuitem);
    edit_tileset_info_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &TileSetWindow::on_edit_tileset_info_activated));
    xml->get_widget("army_unit_selector_menuitem", army_unit_selector_menuitem);
    army_unit_selector_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &TileSetWindow::on_army_unit_selector_activated));
    xml->get_widget("roads_picture_menuitem", roads_picture_menuitem);
    roads_picture_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &TileSetWindow::on_roads_picture_activated));
    xml->get_widget("bridges_picture_menuitem", bridges_picture_menuitem);
    bridges_picture_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &TileSetWindow::on_bridges_picture_activated));
    xml->get_widget("fog_picture_menuitem", fog_picture_menuitem);
    fog_picture_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &TileSetWindow::on_fog_picture_activated));
    xml->get_widget("flags_picture_menuitem", flags_picture_menuitem);
    flags_picture_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &TileSetWindow::on_flags_picture_activated));

    xml->get_widget("explosion_picture_menuitem", explosion_picture_menuitem);
    explosion_picture_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &TileSetWindow::on_explosion_picture_activated));
    xml->get_widget("preview_tile_menuitem", preview_tile_menuitem);
    preview_tile_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &TileSetWindow::on_preview_tile_activated));
    xml->get_widget("organize_tilestyles_menuitem", organize_tilestyles_menuitem);
    organize_tilestyles_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &TileSetWindow::on_organize_tilestyles_activated));
    xml->get_widget("smallmap_building_colors_menuitem", 
                    smallmap_building_colors_menuitem);
    smallmap_building_colors_menuitem->signal_activate().connect
      (sigc::mem_fun(this, 
                     &TileSetWindow::on_smallmap_building_colors_activated));
    xml->get_widget ("help_about_menuitem", help_about_menuitem);
    help_about_menuitem->signal_activate().connect
       (sigc::mem_fun(this, &TileSetWindow::on_help_about_activated));
    xml->get_widget("tilestyle_image", tilestyle_image);

    window->signal_delete_event().connect
      (sigc::mem_fun(*this, &TileSetWindow::on_delete_event));

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
    xml->get_widget("image_button", image_button);
    image_button->signal_clicked().connect
      (sigc::mem_fun(*this, &TileSetWindow::on_image_chosen));

    xml->get_widget("tilestyle_standard_image", tilestyle_standard_image);

    if (load_filename != "")
      current_save_filename = load_filename;
    update_tile_panel();
    update_tilestyleset_panel();
    update_tilestyle_panel();
    update_tileset_buttons();
    update_tilestyleset_buttons();

    update_tileset_buttons();
    update_tilestyleset_buttons();
    update_tileset_menuitems();
    update_tile_preview_menuitem();

    tile_smallmap_surface = Gdk::Pixmap::create(Glib::RefPtr<Gdk::Drawable>(0), 32, 32, 24);
    tile_smallmap_surface_gc = Gdk::GC::create(tile_smallmap_surface);

    if (File::exists(autosave))
      {
        std::string m;
        std::list<RecentlyEditedFile*> files = RecentlyEditedFileList::getInstance()->getFilesWithExtension(Tileset::file_extension);
        if (files.size() == 0)
          m = _("Do you want to recover the session?");
        else
          {
            RecentlyEditedTilesetFile *r = dynamic_cast<RecentlyEditedTilesetFile*>(files.front());
            if (r->getName() == "")
              m = String::ucompose(_("Do you want to recover %1 (%2 tiles)?"),
                                   File::get_basename(r->getFileName(), true),
                                   r->getNumberOfTiles());
            else
              m = String::ucompose
                (_("Do you want to recover %1 (%2, %3 tiles)?"),
                                   File::get_basename(r->getFileName(), true),
                                   r->getName(), r->getNumberOfTiles());
          }
        EditorRecoverDialog d(m);
        int response = d.run();
        d.hide();
        //ask if we want to recover the autosave.
        if (response == Gtk::RESPONSE_ACCEPT)
          {
            load_tileset (autosave);
            return;
          }
      }

    if (load_filename.empty() == false)
      load_tileset(load_filename);
}

void
TileSetWindow::update_tileset_menuitems()
{
  if (d_tileset == NULL)
    {
      save_tileset_menuitem->set_sensitive(false);
      save_as_menuitem->set_sensitive(false);
      army_unit_selector_menuitem->set_sensitive(false);
      edit_tileset_info_menuitem->set_sensitive(false);
      explosion_picture_menuitem->set_sensitive(false);
      roads_picture_menuitem->set_sensitive(false);
      bridges_picture_menuitem->set_sensitive(false);
      fog_picture_menuitem->set_sensitive(false);
      flags_picture_menuitem->set_sensitive(false);
      organize_tilestyles_menuitem->set_sensitive(false);
    }
  else
    {
      save_tileset_menuitem->set_sensitive(true);
      save_as_menuitem->set_sensitive(true);
      army_unit_selector_menuitem->set_sensitive(true);
      edit_tileset_info_menuitem->set_sensitive(true);
      explosion_picture_menuitem->set_sensitive(true);
      roads_picture_menuitem->set_sensitive(true);
      bridges_picture_menuitem->set_sensitive(true);
      fog_picture_menuitem->set_sensitive(true);
      flags_picture_menuitem->set_sensitive(true);
      organize_tilestyles_menuitem->set_sensitive(true);
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
      int idx = t->getType();
      tilestyle_combobox->set_active(idx);
      Glib::RefPtr<Gdk::Pixbuf> pixbuf= t->getImage()->to_pixbuf();
      tilestyle_image->clear();
      tilestyle_image->property_pixbuf() = pixbuf;
      tilestyle_image->show_all();
      tilestyle_standard_image->property_pixbuf() = 
        GraphicsCache::getInstance()->getDefaultTileStylePic(idx, d_tileset->getTileSize())->to_pixbuf();
    }
}

void TileSetWindow::fill_tilestyleset_info(TileStyleSet *t)
{
  if (!t || t->getName() == "")
    {
      tilestyles_list->clear();
      image_button->set_label(_("no image set"));
      update_tilestyle_panel();
      return;
    }

  std::string n = d_tileset->getFileFromConfigurationFile(t->getName() + ".png");
  TileStyleSet *set = get_selected_tilestyleset ();
  set->instantiateImages(d_tileset->getTileSize(), n);
  File::erase(n);
  if (t->getName().empty() == false)
    image_button->set_label(File::get_basename(t->getName() + ".png", true));
  else
    image_button->set_label(_("no image set"));
  //add the tilestyles to the tilestyles_treeview
  tilestyles_list->clear();
  for (unsigned int i = 0; i < t->size(); i++)
    {
      Gtk::TreeIter l = tilestyles_list->append();
      (*l)[tilestyles_columns.name] = 
        "0x" + TileStyle::idToString((*t)[i]->getId());
      (*l)[tilestyles_columns.tilestyle] = (*t)[i];
    }
  tilestyles_treeview->set_cursor (Gtk::TreePath ("0"));
}

void
TileSetWindow::update_tilestyleset_panel()
{
  tilestyleset_frame->set_sensitive(true);
  TileStyleSet *t = get_selected_tilestyleset ();
  if (t)
    fill_tilestyleset_info(t);
  else
    {
      fill_tilestyleset_info(NULL);
      tilestyleset_frame->set_sensitive(false);
    }
}

void
TileSetWindow::update_tile_panel()
{
  Gdk::Color black("black");
  //if nothing selected in the treeview, then we don't show anything in
  //the tile panel
  Tile *t = get_selected_tile ();
      
  if (t == NULL)
    {
      //clear all values
      inhibit_needs_saving = true;
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
      inhibit_needs_saving = false;
      return;
    }

  tile_vbox->set_sensitive(true);
  fill_tile_info(t);
  fill_tilestylesets();
}

TileSetWindow::~TileSetWindow()
{
  tile_smallmap_surface.reset();
  delete window;
}

void TileSetWindow::show()
{
  window->show();
}

void TileSetWindow::hide()
{
  window->hide();
}

bool TileSetWindow::on_delete_event(GdkEventAny *e)
{
  hide();

  return true;
}

void TileSetWindow::on_new_tileset_activated()
{
  std::string name = "";
  int id = Tilesetlist::getNextAvailableId(0);
  Tileset *tileset = new Tileset(id, name);
  TileSetInfoDialog d(tileset, File::getUserTilesetDir(), "", false,
                      _("Make a New Tileset"));
  d.set_parent_window(*window);
  int response = d.run();
  if (response != Gtk::RESPONSE_ACCEPT)
    {
      delete tileset;
      return;
    }
  if (d_tileset)
    delete d_tileset;
  d_tileset = tileset;
  std::string dir = File::getUserTilesetDir();
  d_tileset->setDirectory(dir);
  current_save_filename = d_tileset->getConfigurationFile();
  RecentlyEditedFileList *refl = RecentlyEditedFileList::getInstance();
  refl->updateEntry(current_save_filename);
  refl->save();
  d_tileset->setDirectory(File::get_dirname(autosave));
  d_tileset->setBaseName(File::get_basename(autosave));
  d_tileset->save(autosave, Tileset::file_extension);

  //copy images??
  update_tileset_buttons();
  update_tilestyleset_buttons();
  update_tileset_menuitems();
  update_tile_preview_menuitem();

  needs_saving = true;
  update_window_title();
}

void TileSetWindow::on_load_tileset_activated()
{
  Gtk::FileChooserDialog chooser(*window, 
				 _("Choose a Tileset to Load"));
  Gtk::FileFilter sav_filter;
  sav_filter.add_pattern("*" + TILESET_EXT);
  chooser.set_filter(sav_filter);
  chooser.set_current_folder(File::getUserTilesetDir());

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);

  chooser.show_all();
  int res = chooser.run();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      load_tileset(chooser.get_filename());
      chooser.hide();
      needs_saving = false;
      update_window_title();
    }
  update_tile_panel();
}

void TileSetWindow::on_save_as_activated()
{
  std::string orig_basename = d_tileset->getBaseName();
  guint32 orig_id = d_tileset->getId();
  d_tileset->setId(Tilesetlist::getNextAvailableId(orig_id));
  TileSetInfoDialog d(d_tileset, File::getUserTilesetDir(), "", false,
                        _("Save a Copy of a Tileset"));
  d.set_parent_window(*window);
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      std::string new_basename = d_tileset->getBaseName();
      guint32 new_id = d_tileset->getId();
      d_tileset->setId(orig_id);
      d_tileset->setBaseName(orig_basename);
      bool success = Tilesetlist::getInstance()->addToPersonalCollection(d_tileset, new_basename, new_id);
      if (success)
        {
          save_tileset_menuitem->set_sensitive(true);
          current_save_filename = d_tileset->getConfigurationFile();
          RecentlyEditedFileList *refl = RecentlyEditedFileList::getInstance();
          refl->updateEntry(current_save_filename);
          refl->save();
          load_tileset(current_save_filename);
          needs_saving = false;
          update_window_title();
        }
      else
        {
          std::string msg;
          msg = _("Error!  Tileset could not be saved.");
          Gtk::MessageDialog dialog(*window, msg);
          dialog.run();
          dialog.hide();
        }
    }
  else
    d_tileset->setId(orig_id);
}

void TileSetWindow::on_save_tileset_activated()
{
  if (current_save_filename.empty())
    current_save_filename = d_tileset->getConfigurationFile();

  guint32 suggested_tile_size = d_tileset->calculate_preferred_tile_size();
  if (suggested_tile_size != d_tileset->getTileSize())
    {
      TileSizeEditorDialog d(d_tileset->getTileSize(), suggested_tile_size);
      int response = d.run();
      if (response == Gtk::RESPONSE_ACCEPT)
        d_tileset->setTileSize(d.get_selected_tilesize());
    }
  //Reorder the tileset according to the treeview
  d_tileset->clear();
  for (Gtk::TreeIter i = tiles_list->children().begin(),
       end = tiles_list->children().end(); i != end; ++i) 
    d_tileset->push_back((*i)[tiles_columns.tile]);
  bool success = d_tileset->save(autosave, Tileset::file_extension);
  if (success)
    {
      RecentlyEditedFileList::getInstance()->updateEntry(current_save_filename);
      RecentlyEditedFileList::getInstance()->save();
      success = Tileset::copy (autosave, current_save_filename);
      if (success == true)
        {
          needs_saving = false;
          update_window_title();
          tileset_saved.emit(d_tileset->getId());
        }
    }
  if (!success)
    {
      std::string msg;
      msg = _("Error!  Tileset could not be saved.");
      Gtk::MessageDialog dialog(*window, msg);
      dialog.run();
      dialog.hide();
    }
}

bool TileSetWindow::quit()
{
  if (needs_saving)
    {
      EditorQuitDialog d;
      int response = d.run();
      d.set_parent_window(*window);
      d.hide();
      
      if (response == Gtk::RESPONSE_CANCEL) //we don't want to quit
	return false;

      else if (response == Gtk::RESPONSE_ACCEPT) // save and quit
	on_save_tileset_activated();
      //else if (Response == Gtk::CLOSE) // don't save just quit
      window->hide();
    }
  else
    window->hide();
  File::erase(autosave);
  if (d_tileset)
    delete d_tileset;
  return true;
}

bool TileSetWindow::on_window_closed(GdkEventAny *event)
{
  return !quit();
}

void TileSetWindow::on_quit_activated()
{
  quit();
}

void TileSetWindow::on_edit_tileset_info_activated()
{
  TileSetInfoDialog d(d_tileset, File::get_dirname(current_save_filename), 
                      File::get_basename(current_save_filename), true);
  d.set_parent_window(*window);
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      needs_saving = true;
      update_window_title();
    }
}

void TileSetWindow::on_help_about_activated()
{
  Gtk::AboutDialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/../about-dialog.ui");

  xml->get_widget("dialog", dialog);
  dialog->set_transient_for(*window);
  dialog->set_icon_from_file(File::getMiscFile("various/tileset_icon.png"));

  dialog->set_version(PACKAGE_VERSION);
  dialog->set_logo(GraphicsCache::getMiscPicture("tileset_icon.png")->to_pixbuf());
  dialog->show_all();
  dialog->run();
  dialog->hide();
  delete dialog;

  return;
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
      (*l)[tilestylesets_columns.tilestyleset] = *it;
    }
  tilestylesets_treeview->set_cursor (Gtk::TreePath ("0"));
}

void TileSetWindow::fill_tile_info(Tile *tile)
{
  inhibit_needs_saving = true;
  tile_name_entry->set_text(tile->getName());
  tile_type_combobox->set_active(tile->getTypeIndex());
  tile_moves_spinbutton->set_value(tile->getMoves());
  tile_smallmap_pattern_combobox->set_active(tile->getSmallTile()->getPattern());
  tile_smallmap_first_colorbutton->set_sensitive(true);
  fill_tilestylesets();
  fill_colours(tile);
  fill_tile_smallmap(tile);
  inhibit_needs_saving = false;
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
  needs_saving = true;
  update_window_title();
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
  
      needs_saving = true;
      update_window_title();
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
      t->getSmallTile()->setColor(tile_smallmap_first_colorbutton->get_color());
      fill_tile_smallmap(t);
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
      t->getSmallTile()->setSecondColor(tile_smallmap_second_colorbutton->get_color());
      fill_tile_smallmap(t);
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
      t->getSmallTile()->setThirdColor(tile_smallmap_third_colorbutton->get_color());
      fill_tile_smallmap(t);
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void TileSetWindow::fill_colours(Tile *tile)
{
  Gdk::Color c;
  switch (tile->getSmallTile()->getPattern())
    {
    case SmallTile::SOLID:
      tile_smallmap_second_colorbutton->set_sensitive(false);
      tile_smallmap_third_colorbutton->set_sensitive(false);
      tile_smallmap_first_colorbutton->set_color(tile->getSmallTile()->getColor());
      break;
    case SmallTile::STIPPLED: case SmallTile::SUNKEN:
      tile_smallmap_second_colorbutton->set_sensitive(true);
      tile_smallmap_third_colorbutton->set_sensitive(false);
      tile_smallmap_first_colorbutton->set_color(tile->getSmallTile()->getColor());
      tile_smallmap_second_colorbutton->set_color(tile->getSmallTile()->getSecondColor());
      break;
    case SmallTile::RANDOMIZED: case SmallTile::TABLECLOTH: case SmallTile::DIAGONAL: case SmallTile::CROSSHATCH: case SmallTile::SUNKEN_STRIPED:
      tile_smallmap_second_colorbutton->set_sensitive(true);
      tile_smallmap_third_colorbutton->set_sensitive(true);
      tile_smallmap_first_colorbutton->set_color(tile->getSmallTile()->getColor());
      tile_smallmap_second_colorbutton->set_color(tile->getSmallTile()->getSecondColor());
      tile_smallmap_third_colorbutton->set_color(tile->getSmallTile()->getThirdColor());
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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

      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
	  OverviewMap::draw_terrain_tile (tile_smallmap_surface,
					  tile_smallmap_surface_gc,
					  tile->getSmallTile()->getPattern(),
					  tile->getSmallTile()->getColor(),
					  tile->getSmallTile()->getSecondColor(),
					  tile->getSmallTile()->getThirdColor(),
					  i, j, shadowed);
	}
    }

  tile_smallmap_image->property_pixmap() = tile_smallmap_surface;
}

void TileSetWindow::choose_and_add_or_replace_tilestyleset(std::string replace_filename)
{
  Gtk::FileChooserDialog chooser(*window, _("Choose an Image"),
				 Gtk::FILE_CHOOSER_ACTION_OPEN);
  Gtk::FileFilter sav_filter;
  sav_filter.add_pattern("*.png");
  chooser.set_filter(sav_filter);
  chooser.set_current_folder(Glib::get_home_dir());

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);

  chooser.show_all();
  int res = chooser.run();

  if (res != Gtk::RESPONSE_ACCEPT)
    return;

  std::string selected_filename = chooser.get_filename();
  if (selected_filename.empty())
    return;

  if (TileStyleSet::validate_image(selected_filename) == false)
    return;

  if (replace_filename.empty() == false)
    on_remove_tilestyleset_clicked();
  
  Tile *tile = get_selected_tile();
  bool success = d_tileset->addTileStyleSet(tile, selected_filename);
  if (!success)
    return;

  if (replace_filename.empty() == true)
    d_tileset->replaceFileInConfigurationFile(replace_filename, 
                                              selected_filename);
  else
    d_tileset->addFileInConfigurationFile(selected_filename);

  //now make a new one
  TileStyleSet *set = tile->back();
  Gtk::TreeIter i = tilestylesets_list->append();
  (*i)[tilestylesets_columns.name] = set->getName();
  (*i)[tilestylesets_columns.tilestyleset] = set;

  tilestylesets_treeview->set_cursor (Gtk::TreePath (String::ucompose("%1", tile->size() - 1)));

  needs_saving = true;
  update_window_title();
}

void TileSetWindow::on_add_tilestyleset_clicked()
{
  choose_and_add_or_replace_tilestyleset("");
  return;
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
      needs_saving = true;
      fill_tilestyleset_info(NULL);
      update_window_title();
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
        GraphicsCache::getInstance()->getDefaultTileStylePic(idx, d_tileset->getTileSize())->to_pixbuf();
    }
}

void TileSetWindow::on_image_chosen()
{
  TileStyleSet *set = get_selected_tilestyleset();

  choose_and_add_or_replace_tilestyleset(set->getName() + ".png");

}

void TileSetWindow::on_organize_tilestyles_activated()
{
  TileStyleOrganizerDialog d(get_selected_tile());
  d.tilestyle_selected.connect
    (sigc::mem_fun(this, &TileSetWindow::on_tilestyle_id_selected));
  d.set_parent_window(*window);
  d.run();
}

void TileSetWindow::on_smallmap_building_colors_activated()
{
  TilesetSmallmapBuildingColorsDialog d(d_tileset);
  d.set_parent_window(*window);
  d.run();
}
    
void TileSetWindow::on_tilestyle_id_selected(guint32 id)
{
  Tile *t = NULL;
  TileStyleSet *s = NULL;
  TileStyle *style = NULL;
  bool found = d_tileset->getTileStyle(id, &t, &s, &style);
  if (found)
    {
      select_tile(t);
      select_tilestyleset(s);
      select_tilestyle(style);
    }
}

void TileSetWindow::select_tile(Tile *tile)
{
  for (guint32 i = 0; i < tiles_list->children().size(); i++)
    {
      Gtk::TreeIter iter = tiles_list->children()[i];
      Gtk::TreeModel::Row row = *iter;
      Tile *t = row[tiles_columns.tile];
      if (tile == t)
        {
          Glib::RefPtr<Gtk::TreeSelection> s = tiles_treeview->get_selection();
          s->select(row);
          return;
        }
    }
}

void TileSetWindow::select_tilestyleset(TileStyleSet *set)
{
  for (guint32 i = 0; i < tilestylesets_list->children().size(); i++)
    {
      Gtk::TreeIter iter = tilestylesets_list->children()[i];
      Gtk::TreeModel::Row row = *iter;
      TileStyleSet *t = row[tilestylesets_columns.tilestyleset];
      if (set == t)
        {
          Glib::RefPtr<Gtk::TreeSelection> s = 
            tilestylesets_treeview->get_selection();
          s->select(row);
          return;
        }
    }
}

void TileSetWindow::select_tilestyle(TileStyle *style)
{
  for (guint32 i = 0; i < tilestyles_list->children().size(); i++)
    {
      Gtk::TreeIter iter = tilestyles_list->children()[i];
      Gtk::TreeModel::Row row = *iter;
      TileStyle *t = row[tilestyles_columns.tilestyle];
      if (style == t)
        {
          Glib::RefPtr<Gtk::TreeSelection> s = 
            tilestyles_treeview->get_selection();
          s->select(row);
          return;
        }
    }
}

void TileSetWindow::on_preview_tile_activated()
{
  Tile *tile = get_selected_tile();
  if (tile)
    {
      //determine transition tile type
      Tile *sec = NULL;
      int idx = -1;
      if (tile->getType() == Tile::MOUNTAIN)
        idx = d_tileset->getIndex(Tile::HILLS);
      else
        idx = d_tileset->getIndex(Tile::GRASS);
      if (idx > -1)
        sec = (*d_tileset)[idx];
      TilePreviewDialog d(tile, sec, d_tileset->getTileSize());
      d.tilestyle_selected.connect
        (sigc::mem_fun(this, &TileSetWindow::on_tilestyle_id_selected));
      d.set_icon_from_file(File::getMiscFile("various/tileset_icon.png"));
      d.run();
    }
}
      
void TileSetWindow::on_roads_picture_activated()
{
  std::string filename = "";
  if (d_tileset->getRoadsFilename().empty() == false)
    filename = d_tileset->getFileFromConfigurationFile(d_tileset->getRoadsFilename() +".png");
  ImageEditorDialog d(filename, ROAD_TYPES);
  d.set_icon_from_file(File::getMiscFile("various/tileset_icon.png"));
  d.set_parent_window(*window);
  int response = d.run();
  if (filename.empty() == false)
    File::erase(filename);
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_selected_filename() != filename)
        {
          d_tileset->replaceFileInConfigurationFile(d_tileset->getRoadsFilename()+".png", d.get_selected_filename());

          d_tileset->setRoadsFilename
            (File::get_basename(d.get_selected_filename()));
          needs_saving = true;
          update_window_title();
        }
    }
}

void TileSetWindow::on_bridges_picture_activated()
{
  std::string filename = "";
  if (d_tileset->getBridgesFilename().empty() == false)
    filename = d_tileset->getFileFromConfigurationFile(d_tileset->getBridgesFilename() +".png");
  ImageEditorDialog d(filename, BRIDGE_TYPES);
  d.set_icon_from_file(File::getMiscFile("various/tileset_icon.png"));
  d.set_parent_window(*window);
  int response = d.run();
  if (filename.empty() == false)
    File::erase(filename);
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_selected_filename() != filename)
        {
          d_tileset->replaceFileInConfigurationFile(d_tileset->getBridgesFilename()+".png", d.get_selected_filename());

          d_tileset->setBridgesFilename
            (File::get_basename(d.get_selected_filename()));
          needs_saving = true;
          update_window_title();
        }
    }
}
void TileSetWindow::on_fog_picture_activated()
{
  std::string filename = "";
  if (d_tileset->getFogFilename().empty() == false)
    filename = d_tileset->getFileFromConfigurationFile(d_tileset->getFogFilename() +".png");
  ImageEditorDialog d(filename, FOG_TYPES);
  d.set_icon_from_file(File::getMiscFile("various/tileset_icon.png"));
  d.set_parent_window(*window);
  int response = d.run();
  if (filename.empty() == false)
    File::erase(filename);
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_selected_filename() != filename)
        {
          d_tileset->replaceFileInConfigurationFile(d_tileset->getFogFilename()+".png", d.get_selected_filename());

          d_tileset->setFogFilename
            (File::get_basename(d.get_selected_filename()));
          needs_saving = true;
          update_window_title();
        }
    }
}

void TileSetWindow::on_flags_picture_activated()
{
  TilesetFlagEditorDialog d(d_tileset);
  d.set_icon_from_file(File::getMiscFile("various/tileset_icon.png"));
  d.set_parent_window(*window);
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      needs_saving = true;
      update_window_title();
    }
}

void TileSetWindow::on_army_unit_selector_activated()
{
  TilesetSelectorEditorDialog d(d_tileset);
  d.set_icon_from_file(File::getMiscFile("various/tileset_icon.png"));
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      needs_saving = true;
      update_window_title();
    }
}

void TileSetWindow::on_explosion_picture_activated()
{
  TilesetExplosionPictureEditorDialog d(d_tileset);
  d.set_icon_from_file(File::getMiscFile("various/tileset_icon.png"));
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      needs_saving = true;
      update_window_title();
    }
}

bool TileSetWindow::load_tileset(std::string filename)
{
  std::string old_current_save_filename = current_save_filename;
  current_save_filename = filename;
  if (filename != autosave)
    Tileset::copy(current_save_filename, autosave);
  else
    {
      //go get the real name of the file
      std::list<RecentlyEditedFile*> files = RecentlyEditedFileList::getInstance()->getFilesWithExtension(Tileset::file_extension);
      if (files.size() > 0)
        current_save_filename = files.front()->getFileName();
    }

  Tileset *tileset = Tileset::create(autosave);
  if (tileset == NULL)
    {
      std::string msg;
      msg = _("Error!  Tileset could not be loaded.");
      Gtk::MessageDialog dialog(*window, msg);
      current_save_filename = old_current_save_filename;
      dialog.run();
      dialog.hide();
      return false;
    }
  if (File::nameEndsWith(current_save_filename, "/autosave" + Tileset::file_extension) == false)
    {
      RecentlyEditedFileList::getInstance()->addEntry(current_save_filename);
      RecentlyEditedFileList::getInstance()->save();
    }
  if (d_tileset)
    delete d_tileset;
  d_tileset = tileset;

  d_tileset->setBaseName(File::get_basename(autosave));
  d_tileset->instantiateImages();

  tilestyles_list->clear();
  tilestylesets_list->clear();
  tiles_list->clear();
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
  update_window_title();
  return true;
}

void TileSetWindow::update_window_title()
{
  std::string title = "";
  if (needs_saving)
    title += "*";
  title += File::get_basename(current_save_filename, true);
  title += " - ";
  title += _("LordsAWar! Tileset Editor");
  window->set_title(title);
}
