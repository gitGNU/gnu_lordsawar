//  Copyright (C) 2010, 2014 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>

#include "tileset-smallmap-building-colors-dialog.h"

#include "glade-helpers.h"
#include "ucompose.hpp"
#include "File.h"
#include "defs.h"
#include "tileset.h"


TilesetSmallmapBuildingColorsDialog::TilesetSmallmapBuildingColorsDialog(Tileset *tileset)
{
  d_tileset = tileset;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file
    (get_glade_path() + "/tileset-smallmap-building-colors-dialog.ui");

  xml->get_widget("dialog", dialog);
  xml->get_widget("road_colorbutton", road_colorbutton);
  road_colorbutton->set_rgba(tileset->getRoadColor());
  road_colorbutton->signal_color_set().connect
    (sigc::mem_fun(this, 
                   &TilesetSmallmapBuildingColorsDialog::on_road_color_chosen));
  xml->get_widget("ruin_colorbutton", ruin_colorbutton);
  ruin_colorbutton->set_rgba(tileset->getRuinColor());
  ruin_colorbutton->signal_color_set().connect
    (sigc::mem_fun(this, 
                   &TilesetSmallmapBuildingColorsDialog::on_ruin_color_chosen));
  xml->get_widget("temple_colorbutton", temple_colorbutton);
  temple_colorbutton->set_rgba(tileset->getTempleColor());
  temple_colorbutton->signal_color_set().connect
    (sigc::mem_fun
     (this, &TilesetSmallmapBuildingColorsDialog::on_temple_color_chosen));
}

TilesetSmallmapBuildingColorsDialog::~TilesetSmallmapBuildingColorsDialog()
{
  delete dialog;
}
void TilesetSmallmapBuildingColorsDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
}

int TilesetSmallmapBuildingColorsDialog::run()
{
    dialog->show_all();
    int response = dialog->run();
    return response;
}

void TilesetSmallmapBuildingColorsDialog::hide()
{
  dialog->hide();
}

void TilesetSmallmapBuildingColorsDialog::on_road_color_chosen()
{
  d_tileset->setRoadColor(road_colorbutton->get_rgba());
}

void TilesetSmallmapBuildingColorsDialog::on_ruin_color_chosen()
{
  d_tileset->setRuinColor(ruin_colorbutton->get_rgba());
}

void TilesetSmallmapBuildingColorsDialog::on_temple_color_chosen()
{
  d_tileset->setTempleColor(temple_colorbutton->get_rgba());
}
