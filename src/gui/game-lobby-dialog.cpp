//  Copyright (C) 2008 Ben Asselstine
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

#include <libglademm/xml.h>
#include <gtkmm/eventbox.h>
#include <sigc++/functors/mem_fun.h>
#include <gtkmm/label.h>

#include "game-lobby-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../File.h"
#include "../citylist.h"
#include "../playerlist.h"
#include "game-options-dialog.h"
#include "../GraphicsCache.h"
#include "../network_player.h"

namespace
{
    Glib::ustring player_type_to_string(Uint32 type)
    {
	switch (type)
	{
	case Player::HUMAN: return HUMAN_PLAYER_TYPE;
	case Player::AI_FAST: return EASY_PLAYER_TYPE;
	case Player::AI_SMART: return HARD_PLAYER_TYPE;
	default: return NO_PLAYER_TYPE;
	}
    }
}

void GameLobbyDialog::update_city_map()
{
  if (d_game_scenario->s_hidden_map == false)
    {
      citymap.reset(new CityMap());
      citymap->map_changed.connect
	(sigc::mem_fun(this, &GameLobbyDialog::on_map_changed));
    }
  else
    {
      map_image->property_file() = 
	File::getMiscFile("various/city_occupied.png");
    }
}

void GameLobbyDialog::initDialog(GameScenario *gamescenario)
{
  d_game_scenario = gamescenario;
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/game-lobby-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    xml->get_widget("player_treeview", player_treeview);
    player_treeview->get_selection()->signal_changed().connect
          (sigc::mem_fun(*this, &GameLobbyDialog::on_player_selected));
    xml->get_widget("sit_button", sit_button);
    sit_button->signal_clicked().connect
      (sigc::mem_fun(this, &GameLobbyDialog::on_sit_clicked));
    xml->get_widget("map_image", map_image);
    xml->get_widget("turn_label", turn_label);
    xml->get_widget("scenario_name_label", scenario_name_label);
    xml->get_widget("cities_label", cities_label);

    update_city_map();

    Gtk::EventBox *map_eventbox;
    xml->get_widget("map_eventbox", map_eventbox);
    xml->connect_clicked(
	"show_options_button",
	sigc::mem_fun(*this, &GameLobbyDialog::on_show_options_clicked));

  update_player_details();
  update_buttons();
}

void GameLobbyDialog::update_buttons()
{
  if (player_treeview->get_selection()->get_selected() == 0)
    sit_button->set_sensitive(false);
  else
    {
      Glib::RefPtr<Gtk::TreeSelection> selection;
      selection = player_treeview->get_selection();
      Gtk::TreeModel::iterator iterrow = selection->get_selected();
      if (iterrow)
	{
	  Gtk::TreeModel::Row row = *iterrow;
	  if (row[player_columns.status] == PLAYER_NOT_HERE)
	    sit_button->set_sensitive(true);
	  else
	    sit_button->set_sensitive(false);
	}
      else
	sit_button->set_sensitive(false);

    }
}

void
GameLobbyDialog::update_player_details()
{
  player_list.clear();
  player_list = Gtk::ListStore::create(player_columns);
  // setup the player settings
  player_treeview->set_model(player_list);

  player_treeview->append_column("", player_columns.shield);
  // the type column
  player_type_list = Gtk::ListStore::create(player_type_columns);
  Gtk::TreeModel::iterator i;
  i = player_type_list->append();
  (*i)[player_type_columns.type] = HUMAN_PLAYER_TYPE;
  i = player_type_list->append();
  (*i)[player_type_columns.type] = EASY_PLAYER_TYPE;
  i = player_type_list->append();
  (*i)[player_type_columns.type] = HARD_PLAYER_TYPE;
  i = player_type_list->append();
  (*i)[player_type_columns.type] = NO_PLAYER_TYPE;

  type_renderer.property_model() = player_type_list;
  type_renderer.property_text_column() = 0;
  type_renderer.property_has_entry() = false;
  type_renderer.property_editable() = d_has_ops;

  type_renderer.signal_edited()
    .connect(sigc::mem_fun(*this, &GameLobbyDialog::on_type_edited));
  type_column.set_cell_data_func
    ( type_renderer, sigc::mem_fun(*this, &GameLobbyDialog::cell_data_type));
  player_treeview->append_column(type_column);


  // the name
  if (d_has_ops)
    player_treeview->append_column_editable(_("Name"), player_columns.name);
  else
    player_treeview->append_column(_("Name"), player_columns.name);

  //the status
  status_renderer.property_model() = player_status_list;
  status_renderer.property_text_column() = 0;
  status_renderer.property_has_entry() = false;
  status_renderer.property_editable() = false;
  status_column.set_cell_data_func
    (status_renderer, sigc::mem_fun(*this, &GameLobbyDialog::cell_data_status));
  player_treeview->append_column(status_column);

  //if it's this player's turn
  player_treeview->append_column(_("Turn"), player_columns.turn);

  Playerlist *pl = Playerlist::getInstance();

  for (Playerlist::iterator i = pl->begin(), end = pl->end(); i != end; ++i)
    {
      Player *player = *i;
      if (player == pl->getNeutral())
	continue;
      add_player(player_type_to_string(player->getType()), player->getName(), 
		 player);
    }
}

