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

#include <sigc++/functors/mem_fun.h>

#include "stack-info-dialog.h"

#include <gtkmm.h>
#include "glade-helpers.h"
#include "input-helpers.h"
#include "image-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "army.h"
#include "player.h"
#include "armysetlist.h"
#include "playerlist.h"
#include "stack.h"
#include "GraphicsCache.h"
#include "Tile.h"
#include <assert.h>
#include "stacktile.h"
#include "GameMap.h"

StackInfoDialog::StackInfoDialog(Gtk::Window &parent, Vector<int> pos)
{
  army_info_tip = NULL;
  tile = pos;
  currently_selected_stack = NULL;
    
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/stack-info-dialog.ui");

    xml->get_widget("dialog", dialog);
    dialog->set_transient_for(parent);
    xml->get_widget("stack_table", stack_table);
    xml->get_widget("group_button", group_button);
    group_button->signal_clicked().connect
      (sigc::mem_fun(*this, &StackInfoDialog::on_group_clicked));
    xml->get_widget("ungroup_button", ungroup_button);
    ungroup_button->signal_clicked().connect
      (sigc::mem_fun(*this, &StackInfoDialog::on_ungroup_clicked));
    fill_stack_info();
}

StackInfoDialog::~StackInfoDialog()
{
  if (army_info_tip != NULL)
    delete army_info_tip;
  delete dialog;
}

void StackInfoDialog::hide()
{
  dialog->hide();
}

void StackInfoDialog::run()
{
    static int width = -1;
    static int height = -1;

    if (width != -1 && height != -1)
	dialog->set_default_size(width, height);
    
    dialog->show();
    dialog->run();

    dialog->get_size(width, height);

}

void StackInfoDialog::addStack(Stack *s, guint32 &idx)
{
  s->sortForViewing(true);
  Stack *target = new Stack(Playerlist::getInstance()->getNeutral(), s->getPos());
  ArmyProto *baseproto = ArmyProto::createScout();
  Army *army = new Army(*baseproto);
  delete baseproto;
  target->add(army);
  Fight fight(s, target, Fight::FOR_KICKS);
  delete target;

  bool first = true;
  guint32 colour_id = 0;
  if (colour_id == s->getOwner()->getId())
    colour_id = Shield::get_next_shield(colour_id);
  for (Stack::iterator it = s->begin(); it != s->end(); it++)
    {
      guint32 str = fight.getModifiedStrengthBonus(*it);
      addArmy(first, s, *it, str, idx, colour_id);
      if (first == true)
	first = false;
      idx++;
      colour_id = Shield::get_next_shield(colour_id);
      if (colour_id == s->getOwner()->getId())
        colour_id = Shield::get_next_shield(colour_id);
    }
}

void StackInfoDialog::addArmy (bool first, Stack *s, Army *h, guint32 modified_strength, int idx, guint32 colour_id)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  Player *player = h->getOwner();
    
  bool greyed_out = s->getId() != currently_selected_stack->getId();
  Gtk::ToggleButton *toggle = manage(new Gtk::ToggleButton);
  Glib::RefPtr<Gdk::Pixbuf> pixbuf= 
    gc->getCircledArmyPic(player->getArmyset(), h->getTypeId(), player, NULL,
                          greyed_out, !greyed_out ? player->getId() : colour_id,
                          true)->to_pixbuf();
  
  Gtk::Image *image = NULL;
  guint32 move_bonus = h->getStat(Army::MOVE_BONUS);
  bool ship = h->getStat(Army::SHIP);
  if (ship || move_bonus == (Tile::GRASS | Tile::WATER | Tile::FOREST | 
			     Tile::HILLS | Tile::SWAMP | Tile::MOUNTAIN))
    {
      image = new Gtk::Image();
      image->property_pixbuf() = gc->getMoveBonusPic(move_bonus, ship)->to_pixbuf();
    }

  armies.push_back(h);
  toggle->set_active(s->getId() == currently_selected_stack->getId());
  toggle->signal_toggled().connect
    (sigc::bind(sigc::mem_fun(this, 
			      &StackInfoDialog::on_army_toggled), toggle, s, h));
  toggle->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
  toggles.push_back(toggle);
  toggle->signal_button_press_event().connect
    (sigc::bind(sigc::mem_fun(*this, &StackInfoDialog::on_army_button_event),
		toggle), false);
  toggle->signal_button_release_event().connect
    (sigc::bind(sigc::mem_fun(*this, &StackInfoDialog::on_army_button_event),
		toggle), false);
  Gtk::Image *army_image = new Gtk::Image();
  army_image->property_pixbuf() = pixbuf;
  toggle->add(*manage(army_image));

  Gtk::Label *name = new Gtk::Label(h->getName());
  Glib::ustring str = "";
   
  unsigned int strength_value = h->getStat(Army::STRENGTH);
  str = String::ucompose("%1", strength_value);
  if (modified_strength != strength_value)
    str += String::ucompose(" (%1)", modified_strength);
      
  Gtk::Label *strength = new Gtk::Label(str);

  Gtk::Label *bonus = new Gtk::Label(h->getArmyBonusDescription());
  Gtk::Label *moves = new Gtk::Label(String::ucompose("%1", h->getMoves()));
	
  if (first)
    {
      Gtk::RadioButton *radio;
      if (radios.size() > 0)
	{
	  Gtk::RadioButtonGroup b;
	  b = radios.front()->get_group();
	  radio = new Gtk::RadioButton(b);
	}
      else
	radio = new Gtk::RadioButton();
      radios.push_back(radio);
      radio->set_active(s->getId() == currently_selected_stack->getId());
      radio->signal_toggled().connect
	(sigc::bind(sigc::mem_fun(this, &StackInfoDialog::on_stack_toggled), 
		    radio, s));
      stack_table->attach(*manage(radio), 0, 1, idx, idx + 1,
			  Gtk::SHRINK, Gtk::SHRINK);
    }

  stack_table->attach(*manage(toggle), 1, 2, idx, idx + 1,
		      Gtk::SHRINK, Gtk::SHRINK);
  stack_table->attach(*manage(name), 2, 3, idx, idx + 1,
		      Gtk::SHRINK, Gtk::SHRINK);
  stack_table->attach(*manage(strength), 3, 4, idx, idx+1,
		      Gtk::SHRINK, Gtk::SHRINK);
  stack_table->attach(*manage(moves), 4, 5, idx, idx+1,
		      Gtk::SHRINK, Gtk::SHRINK);
  if (image)
    stack_table->attach(*manage(image), 5, 6, idx, idx+1,
			Gtk::SHRINK, Gtk::SHRINK);
  stack_table->attach(*manage(bonus), 6, 7, idx, idx + 1,
		      Gtk::SHRINK, Gtk::SHRINK);
          
}
       
