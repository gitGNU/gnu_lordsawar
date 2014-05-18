//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2010, 2012, 2014 Ben Asselstine
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

#include <assert.h>
#include <sigc++/functors/mem_fun.h>
#include <gtkmm.h>

#include "new-map-dialog.h"

#include "glade-helpers.h"
#include "defs.h"
#include "File.h"
#include "tileset.h"
#include "tilesetlist.h"
#include "armysetlist.h"
#include "citysetlist.h"
#include "shieldsetlist.h"
#include "ucompose.hpp"
#include "GameMap.h"


NewMapDialog::NewMapDialog()
{
    map_set = false;
    
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path() + "/new-map-dialog.ui");

    xml->get_widget("dialog", dialog);

    xml->get_widget("map_size_combobox", map_size_combobox);

    xml->get_widget("custom_size_table", custom_size_table);
    xml->get_widget("width_spinbutton", width_spinbutton);
    xml->get_widget("height_spinbutton", height_spinbutton);
    xml->get_widget("random_map_container", random_map_container);
    xml->get_widget("grass_scale", grass_scale);
    xml->get_widget("water_scale", water_scale);
    xml->get_widget("swamp_scale", swamp_scale);
    xml->get_widget("forest_scale", forest_scale);
    xml->get_widget("hills_scale", hills_scale);
    xml->get_widget("mountains_scale", mountains_scale);
    xml->get_widget("cities_scale", cities_scale);
    xml->get_widget("ruins_scale", ruins_scale);
    xml->get_widget("temples_scale", temples_scale);
    xml->get_widget("signposts_scale", signposts_scale);
    xml->get_widget("accept_button", accept_button);

    // fill in tile themes combobox
    
    guint32 counter = 0;
    guint32 default_id = 0;
    Gtk::Box *box;

    //fill in tile sizes combobox
    tile_size_combobox = manage(new Gtk::ComboBoxText);
    std::list<guint32> sizes;
    Tilesetlist::getInstance()->getSizes(sizes);
    Citysetlist::getInstance()->getSizes(sizes);
    Armysetlist::getInstance()->getSizes(sizes);
    for (std::list<guint32>::iterator it = sizes.begin(); it != sizes.end();
	 it++)
      {
	Glib::ustring s = String::ucompose("%1x%1", *it);
	tile_size_combobox->append(s);
	if ((*it) == Tileset::getDefaultTileSize())
	  default_id = counter;
	counter++;
      }
    tile_size_combobox->set_active(default_id);
    xml->get_widget("tile_size_box", box);
    box->pack_start(*tile_size_combobox, Gtk::PACK_SHRINK);
    tile_size_combobox->signal_changed().connect
      (sigc::mem_fun(*this, &NewMapDialog::on_tile_size_changed));

    // make new tile themes combobox
    tile_theme_combobox = manage(new Gtk::ComboBoxText);
    xml->get_widget("tile_theme_box", box);
    box->pack_start(*tile_theme_combobox, Gtk::PACK_SHRINK);

    // make new army themes combobox
    army_theme_combobox = manage(new Gtk::ComboBoxText);
    xml->get_widget("army_theme_box", box);
    box->pack_start(*army_theme_combobox, Gtk::PACK_SHRINK);

    // make new city themes combobox
    city_theme_combobox = manage(new Gtk::ComboBoxText);
    xml->get_widget("city_theme_box", box);
    box->pack_start(*city_theme_combobox, Gtk::PACK_SHRINK);

    counter = 0;
    default_id = 0;
    shield_theme_combobox = manage(new Gtk::ComboBoxText);
    Shieldsetlist *sl = Shieldsetlist::getInstance();
    std::list<std::string> shield_themes = sl->getValidNames();
    for (std::list<std::string>::iterator i = shield_themes.begin(),
	 end = shield_themes.end(); i != end; ++i)
      {
	if (*i == _("Default"))
	  default_id = counter;
	shield_theme_combobox->append(Glib::filename_to_utf8(*i));
	counter++;
      }

    shield_theme_combobox->set_active(default_id);

    xml->get_widget("shield_theme_box", box);
    box->pack_start(*shield_theme_combobox, Gtk::PACK_SHRINK);

    on_tile_size_changed();

    // create fill style combobox
    fill_style_combobox = manage(new Gtk::ComboBoxText);

    add_fill_style(Tile::GRASS);
    add_fill_style(Tile::WATER);
    add_fill_style(Tile::FOREST);
    add_fill_style(Tile::HILLS);
    add_fill_style(Tile::MOUNTAIN);
    add_fill_style(Tile::SWAMP);

    fill_style_combobox->append(_("Random"));
    fill_style.push_back(-1);

    Gtk::Alignment *alignment;
    xml->get_widget("fill_style_alignment", alignment);
    alignment->add(*fill_style_combobox);

    fill_style_combobox->signal_changed().connect(
						  sigc::mem_fun(*this, &NewMapDialog::on_fill_style_changed));
    fill_style_combobox->set_active(6);

    // map size
    map_size_combobox->set_active(MAP_SIZE_NORMAL);
    map_size_combobox->signal_changed().connect(
						sigc::mem_fun(*this, &NewMapDialog::on_map_size_changed));
    grass_scale->set_value(78);
    water_scale->set_value(7);
    swamp_scale->set_value(2);
    forest_scale->set_value(3);
    hills_scale->set_value(5);
    signposts_scale->set_value(20);
    mountains_scale->set_value(5);
    on_map_size_changed();
	
    width_spinbutton->set_value(MAP_SIZE_TINY_WIDTH);
    height_spinbutton->set_value(MAP_SIZE_TINY_HEIGHT);
}

