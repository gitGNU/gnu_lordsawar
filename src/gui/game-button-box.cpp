//  Copyright (C) 2011, 2014, 2015 Ben Asselstine
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

#include "game-button-box.h"
#include "builder-cache.h"
#include "File.h"
#include "game.h"
#include "GameScenario.h"
#include "playerlist.h"
#include "player.h"
#include "ImageCache.h"

Glib::ustring GameButtonBox::get_file(Configuration::UiFormFactor factor)
{
  Glib::ustring file = "";
  switch (factor)
    {
    case Configuration::UI_FORM_FACTOR_DESKTOP:
      file = "game-button-box-desktop.ui";
      break;
    case Configuration::UI_FORM_FACTOR_NETBOOK:
      file = "game-button-box-netbook.ui";
      break;
    case Configuration::UI_FORM_FACTOR_LARGE_SCREEN:
      file = "game-button-box-large-screen.ui";
      break;
    }
  return file;
}

GameButtonBox * GameButtonBox::create(guint32 factor)
{
  Glib::ustring file = get_file(Configuration::UiFormFactor(factor));
  Glib::RefPtr<Gtk::Builder> xml = BuilderCache::get(file);

  GameButtonBox *box;
  xml->get_widget_derived("box", box);
  box->add_pictures_to_buttons(factor);
  return box;
}

void GameButtonBox::pad_image(Gtk::Image *image)
{
  int padding = 0;
  switch (Configuration::UiFormFactor(d_factor))
    {
    case Configuration::UI_FORM_FACTOR_DESKTOP:
      padding = 0;
      break;
    case Configuration::UI_FORM_FACTOR_NETBOOK:
      padding = 0;
      break;
    case Configuration::UI_FORM_FACTOR_LARGE_SCREEN:
      padding = 3;
      break;
    }
  image->property_xpad() = padding;
  image->property_ypad() = padding;
}

void GameButtonBox::add_picture_to_button (guint32 icontype, Gtk::Button *button, bool arrow)
{
  int s = get_icon_size(d_factor);
  Gtk::Image *image = new Gtk::Image();
  PixMask *pixmask;
  if (arrow)
    pixmask = ImageCache::getInstance()->getArrowImage(icontype, s);
  else
    pixmask = ImageCache::getInstance()->getGameButtonImage(icontype, s);
  if (pixmask == NULL)
    return;
  image->property_pixbuf() = pixmask->to_pixbuf();
  pad_image(image);
  button->add(*manage(image));
}

void GameButtonBox::add_pictures_to_buttons(guint32 factor)
{
  d_factor = factor;
  add_picture_to_button (ImageCache::NEXT_MOVABLE_STACK, next_movable_button);
  add_picture_to_button (ImageCache::CENTER_ON_STACK, center_button);
  add_picture_to_button (ImageCache::DIPLOMACY_NO_PROPOSALS, diplomacy_button);
  add_picture_to_button (ImageCache::STACK_DEFEND, defend_button);
  add_picture_to_button (ImageCache::STACK_PARK, park_button);
  add_picture_to_button (ImageCache::STACK_DESELECT, deselect_button);
  add_picture_to_button (ImageCache::STACK_SEARCH, search_button);
  add_picture_to_button (ImageCache::STACK_MOVE, move_button);
  add_picture_to_button (ImageCache::MOVE_ALL_STACKS, move_all_button);
  add_picture_to_button (ImageCache::END_TURN, end_turn_button);
  add_picture_to_button (ImageCache::NORTHWEST, nw_keypad_button, true);
  add_picture_to_button (ImageCache::NORTH, n_keypad_button, true);
  add_picture_to_button (ImageCache::NORTHEAST, ne_keypad_button, true);
  add_picture_to_button (ImageCache::WEST, w_keypad_button, true);
  add_picture_to_button (ImageCache::EAST, e_keypad_button, true);
  add_picture_to_button (ImageCache::SOUTHWEST, sw_keypad_button, true);
  add_picture_to_button (ImageCache::SOUTH, s_keypad_button, true);
  add_picture_to_button (ImageCache::SOUTHEAST, se_keypad_button, true);
}

GameButtonBox::GameButtonBox(BaseObjectType* baseObject, const Glib::RefPtr<Gtk::Builder> &xml)
  : Gtk::Box(baseObject)
{
  xml->get_widget("next_movable_button", next_movable_button);
  xml->get_widget("center_button", center_button);
  xml->get_widget("diplomacy_button", diplomacy_button);
  xml->get_widget("defend_button", defend_button);
  xml->get_widget("park_button", park_button);
  xml->get_widget("deselect_button", deselect_button);
  xml->get_widget("search_button", search_button);
  xml->get_widget("move_button", move_button);
  xml->get_widget("move_all_button", move_all_button);
  xml->get_widget("end_turn_button", end_turn_button);
  xml->get_widget("nw_keypad_button", nw_keypad_button);
  xml->get_widget("n_keypad_button", n_keypad_button);
  xml->get_widget("ne_keypad_button", ne_keypad_button);
  xml->get_widget("e_keypad_button", e_keypad_button);
  xml->get_widget("w_keypad_button", w_keypad_button);
  xml->get_widget("sw_keypad_button", sw_keypad_button);
  xml->get_widget("s_keypad_button", s_keypad_button);
  xml->get_widget("se_keypad_button", se_keypad_button);
}

void GameButtonBox::drop_connections()
{
  std::list<sigc::connection>::iterator it = connections.begin();
  for (; it != connections.end(); it++) 
    (*it).disconnect();
  connections.clear();
}

