//  Copyright (C) 2011, 2014 Ben Asselstine
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

#include "stack-tile-box.h"
#include "stacktile.h"
#include "stack.h"
#include "stack-army-button.h"

#include "army-info-tip.h"
#include "ucompose.hpp"
#include "defs.h"
#include "GraphicsCache.h"
#include "File.h"
#include "playerlist.h"
#include "player.h"
#include "armysetlist.h"
#include "GameMap.h"
#include "army.h"

Glib::ustring StackTileBox::get_file(Configuration::UiFormFactor factor)
{
  Glib::ustring file = "";
  switch (factor)
    {
    case Configuration::UI_FORM_FACTOR_DESKTOP:
      file = File::getUIFile("stack-tile-box-desktop.ui");
      break;
    case Configuration::UI_FORM_FACTOR_NETBOOK:
      file = File::getUIFile("stack-tile-box-netbook.ui");
      break;
    case Configuration::UI_FORM_FACTOR_LARGE_SCREEN:
      file = File::getUIFile("stack-tile-box-large-screen.ui");
      break;
    }
  return file;
}

StackTileBox * StackTileBox::create(guint32 factor)
{
  Glib::ustring file = get_file(Configuration::UiFormFactor(factor));
  Glib::RefPtr<Gtk::Builder> xml = Gtk::Builder::create_from_file(file);

  StackTileBox *box;
  xml->get_widget_derived("box", box);
  box->d_factor = factor;
  return box;
}

void StackTileBox::pad_image(Gtk::Image *image)
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

StackTileBox::StackTileBox(BaseObjectType* baseObject, const Glib::RefPtr<Gtk::Builder> &xml)
  : Gtk::Box(baseObject)
{
  d_inhibit = false;
  army_info_tip = NULL;
  xml->get_widget("stack_info_box", stack_info_box);
  xml->get_widget("stack_info_container", stack_info_container);
  xml->get_widget("group_moves_label", group_moves_label);
  xml->get_widget("group_togglebutton", group_ungroup_toggle);
  group_ungroup_toggle->signal_toggled().connect
    (sigc::bind(sigc::mem_fun(*this, &StackTileBox::on_group_toggled),
                group_ungroup_toggle));
  xml->get_widget("terrain_image", terrain_image);
}

void StackTileBox::drop_connections()
{
  std::list<sigc::connection>::iterator it = connections.begin();
  for (; it != connections.end(); it++) 
    (*it).disconnect();
  connections.clear();
}

StackTileBox::~StackTileBox()
{
  drop_connections();
  if (army_info_tip)
    delete army_info_tip;
}

void StackTileBox::on_stack_toggled(StackArmyButton *radio, Stack *stack)
{
  if (d_inhibit)
    return;
  if (stack == currently_selected_stack)
    return;
  currently_selected_stack = stack;
  Playerlist::getActiveplayer()->setActivestack(stack);
  on_stack_info_changed(stack);
  stack_composition_modified.emit(stack);
}

void StackTileBox::on_army_toggled(StackArmyButton *toggle, Stack *stack, Army *army)
{
  if (d_inhibit)
    return;
  for (stack_army_buttons_type::iterator i = stack_army_buttons.begin(),
       end = stack_army_buttons.end(); i != end; ++i)
    (*i)->update_stack_button(toggle == *i);
  Player *p = Playerlist::getActiveplayer();
  Stack *s = p->getActivestack();
  group_ungroup_toggle->set_sensitive(false);
  if (toggle->get_active() == true)
    {
      if (stack->size() > 1)
	{
	  Stack *new_stack = p->stackSplitArmy(stack, army);
	  if (new_stack)
	    p->stackJoin(currently_selected_stack, new_stack);
	}
      else
	p->stackJoin(currently_selected_stack, stack);
      currently_selected_stack->sortForViewing(true);
    }
  else
    {
      p->stackSplitArmy(stack, army);
      stack->sortForViewing(true);
    }
  on_stack_info_changed(s);
  group_ungroup_toggle->set_sensitive(true);
  stack_composition_modified.emit(s);
}

void StackTileBox::on_group_toggled(Gtk::ToggleButton *toggle)
{
  if (toggle->get_sensitive() == false)
    return;
  bool active = toggle->get_active();
      
  clear_army_buttons();
      
  StackTile *s = GameMap::getStacks(currently_selected_stack->getPos());
  stack_tile_group_toggle.emit(true);
  if (active)
    {
      s->group(Playerlist::getActiveplayer(), currently_selected_stack);
      currently_selected_stack->sortForViewing(true);
    }
  else
    s->ungroup(Playerlist::getActiveplayer());
  stack_tile_group_toggle.emit(false);

  on_stack_info_changed(currently_selected_stack);
  stack_composition_modified.emit(currently_selected_stack);
}

