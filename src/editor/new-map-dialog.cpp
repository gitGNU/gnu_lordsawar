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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.*

#include <config.h>

#include <assert.h>
#include <sigc++/functors/mem_fun.h>
#include <libglademm/xml.h>
#include <gtkmm/alignment.h>

#include "new-map-dialog.h"

#include "glade-helpers.h"
#include "../defs.h"
#include "../File.h"
#include "../tileset.h"
#include "../tilesetlist.h"
#include "../GameMap.h"


NewMapDialog::NewMapDialog()
{
    map_set = false;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/new-map-dialog.glade");

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
    // fill in tile themes combobox
    tile_theme_combobox = manage(new Gtk::ComboBoxText);
    
    Tilesetlist *tl = Tilesetlist::getInstance();
    std::list<std::string> tile_themes = tl->getNames();
    for (std::list<std::string>::iterator i = tile_themes.begin(),
	     end = tile_themes.end(); i != end; ++i)
	tile_theme_combobox->append_text(Glib::filename_to_utf8(*i));

    tile_theme_combobox->set_active(0);

    Gtk::Box *tile_set_box;
    xml->get_widget("tile_set_box", tile_set_box);
    tile_set_box->pack_start(*tile_theme_combobox, Gtk::PACK_SHRINK);


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
    if (response == 0)		// accepted
    {
	switch (map_size_combobox->get_active_row_number()) {
	case MAP_SIZE_SMALL:
	    map.width = 70;
	    map.height = 105;
	    break;
	
	case MAP_SIZE_TINY:
	    map.width = 50;
	    map.height = 75;
	    break;
	
	case MAP_SIZE_NORMAL:
	default:
	    map.width = 112;
	    map.height = 156;
	    break;
	}
	
	int row = fill_style_combobox->get_active_row_number();
	assert(row >= 0 && row < int(fill_style.size()));
	map.fill_style = fill_style[row];

	map.tileset = Tilesetlist::getInstance()->getTilesetDir
	  (Glib::filename_from_utf8(tile_theme_combobox->get_active_text()));

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
    TileSet *tileset = GameMap::getInstance()->getTileSet();
    Tile *tile = (*tileset)[tileset->getIndex(tile_type)];
    fill_style_combobox->append_text(tile->getName());
    fill_style.push_back(tile_type);
}

