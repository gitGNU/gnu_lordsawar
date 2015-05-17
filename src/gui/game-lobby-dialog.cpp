//  Copyright (C) 2008, 2009, 2011, 2014 Ben Asselstine
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
#include <map>
#include <sigc++/functors/mem_fun.h>

#include "game-lobby-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "citylist.h"
#include "playerlist.h"
#include "game-options-dialog.h"
#include "ImageCache.h"
#include "network_player.h"
#include "game-client.h"
#include "game-server.h"
#include "shieldsetlist.h"
#include "NextTurnNetworked.h"
#include "recently-played-game-list.h"
#include "game-parameters.h"

namespace
{
    Glib::ustring player_type_to_string(guint32 type)
    {
      Glib::ustring s = 
        GameParameters::player_param_to_string(GameParameters::player_type_to_player_param(type));
      if (s == NETWORKED_PLAYER_TYPE)
        return "";
      else
        return s;
    }
}

void GameLobbyDialog::update_city_map()
{
  if (d_game_scenario->s_hidden_map == false)
    {
      if (citymap)
	delete citymap;
      citymap = new CityMap();
      citymap->map_changed.connect
	(sigc::mem_fun(this, &GameLobbyDialog::on_map_changed));
      if (d_game_scenario->getRound() > 1)
	{
	  citymap->resize();
	  citymap->draw(Playerlist::getActiveplayer());
	}
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
  bool broken = false;
  Shieldsetlist::getInstance()->instantiateImages(broken);
  d_game_scenario = gamescenario;
  d_game_station = game_station;
  d_next_turn = next_turn;
  d_play_message_received = false;
  citymap = NULL;

    xml->get_widget("player_treeview", player_treeview);
    player_treeview->get_selection()->signal_changed().connect
          (sigc::mem_fun(*this, &GameLobbyDialog::on_player_selected));
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
    xml->get_widget("show_options_button", show_options_button);
    show_options_button->signal_clicked().connect
	(sigc::mem_fun(*this, &GameLobbyDialog::on_show_options_clicked));

    game_station->remote_player_moved.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_remote_player_ends_turn));
    game_station->remote_player_starts_move.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_remote_player_starts_turn));
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
    game_station->player_changes_name.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_player_changes_name));
    game_station->player_changes_type.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_player_changes_type));
    game_station->remote_player_named.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_remote_player_changes_name));
    game_station->chat_message_received.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_chatted));
    game_station->playerlist_reorder_received.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_reorder_playerlist));
    game_station->round_begins.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_reorder_playerlist));
    game_station->remote_player_died.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_player_died));
    game_station->local_player_died.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_player_died));
    game_station->nickname_changed.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_nickname_changed));
    game_station->game_may_begin.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_play_message_received));
    game_station->player_gets_turned_off.connect
      (sigc::mem_fun(*this, &GameLobbyDialog::on_player_turned_off));

    update_player_details();
    update_buttons();

    people_list = Gtk::ListStore::create(people_columns);
    // setup the player settings
    people_treeview->set_model(people_list);
}

void GameLobbyDialog::update_buttons()
{
  if (d_play_button_clicked == true)
    return;
  //if any types aren't networked, we can play.
  //if all types are networked then we can't.
  if (d_has_ops)
    {
      Gtk::TreeModel::Children kids = player_list->children();
      for (Gtk::TreeModel::Children::iterator i = kids.begin(); 
           i != kids.end(); i++)
        {
          Gtk::TreeModel::Row row = *i;
          if (row[player_columns.type] != "")
            {
              play_button->set_sensitive(true);
              return;
            }
        }
      play_button->set_sensitive(false);
    }
  else
    {
      bool found = false;
      //do we have a horse in the race?
      Gtk::TreeModel::Children kids = player_list->children();
      for (Gtk::TreeModel::Children::iterator i = kids.begin(); 
           i != kids.end(); i++)
        {
          Gtk::TreeModel::Row row = *i;
          if (row[player_columns.person] == d_game_station->getNickname())
            {
              found = true;
              break;
            }
        }
      if (found)
        play_button->set_sensitive(d_play_message_received);
      else
        play_button->set_sensitive(false);
    }
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
  player_list->set_sort_column (player_columns.order, Gtk::SORT_ASCENDING);

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

  guint32 count = 0;
  for (Playerlist::iterator i = pl->begin(), end = pl->end(); i != end; ++i)
    {
      Player *player = *i;
      if (player == pl->getNeutral())
	continue;
      if (player->isDead())
	continue;
      add_player(count, player_type_to_string(player->getType()), player->getName(), 
		 player);
      count++;
    }
  update_turn_indicator();
}

