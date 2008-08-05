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
#include "../game-client.h"
#include "../game-server.h"
#include "../shieldsetlist.h"

namespace
{
    Glib::ustring player_type_to_string(Uint32 type)
    {
	switch (type)
	{
	case Player::HUMAN: return HUMAN_PLAYER_TYPE;
	case Player::AI_FAST: return EASY_PLAYER_TYPE;
	case Player::AI_SMART: return HARD_PLAYER_TYPE;
	case Player::NETWORKED: return NETWORKED_PLAYER_TYPE;
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
  Shieldsetlist::getInstance()->instantiatePixmaps();
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
    xml->get_widget("play_button", play_button);
    play_button->signal_clicked().connect
      (sigc::mem_fun(this, &GameLobbyDialog::on_play_clicked));
    xml->get_widget("cancel_button", cancel_button);
    cancel_button->signal_clicked().connect
      (sigc::mem_fun(this, &GameLobbyDialog::on_cancel_clicked));
    xml->get_widget("map_image", map_image);
    xml->get_widget("turn_label", turn_label);
    xml->get_widget("scenario_name_label", scenario_name_label);
    xml->get_widget("cities_label", cities_label);
    xml->get_widget("chat_scrolledwindow", chat_scrolledwindow);
    xml->get_widget("chat_textview", chat_textview);
    xml->get_widget("chat_entry", chat_entry);

    chat_entry->signal_key_release_event().connect_notify
          (sigc::mem_fun(*this, &GameLobbyDialog::on_chat_key_pressed));

    update_city_map();

    Gtk::EventBox *map_eventbox;
    xml->get_widget("map_eventbox", map_eventbox);
    xml->connect_clicked(
	"show_options_button",
	sigc::mem_fun(*this, &GameLobbyDialog::on_show_options_clicked));

  if (GameServer::getInstance()->isListening())
    {
      GameServer *game_server = GameServer::getInstance();
      game_server->remote_player_moved.connect
	(sigc::mem_fun(*this, &GameLobbyDialog::on_remote_player_ends_turn));
      game_server->remote_participant_joins.connect
	(sigc::mem_fun(*this, &GameLobbyDialog::on_remote_participant_joins));
      game_server->remote_participant_departs.connect
	(sigc::mem_fun(*this, &GameLobbyDialog::on_remote_participant_departs));
      game_server->player_sits.connect
	(sigc::mem_fun(*this, &GameLobbyDialog::on_player_sits));
      game_server->player_stands.connect
	(sigc::mem_fun(*this, &GameLobbyDialog::on_player_stands));
      game_server->remote_player_named.connect
	(sigc::mem_fun(*this, &GameLobbyDialog::on_remote_player_changes_name));
      game_server->chat_message_received.connect
	(sigc::mem_fun(*this, &GameLobbyDialog::on_chatted));
      game_server->remote_player_died.connect
	(sigc::mem_fun(*this, &GameLobbyDialog::on_remote_player_died));
    }
  else
    {
      GameClient *game_client = GameClient::getInstance();
      game_client->remote_player_moved.connect
	(sigc::mem_fun(*this, &GameLobbyDialog::on_remote_player_ends_turn));
      game_client->remote_participant_joins.connect
	(sigc::mem_fun(*this, &GameLobbyDialog::on_remote_participant_joins));
      game_client->remote_participant_departs.connect
	(sigc::mem_fun(*this, &GameLobbyDialog::on_remote_participant_departs));
      game_client->player_sits.connect
	(sigc::mem_fun(*this, &GameLobbyDialog::on_player_sits));
      game_client->player_stands.connect
	(sigc::mem_fun(*this, &GameLobbyDialog::on_player_stands));
      game_client->remote_player_named.connect
	(sigc::mem_fun(*this, &GameLobbyDialog::on_remote_player_changes_name));
      game_client->chat_message_received.connect
	(sigc::mem_fun(*this, &GameLobbyDialog::on_chatted));
      game_client->remote_player_died.connect
	(sigc::mem_fun(*this, &GameLobbyDialog::on_remote_player_died));
    }
  update_player_details();
  update_buttons();

}

void GameLobbyDialog::update_buttons()
{
  //if any types aren't networked, we can play.
  //if all types are networked then we can't.
  Gtk::TreeModel::Children kids = player_list->children();
  for (Gtk::TreeModel::Children::iterator i = kids.begin(); 
       i != kids.end(); i++)
    {
      Gtk::TreeModel::Row row = *i;
      if (row[player_columns.type] != NETWORKED_PLAYER_TYPE)
	{
	  play_button->set_sensitive(true);
	  return;
	}
    }
  play_button->set_sensitive(false);
}

void
GameLobbyDialog::update_player_details()
{
  if (player_list)
    {
      player_list->clear();
      player_list.reset();
    }
  player_list = Gtk::ListStore::create(player_columns);
  // setup the player settings
  player_treeview->set_model(player_list);

  player_treeview->remove_all_columns();


  player_treeview->append_column("", player_columns.shield);

  //the sitting toggle
  sitting_renderer.property_mode() = Gtk::CELL_RENDERER_MODE_EDITABLE;
  sitting_renderer.property_activatable() = true;
  sitting_renderer.signal_editing_started().connect
    (sigc::mem_fun(*this, &GameLobbyDialog::on_sitting_changed));
  sitting_column.set_cell_data_func
    (sitting_renderer, sigc::mem_fun(*this, &GameLobbyDialog::cell_data_sitting));
  player_treeview->append_column(sitting_column);

  // the name
  if (d_has_ops)
    player_treeview->append_column_editable(_("Name"), player_columns.name);
  else
    player_treeview->append_column(_("Name"), player_columns.name);

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
      if (player->isDead())
	continue;
      add_player(player_type_to_string(player->getType()), player->getName(), 
		 player);
    }
}



void GameLobbyDialog::on_sitting_changed(Gtk::CellEditable *editable,
					 const Glib::ustring &path)
{
  Playerlist *pl = Playerlist::getInstance();
  Player *player;
  Glib::RefPtr < Gtk::TreeSelection > selection = 
    player_treeview->get_selection();
  Gtk::TreeModel::iterator iter = selection->get_selected();
  if (!selection->count_selected_rows())
    return;
  //maybe we can't make other ppl stand up
  if ((*iter)[player_columns.sitting] &&
      (*iter)[player_columns.type] == NETWORKED_PLAYER_TYPE && d_has_ops == false)
    return;

  player = pl->getPlayer((*iter)[player_columns.player_id]);

  if (!(*iter)[player_columns.sitting])
    player_sat_down.emit(player);
  else
    player_stood_up.emit(player);
}

GameLobbyDialog::GameLobbyDialog(GameScenario *game_scenario, bool has_ops)
	:type_column(_("Type"), type_renderer),
	status_column(_("Status"), status_renderer),
	sitting_column(_("Seated"), sitting_renderer)
{
  d_has_ops = has_ops;
  initDialog(game_scenario);
  update_scenario_details();
}

GameLobbyDialog::~GameLobbyDialog()
{
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

void GameLobbyDialog::hide()
{
  dialog->hide();
}

bool GameLobbyDialog::run()
{
  Playerlist *pl = Playerlist::getInstance();

  if (d_game_scenario->s_hidden_map == false)
    {
      citymap->resize();
      citymap->draw();
    }

  dialog->show_all();
  if (GameServer::getInstance()->isListening() == false)
    GameClient::getInstance()->request_seat_manifest();
  else
    {
      //seat the ai players
      for (Playerlist::iterator i = pl->begin(), end = pl->end(); i != end; ++i)
	{
	  Player *player = *i;
	  if (player == pl->getNeutral())
	    continue;
	  if (player->isDead())
	    continue;
	  if (player->getType() == Player::HUMAN)
	    continue;
	  if (player->getType() == Player::NETWORKED)
	    continue;

	  player_sat_down.emit(player);
	}
    }
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

void GameLobbyDialog::cell_data_sitting(Gtk::CellRenderer *renderer,
					const Gtk::TreeIter& i)
{
  dynamic_cast<Gtk::CellRendererToggle*>(renderer)->set_active((*i)[player_columns.sitting]);
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
  (*i)[player_columns.player_id] = player->getId();
  if (player->getType() == Player::NETWORKED)
    {
      //do we have the particpant?
      if (dynamic_cast<NetworkPlayer*>(player)->isConnected() == true)
	{
	  (*i)[player_columns.sitting] = true;
	  //we do?  is it the active player?
	  if (Playerlist::getInstance()->getActiveplayer() == player)
	    (*i)[player_columns.status] = PLAYER_MOVING;
	  else
	    (*i)[player_columns.status] = PLAYER_WATCHING;
	}
      else
	{
	  //otherwise, the player is not here to play.
	  (*i)[player_columns.status] = PLAYER_NOT_HERE;
	  (*i)[player_columns.sitting] = false;
	}
    }
  else
    {
      if (Playerlist::getInstance()->getActiveplayer() == player)
	(*i)[player_columns.status] = PLAYER_MOVING;
      else
	(*i)[player_columns.status] = PLAYER_WATCHING;
      (*i)[player_columns.sitting] = true;
    }

  player_treeview->get_selection()->select(i);
}

void GameLobbyDialog::on_player_selected()
{
  update_buttons();
}

void GameLobbyDialog::on_remote_participant_joins()
{
}

void GameLobbyDialog::on_remote_participant_departs()
{
}

void GameLobbyDialog::on_player_sits(Player *p)
{
  if (!p)
    return;
  Gtk::TreeModel::Children kids = player_list->children();
  //look for the row that has the right player id.
  for (Gtk::TreeModel::Children::iterator i = kids.begin(); 
       i != kids.end(); i++)
    {
      Gtk::TreeModel::Row row = *i;
      if (row[player_columns.player_id] == p->getId())
	{
	  row[player_columns.sitting] = true;
	  row[player_columns.type] = player_type_to_string(p->getType());
	  if (Playerlist::getInstance()->getActiveplayer() == p)
	    row[player_columns.status] = PLAYER_MOVING;
	  else
	    row[player_columns.status] = PLAYER_WATCHING;
	  update_buttons();
	  return;
	}
    }
}

void GameLobbyDialog::on_player_stands(Player *p)
{
  if (!p)
    return;
  Gtk::TreeModel::Children kids = player_list->children();
  for (Gtk::TreeModel::Children::iterator i = kids.begin(); 
       i != kids.end(); i++)
    {
      Gtk::TreeModel::Row row = *i;
      if (row[player_columns.player_id] == p->getId())
	{
	  row[player_columns.sitting] = false;
	  row[player_columns.type] = player_type_to_string(p->getType());
	  row[player_columns.status] = PLAYER_NOT_HERE;
	  update_buttons();
	  return;
	}
    }
}

void GameLobbyDialog::on_remote_player_ends_turn(Player *p)
{
  update_scenario_details();
  update_city_map();
}

void GameLobbyDialog::on_remote_player_changes_name(Player *p)
{
  Gtk::TreeModel::Children kids = player_list->children();
  for (Gtk::TreeModel::Children::iterator i = kids.begin(); 
       i != kids.end(); i++)
    {
      Gtk::TreeModel::Row row = *i;
      if (row[player_columns.player_id] == p->getId())
	{
	  row[player_columns.name] = p->getName();
	  return;
	}
    }
}

void GameLobbyDialog::on_play_clicked()
{
}

void GameLobbyDialog::on_cancel_clicked()
{
}

void GameLobbyDialog::on_chat_key_pressed(GdkEventKey *event)
{
  if (event->keyval == 65293) //enter
    {
      if (chat_entry->get_text().length() > 0)
	message_sent(chat_entry->get_text());
      chat_entry->set_text("");
    }
  return;
}

void GameLobbyDialog::on_chatted(std::string nickname, std::string message)
{
  //if nickname is empty, then the message holds it.
  std::string new_text;
  if (nickname == "")
    new_text = chat_textview->get_buffer()->get_text() + "\n" + message;
  else
    new_text = chat_textview->get_buffer()->get_text() + "\n" + message;


  chat_textview->get_buffer()->set_text(new_text);
  chat_scrolledwindow->get_vadjustment()->set_value(chat_scrolledwindow->get_vadjustment()->get_upper());
}

void GameLobbyDialog::on_remote_player_died(Player *p)
{
  if (!p)
    return;

  Gtk::TreeNodeChildren rows = player_list->children();
  for(Gtk::TreeIter row = rows.begin(); row != rows.end(); ++row)
    {
      Gtk::TreeModel::Row my_row = *row;
      if (my_row[player_columns.player_id] == p->getId())
	{
	  player_list->erase(row);
	  return;
	}
    }
}
