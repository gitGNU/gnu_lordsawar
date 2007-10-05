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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include <config.h>

#include <iostream>
#include <iomanip>
#include <assert.h>
#include <libgen.h>

#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>

#include <gtkmm/widget.h>
#include <gtkmm/menuitem.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/image.h>
#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/stock.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/menu.h>
#include <gtkmm/menuitem.h>

#include "armyset-window.h"

#include "../gui/gtksdl.h"
#include "../gui/image-helpers.h"
#include "../gui/input-helpers.h"
#include "../gui/error-utils.h"

#include "../defs.h"
#include "../armysetlist.h"
#include "../Tile.h"

#include "../ucompose.hpp"

#include "glade-helpers.h"


ArmySetWindow::ArmySetWindow()
{
  d_armyset = NULL;
    sdl_inited = false;
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/armyset-window.glade");

    Gtk::Window *w = 0;
    xml->get_widget("window", w);
    window.reset(w);

    xml->get_widget("army_image", army_image);
    xml->get_widget("name_entry", name_entry);
    xml->get_widget("armies_treeview", armies_treeview);
    xml->get_widget("description_textview", description_textview);
    xml->get_widget("image_filechooserbutton", image_filechooserbutton);
    xml->get_widget("production_spinbutton", production_spinbutton);
    production_spinbutton->signal_changed().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_production_changed));
    xml->get_widget("cost_spinbutton", cost_spinbutton);
    cost_spinbutton->signal_changed().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_cost_changed));
    xml->get_widget("upkeep_spinbutton", upkeep_spinbutton);
    upkeep_spinbutton->signal_changed().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_upkeep_changed));
    xml->get_widget("strength_spinbutton", strength_spinbutton);
    strength_spinbutton->signal_changed().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_strength_changed));
    xml->get_widget("moves_spinbutton", moves_spinbutton);
    moves_spinbutton->signal_changed().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_moves_changed));
    xml->get_widget("exp_spinbutton", exp_spinbutton);
    exp_spinbutton->signal_changed().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_exp_changed));
    xml->get_widget("hero_checkbutton", hero_checkbutton);
    hero_checkbutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_hero_toggled));
    xml->get_widget("awardable_checkbutton", awardable_checkbutton);
    awardable_checkbutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_awardable_toggled));
    xml->get_widget("defends_ruins_checkbutton", defends_ruins_checkbutton);
    defends_ruins_checkbutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_defends_ruins_toggled));
    xml->get_widget("sight_spinbutton", sight_spinbutton);
    sight_spinbutton->signal_changed().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_sight_changed));
    xml->get_widget("move_forests_checkbutton", move_forests_checkbutton);
    move_forests_checkbutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_move_forests_toggled));
    xml->get_widget("move_marshes_checkbutton", move_marshes_checkbutton);
    move_marshes_checkbutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_move_marshes_toggled));
    xml->get_widget("move_hills_checkbutton", move_hills_checkbutton);
    move_hills_checkbutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_move_hills_toggled));
    xml->get_widget("move_mountains_checkbutton", move_mountains_checkbutton);
    move_mountains_checkbutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_move_mountains_toggled));
    xml->get_widget("can_fly_checkbutton", can_fly_checkbutton);
    can_fly_checkbutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_can_fly_toggled));
    xml->get_widget("add1strinopen_checkbutton", add1strinopen_checkbutton);
    add1strinopen_checkbutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_add1strinopen_toggled));
    xml->get_widget("add2strinopen_checkbutton", add2strinopen_checkbutton);
    add2strinopen_checkbutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_add2strinopen_toggled));
    xml->get_widget("add1strinforest_checkbutton", add1strinforest_checkbutton);
    add1strinforest_checkbutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_add1strinforest_toggled));
    xml->get_widget("add1strinhills_checkbutton", add1strinhills_checkbutton);
    add1strinhills_checkbutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_add1strinhills_toggled));
    xml->get_widget("add1strincity_checkbutton", add1strincity_checkbutton);
    add1strincity_checkbutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_add1strincity_toggled));
    xml->get_widget("add2strincity_checkbutton", add2strincity_checkbutton);
    add2strincity_checkbutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_add2strincity_toggled));
    xml->get_widget("add1stackinhills_checkbutton", 
		    add1stackinhills_checkbutton);
    add1stackinhills_checkbutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_add1stackinhills_toggled));
    xml->get_widget("suballcitybonus_checkbutton", suballcitybonus_checkbutton);
    suballcitybonus_checkbutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_suballcitybonus_toggled));
    xml->get_widget("sub1enemystack_checkbutton", sub1enemystack_checkbutton);
    sub1enemystack_checkbutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_sub1enemystack_toggled));
    xml->get_widget("add1stack_checkbutton", add1stack_checkbutton);
    add1stack_checkbutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_add1stack_toggled));
    xml->get_widget("add2stack_checkbutton", add2stack_checkbutton);
    add2stack_checkbutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_add2stack_toggled));
    xml->get_widget("suballnonherobonus_checkbutton", 
		    suballnonherobonus_checkbutton);
    suballnonherobonus_checkbutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_suballnonherobonus_toggled));
    xml->get_widget("suballherobonus_checkbutton", suballherobonus_checkbutton);
    suballherobonus_checkbutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_suballherobonus_toggled));
    xml->get_widget("add_army_button", add_army_button);
    xml->get_widget("remove_army_button", remove_army_button);
    xml->get_widget("army_vbox", army_vbox);
    // connect callbacks for the menu
    xml->connect_clicked
      ("load_armyset_menuitem",
       sigc::mem_fun(this, &ArmySetWindow::on_load_armyset_activated));
    xml->connect_clicked
      ("save_armyset_menuitem",
       sigc::mem_fun(this, &ArmySetWindow::on_save_armyset_activated));
    xml->connect_clicked
      ("save_armyset_as_menuitem", 
       sigc::mem_fun(this, &ArmySetWindow::on_save_armyset_as_activated));
    xml->connect_clicked 
      ("quit_menuitem", 
       sigc::mem_fun(this, &ArmySetWindow::on_quit_activated));
    xml->connect_clicked
      ("edit_armyset_info_menuitem", 
       sigc::mem_fun(this, &ArmySetWindow::on_edit_armyset_info_activated));

    w->signal_delete_event().connect(
	sigc::mem_fun(*this, &ArmySetWindow::on_delete_event));

    armies_list = Gtk::ListStore::create(armies_columns);
    armies_treeview->set_model(armies_list);
    armies_treeview->append_column("", armies_columns.name);
    armies_treeview->set_headers_visible(false);
    armies_treeview->get_selection()->signal_changed().connect
      (sigc::mem_fun(*this, &ArmySetWindow::on_army_selected));
    armies_treeview->set_reorderable(true);

    update_army_panel();
    update_armyset_buttons();

    xml->get_widget("sdl_container", sdl_container);
}

