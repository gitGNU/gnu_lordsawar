//  Copyright (C) 2007, Ole Laursen
//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    xml->get_widget("map_size_combobox", map_size_combobox);
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
    
    Uint32 counter = 0;
    Uint32 default_id = 0;
    Gtk::Box *box;

    //fill in tile sizes combobox
    tile_size_combobox = manage(new Gtk::ComboBoxText);
    std::list<Uint32> sizes;
    Tilesetlist::getInstance()->getSizes(sizes);
    Citysetlist::getInstance()->getSizes(sizes);
    Armysetlist::getInstance()->getSizes(sizes);
    for (std::list<Uint32>::iterator it = sizes.begin(); it != sizes.end();
	 it++)
      {
	Glib::ustring s = String::ucompose("%1x%1", *it);
	tile_size_combobox->append_text(s);
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
    std::list<std::string> shield_themes = sl->getNames();
    for (std::list<std::string>::iterator i = shield_themes.begin(),
	 end = shield_themes.end(); i != end; ++i)
      {
	if (*i == _("Default"))
	  default_id = counter;
	shield_theme_combobox->append_text(Glib::filename_to_utf8(*i));
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

    fill_style_combobox->append_text(_("Random"));
    fill_style.push_back(-1);

    Gtk::Alignment *alignment;
    xml->get_widget("fill_style_alignment", alignment);
    alignment->add(*fill_style_combobox);

    fill_style_combobox->signal_changed().connect(
						  sigc::mem_fun(*this, &NewMapDialog::on_fill_style_changed));
    fill_style_combobox->set_active(0);

    // map size
    map_size_combobox->set_active(MAP_SIZE_NORMAL);
    map_size_combobox->signal_changed().connect(
						sigc::mem_fun(*this, &NewMapDialog::on_map_size_changed));
    on_map_size_changed();
}

NewMapDialog::~NewMapDialog()
{
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
    break;

  case MAP_SIZE_TINY:
    cities_scale->set_value(10);
    ruins_scale->set_value(15);
    temples_scale->set_value(15);
    break;

  case MAP_SIZE_NORMAL:
  default:
    cities_scale->set_value(20);
    ruins_scale->set_value(25);
    temples_scale->set_value(25);
    break;
  }
}

void NewMapDialog::add_fill_style(Tile::Type tile_type)
{
  Tileset *tileset = GameMap::getInstance()->getTileset();
  Tile *tile = (*tileset)[tileset->getIndex(tile_type)];
  fill_style_combobox->append_text(tile->getName());
  fill_style.push_back(tile_type);
}

Uint32 NewMapDialog::get_active_tile_size()
{
  return (Uint32) atoi(tile_size_combobox->get_active_text().c_str());
}

void NewMapDialog::on_tile_size_changed()
{
  Uint32 default_id = 0;
  Uint32 counter = 0;

  tile_theme_combobox->clear_items();
  Tilesetlist *tl = Tilesetlist::getInstance();
  std::list<std::string> tile_themes = tl->getNames(get_active_tile_size());
  for (std::list<std::string>::iterator i = tile_themes.begin(),
       end = tile_themes.end(); i != end; ++i)
    {
      if (*i == _("Default"))
	default_id = counter;
      tile_theme_combobox->append_text(Glib::filename_to_utf8(*i));
      counter++;
    }

  tile_theme_combobox->set_active(default_id);
  if (tile_theme_combobox->get_children().size() == 0)
    accept_button->set_sensitive(false);

  army_theme_combobox->clear_items();
  Armysetlist *al = Armysetlist::getInstance();
  std::list<std::string> army_themes = al->getNames(get_active_tile_size());
  counter = 0;
  default_id = 0;
  for (std::list<std::string>::iterator i = army_themes.begin(),
       end = army_themes.end(); i != end; ++i)
    {
      if (*i == _("Default"))
	default_id = counter;
      army_theme_combobox->append_text(Glib::filename_to_utf8(*i));
      counter++;
    }

  army_theme_combobox->set_active(default_id);
  if (army_theme_combobox->get_children().size() == 0)
    accept_button->set_sensitive(false);

  city_theme_combobox->clear_items();
  Citysetlist *cl = Citysetlist::getInstance();
  std::list<std::string> city_themes = cl->getNames(get_active_tile_size());
  counter = 0;
  default_id = 0;
  for (std::list<std::string>::iterator i = city_themes.begin(),
       end = city_themes.end(); i != end; ++i)
    {
      if (*i == _("Default"))
	default_id = counter;
      city_theme_combobox->append_text(Glib::filename_to_utf8(*i));
      counter++;
    }

  city_theme_combobox->set_active(default_id);
  if (city_theme_combobox->get_children().size() == 0)
    accept_button->set_sensitive(false);
}
