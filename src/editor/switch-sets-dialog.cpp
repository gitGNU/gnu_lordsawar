//  Copyright (C) 2009, 2010, 2012, 2014 Ben Asselstine
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

#include "switch-sets-dialog.h"

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
#include "ruinlist.h"
#include "citylist.h"
#include "armyset.h"
#include "shieldset.h"
#include "cityset.h"


SwitchSetsDialog::SwitchSetsDialog()
{
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path() + "/switch-sets-dialog.ui");

    xml->get_widget("dialog", dialog);
    dialog->set_icon_from_file(File::getMiscFile("various/tileset_icon.png"));

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
	if ((*it) == GameMap::getInstance()->getTileSize())
	  default_id = counter;
	counter++;
      }
    tile_size_combobox->set_active(default_id);
    xml->get_widget("tile_size_box", box);
    box->pack_start(*tile_size_combobox, Gtk::PACK_SHRINK);
    tile_size_combobox->signal_changed().connect
      (sigc::mem_fun(*this, &SwitchSetsDialog::on_tile_size_changed));

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
	if (*i == GameMap::getInstance()->getShieldsetName())
	  default_id = counter;
	shield_theme_combobox->append(Glib::filename_to_utf8(*i));
	counter++;
      }

    shield_theme_combobox->set_active(default_id);

    xml->get_widget("shield_theme_box", box);
    box->pack_start(*shield_theme_combobox, Gtk::PACK_SHRINK);

    on_tile_size_changed();

}

SwitchSetsDialog::~SwitchSetsDialog()
{
  delete dialog;
}

void SwitchSetsDialog::set_parent_window(Gtk::Window &parent)
{
  dialog->set_transient_for(parent);
}

guint32 SwitchSetsDialog::get_active_tile_size()
{
  return (guint32) atoi(tile_size_combobox->get_active_text().c_str());
}

void SwitchSetsDialog::on_tile_size_changed()
{
  guint32 default_id = 0;
  guint32 counter = 0;

  tile_theme_combobox->remove_all();
  Tilesetlist *tl = Tilesetlist::getInstance();
  std::list<std::string> tile_themes = tl->getValidNames(get_active_tile_size());
  for (std::list<std::string>::iterator i = tile_themes.begin(),
       end = tile_themes.end(); i != end; ++i)
    {
      if (*i == GameMap::getInstance()->getTilesetName())
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
  int armyset = Playerlist::getActiveplayer()->getArmyset();
  for (std::list<std::string>::iterator i = army_themes.begin(),
       end = army_themes.end(); i != end; ++i)
    {
      if (*i == Armysetlist::getInstance()->getArmyset(armyset)->getName())
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
  Cityset *active = GameMap::getCityset();
  for (std::list<std::string>::iterator i = city_themes.begin(),
       end = city_themes.end(); i != end; ++i)
    {
      if (*i == active->getName())
	default_id = counter;
      //only append it if the tile widths are identical.
      //Cityset *cityset = 
	//cl->getCityset(cl->getCitysetDir(*i, get_active_tile_size()));
      //if (active->tileWidthsEqual(cityset) == true)
	city_theme_combobox->append(Glib::filename_to_utf8(*i));
      counter++;
    }

  city_theme_combobox->set_active(default_id);
  if (city_theme_combobox->get_children().size() == 0)
    accept_button->set_sensitive(false);
}

int SwitchSetsDialog::run()
{
  dialog->show_all();
  int response = dialog->run();
  if (response != Gtk::RESPONSE_ACCEPT)	// accepted
    return response;
  std::string subdir;
  subdir = Tilesetlist::getInstance()->getTilesetDir
    (Glib::filename_from_utf8(tile_theme_combobox->get_active_text()),
     get_active_tile_size());
  selected_tileset = Tilesetlist::getInstance()->getTileset(subdir);

  subdir = Shieldsetlist::getInstance()->getShieldsetDir
    (Glib::filename_from_utf8(shield_theme_combobox->get_active_text()));
  selected_shieldset = Shieldsetlist::getInstance()->getShieldset(subdir);

  subdir = Citysetlist::getInstance()->getCitysetDir
    (Glib::filename_from_utf8(city_theme_combobox->get_active_text()),
     get_active_tile_size());
  selected_cityset = Citysetlist::getInstance()->getCityset(subdir);

  subdir = Armysetlist::getInstance()->getArmysetDir
    (Glib::filename_from_utf8(army_theme_combobox->get_active_text()),
     get_active_tile_size());
  selected_armyset = Armysetlist::getInstance()->getArmyset(subdir);

  GameMap::getInstance()->switchArmysets(selected_armyset);
  if (selected_cityset->getBaseName() != GameMap::getCityset()->getBaseName())
    GameMap::getInstance()->switchCityset(selected_cityset);
  if (selected_shieldset->getBaseName() != GameMap::getInstance()->getShieldsetName())
    GameMap::getInstance()->switchShieldset(selected_shieldset);
  if (selected_tileset->getBaseName() != GameMap::getInstance()->getTilesetName())
    GameMap::getInstance()->switchTileset(selected_tileset);
  return response;
}