NewMapDialog::~NewMapDialog()
{
  delete dialog;
}

void NewMapDialog::set_parent_window(Gtk::Window &parent)
{
  dialog->set_transient_for(parent);
}

void NewMapDialog::run()
{
  dialog->show_all();
  int response = dialog->run();
  if (response == Gtk::RESPONSE_ACCEPT)	// accepted
    {
      switch (map_size_combobox->get_active_row_number()) {
      case MAP_SIZE_SMALL:
	map.width = MAP_SIZE_SMALL_WIDTH;
	map.height = MAP_SIZE_SMALL_HEIGHT;
	break;

      case MAP_SIZE_TINY:
	map.width = MAP_SIZE_TINY_WIDTH;
	map.height = MAP_SIZE_TINY_HEIGHT;
	break;

      case MAP_SIZE_NORMAL:
      default:
	map.width = MAP_SIZE_NORMAL_WIDTH;
	map.height = MAP_SIZE_NORMAL_HEIGHT;
	break;

      case MAP_SIZE_CUSTOM:
	map.width = int(width_spinbutton->get_value());
	map.height = int(height_spinbutton->get_value());
        break;
      }

      int row = fill_style_combobox->get_active_row_number();
      assert(row >= 0 && row < int(fill_style.size()));
      map.fill_style = fill_style[row];

      map.tileset = Tilesetlist::getInstance()->getTilesetDir
	(Glib::filename_from_utf8(tile_theme_combobox->get_active_text()),
	 get_active_tile_size());

      map.shieldset = Shieldsetlist::getInstance()->getShieldsetDir
	(Glib::filename_from_utf8(shield_theme_combobox->get_active_text()));

      map.cityset = Citysetlist::getInstance()->getCitysetDir
	(Glib::filename_from_utf8(city_theme_combobox->get_active_text()),
	 get_active_tile_size());

      map.armyset = Armysetlist::getInstance()->getArmysetDir
	(Glib::filename_from_utf8(army_theme_combobox->get_active_text()),
	 get_active_tile_size());

      if (map.fill_style == -1)
	{
	  map.grass = int(grass_scale->get_value());
	  map.water = int(water_scale->get_value());
	  map.swamp = int(swamp_scale->get_value());
	  map.forest = int(forest_scale->get_value());
	  map.hills = int(hills_scale->get_value());
	  map.mountains = int(mountains_scale->get_value());
	  map.cities = int(cities_scale->get_value());
	  map.ruins = int(ruins_scale->get_value());
	  map.temples = int(temples_scale->get_value());
	  map.signposts = int(signposts_scale->get_value());
	}

      map_set = true;
    }
  else
    map_set = false;
}

