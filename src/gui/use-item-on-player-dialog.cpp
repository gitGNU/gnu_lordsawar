//  Copyright (C) 2010 Ben Asselstine
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

#include "use-item-on-player-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "GameMap.h"
#include "File.h"
#include "GraphicsCache.h"
#include "player.h"

UseItemOnPlayerDialog::UseItemOnPlayerDialog()
{
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/use-item-on-player-dialog.ui");

    xml->get_widget("dialog", dialog);
    decorate(dialog);
    window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));

    players_list = Gtk::ListStore::create(players_columns);
    xml->get_widget("playertreeview", player_treeview);
    player_treeview->set_model(players_list);
    player_treeview->append_column("", players_columns.image);
    player_treeview->append_column("", players_columns.name);
    player_treeview->get_selection()->signal_changed().connect
      (sigc::mem_fun(*this, &UseItemOnPlayerDialog::on_player_selected));

    xml->get_widget("map_image", map_image);
    xml->get_widget("continue_button", continue_button);

    citymap = new CityMap();
    citymap->map_changed.connect(
	sigc::mem_fun(this, &UseItemOnPlayerDialog::on_map_changed));

    Gtk::EventBox *map_eventbox;
    xml->get_widget("map_eventbox", map_eventbox);

    Playerlist *pl = Playerlist::getInstance();
    for (Playerlist::iterator it = pl->begin(); it != pl->end(); it++)
      if ((*it) != pl->getActiveplayer() && pl->getNeutral() != (*it) &&
          (*it)->isDead() == false)
        addPlayer(*it);

  continue_button->set_sensitive(false);

}

UseItemOnPlayerDialog::~UseItemOnPlayerDialog()
{
  delete dialog;
  delete citymap;
}

void UseItemOnPlayerDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

Player *UseItemOnPlayerDialog::grabSelectedPlayer()
{
    Glib::RefPtr<Gtk::TreeView::Selection> sel;
    sel = player_treeview->get_selection();
    if (sel == true)
      {
        Gtk::TreeModel::iterator it = sel->get_selected();
        Gtk::TreeModel::Row row = *it;
        return row[players_columns.player];
      }
}

void UseItemOnPlayerDialog::hide()
{
  dialog->hide();
}

Player*UseItemOnPlayerDialog::run()
{
    citymap->resize();
    citymap->draw(Playerlist::getActiveplayer());

    dialog->show_all();
    dialog->run();
  
    Player *player = grabSelectedPlayer();

    return player;
}

void UseItemOnPlayerDialog::on_map_changed(Glib::RefPtr<Gdk::Pixmap> map)
{
  map_image->property_pixmap() = map;
}

void UseItemOnPlayerDialog::addPlayer(Player *player)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  Gtk::TreeIter i = players_list->append();
  (*i)[players_columns.name] = player->getName();
  (*i)[players_columns.image] = gc->getShieldPic(2, player)->to_pixbuf();
  (*i)[players_columns.player] = player;
}

void UseItemOnPlayerDialog::on_player_selected()
{
  continue_button->set_sensitive(true);
}

