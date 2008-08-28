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
#include <sigc++/functors/mem_fun.h>

#include "stack-info-dialog.h"

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

StackInfoDialog::StackInfoDialog(Stack *s)
{
  stack = s;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/stack-info-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    decorate(dialog.get());
    window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));

    xml->get_widget("stack_table", stack_table);

    xml->connect_clicked
      ("group_button", 
       sigc::mem_fun(*this, &StackInfoDialog::on_group_clicked));
    xml->connect_clicked
      ("ungroup_button", 
       sigc::mem_fun(*this, &StackInfoDialog::on_ungroup_clicked));
    fill_stack_info();
}

void StackInfoDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
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

void StackInfoDialog::addArmy (Army *h, Uint32 modified_strength, int idx)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  Player *player = h->getOwner();
    
  Gtk::ToggleButton *toggle = manage(new Gtk::ToggleButton);
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
    to_pixbuf(gc->getArmyPic(player->getArmyset(), h->getTypeId(), player, 
			     NULL));
  
  Gtk::Image *image = NULL;
  Uint32 move_bonus = h->getStat(Army::MOVE_BONUS);
  bool ship = h->getStat(Army::SHIP);
  if (ship || move_bonus == (Tile::GRASS | Tile::WATER | Tile::FOREST | 
			     Tile::HILLS | Tile::SWAMP | Tile::MOUNTAIN))
    image = new Gtk::Image(to_pixbuf(gc->getMoveBonusPic(move_bonus, ship)));

  armies.push_back(h);
  toggle->set_active(h->isGrouped());
  toggle->signal_toggled().connect
    (sigc::bind(sigc::mem_fun(this, 
			      &StackInfoDialog::on_army_toggled), toggle));
  toggle->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
  toggles.push_back(toggle);
  toggle->signal_button_press_event().connect
    (sigc::bind(sigc::mem_fun(*this, &StackInfoDialog::on_army_button_event),
		toggle), false);
  toggle->signal_button_release_event().connect
    (sigc::bind(sigc::mem_fun(*this, &StackInfoDialog::on_army_button_event),
		toggle), false);
  toggle->add(*manage(new Gtk::Image(pixbuf)));

  Gtk::Label *name = new Gtk::Label(h->getName());
  Glib::ustring s = "";
   
  unsigned int strength_value = h->getStat(Army::STRENGTH);
  s = String::ucompose("%1", strength_value);
  if (h->isGrouped() == true && modified_strength != strength_value)
    s += String::ucompose(" (%1)", modified_strength);
      
  Gtk::Label *strength = new Gtk::Label(s);

  Gtk::Label *bonus = new Gtk::Label(h->getArmyBonusDescription());
  s = String::ucompose("%1", h->getMoves());
  Gtk::Label *moves = new Gtk::Label(s);
	
  stack_table->attach(*manage(toggle), 0, 1, idx, idx + 1,
		      Gtk::SHRINK, Gtk::SHRINK);
  stack_table->attach(*manage(name), 1, 2, idx, idx + 1,
		      Gtk::SHRINK, Gtk::SHRINK);
  stack_table->attach(*manage(strength), 2, 3, idx, idx+1,
		      Gtk::SHRINK, Gtk::SHRINK);
  stack_table->attach(*manage(moves), 3, 4, idx, idx+1,
		      Gtk::SHRINK, Gtk::SHRINK);
  if (image)
    stack_table->attach(*manage(image), 4, 5, idx, idx+1,
			Gtk::SHRINK, Gtk::SHRINK);
  stack_table->attach(*manage(bonus), 5, 6, idx, idx + 1,
		      Gtk::SHRINK, Gtk::SHRINK);
          
}
       
void StackInfoDialog::on_group_clicked()
{
  stack->group();
  fill_stack_info();
}

void StackInfoDialog::on_ungroup_clicked()
{
  stack->ungroup();
  fill_stack_info();
}

void StackInfoDialog::fill_stack_info()
{
  int idx = 1;
  Stack *target = new Stack(*stack);
  //very delicately remove the armies that aren't grouped so that they 
  //don't get any boni
  std::list<Army*> ungrouped;
  for (Stack::iterator it = stack->begin(); it != stack->end(); it++)
    {
      if ((*it)->isGrouped() == false)
	{
	  ungrouped.push_back(*it);
	  stack->erase(it);
	  it = stack->begin();
	}
    }
  Fight fight(stack, target, Fight::FOR_KICKS);
  //now add them back
  for(std::list<Army*>::iterator it = ungrouped.begin(); 
      it != ungrouped.end(); it++)
    stack->push_back(*it);
  ungrouped.clear();
  stack->sortForViewing(true);

  armies.clear();
  toggles.clear();
  stack_table->foreach(sigc::mem_fun(stack_table, &Gtk::Container::remove));
  stack_table->resize(5, MAX_STACK_SIZE);

  Gtk::Label *str = new Gtk::Label(_("Str"));
  stack_table->attach(*manage(str), 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK);

  Gtk::Label *moves = new Gtk::Label(_("Move"));
  stack_table->attach(*manage(moves), 3, 4, 0, 1, Gtk::SHRINK, Gtk::SHRINK);

  Gtk::Label *bonus = new Gtk::Label(_("Bonus"));
  stack_table->attach(*manage(bonus), 5, 6, 0, 1, Gtk::SHRINK, Gtk::SHRINK);

  for (Stack::iterator it = stack->begin(); it != stack->end(); it++)
    {
      Uint32 str = fight.getModifiedStrengthBonus(*it);
      addArmy(*it, str, idx);
      idx++;
    }
  stack_table->show_all();
  delete target;
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
	army_info_tip.reset(new ArmyInfoTip(toggle, army));
      return true;
    }
  else if (event.button == MouseButtonEvent::RIGHT_BUTTON
	   && event.state == MouseButtonEvent::RELEASED) 
    {
      army_info_tip.reset();
      return true;
    }

  return false;
}

void StackInfoDialog::on_army_toggled(Gtk::ToggleButton *toggle)
{
  //which army has this toggle?
  Stack::iterator it = stack->begin();
  for (unsigned int i = 0; i < toggles.size(); ++i) 
    {
      if (toggle == toggles[i])
	{
	  //found the toggle, now find the army.
	  for (unsigned int j = 0; j < i; j++)
	    it++;
	  (*it)->setGrouped(!(*it)->isGrouped());
	  stack->sortForViewing(true);
	  fill_stack_info();
	  break;
	}
    }
}

