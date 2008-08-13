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
#include "../NextTurnNetworked.h"

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

void GameLobbyDialog::initDialog(GameScenario *gamescenario, 
				 NextTurnNetworked *next_turn,
				 GameStation *game_station)
{
  Shieldsetlist::getInstance()->instantiatePixmaps();
  d_game_scenario = gamescenario;
  d_game_station = game_station;
  d_next_turn = next_turn;
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/game-lobby-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    decorate(dialog.get());
    window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));

    xml->get_widget("player_treeview", player_treeview);
    player_treeview->get_selection()->signal_changed().connect
          (sigc::mem_fun(*this, &GameLobbyDialog::on_player_selected));
    //player_treeview->get_selection()->set_mode(Gtk::SELECTION_NONE);
    xml->get_widget("people_treeview", people_treeview);
    people_treeview->property_headers_visible() = true;
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
    chat_scrolledwindow->property_hscrollbar_policy() = Gtk::POLICY_NEVER;
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

    game_station->remote_player_moved.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_remote_player_ends_turn));
    game_station->local_player_moved.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_local_player_ends_turn));
    game_station->local_player_starts_move.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_local_player_starts_turn));
    game_station->remote_participant_joins.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_remote_participant_joins));
    game_station->remote_participant_departs.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_remote_participant_departs));
    game_station->player_sits.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_player_sits));
    game_station->player_stands.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_player_stands));
    game_station->remote_player_named.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_remote_player_changes_name));
    game_station->chat_message_received.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_chatted));
    game_station->playerlist_reorder_received.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_reorder_playerlist));
    game_station->remote_player_died.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_player_died));
    game_station->local_player_died.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_player_died));

    update_player_details();
    update_buttons();

    people_list = Gtk::ListStore::create(people_columns);
    // setup the player settings
    people_treeview->set_model(people_list);

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

  player_treeview->append_column(_("Person"), player_columns.person);

  // the name column
  player_name_list = Gtk::ListStore::create(player_name_columns);

  name_renderer.property_editable() = true;

  name_renderer.signal_edited()
    .connect(sigc::mem_fun(*this, &GameLobbyDialog::on_name_edited));
  name_column.set_cell_data_func
    ( name_renderer, sigc::mem_fun(*this, &GameLobbyDialog::cell_data_name));
  player_treeview->append_column(name_column);

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
  Gtk::TreeModel::iterator iter = player_treeview->get_model()->get_iter(path);
  Playerlist *pl = Playerlist::getInstance();
  Player *player;
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

GameLobbyDialog::GameLobbyDialog(GameScenario *game_scenario, 
				 NextTurnNetworked *next_turn, 
				 GameStation *game_station,
				 bool has_ops)
:name_column(_("Name"), name_renderer),
    type_column(_("Type"), type_renderer),
    sitting_column(_("Seated"), sitting_renderer)
{
  d_has_ops = has_ops;
  initDialog(game_scenario, next_turn, game_station);
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

void GameLobbyDialog::show()
{
  dialog->show_all();
  return;
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
  //fixme, make a player->changeType method, and send it as an action
}

void GameLobbyDialog::cell_data_name(Gtk::CellRenderer *renderer,
				     const Gtk::TreeIter& i)
{
  Playerlist *pl = Playerlist::getInstance();
  Player *player;
  player = pl->getPlayer((*i)[player_columns.player_id]);

  dynamic_cast<Gtk::CellRendererText*>(renderer)->property_text()
    = player->getName();
}

void GameLobbyDialog::on_name_edited(const Glib::ustring &path,
				     const Glib::ustring &new_text)
{
  Playerlist *pl = Playerlist::getInstance();
  Player *player;
  Gtk::TreeModel::iterator iter = player_treeview->get_model()->get_iter(path);
  if ((*iter)[player_columns.sitting] == false)
    return;
  player = pl->getPlayer((*iter)[player_columns.player_id]);

  player->rename(new_text);
  (*player_list->get_iter(Gtk::TreePath(path)))[player_columns.name]
    = new_text;
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
	}
      else
	{
	  //otherwise, the player is not here to play.
	  (*i)[player_columns.sitting] = false;
	}
    }
  else
    {
      (*i)[player_columns.sitting] = true;
    }

}

void GameLobbyDialog::on_player_selected()
{

}

void GameLobbyDialog::on_remote_participant_joins(std::string nickname)
{
  Gtk::TreeIter j = people_list->append();
  (*j)[people_columns.nickname] = nickname;
}

void GameLobbyDialog::on_remote_participant_departs(std::string nickname)
{
  //iterate through and remove the nickname
  Gtk::TreeNodeChildren rows = people_list->children();
  for(Gtk::TreeIter row = rows.begin(); row != rows.end(); ++row)
    {
      Gtk::TreeModel::Row my_row = *row;
      if (my_row[people_columns.nickname] == nickname)
	{
	  people_list->erase(row);
	  return;
	}
    }
}

