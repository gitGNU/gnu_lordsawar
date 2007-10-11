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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef NEW_MAP_DIALOG_H
#define NEW_MAP_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <gtkmm/window.h>
#include <gtkmm/dialog.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/combobox.h>
#include <gtkmm/widget.h>
#include <gtkmm/scale.h>

#include "../Tile.h"
#include "../game-parameters.h"

class NewMapDialog
{
 public:
    NewMapDialog();
    ~NewMapDialog();

    void set_parent_window(Gtk::Window &parent);

    void run();

    struct Map
    {
	int fill_style;
	int width, height;
	int grass, water, swamp, forest, hills, mountains;
	int cities, ruins, temples;
	int signposts;
    };

    Map map;
    
    bool map_set;
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;

    Gtk::ComboBox *map_size_combobox;
    Gtk::ComboBoxText *fill_style_combobox;
    Gtk::Widget *random_map_container;
    Gtk::Scale *grass_scale;
    Gtk::Scale *water_scale;
    Gtk::Scale *swamp_scale;
    Gtk::Scale *forest_scale;
    Gtk::Scale *hills_scale;
    Gtk::Scale *mountains_scale;
    Gtk::Scale *cities_scale;
    Gtk::Scale *ruins_scale;
    Gtk::Scale *temples_scale;
    Gtk::Scale *signposts_scale;

    enum { MAP_SIZE_NORMAL = 0, MAP_SIZE_SMALL, MAP_SIZE_TINY };

    void on_fill_style_changed();
    void on_map_size_changed();

    void add_fill_style(Tile::Type tile_type);

    std::vector<int> fill_style;
};

#endif