void GameLobbyDialog::on_sitting_changed(Gtk::CellEditable *editable,
					 const Glib::ustring &path)
{
  Gtk::TreeModel::iterator iter = player_treeview->get_model()->get_iter(path);
  Playerlist *pl = Playerlist::getInstance();
  Player *player;
  //maybe we can't make other ppl stand up
  if ((*iter)[player_columns.sitting] &&
      (*iter)[player_columns.person] != d_game_station->getNickname() && 
      d_has_ops == false)
    return;

  sitting_renderer.set_sensitive(false);
  player = pl->getPlayer((*iter)[player_columns.player_id]);
  d_player_id_of_sit_or_stand_request = player->getId();

  if (!(*iter)[player_columns.sitting])
    player_sat_down.emit(player);
  else
    player_stood_up.emit(player);
}

GameLobbyDialog::GameLobbyDialog(Gtk::Window &parent,
                                 GameScenario *game_scenario, 
				 NextTurnNetworked *next_turn, 
				 GameStation *game_station,
				 bool has_ops)
 : LwDialog(parent, "game-lobby-dialog.ui"),
    name_column(_("Name"), name_renderer),
    type_column(_("Type"), type_renderer),
    sitting_column(_("Controlled"), sitting_renderer)
{
  d_has_ops = has_ops;
  d_play_button_clicked = false;
  d_play_message_received = false;
  initDialog(game_scenario, next_turn, game_station);
  update_scenario_details();
  d_player_id_of_sit_or_stand_request = MAX_PLAYERS + 1;
  d_player_id_of_name_change_request = MAX_PLAYERS + 1;
  d_player_id_of_type_change_request = MAX_PLAYERS + 1;
  name_column.set_resizable();
  name_column.set_expand();
  name_column.set_min_width(115);
  name_renderer.property_ellipsize() = Pango::ELLIPSIZE_END;
  player_treeview->get_column(2)->set_min_width(75);
  player_treeview->get_column(2)->set_resizable();
}

GameLobbyDialog::~GameLobbyDialog()
{
  if (citymap)
    delete citymap;
}