GameLobbyDialog::GameLobbyDialog(std::string filename, bool has_ops)
  :type_column(_("Type"), type_renderer), 
    status_column(_("Status"), status_renderer)
{
  d_has_ops = has_ops;
  bool broken = false;
  d_destroy_gamescenario = true;
  GameScenario *game_scenario = new GameScenario(filename, broken);
  initDialog(game_scenario);
  update_scenario_details();
}

GameLobbyDialog::GameLobbyDialog(GameScenario *game_scenario, bool has_ops)
  :type_column(_("Type"), type_renderer),
    status_column(_("Status"), status_renderer)
{
  d_has_ops = has_ops;
  initDialog(game_scenario);
  update_scenario_details();
}

GameLobbyDialog::~GameLobbyDialog()
{
  if (d_destroy_gamescenario)
    delete d_game_scenario;
  else
    {
      std::string filename = File::getSavePath() + "network.sav";
      d_game_scenario->saveGame(filename);
    }
}

void GameLobbyDialog::update_scenario_details()
{
    
  Glib::ustring s;
  s = String::ucompose("%1", d_game_scenario->getRound());
  turn_label->set_text(s);
  scenario_name_label->set_text(d_game_scenario->getName());
  s = String::ucompose("%1", Citylist::getInstance()->size());
  cities_label->set_text(s);

  //select the player whose turn it is.
  int i = 0;
  Playerlist *pl = Playerlist::getInstance();
  for (Playerlist::iterator it = pl->begin(); it != pl->end(); it++)
    {
      if (*it == pl->getActiveplayer())
	break;
      i++;
    }
  Gtk::TreeModel::Row row = player_treeview->get_model()->children()[i];
  player_treeview->get_selection()->select(row);

}
void GameLobbyDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

bool GameLobbyDialog::run()
{
  if (d_game_scenario->s_hidden_map == false)
    {
      citymap->resize();
      citymap->draw();
    }

    dialog->show_all();
    int response = dialog->run();

    if (response == 0)
	return true;
    else
	return false;
}

void GameLobbyDialog::on_map_changed(SDL_Surface *map)
{
    map_image->property_pixbuf() = to_pixbuf(map);
}

void GameLobbyDialog::on_show_options_clicked()
{
  GameOptionsDialog gd(true);
  gd.run();
}

void GameLobbyDialog::cell_data_type(Gtk::CellRenderer *renderer,
				     const Gtk::TreeIter& i)
{
  dynamic_cast<Gtk::CellRendererText*>(renderer)->property_text()
    = (*i)[player_columns.type];
}

void GameLobbyDialog::on_type_edited(const Glib::ustring &path,
				     const Glib::ustring &new_text)
{
  (*player_list->get_iter(Gtk::TreePath(path)))[player_columns.type]
    = new_text;
}

void GameLobbyDialog::cell_data_status(Gtk::CellRenderer *renderer,
				       const Gtk::TreeIter& i)
{
  Glib::ustring s;
  if ((*i)[player_columns.status] != "")
    s = String::ucompose("<b> - %1 - </b>", (*i)[player_columns.status]);
  else
    s = "";
  dynamic_cast<Gtk::CellRendererText*>(renderer)->property_markup() = s;
}

void GameLobbyDialog::add_player(const Glib::ustring &type,
			       const Glib::ustring &name, Player *player)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  Gtk::TreeIter i = player_list->append();
  if (player == Playerlist::getInstance()->getActiveplayer())
    (*i)[player_columns.turn] = 
      to_pixbuf(gc->getCursorPic(GraphicsCache::SWORD));
  (*i)[player_columns.shield] = to_pixbuf(gc->getShieldPic(1, player));
  (*i)[player_columns.type] = type;
  (*i)[player_columns.name] = name;
  if (player->getType() == Player::NETWORKED)
    {
      //do we have the particpant?
      if (dynamic_cast<NetworkPlayer*>(player)->isConnected() == true)
	{
	  //we do?  is it the active player?
	  if (Playerlist::getInstance()->getActiveplayer() == player)
	    (*i)[player_columns.status] = PLAYER_MOVING;
	  else
	    (*i)[player_columns.status] = PLAYER_WATCHING;
	}
      else
	//otherwise, the player is not here to play.
	(*i)[player_columns.status] = PLAYER_NOT_HERE;
      //hackola:
      if (player->getId() == 7)
	(*i)[player_columns.status] = PLAYER_NOT_HERE;
      if (player->getId() == 6)
	(*i)[player_columns.status] = "";
    }
  else
    {
      if (Playerlist::getInstance()->getActiveplayer() == player)
	(*i)[player_columns.status] = PLAYER_MOVING;
      else
	(*i)[player_columns.status] = PLAYER_WATCHING;
    }
  (*i)[player_columns.player] = player;

  player_treeview->get_selection()->select(i);
}

void GameLobbyDialog::on_player_selected()
{
  update_buttons();
}

void GameLobbyDialog::on_remote_player_joins()
{
  update_player_details();
  update_buttons();
}

void GameLobbyDialog::on_remote_player_departs()
{
  update_player_details();
  update_buttons();
}

void GameLobbyDialog::on_remote_player_ends_turn()
{
  update_scenario_details();
  update_city_map();
}

void GameLobbyDialog::on_remote_player_changes_name()
{
  update_player_details();
  update_buttons();
}
      
void GameLobbyDialog::on_remote_player_changes_type()
{
  update_player_details();
  update_buttons();
}
 
void GameLobbyDialog::on_sit_clicked()
{
}