void NewMapDialog::on_fill_style_changed()
{
  int row = fill_style_combobox->get_active_row_number();
  assert(row >= 0 && row < int(fill_style.size()));
  bool random_selected = fill_style[row] == -1;
  random_map_container->set_sensitive(random_selected);
}

void NewMapDialog::on_map_size_changed()
{
  switch (map_size_combobox->get_active_row_number()) {
  case MAP_SIZE_SMALL:
    cities_scale->set_value(15);
    ruins_scale->set_value(20);
    temples_scale->set_value(20);
    custom_size_table->set_sensitive(false);
    break;

  case MAP_SIZE_TINY:
    cities_scale->set_value(10);
    ruins_scale->set_value(15);
    temples_scale->set_value(15);
    custom_size_table->set_sensitive(false);
    break;

  case MAP_SIZE_NORMAL:
  default:
    cities_scale->set_value(20);
    ruins_scale->set_value(25);
    temples_scale->set_value(25);
    custom_size_table->set_sensitive(false);
    break;
  case MAP_SIZE_CUSTOM:
    cities_scale->set_value(20);
    ruins_scale->set_value(25);
    temples_scale->set_value(25);
    custom_size_table->set_sensitive(true);
    break;
  }
}

void NewMapDialog::add_fill_style(Tile::Type tile_type)
{
  Tileset *tileset = GameMap::getTileset();
  Tile *tile = (*tileset)[tileset->getIndex(tile_type)];
  fill_style_combobox->append(tile->getName());
  fill_style.push_back(tile_type);
}

guint32 NewMapDialog::get_active_tile_size()
{
  return (guint32) atoi(tile_size_combobox->get_active_text().c_str());
}

void NewMapDialog::on_tile_size_changed()
{
  guint32 default_id = 0;
  guint32 counter = 0;

  tile_theme_combobox->remove_all();
  Tilesetlist *tl = Tilesetlist::getInstance();
  std::list<std::string> tile_themes = tl->getValidNames(get_active_tile_size());
  for (std::list<std::string>::iterator i = tile_themes.begin(),
       end = tile_themes.end(); i != end; ++i)
    {
      if (*i == _("Default"))
	default_id = counter;
      tile_theme_combobox->append(Glib::filename_to_utf8(*i));
      counter++;
    }

  tile_theme_combobox->set_active(default_id);
  if (tile_theme_combobox->get_children().size() == 0)
    accept_button->set_sensitive(false);

  army_theme_combobox->remove_all();
  Armysetlist *al = Armysetlist::getInstance();
  std::list<std::string> army_themes = al->getValidNames(get_active_tile_size());
  counter = 0;
  default_id = 0;
  for (std::list<std::string>::iterator i = army_themes.begin(),
       end = army_themes.end(); i != end; ++i)
    {
      if (*i == _("Default"))
	default_id = counter;
      army_theme_combobox->append(Glib::filename_to_utf8(*i));
      counter++;
    }

  army_theme_combobox->set_active(default_id);
  if (army_theme_combobox->get_children().size() == 0)
    accept_button->set_sensitive(false);

  city_theme_combobox->remove_all();
  Citysetlist *cl = Citysetlist::getInstance();
  std::list<std::string> city_themes = cl->getValidNames(get_active_tile_size());
  counter = 0;
  default_id = 0;
  for (std::list<std::string>::iterator i = city_themes.begin(),
       end = city_themes.end(); i != end; ++i)
    {
      if (*i == _("Default"))
	default_id = counter;
      city_theme_combobox->append(Glib::filename_to_utf8(*i));
      counter++;
    }

  city_theme_combobox->set_active(default_id);
  if (city_theme_combobox->get_children().size() == 0)
    accept_button->set_sensitive(false);
}