void StackTileBox::clear_army_buttons()
{
  for (stack_army_buttons_type::iterator i = stack_army_buttons.begin(),
       end = stack_army_buttons.end(); i != end; ++i)
    delete *i;
  stack_army_buttons.clear();
}

void StackTileBox::show_stack(StackTile *s)
{
  Player *p = Playerlist::getActiveplayer();
  stack_army_buttons.clear(); 
  std::list<Stack *> stks;
  stks = s->getFriendlyStacks(p);
  unsigned int count= 0;
	    
  guint32 colour_id = 0;
  if (colour_id == p->getId())
    colour_id = Shield::get_next_shield(colour_id);
  for (std::list<Stack *>::iterator j = stks.begin(); j != stks.end(); j++)
    {
      bool first = true;
      for (Stack::iterator i = (*j)->begin(); i != (*j)->end(); ++i)
	{
          Stack *stack = NULL;
	  if (first == true)
            {
              first = false;
              stack = *j;
            }

          bool toggled = (*j) == currently_selected_stack;
          StackArmyButton *button = 
            StackArmyButton::create(d_factor, stack, *i, 
                                    toggled ? p->getId() : colour_id, 
                                    toggled);
          if (stack)
            button->update_stack_button((*j) == currently_selected_stack);
          Gtk::VBox *box = new Gtk::VBox();
          button->get_parent()->remove(*button);
          box->pack_start(*Gtk::manage(button), Gtk::PACK_SHRINK);
          button->reparent(*box);
	  button->army_toggled.connect
	    (sigc::bind(sigc::mem_fun(*this, &StackTileBox::on_army_toggled),
			button, *j, *i));
          button->stack_clicked.connect
            (sigc::bind(sigc::mem_fun(*this, &StackTileBox::on_stack_toggled),
                        button, *j));
          stack_army_buttons.push_back(button);
          stack_info_box->pack_start(*Gtk::manage(box), Gtk::PACK_SHRINK);
	  count++;
	}

      colour_id = Shield::get_next_shield(colour_id);
      if (colour_id== p->getId())
        colour_id = Shield::get_next_shield(colour_id);
    }

  for (unsigned int i = count ; i < MAX_ARMIES_ON_A_SINGLE_TILE; i++)
    {
      StackArmyButton *button = StackArmyButton::create(d_factor, NULL, NULL,
                                                        Shield::NEUTRAL, false);
      Gtk::VBox *box = new Gtk::VBox();
      button->get_parent()->remove(*button);
      box->pack_start(*Gtk::manage(button), Gtk::PACK_SHRINK);
      button->reparent(*box);
      button->set_sensitive(false);
      stack_army_buttons.push_back(button);
      stack_info_box->pack_start(*Gtk::manage(box), Gtk::PACK_SHRINK);
    }

  stack_info_box->show_all();
  fill_in_group_info(s, currently_selected_stack);
  stack_info_container->show_all();
}

void StackTileBox::toggle_group_ungroup()
{
  group_ungroup_toggle->set_active(!group_ungroup_toggle->get_active());
}

void StackTileBox::fill_in_group_info (StackTile *stile, Stack *s)
{
  guint32 bonus = s->calculateMoveBonus();
  GraphicsCache *gc = GraphicsCache::getInstance();
  terrain_image->property_pixbuf() = gc->getMoveBonusPic(bonus, s->hasShip())->to_pixbuf();
  group_moves_label->set_markup(String::ucompose("<b>%1</b>", s->getMoves()));
  group_ungroup_toggle->set_sensitive(false);
  if (stile->getFriendlyStacks(s->getOwner()).size() != 1)
    group_ungroup_toggle->set_active(false);
  else
    group_ungroup_toggle->set_active(true);
  if (group_ungroup_toggle->get_active() == true)
    group_ungroup_toggle->set_label(_("UnGrp"));
  else
    group_ungroup_toggle->set_label(_("Grp"));
  group_ungroup_toggle->set_sensitive(true);
}

void StackTileBox::reset()
{
  clear_army_buttons();
}
  
void StackTileBox::on_stack_info_changed(Stack *s)
{
  clear_army_buttons();

  set_selected_stack(s);

  if (s->getOwner()->getType() == Player::HUMAN)
    {
      StackTile *stile = GameMap::getStacks(s->getPos());
      stile->setDefending(s->getOwner(), false);
      stile->setParked(s->getOwner(), false);
      show_stack(stile);
    }
}