GameButtonBox::~GameButtonBox()
{
  drop_connections();
}

int GameButtonBox::get_icon_size(guint32 factor)
{
  int s = 0;
  switch (Configuration::UiFormFactor(factor))
    {
    case Configuration::UI_FORM_FACTOR_DESKTOP:
      s = 1;
      break;
    case Configuration::UI_FORM_FACTOR_NETBOOK:
      s = 0;
      break;
    case Configuration::UI_FORM_FACTOR_LARGE_SCREEN:
      s = 2;
      break;
    }
  return s;
}

void GameButtonBox::setup_signals(Game *game, guint32 factor)
{
  d_factor = factor;
  drop_connections();
  setup_button(next_movable_button,
               sigc::mem_fun(game, &Game::select_next_movable_stack),
               game->can_select_next_movable_stack);
  setup_button(defend_button,
               sigc::mem_fun(game, &Game::defend_selected_stack),
               game->can_defend_selected_stack);
  setup_button(park_button,
               sigc::mem_fun(game, &Game::park_selected_stack),
               game->can_park_selected_stack);
  setup_button(deselect_button,
               sigc::mem_fun(game, &Game::deselect_selected_stack),
               game->can_deselect_selected_stack);
  setup_button(search_button,
               sigc::mem_fun(game, &Game::search_selected_stack),
               game->can_search_selected_stack);
  setup_button(move_button,
               sigc::mem_fun(game, &Game::move_selected_stack_along_path),
               game->can_move_selected_stack_along_path);
  setup_button(move_all_button,
               sigc::mem_fun(game, &Game::move_all_stacks),
               game->can_move_all_stacks);
  setup_button(end_turn_button,
               sigc::mem_fun(game, &Game::end_turn),
               game->can_end_turn);
  setup_button(center_button,
               sigc::mem_fun(game, &Game::center_selected_stack),
               game->can_center_selected_stack);
  setup_button(nw_keypad_button,
               sigc::mem_fun(game, &Game::move_selected_stack_northwest),
               game->can_end_turn);
  setup_button(n_keypad_button,
               sigc::mem_fun(game, &Game::move_selected_stack_north),
               game->can_end_turn);
  setup_button(ne_keypad_button,
               sigc::mem_fun(game, &Game::move_selected_stack_northeast),
               game->can_end_turn);
  setup_button(e_keypad_button,
               sigc::mem_fun(game, &Game::move_selected_stack_east),
               game->can_end_turn);
  setup_button(w_keypad_button,
               sigc::mem_fun(game, &Game::move_selected_stack_west),
               game->can_end_turn);
  setup_button(sw_keypad_button,
               sigc::mem_fun(game, &Game::move_selected_stack_southwest),
               game->can_end_turn);
  setup_button(s_keypad_button,
               sigc::mem_fun(game, &Game::move_selected_stack_south),
               game->can_end_turn);
  setup_button(se_keypad_button,
               sigc::mem_fun(game, &Game::move_selected_stack_southeast),
               game->can_end_turn);
  connections.push_back 
    (game->received_diplomatic_proposal.connect 
     (sigc::mem_fun(*this, &GameButtonBox::change_diplomacy_button_image)));
  connections.push_back 
    (game->can_end_turn.connect 
     (sigc::mem_fun(*this, &GameButtonBox::update_diplomacy_button)));
  connections.push_back
    (diplomacy_button->signal_clicked().connect
     (diplomacy_clicked, &sigc::signal<void>::emit));
}

void GameButtonBox::setup_button(Gtk::Button *button, sigc::slot<void> slot,
                                 sigc::signal<void, bool> &game_signal)
{
  connections.push_back (button->signal_clicked().connect(slot));
  connections.push_back 
    (game_signal.connect(sigc::mem_fun(button, &Gtk::Widget::set_sensitive)));
}

void GameButtonBox::update_diplomacy_button (bool sensitive)
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    {
      diplomacy_button->set_sensitive (false);
      return;
    }
  if (GameScenario::s_diplomacy == false)
    {
      diplomacy_button->set_sensitive (false);
      return;
    }
  diplomacy_button->set_sensitive(sensitive);
}

void GameButtonBox::change_diplomacy_button_image (bool proposals_present)
{
  ImageCache *gc = ImageCache::getInstance();
  /* switch up the image. */
  int s = GameButtonBox::get_icon_size(Configuration::s_ui_form_factor);
  if (proposals_present)
    {
      Gtk::Image *proposals_present_image = new Gtk::Image();
      proposals_present_image->property_pixbuf() = 
        gc->getGameButtonImage(ImageCache::DIPLOMACY_NEW_PROPOSALS, s)->to_pixbuf();
      pad_image(proposals_present_image);
      diplomacy_button->property_image() = proposals_present_image;
    }
  else
    {
      Gtk::Image *proposals_not_present_image = new Gtk::Image();
      proposals_not_present_image->property_pixbuf() = 
        gc->getGameButtonImage(ImageCache::DIPLOMACY_NO_PROPOSALS, s)->to_pixbuf();
      pad_image(proposals_not_present_image);
      diplomacy_button->property_image() = proposals_not_present_image;
    }
}

void GameButtonBox::give_some_cheese()
{
  end_turn_button->set_sensitive(false);
}

bool GameButtonBox::get_end_turn_button_sensitive()
{
  return end_turn_button->get_sensitive();
}