void
ArmySetWindow::update_armyset_buttons()
{
  if (!armies_treeview->get_selection()->get_selected())
    remove_army_button->set_sensitive(false);
  else
    remove_army_button->set_sensitive(true);
}

void
ArmySetWindow::update_army_panel()
{
  //if nothing selected in the treeview, then we don't show anything in
  //the army panel
  if (armies_treeview->get_selection()->get_selected() == 0)
    {
      army_vbox->set_sensitive(false);
      return;
    }
  army_vbox->set_sensitive(true);
}
ArmySetWindow::~ArmySetWindow()
{
}

void ArmySetWindow::show()
{
    sdl_container->show_all();
    window->show();
}

void ArmySetWindow::hide()
{
    window->hide();
}

namespace 
{
    void surface_attached_helper(GtkSDL *gtksdl, gpointer data)
    {
	static_cast<ArmySetWindow *>(data)->on_sdl_surface_changed();
    }
}

void ArmySetWindow::init(int width, int height)
{
    sdl_widget
	= Gtk::manage(Glib::wrap(gtk_sdl_new(width, height, 0, SDL_SWSURFACE)));

    sdl_widget->set_flags(Gtk::CAN_FOCUS);

    sdl_widget->grab_focus();
    sdl_widget->add_events(Gdk::KEY_PRESS_MASK |
		  Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
	          Gdk::POINTER_MOTION_MASK | Gdk::LEAVE_NOTIFY_MASK);

    // connect to the special signal that signifies that a new surface has been
    // generated and attached to the widget
    g_signal_connect(G_OBJECT(sdl_widget->gobj()), "surface-attached",
		     G_CALLBACK(surface_attached_helper), this);
    
    sdl_container->add(*sdl_widget);
}

bool ArmySetWindow::on_delete_event(GdkEventAny *e)
{
    hide();
    
    return true;
}