void StackInfoDialog::on_group_clicked()
{
  StackTile *stile = GameMap::getStacks(tile);
  Stack *stack = stile->group(Playerlist::getActiveplayer());
  currently_selected_stack = stack;
  stack->sortForViewing(true);
  fill_stack_info();
}

void StackInfoDialog::on_ungroup_clicked()
{
  StackTile *stile = GameMap::getStacks(tile);
  stile->ungroup(Playerlist::getActiveplayer());
  fill_stack_info();
}

void StackInfoDialog::fill_stack_info()
{
      
  StackTile *stile = GameMap::getStacks(tile);
  guint32 idx = 1;

  armies.clear();
  toggles.clear();
  radios.clear();
  stack_table->foreach(sigc::mem_fun(stack_table, &Gtk::Container::remove));
  stack_table->resize(6, MAX_ARMIES_ON_A_SINGLE_TILE);

  Pango::AttrList attrs;
  Pango::Attribute weight = Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
  attrs.insert(weight);
  Gtk::Label *str = new Gtk::Label(_("Str"));
  str->set_attributes(attrs);
  stack_table->attach(*manage(str), 3, 4, 0, 1, Gtk::SHRINK, Gtk::SHRINK);

  Gtk::Label *moves = new Gtk::Label(_("Move"));
  moves->set_attributes(attrs);
  stack_table->attach(*manage(moves), 4, 5, 0, 1, Gtk::SHRINK, Gtk::SHRINK);

  Gtk::Label *bonus = new Gtk::Label(_("Bonus"));
  bonus->set_attributes(attrs);
  stack_table->attach(*manage(bonus), 6, 7, 0, 1, Gtk::SHRINK, Gtk::SHRINK);

  std::list<Stack*> stks;
  stks = stile->getFriendlyStacks(Playerlist::getActiveplayer());
  if (currently_selected_stack == NULL)
    currently_selected_stack = stks.front();
  for (std::list<Stack *>::iterator i = stks.begin(); i != stks.end(); i++)
    addStack(*i, idx);
  stack_table->show_all();
}

bool StackInfoDialog::on_army_button_event(GdkEventButton *e, Gtk::ToggleButton *toggle)
{
  MouseButtonEvent event = to_input_event(e);
  if (event.button == MouseButtonEvent::RIGHT_BUTTON
      && event.state == MouseButtonEvent::PRESSED) 
    {
      int slot = -1;
      for (unsigned int i = 0; i < toggles.size(); ++i) 
	{
	  if (toggle == toggles[i])
	    slot = i;
	}
      assert(slot != -1);

      const Army *army = armies[slot];

      if (army)
	{
	  if (army_info_tip != NULL)
	    delete army_info_tip;
	  army_info_tip = new ArmyInfoTip(toggle, army);
	}
      return true;
    }
  else if (event.button == MouseButtonEvent::RIGHT_BUTTON
	   && event.state == MouseButtonEvent::RELEASED) 
    {
      if (army_info_tip != NULL)
	{
      	  delete army_info_tip;
	  army_info_tip = NULL;
	}
      return true;
    }

  return false;
}

void StackInfoDialog::on_army_toggled(Gtk::ToggleButton *toggle, Stack *stack, Army *army)
{
  Player *p = Playerlist::getActiveplayer();
  group_button->set_sensitive(false);
  ungroup_button->set_sensitive(false);
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
  group_button->set_sensitive(true);
  ungroup_button->set_sensitive(true);
  fill_stack_info();
}

void StackInfoDialog::on_stack_toggled(Gtk::RadioButton *radio, Stack *stack)
{
  if (radio->get_active() == true)
    {
      if (stack == currently_selected_stack)
	return;
      currently_selected_stack = stack;
      fill_stack_info();
    }
}