void GameLobbyDialog::update_scenario_details()
{

  Glib::ustring s;
  s = String::ucompose("%1", d_game_scenario->getRound());
  turn_label->set_text(s);
  scenario_name_label->set_text(d_game_scenario->getName());
  s = String::ucompose("%1", Citylist::getInstance()->size());
  cities_label->set_text(s);

  update_city_map();
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

void GameLobbyDialog::on_map_changed(Cairo::RefPtr<Cairo::Surface> map)
{
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
    Gdk::Pixbuf::create(map, 0, 0, citymap->get_width(), citymap->get_height());
  map_image->property_pixbuf() = pixbuf;
}

void GameLobbyDialog::on_show_options_clicked()
{
  GameOptionsDialog gd(*dialog, true);
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
  Gtk::TreeModel::iterator iter = player_treeview->get_model()->get_iter(path);
      
  if ((*iter)[player_columns.sitting] == false)
    return;
  if ((*iter)[player_columns.person] != d_game_station->getNickname())
    return;
  type_renderer.set_sensitive(false);
  Playerlist *pl = Playerlist::getInstance();
  Player *player = pl->getPlayer((*iter)[player_columns.player_id]);
  d_player_id_of_type_change_request = player->getId();

  int type = GameParameters::player_param_string_to_player_param(new_text);
  player_changed_type.emit (player, type);
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
  if (d_has_ops == false)
    {
      if ((*iter)[player_columns.sitting] == false)
        return;
      if ((*iter)[player_columns.person] != d_game_station->getNickname())
        return;
    }
  Glib::ustring new_name = String::utrim(new_text);
  if (new_name.empty() == true)
    return;

  name_renderer.set_sensitive(false);
  player = pl->getPlayer((*iter)[player_columns.player_id]);
  d_player_id_of_name_change_request = player->getId();

  //here's where we send the message saying that the name has changed.
  player_changed_name.emit(player, new_name);
}

void GameLobbyDialog::cell_data_sitting(Gtk::CellRenderer *renderer,
					const Gtk::TreeIter& i)
{
  dynamic_cast<Gtk::CellRendererToggle*>(renderer)->set_active((*i)[player_columns.sitting]);
}

void GameLobbyDialog::add_player(guint32 order, const Glib::ustring &type,
				 const Glib::ustring &name, Player *player)
{
  ImageCache *gc = ImageCache::getInstance();
  Gtk::TreeIter i = player_list->append();
  (*i)[player_columns.order] = order;
  (*i)[player_columns.shield] = gc->getShieldPic(1, player)->to_pixbuf();
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

void GameLobbyDialog::on_remote_participant_joins(Glib::ustring nickname)
{
  Gtk::TreeIter j = people_list->append();
  (*j)[people_columns.nickname] = nickname;
}

void GameLobbyDialog::on_remote_participant_departs(Glib::ustring nickname)
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

void GameLobbyDialog::on_player_changes_name(Player *p, Glib::ustring name)
{
  if (!p)
    return;
  if (p->getId() == d_player_id_of_name_change_request)
    {
      name_renderer.set_sensitive(true);
      d_player_id_of_name_change_request = MAX_PLAYERS + 1;
    }
  //look for the row that has the right player id, and change the name
  Gtk::TreeModel::Children kids = player_list->children();
  for (Gtk::TreeModel::Children::iterator i = kids.begin(); 
       i != kids.end(); i++)
    {
      Gtk::TreeModel::Row row = *i;
      if (row[player_columns.player_id] == p->getId())
        {
          p->setName(name);
          row[player_columns.name] = name;
        }
    }
}

void GameLobbyDialog::on_player_changes_type(Player *p, int type)
{
  if (!p)
    return;
  if (p->getId() == d_player_id_of_type_change_request)
    {
      type_renderer.set_sensitive(true);
      d_player_id_of_type_change_request = MAX_PLAYERS + 1;
    }
  Gtk::TreeModel::Children kids = player_list->children();
  //look for the row that has the right player id.
  for (Gtk::TreeModel::Children::iterator i = kids.begin(); 
       i != kids.end(); i++)
    {
      Gtk::TreeModel::Row row = *i;
      if (row[player_columns.player_id] == p->getId())
	{
          Glib::ustring s = GameParameters::player_param_to_string(type);
          if (s == NETWORKED_PLAYER_TYPE)
            row[player_columns.type] = "";
          else
            row[player_columns.type] = s;
	  update_buttons();
	  return;
	}
    }
}

void GameLobbyDialog::on_player_sits(Player *p, Glib::ustring nickname)
{
  if (!p)
    return;
  if (p->getId() == d_player_id_of_sit_or_stand_request)
    {
      sitting_renderer.set_sensitive(true);
      d_player_id_of_sit_or_stand_request = MAX_PLAYERS + 1;
    }
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

void GameLobbyDialog::on_player_stands(Player *p, Glib::ustring nickname)
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
  update_turn_indicator();
  update_scenario_details();
}

void GameLobbyDialog::update_turn_indicator()
{
  ImageCache *gc = ImageCache::getInstance();
  Gtk::TreeModel::Children kids = player_list->children();
  for (Gtk::TreeModel::Children::iterator i = kids.begin(); 
       i != kids.end(); i++)
    {
      Gtk::TreeModel::Row row = *i;
      Player *active = Playerlist::getActiveplayer();
      if (active)
        {
          if (row[player_columns.player_id] == active->getId())
            (*i)[player_columns.turn] = gc->getCursorPic(ImageCache::SWORD)->to_pixbuf();
          else
            {
              Glib::RefPtr<Gdk::Pixbuf> empty_pic
                = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, 4, 4);
              empty_pic->fill(0x00000000);
              (*i)[player_columns.turn] = empty_pic;
            }
        }
    }
}

void GameLobbyDialog::on_remote_player_starts_turn(Player *p)
{
  update_turn_indicator();
  update_scenario_details();
}

void GameLobbyDialog::on_local_player_starts_turn(Player *p)
{
  update_turn_indicator();
  update_scenario_details();
}

void GameLobbyDialog::on_remote_player_ends_turn(Player *p)
{
  if (GameServer::getInstance()->isListening() == false)
    {
      RecentlyPlayedGameList *rpgl = RecentlyPlayedGameList::getInstance();
      rpgl->updateEntry(d_game_scenario);
      rpgl->save();
    }
  update_turn_indicator();
  update_scenario_details();
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

Player* GameLobbyDialog::get_selected_player(Glib::ustring &nick, bool &sitting)
{
  Glib::RefPtr<Gtk::TreeSelection> selection = player_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();
  if (iterrow)
    {
      Gtk::TreeModel::Row row = *iterrow;
      nick = row[player_columns.name];
      sitting = row[player_columns.sitting];
      return 
        Playerlist::getInstance()->getPlayer(row[player_columns.player_id]);
    }
  else
    return NULL;
}

void GameLobbyDialog::on_play_clicked()
{
  if (d_play_button_clicked == false)
    {
      //we only get here on the first time play is clicked
      //otherwise it just shows the form (Driver::start_network_game_requested)
      hide();
      if (d_has_ops)
        {
          lock_down();
          game_may_begin.emit();
        }
      play_button->set_sensitive(false);
      play_button->property_visible() = false;
      start_network_game.emit(d_game_scenario, d_next_turn);
      d_play_button_clicked = true;
    }
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

void GameLobbyDialog::on_chatted(Glib::ustring nickname, Glib::ustring message)
{
  Glib::ustring new_text;
  new_text = chat_textview->get_buffer()->get_text() + "\n" + message;
  chat_textview->get_buffer()->set_text(new_text);
  while (g_main_context_iteration(NULL, FALSE)); //doEvents
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
  sort_player_list_by_turn_order();
}

bool GameLobbyDialog::run()
{
  Playerlist *pl = Playerlist::getInstance();

  if (d_game_scenario->s_hidden_map == false)
    {
      citymap->resize();
      citymap->draw(Playerlist::getActiveplayer());
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
  if (response == Gtk::RESPONSE_ACCEPT)
    return true;
  else
    return false;
}

void GameLobbyDialog::on_nickname_changed(Glib::ustring old_name, Glib::ustring new_name)
{
  d_game_station->setNickname(new_name);
  //iterate through and find the nickname
  Gtk::TreeNodeChildren rows = people_list->children();
  for(Gtk::TreeIter row = rows.begin(); row != rows.end(); ++row)
    {
      Gtk::TreeModel::Row my_row = *row;
      if (my_row[people_columns.nickname] == old_name)
        {
          my_row[people_columns.nickname] = new_name;
          return;
        }
      Glib::ustring match = String::ucompose ("[%1]", old_name);
      if (my_row[people_columns.nickname] == match)
        {
          my_row[people_columns.nickname] = String::ucompose("[%1]", new_name);
          return;
        }
    }
  
}
    
void GameLobbyDialog::lock_down ()
{
  name_renderer.set_sensitive(false);
  name_renderer.property_editable() = false;
  type_renderer.set_sensitive(false);
  type_renderer.property_editable() = false;
}

void GameLobbyDialog::on_play_message_received()
{
  d_play_message_received = true;
  update_buttons();
  lock_down();
}
    
void GameLobbyDialog::on_player_turned_off(Player *player)
{
  Gtk::TreeNodeChildren rows = player_list->children();
  for(Gtk::TreeIter row = rows.begin(); row != rows.end(); ++row)
    {
      Gtk::TreeModel::Row my_row = *row;
      if (my_row[player_columns.player_id] == player->getId())
        {
          player_list->erase(row);
          return;
        }
    }
}

void GameLobbyDialog::sort_player_list_by_turn_order()
{
  Playerlist *pl = Playerlist::getInstance();
  std::map<guint32, guint32> id_order;

  guint32 count = 0;
  for (Playerlist::iterator i = pl->begin(), end = pl->end(); i != end; ++i)
    {
      Player *player = *i;
      if (player == pl->getNeutral())
	continue;
      if (player->isDead())
	continue;
      id_order[player->getId()] = count;
      count++;
    }

  Gtk::TreeNodeChildren rows = player_list->children();
  for(Gtk::TreeIter row = rows.begin(); row != rows.end(); ++row)
    {
      Gtk::TreeModel::Row my_row = *row;
      my_row[player_columns.order] = 
        id_order[my_row[player_columns.player_id]];
    }
}

void GameLobbyDialog::clean_up_players()
{
  player_list->clear();
  //player_list.reset();
}