bool ArmySetWindow::load(std::string tag, XML_Helper *helper)
{
  if (tag == "armyset")
    d_armyset = new Armyset(helper);
  return true;
}

void ArmySetWindow::on_load_armyset_activated()
{
    Gtk::FileChooserDialog chooser(*window.get(), 
				   _("Choose an Armyset to Load"));
    Gtk::FileFilter sav_filter;
    sav_filter.add_pattern("*.xml");
    chooser.set_filter(sav_filter);
    chooser.set_current_folder(Configuration::s_savePath);

    chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
    chooser.set_default_response(Gtk::RESPONSE_ACCEPT);
	
    chooser.show_all();
    int res = chooser.run();
    
    if (res == Gtk::RESPONSE_ACCEPT)
      {
	current_save_filename = chooser.get_filename();
	chooser.hide();

	XML_Helper helper(current_save_filename, std::ios::in, false);

	helper.registerTag("armyset", 
			   sigc::mem_fun((*this), &ArmySetWindow::load));

  
	if (!helper.parse())
	  {
	    std::cerr <<_("Error, while loading an armyset. Armyset Name: ");
	    std::cerr <<current_save_filename <<std::endl <<std::flush;
	    exit(-1);
	  }

	char *dir = strdup(current_save_filename.c_str());
	dir = basename (dir);
	char *tmp = strchr (dir, '.');
	if (tmp)
	  tmp[0] = '\0';
	d_armyset->setSubDir(dir);
	d_armyset->instantiatePixmaps();
	Uint32 max = d_armyset->getSize();
	for (unsigned int i = 0; i < max; i++)
	  addArmyType(i);
	if (max)
	  {
	    Gtk::TreeModel::Row row;
	    row = armies_treeview->get_model()->children()[0];
	    if(row)
	      armies_treeview->get_selection()->select(row);
	  }
      }
    update_armyset_buttons();
    update_army_panel();
}

void ArmySetWindow::on_save_armyset_activated()
{
  if (current_save_filename.empty())
    on_save_armyset_as_activated();
  else
    {
      XML_Helper helper(current_save_filename, std::ios::out, false);
      helper.openTag("armyset");
      d_armyset->save(&helper);
      helper.closeTag();
      helper.close();
    }
}

void ArmySetWindow::on_save_armyset_as_activated()
{
  Gtk::FileChooserDialog chooser(*window.get(), _("Choose a Name"),
				 Gtk::FILE_CHOOSER_ACTION_SAVE);
  Gtk::FileFilter sav_filter;
  sav_filter.add_pattern("*.xml");
  chooser.set_filter(sav_filter);
  chooser.set_current_folder(Configuration::s_savePath);

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);

  chooser.show_all();
  int res = chooser.run();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      current_save_filename = chooser.get_filename();
      chooser.hide();
      on_save_armyset_activated();
    }
}

void ArmySetWindow::on_quit_activated()
{
  // FIXME: ask
  bool end = true;

  if (end) {
  }
  window->hide();
}
void ArmySetWindow::on_edit_armyset_info_activated()
{
  //ArmySetInfoDialog d;
  //d.set_parent_window(*window.get());
  //d.run();
}
void ArmySetWindow::addArmyType(Uint32 army_type)
{
  Army *a;
  //go get army_type in d_armyset
  a = d_armyset->lookupArmyByType(army_type);
  Gtk::TreeIter i = armies_list->append();
  (*i)[armies_columns.name] = a->getName();
  (*i)[armies_columns.army] = a;
}

void ArmySetWindow::on_sdl_surface_changed()
{
  if (!sdl_inited) {
    sdl_inited = true;
    sdl_initialized.emit();
  }
}

void ArmySetWindow::on_army_selected()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) {
    // Row selected
    Gtk::TreeModel::Row row = *iterrow;

    Army *a = row[armies_columns.army];
    fill_army_info(a);
  }
  update_army_panel();
  update_armyset_buttons();
}