void GameLobbyDialog::on_player_sits(Player *p, std::string nickname)
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
	  row[player_columns.person] = nickname;
	  update_buttons();
	  return;
	}
    }
}

void GameLobbyDialog::on_player_stands(Player *p, std::string nickname)
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
	  row[player_columns.person] = "";
	  row[player_columns.type] = player_type_to_string(p->getType());
	  update_buttons();
	  return;
	}
    }
}

void GameLobbyDialog::on_local_player_ends_turn(Player *p)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  Gtk::TreeModel::Children kids = player_list->children();
  for (Gtk::TreeModel::Children::iterator i = kids.begin(); 
       i != kids.end(); i++)
    {
      Gtk::TreeModel::Row row = *i;
      if (row[player_columns.player_id] == p->getId())
	{
	  Glib::RefPtr<Gdk::Pixbuf> empty_pic
	    = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, 1, 1);
	  empty_pic->fill(0x00000000);
	  row[player_columns.turn] = empty_pic;
	}
    }
  update_scenario_details();
  update_city_map();
}
void GameLobbyDialog::on_local_player_starts_turn(Player *p)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  Gtk::TreeModel::Children kids = player_list->children();
  for (Gtk::TreeModel::Children::iterator i = kids.begin(); 
       i != kids.end(); i++)
    {
      Gtk::TreeModel::Row row = *i;
      Player *active = Playerlist::getActiveplayer();
      if (row[player_columns.player_id] == active->getId())
	(*i)[player_columns.turn] = 
	  to_pixbuf(gc->getCursorPic(GraphicsCache::SWORD));
    }
  update_scenario_details();
  update_city_map();
}
void GameLobbyDialog::on_remote_player_ends_turn(Player *p)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  Gtk::TreeModel::Children kids = player_list->children();
  for (Gtk::TreeModel::Children::iterator i = kids.begin(); 
       i != kids.end(); i++)
    {
      Gtk::TreeModel::Row row = *i;
      if (row[player_columns.player_id] == p->getId())
	{
	  Glib::RefPtr<Gdk::Pixbuf> empty_pic
	    = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, 1, 1);
	  empty_pic->fill(0x00000000);
	  row[player_columns.turn] = empty_pic;
	}
      Player *active = Playerlist::getActiveplayer();
      if (row[player_columns.player_id] == active->getId())
	(*i)[player_columns.turn] = 
	  to_pixbuf(gc->getCursorPic(GraphicsCache::SWORD));
    }
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
  hide();
  //emit a signal saying to start a network game.
  start_network_game.emit(d_game_scenario, d_next_turn);
}

void GameLobbyDialog::on_cancel_clicked()
{
  hide();
}

void GameLobbyDialog::on_chat_key_pressed(GdkEventKey *event)
{
  if (event->keyval == 65293) //enter
    {
      if (chat_entry->get_text().length() > 0)
	message_sent.emit(chat_entry->get_text());
      chat_entry->set_text("");
    }
  return;
}

void GameLobbyDialog::on_chatted(std::string nickname, std::string message)
{
  std::string new_text;
  new_text = chat_textview->get_buffer()->get_text() + "\n" + message;
  chat_textview->get_buffer()->set_text(new_text);
  chat_scrolledwindow->get_vadjustment()->set_value(chat_scrolledwindow->get_vadjustment()->get_upper());
}

void GameLobbyDialog::on_player_died(Player *p)
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

void GameLobbyDialog::on_reorder_playerlist()
{
  int count = 0;
  std::list<int> new_order;
  Playerlist *pl = Playerlist::getInstance();
  for (Playerlist::iterator it = pl->begin(); it != pl->end(); it++)
    {
      Gtk::TreeNodeChildren rows = player_list->children();
      for(Gtk::TreeIter row = rows.begin(); row != rows.end(); ++row)
	{
	  Gtk::TreeModel::Row my_row = *row;
	  if (my_row[player_columns.player_id] == (*it)->getId())
	    {
	      new_order.push_back(count);
	      count++;
	      break;
	    }
	}
    }
  player_list->reorder(new_order);
}

bool GameLobbyDialog::run()
{
  Playerlist *pl = Playerlist::getInstance();

  if (d_game_scenario->s_hidden_map == false)
    {
      citymap->resize();
      citymap->draw();
    }

  people_treeview->remove_all_columns();
  people_treeview->append_column(_("People"), people_columns.nickname);
  if (GameServer::getInstance()->isListening() == false)
    {
      GameClient::getInstance()->request_seat_manifest();
      Gtk::TreeIter j = people_list->append();
      (*j)[people_columns.nickname] = "[" + GameClient::getInstance()->getNickname() + "]";
    }
  else
    {
      Gtk::TreeIter j = people_list->append();
      (*j)[people_columns.nickname] = "[" + GameServer::getInstance()->getNickname() + "]";
      //automatically seat the ai players
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
  if (response == 1)
    return true;
  else
    return false;
}