void ArmySetWindow::fill_army_info(Army *army)
{
  army_image->property_pixbuf() = to_pixbuf(army->getPixmap());
  name_entry->set_text(army->getName());
  description_textview->get_buffer()->set_text(army->getDescription());
  production_spinbutton->set_value(army->getProduction());
  cost_spinbutton->set_value(army->getProductionCost());
  upkeep_spinbutton->set_value(army->getUpkeep());
  strength_spinbutton->set_value(army->getStat(Army::STRENGTH, false));
  moves_spinbutton->set_value(army->getStat(Army::MOVES, false));
  exp_spinbutton->set_value(int(army->getXpReward()));
  hero_checkbutton->set_active(army->isHero());
  awardable_checkbutton->set_active(army->getAwardable());
  defends_ruins_checkbutton->set_active(army->getDefendsRuins());
  sight_spinbutton->set_value(army->getStat(Army::SIGHT, false));

  Uint32 bonus = army->getMoveBonus();
  can_fly_checkbutton->set_active (bonus == 
				   (Tile::GRASS | Tile::WATER | 
				    Tile::FOREST | Tile::HILLS | 
				    Tile::MOUNTAIN | Tile::SWAMP));
  if (can_fly_checkbutton->get_active() == false)
    {
      move_forests_checkbutton->set_active
	((bonus & Tile::FOREST) == Tile::FOREST);
      move_marshes_checkbutton->set_active
	((bonus & Tile::SWAMP) == Tile::SWAMP);
      move_hills_checkbutton->set_active
	((bonus & Tile::HILLS) == Tile::HILLS);
      move_mountains_checkbutton->set_active
	((bonus & Tile::MOUNTAIN) == Tile::MOUNTAIN);
    }
  else
    {
      move_forests_checkbutton->set_active(false);
      move_marshes_checkbutton->set_active(false);
      move_hills_checkbutton->set_active(false);
      move_mountains_checkbutton->set_active(false);
    }
  bonus = army->getStat(Army::ARMY_BONUS, false);
  add1strinopen_checkbutton->set_active
    ((bonus & Army::ADD1STRINOPEN) == Army::ADD1STRINOPEN);
  add2strinopen_checkbutton->set_active
    ((bonus & Army::ADD2STRINOPEN) == Army::ADD2STRINOPEN);
  add1strinforest_checkbutton->set_active
    ((bonus & Army::ADD1STRINFOREST) == Army::ADD1STRINFOREST);
  add1strinhills_checkbutton->set_active
    ((bonus & Army::ADD1STRINHILLS) == Army::ADD1STRINHILLS);
  add1strincity_checkbutton->set_active
    ((bonus & Army::ADD1STRINCITY) == Army::ADD1STRINCITY);
  add2strincity_checkbutton->set_active
    ((bonus & Army::ADD2STRINCITY) == Army::ADD2STRINCITY);
  add1stackinhills_checkbutton->set_active
    ((bonus & Army::ADD1STACKINHILLS) == Army::ADD1STACKINHILLS);
  suballcitybonus_checkbutton->set_active
    ((bonus & Army::SUBALLCITYBONUS) == Army::SUBALLCITYBONUS);
  sub1enemystack_checkbutton->set_active
    ((bonus & Army::SUB1ENEMYSTACK) == Army::SUB1ENEMYSTACK);
  add1stack_checkbutton->set_active
    ((bonus & Army::ADD1STACK) == Army::ADD1STACK);
  add2stack_checkbutton->set_active
    ((bonus & Army::ADD2STACK) == Army::ADD2STACK);
  suballnonherobonus_checkbutton->set_active
    ((bonus & Army::SUBALLNONHEROBONUS) == Army::SUBALLNONHEROBONUS);
  suballherobonus_checkbutton->set_active
    ((bonus & Army::SUBALLHEROBONUS) == Army::SUBALLHEROBONUS);
}

void ArmySetWindow::on_production_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      a->setProduction(int(production_spinbutton->get_value()));
    }
}
void ArmySetWindow::on_cost_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      a->setProductionCost(int(cost_spinbutton->get_value()));
    }
}
void ArmySetWindow::on_upkeep_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      a->setUpkeep(int(upkeep_spinbutton->get_value()));
    }
}
void ArmySetWindow::on_strength_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      a->setStat(Army::STRENGTH, int(strength_spinbutton->get_value()));
    }
}
void ArmySetWindow::on_moves_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      a->setStat(Army::MOVES, int(moves_spinbutton->get_value()));
    }
}
void ArmySetWindow::on_exp_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      a->setXpReward(int(exp_spinbutton->get_value()));
    }
}
void ArmySetWindow::on_sight_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      a->setStat(Army::SIGHT, int(sight_spinbutton->get_value()));
    }
}
void ArmySetWindow::on_hero_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      //a->setHero(hero_checkbutton->get_active());
    }
}
void ArmySetWindow::on_awardable_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      a->setAwardable(awardable_checkbutton->get_active());
    }
}
void ArmySetWindow::on_defends_ruins_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      a->setDefendsRuins(defends_ruins_checkbutton->get_active());
    }
}
void ArmySetWindow::on_move_forests_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      Uint32 bonus = a->getMoveBonus();
      if (move_forests_checkbutton->get_active() == true)
	{
	  //if (can_fly_checkbutton->get_active())
	    //can_fly_checkbutton->set_active(false);
	  bonus |= Tile::FOREST;
	}
      else
	{
	  if (bonus & Tile::FOREST)
	    bonus ^= Tile::FOREST;
	}
      a->setStat(Army::MOVE_BONUS, bonus);
    }
}
void ArmySetWindow::on_move_marshes_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      Uint32 bonus = a->getMoveBonus();
      if (move_marshes_checkbutton->get_active() == true)
	{
	  //if (can_fly_checkbutton->get_active())
	    //can_fly_checkbutton->set_active(false);
	  bonus |= Tile::SWAMP;
	}
      else
	{
	  if (bonus & Tile::SWAMP)
	    bonus ^= Tile::SWAMP;
	}
      a->setStat(Army::MOVE_BONUS, bonus);
    }
}
void ArmySetWindow::on_move_hills_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      Uint32 bonus = a->getMoveBonus();
      if (move_hills_checkbutton->get_active() == true)
	{
	  //if (can_fly_checkbutton->get_active())
	    //can_fly_checkbutton->set_active(false);
	  bonus |= Tile::HILLS;
	}
      else
	{
	  if (bonus & Tile::HILLS)
	    bonus ^= Tile::HILLS;
	}
      a->setStat(Army::MOVE_BONUS, bonus);
    }
}
void ArmySetWindow::on_move_mountains_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      Uint32 bonus = a->getMoveBonus();
      if (move_mountains_checkbutton->get_active() == true)
	{
	  //if (can_fly_checkbutton->get_active())
	    //can_fly_checkbutton->set_active(false);
	  bonus |= Tile::MOUNTAIN;
	}
      else
	{
	  if (bonus & Tile::MOUNTAIN)
	    bonus ^= Tile::MOUNTAIN;
	}
      a->setStat(Army::MOVE_BONUS, bonus);
    }
}
void ArmySetWindow::on_can_fly_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      Uint32 bonus = a->getMoveBonus();
      if (can_fly_checkbutton->get_active() == true)
	{
	  bonus = (Tile::GRASS | Tile::WATER | Tile::FOREST | Tile::HILLS |
		   Tile::MOUNTAIN | Tile::SWAMP);
	}
      else
	{
	  bonus = 0;
	}
      a->setStat(Army::MOVE_BONUS, bonus);
    }
}
void ArmySetWindow::on_add1strinopen_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      Uint32 bonus = a->getStat(Army::ARMY_BONUS, false);
      if (add1strinopen_checkbutton->get_active() == true)
	bonus |= Army::ADD1STRINOPEN;
      else
	{
	  if (bonus & Army::ADD1STRINOPEN)
	    bonus ^= Army::ADD1STRINOPEN;
	}
      a->setStat(Army::ARMY_BONUS, bonus);
    }
}
void ArmySetWindow::on_add2strinopen_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      Uint32 bonus = a->getStat(Army::ARMY_BONUS, false);
      if (add2strinopen_checkbutton->get_active() == true)
	bonus |= Army::ADD2STRINOPEN ;
      else
	{
	  if (bonus & Army::ADD2STRINOPEN)
	    bonus ^= Army::ADD2STRINOPEN;
	}
      a->setStat(Army::ARMY_BONUS, bonus);
    }
}
void ArmySetWindow::on_add1strinforest_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      Uint32 bonus = a->getStat(Army::ARMY_BONUS, false);
      if (add1strinforest_checkbutton->get_active() == true)
	bonus |= Army::ADD1STRINFOREST;
      else
	{
	  if (bonus & Army::ADD1STRINFOREST)
	    bonus ^= Army::ADD1STRINFOREST;
	}
      a->setStat(Army::ARMY_BONUS, bonus);
    }
}
void ArmySetWindow::on_add1strinhills_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      Uint32 bonus = a->getStat(Army::ARMY_BONUS, false);
      if (add1strinhills_checkbutton->get_active() == true)
	bonus |= Army::ADD1STRINHILLS;
      else
	{
	  if (bonus & Army::ADD1STRINHILLS)
	    bonus ^= Army::ADD1STRINHILLS;
	}
      a->setStat(Army::ARMY_BONUS, bonus);
    }
}
void ArmySetWindow::on_add1strincity_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      Uint32 bonus = a->getStat(Army::ARMY_BONUS, false);
      if (add1strincity_checkbutton->get_active() == true)
	bonus |= Army::ADD1STRINCITY ;
      else
	{
	  if (bonus & Army::ADD1STRINCITY)
	    bonus ^= Army::ADD1STRINCITY;
	}
      a->setStat(Army::ARMY_BONUS, bonus);
    }
}
void ArmySetWindow::on_add2strincity_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      Uint32 bonus = a->getStat(Army::ARMY_BONUS, false);
      if (add2strincity_checkbutton->get_active() == true)
	bonus |= Army::ADD2STRINCITY ;
      else
	{
	  if (bonus & Army::ADD2STRINCITY)
	    bonus ^= Army::ADD2STRINCITY;
	}
      a->setStat(Army::ARMY_BONUS, bonus);
    }
}
void ArmySetWindow::on_add1stackinhills_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      Uint32 bonus = a->getStat(Army::ARMY_BONUS, false);
      if (add1stackinhills_checkbutton->get_active() == true)
	bonus |= Army::ADD1STACKINHILLS;
      else
	{
	  if (bonus & Army::ADD1STACKINHILLS)
	    bonus ^= Army::ADD1STACKINHILLS;
	}
      a->setStat(Army::ARMY_BONUS, bonus);
    }
}
void ArmySetWindow::on_suballcitybonus_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      Uint32 bonus = a->getStat(Army::ARMY_BONUS, false);
      if (suballcitybonus_checkbutton->get_active() == true)
	bonus |= Army::SUBALLCITYBONUS;
      else
	{
	  if (bonus & Army::SUBALLCITYBONUS)
	    bonus ^= Army::SUBALLCITYBONUS;
	}
      a->setStat(Army::ARMY_BONUS, bonus);
    }
}
void ArmySetWindow::on_sub1enemystack_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      Uint32 bonus = a->getStat(Army::ARMY_BONUS, false);
      if (sub1enemystack_checkbutton->get_active() == true)
	bonus |= Army::SUB1ENEMYSTACK;
      else
	{
	  if (bonus & Army::SUB1ENEMYSTACK)
	    bonus ^= Army::SUB1ENEMYSTACK;
	}
      a->setStat(Army::ARMY_BONUS, bonus);
    }
}
void ArmySetWindow::on_add1stack_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      Uint32 bonus = a->getStat(Army::ARMY_BONUS, false);
      if (add1stack_checkbutton->get_active() == true)
	bonus |= Army::ADD1STACK;
      else
	{
	  if (bonus & Army::ADD1STACK)
	    bonus ^= Army::ADD1STACK;
	}
      a->setStat(Army::ARMY_BONUS, bonus);
    }
}
void ArmySetWindow::on_add2stack_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      Uint32 bonus = a->getStat(Army::ARMY_BONUS, false);
      if (add2stack_checkbutton->get_active() == true)
	bonus |= Army::ADD2STACK;
      else
	{
	  if (bonus & Army::ADD2STACK)
	    bonus ^= Army::ADD2STACK;
	}
      a->setStat(Army::ARMY_BONUS, bonus);
    }
}
void ArmySetWindow::on_suballnonherobonus_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      Uint32 bonus = a->getStat(Army::ARMY_BONUS, false);
      if (suballnonherobonus_checkbutton->get_active() == true)
	bonus |= Army::SUBALLNONHEROBONUS;
      else
	{
	  if (bonus & Army::SUBALLNONHEROBONUS)
	    bonus ^= Army::SUBALLNONHEROBONUS;
	}
      a->setStat(Army::ARMY_BONUS, bonus);
    }
}
void ArmySetWindow::on_suballherobonus_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Army *a = row[armies_columns.army];
      Uint32 bonus = a->getStat(Army::ARMY_BONUS, false);
      if (suballherobonus_checkbutton->get_active() == true)
	bonus |= Army::SUBALLHEROBONUS;
      else
	{
	  if (bonus & Army::SUBALLHEROBONUS)
	    bonus ^= Army::SUBALLHEROBONUS;
	}
      a->setStat(Army::ARMY_BONUS, bonus);
    }
}
