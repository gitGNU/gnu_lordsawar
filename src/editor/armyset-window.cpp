//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

#include <iostream>
#include <iomanip>
#include <assert.h>
#include <libgen.h>
#include <string.h>

#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>

#include <gtkmm.h>
#include "armyset-window.h"
#include "armyset-info-dialog.h"

#include "image-helpers.h"
#include "input-helpers.h"
#include "error-utils.h"

#include "defs.h"
#include "Configuration.h"
#include "GraphicsCache.h"
#include "GraphicsLoader.h"
#include "armysetlist.h"
#include "Tile.h"
#include "File.h"

#include "ucompose.hpp"

#include "glade-helpers.h"


ArmySetWindow::ArmySetWindow()
{
  d_armyset = NULL;
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path() + "/armyset-window.ui");

    xml->get_widget("window", window);
    window->set_icon_from_file(File::getMiscFile("various/castle_icon.png"));

    xml->get_widget("army_image", army_image);
    xml->get_widget("name_entry", name_entry);
    name_entry->signal_changed().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_name_changed));
    xml->get_widget("armies_treeview", armies_treeview);
    xml->get_widget("description_textview", description_textview);
    description_textview->get_buffer()->signal_changed().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_description_changed));
    xml->get_widget("image_filechooserbutton", image_filechooserbutton);
    image_filechooserbutton->signal_selection_changed().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_image_changed));
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
    add_army_button->signal_clicked().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_add_army_clicked));
    xml->get_widget("remove_army_button", remove_army_button);
    remove_army_button->signal_clicked().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_remove_army_clicked));
    xml->get_widget("army_vbox", army_vbox);
    // connect callbacks for the menu
    xml->get_widget("new_armyset_menuitem", new_armyset_menuitem);
    new_armyset_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_new_armyset_activated));
    xml->get_widget("load_armyset_menuitem", load_armyset_menuitem);
    load_armyset_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_load_armyset_activated));
    xml->get_widget("save_armyset_menuitem", save_armyset_menuitem);
    save_armyset_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_save_armyset_activated));
    xml->get_widget("save_armyset_as_menuitem", save_armyset_as_menuitem);
    save_armyset_as_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_save_armyset_as_activated));
    xml->get_widget("quit_menuitem", quit_menuitem);
    quit_menuitem->signal_activate().connect
       (sigc::mem_fun(this, &ArmySetWindow::on_quit_activated));
    xml->get_widget("edit_armyset_info_menuitem", edit_armyset_info_menuitem);
    edit_armyset_info_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_edit_armyset_info_activated));
    xml->get_widget ("help_about_menuitem", help_about_menuitem);
    help_about_menuitem->signal_activate().connect
       (sigc::mem_fun(this, &ArmySetWindow::on_help_about_activated));

    window->signal_delete_event().connect(
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

    update_armyset_buttons();
    update_armyset_menuitems();
}

void
ArmySetWindow::update_armyset_menuitems()
{
  if (d_armyset == NULL)
    {
      save_armyset_as_menuitem->set_sensitive(false);
      save_armyset_menuitem->set_sensitive(false);
    }
  else
    {
      save_armyset_as_menuitem->set_sensitive(true);
      save_armyset_menuitem->set_sensitive(true);
    }
}

void
ArmySetWindow::update_armyset_buttons()
{
  if (!armies_treeview->get_selection()->get_selected())
    remove_army_button->set_sensitive(false);
  else
    remove_army_button->set_sensitive(true);
  if (d_armyset == NULL)
    add_army_button->set_sensitive(false);
  else
    add_army_button->set_sensitive(true);
}

void
ArmySetWindow::update_army_panel()
{
  //if nothing selected in the treeview, then we don't show anything in
  //the army panel
  if (armies_treeview->get_selection()->get_selected() == 0)
    {
      //clear all values
      name_entry->set_text("");
      description_textview->get_buffer()->set_text("");
      production_spinbutton->set_value(0);
      cost_spinbutton->set_value(0);
      upkeep_spinbutton->set_value(0);
      strength_spinbutton->set_value(0);
      moves_spinbutton->set_value(0);
      exp_spinbutton->set_value(0);
      hero_checkbutton->set_active(false);
      awardable_checkbutton->set_active(false);
      defends_ruins_checkbutton->set_active(false);
      sight_spinbutton->set_value(0);
      move_forests_checkbutton->set_active(false);
      move_marshes_checkbutton->set_active(false);
      move_hills_checkbutton->set_active(false);
      move_mountains_checkbutton->set_active(false);
      can_fly_checkbutton->set_active(false);
      add1strinopen_checkbutton->set_active(false);
      add2strinopen_checkbutton->set_active(false);
      add1strinforest_checkbutton->set_active(false);
      add1strinhills_checkbutton->set_active(false);
      add1strincity_checkbutton->set_active(false);
      add2strincity_checkbutton->set_active(false);
      add1stackinhills_checkbutton->set_active(false);
      suballcitybonus_checkbutton->set_active(false);
      sub1enemystack_checkbutton->set_active(false);
      add1stack_checkbutton->set_active(false);
      add2stack_checkbutton->set_active(false);
      suballnonherobonus_checkbutton->set_active(false);
      suballherobonus_checkbutton->set_active(false);
      army_image->clear();
      army_vbox->set_sensitive(false);
      return;
    }
  army_vbox->set_sensitive(true);
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      // Row selected
      Gtk::TreeModel::Row row = *iterrow;

      ArmyProto *a = row[armies_columns.army];
      fill_army_info(a);
    }
}
ArmySetWindow::~ArmySetWindow()
{
  delete window;
}

void ArmySetWindow::show()
{
  window->show();
}

void ArmySetWindow::hide()
{
  window->hide();
}

bool ArmySetWindow::on_delete_event(GdkEventAny *e)
{
  hide();

  return true;
}


bool ArmySetWindow::load(std::string tag, XML_Helper *helper)
{
  if (tag == Armyset::d_tag)
    d_armyset = new Armyset(helper);
  return true;
}

void ArmySetWindow::on_new_armyset_activated()
{
  bool retval;
  current_save_filename.clear();
  if (d_armyset)
    {
      armies_list->clear();
      delete d_armyset;
    }
  std::string name = "";
  guint32 id = 0;
  d_armyset = new Armyset(id, name);
  ArmySetInfoDialog d(d_armyset);
  d.set_parent_window(*window);
  retval = d.run();
  if (retval == false)
    {
      delete d_armyset;
      d_armyset = NULL;
    }

      
  std::string imgpath = Configuration::s_dataPath + "/army/";
  image_filechooserbutton->set_current_folder(imgpath);

  update_armyset_buttons();
  update_armyset_menuitems();
}

void ArmySetWindow::on_load_armyset_activated()
{
  Gtk::FileChooserDialog chooser(*window, 
				 _("Choose an Armyset to Load"));
  Gtk::FileFilter sav_filter;
  sav_filter.add_pattern("*.xml");
  chooser.set_filter(sav_filter);
  chooser.set_current_folder(Configuration::s_savePath);

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);
      
  chooser.set_current_folder(Configuration::s_dataPath + "/army/");

  chooser.show_all();
  int res = chooser.run();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      current_save_filename = chooser.get_filename();
      chooser.hide();

      armies_list->clear();
      if (d_armyset)
	delete d_armyset;
      XML_Helper helper(current_save_filename, std::ios::in, false);

      helper.registerTag(Armyset::d_tag, 
			 sigc::mem_fun((*this), &ArmySetWindow::load));


      if (!helper.parse())
	{
	  std::cerr <<_("Error, while loading an armyset. Armyset Name: ");
	  std::cerr <<current_save_filename <<std::endl <<std::flush;
	  exit(-1);
	}

      char *dir = g_strdup(current_save_filename.c_str());
      dir = basename (dir);
      char *tmp = strchr (dir, '.');
      if (tmp)
	tmp[0] = '\0';
      d_armyset->setSubDir(dir);
      GraphicsLoader::instantiateImages(d_armyset);
      guint32 max = d_armyset->getSize();
      for (unsigned int i = 0; i < max; i++)
	addArmyType(i);
      if (max)
	{
	  Gtk::TreeModel::Row row;
	  row = armies_treeview->get_model()->children()[0];
	  if(row)
	    armies_treeview->get_selection()->select(row);
	}
      std::string imgpath = Configuration::s_dataPath + "/army/" + dir + "/";
      image_filechooserbutton->set_current_folder(imgpath);
    }
  update_armyset_buttons();
  update_armyset_menuitems();
  update_army_panel();
}

void ArmySetWindow::on_save_armyset_activated()
{
  if (current_save_filename.empty())
    on_save_armyset_as_activated();
  else
    {
      //Reorder the armyset according to the treeview
      d_armyset->clear();
      for (Gtk::TreeIter i = armies_list->children().begin(),
	   end = armies_list->children().end(); i != end; ++i) 
	d_armyset->push_back((*i)[armies_columns.army]);
      XML_Helper helper(current_save_filename, std::ios::out, false);
      d_armyset->save(&helper);
      helper.close();
    }
}

void ArmySetWindow::on_save_armyset_as_activated()
{
  Gtk::FileChooserDialog chooser(*window, _("Choose a Name"),
				 Gtk::FILE_CHOOSER_ACTION_SAVE);
  Gtk::FileFilter sav_filter;
  sav_filter.add_pattern("*.xml");
  chooser.set_filter(sav_filter);
  chooser.set_current_folder(Configuration::s_dataPath + "/army/");

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
  ArmySetInfoDialog d(d_armyset);
  d.set_parent_window(*window);
  d.run();
}

void ArmySetWindow::on_help_about_activated()
{
  Gtk::AboutDialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/../about-dialog.ui");

  xml->get_widget("dialog", dialog);
  dialog->set_transient_for(*window);
  dialog->set_icon_from_file(File::getMiscFile("various/castle_icon.png"));

  dialog->set_version(PACKAGE_VERSION);
  dialog->set_logo(GraphicsLoader::getMiscPicture("castle_icon.png")->to_pixbuf());
  dialog->show_all();
  dialog->run();

  return;
}

void ArmySetWindow::addArmyType(guint32 army_type)
{
  ArmyProto *a;
  //go get army_type in d_armyset
  a = d_armyset->lookupArmyByType(army_type);
  Gtk::TreeIter i = armies_list->append();
  (*i)[armies_columns.name] = a->getName();
  (*i)[armies_columns.army] = a;
}

void ArmySetWindow::on_army_selected()
{
  update_army_panel();
  update_armyset_buttons();
}

void ArmySetWindow::fill_army_info(ArmyProto *army)
{
  if (army->getImageName() != "")
    {
      army_image->property_pixbuf() = army->getImage()->to_pixbuf();
    
      std::string path = Configuration::s_dataPath + "/army/" +  
	d_armyset->getSubDir() + "/" + army->getImageName() + ".png";
      image_filechooserbutton->set_filename(path);
    }
  else
    army_image->clear();
  name_entry->set_text(army->getName());
  description_textview->get_buffer()->set_text(army->getDescription());
  double turns = army->getProduction();
  production_spinbutton->set_value(turns);
  cost_spinbutton->set_value(army->getProductionCost());
  upkeep_spinbutton->set_value(army->getUpkeep());
  strength_spinbutton->set_value(army->getStrength());
  moves_spinbutton->set_value(army->getMaxMoves());
  exp_spinbutton->set_value(int(army->getXpReward()));
  hero_checkbutton->set_active(army->isHero());
  awardable_checkbutton->set_active(army->getAwardable());
  defends_ruins_checkbutton->set_active(army->getDefendsRuins());
  sight_spinbutton->set_value(army->getSight());

  guint32 bonus = army->getMoveBonus();
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
  bonus = army->getArmyBonus();
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

void ArmySetWindow::on_name_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      a->setName(name_entry->get_text());
      row[armies_columns.name] = name_entry->get_text();
    }
}

void ArmySetWindow::on_description_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      a->setDescription(description_textview->get_buffer()->get_text());
    }
}

void ArmySetWindow::on_image_changed()
{
  if (image_filechooserbutton->get_filename().empty())
    return;

  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      char *dir = g_strdup(image_filechooserbutton->get_filename().c_str());
      dir = basename (dir);
      char *tmp = strchr (dir, '.');
      if (tmp)
	tmp[0] = '\0';
      std::string path = Configuration::s_dataPath + "/army/" +  
	d_armyset->getSubDir() +"/";
      if (image_filechooserbutton->get_filename().substr(0, path.size()) !=path)
	return;
      a->setImageName(dir);
      GraphicsLoader::instantiateImages(d_armyset);
      army_image->property_pixbuf() = a->getImage()->to_pixbuf();
    }
  return;
}

void ArmySetWindow::on_production_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      if (production_spinbutton->get_value() < 
	  MIN_PRODUCTION_TURNS_FOR_ARMY_UNITS)
	production_spinbutton->set_value(MIN_PRODUCTION_TURNS_FOR_ARMY_UNITS);
      else if (production_spinbutton->get_value() > 
	       MAX_PRODUCTION_TURNS_FOR_ARMY_UNITS)
	production_spinbutton->set_value(MAX_PRODUCTION_TURNS_FOR_ARMY_UNITS);
      else
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
      ArmyProto *a = row[armies_columns.army];
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
      ArmyProto  *a = row[armies_columns.army];
      if (upkeep_spinbutton->get_value() < MIN_UPKEEP_FOR_ARMY_UNITS)
	upkeep_spinbutton->set_value(MIN_UPKEEP_FOR_ARMY_UNITS);
      else if (upkeep_spinbutton->get_value() > MAX_UPKEEP_FOR_ARMY_UNITS)
	upkeep_spinbutton->set_value(MAX_UPKEEP_FOR_ARMY_UNITS);
      else
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
      ArmyProto *a = row[armies_columns.army];
      if (strength_spinbutton->get_value() < MIN_STRENGTH_FOR_ARMY_UNITS)
	strength_spinbutton->set_value(MIN_STRENGTH_FOR_ARMY_UNITS);
      else if (strength_spinbutton->get_value() > MAX_STRENGTH_FOR_ARMY_UNITS)
	strength_spinbutton->set_value(MAX_STRENGTH_FOR_ARMY_UNITS);
      else
	a->setStrength(int(strength_spinbutton->get_value()));
    }
}

void ArmySetWindow::on_moves_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      if (moves_spinbutton->get_value() < MIN_MOVES_FOR_ARMY_UNITS)
	moves_spinbutton->set_value(MIN_MOVES_FOR_ARMY_UNITS);
      else if (moves_spinbutton->get_value() > MAX_MOVES_FOR_ARMY_UNITS)
	moves_spinbutton->set_value(MAX_MOVES_FOR_ARMY_UNITS);
      else
	a->setMaxMoves(int(moves_spinbutton->get_value()));
    }
}

void ArmySetWindow::on_exp_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
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
      ArmyProto *a = row[armies_columns.army];
      a->setSight(int(sight_spinbutton->get_value()));
    }
}

void ArmySetWindow::on_hero_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      a->setHero(hero_checkbutton->get_active());
    }
}

void ArmySetWindow::on_awardable_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
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
      ArmyProto *a = row[armies_columns.army];
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
      ArmyProto *a = row[armies_columns.army];
      guint32 bonus = a->getMoveBonus();
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
      a->setMoveBonus(bonus);
    }
}

void ArmySetWindow::on_move_marshes_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      guint32 bonus = a->getMoveBonus();
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
      a->setMoveBonus(bonus);
    }
}

void ArmySetWindow::on_move_hills_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      guint32 bonus = a->getMoveBonus();
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
      a->setMoveBonus(bonus);
    }
}

void ArmySetWindow::on_move_mountains_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      guint32 bonus = a->getMoveBonus();
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
      a->setMoveBonus(bonus);
    }
}

void ArmySetWindow::on_can_fly_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      guint32 bonus = a->getMoveBonus();
      if (can_fly_checkbutton->get_active() == true)
	{
	  bonus = (Tile::GRASS | Tile::WATER | Tile::FOREST | Tile::HILLS |
		   Tile::MOUNTAIN | Tile::SWAMP);
	}
      else
	{
	  bonus = 0;
	}
      a->setMoveBonus(bonus);
    }
}

void ArmySetWindow::on_add1strinopen_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      guint32 bonus = a->getArmyBonus();
      if (add1strinopen_checkbutton->get_active() == true)
	bonus |= Army::ADD1STRINOPEN;
      else
	{
	  if (bonus & Army::ADD1STRINOPEN)
	    bonus ^= Army::ADD1STRINOPEN;
	}
      a->setArmyBonus(bonus);
    }
}

void ArmySetWindow::on_add2strinopen_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      guint32 bonus = a->getArmyBonus();
      if (add2strinopen_checkbutton->get_active() == true)
	bonus |= Army::ADD2STRINOPEN ;
      else
	{
	  if (bonus & Army::ADD2STRINOPEN)
	    bonus ^= Army::ADD2STRINOPEN;
	}
      a->setArmyBonus(bonus);
    }
}

void ArmySetWindow::on_add1strinforest_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      guint32 bonus = a->getArmyBonus();
      if (add1strinforest_checkbutton->get_active() == true)
	bonus |= Army::ADD1STRINFOREST;
      else
	{
	  if (bonus & Army::ADD1STRINFOREST)
	    bonus ^= Army::ADD1STRINFOREST;
	}
      a->setArmyBonus(bonus);
    }
}

void ArmySetWindow::on_add1strinhills_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      guint32 bonus = a->getArmyBonus();
      if (add1strinhills_checkbutton->get_active() == true)
	bonus |= Army::ADD1STRINHILLS;
      else
	{
	  if (bonus & Army::ADD1STRINHILLS)
	    bonus ^= Army::ADD1STRINHILLS;
	}
      a->setArmyBonus(bonus);
    }
}

void ArmySetWindow::on_add1strincity_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      guint32 bonus = a->getArmyBonus();
      if (add1strincity_checkbutton->get_active() == true)
	bonus |= Army::ADD1STRINCITY ;
      else
	{
	  if (bonus & Army::ADD1STRINCITY)
	    bonus ^= Army::ADD1STRINCITY;
	}
      a->setArmyBonus(bonus);
    }
}

void ArmySetWindow::on_add2strincity_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      guint32 bonus = a->getArmyBonus();
      if (add2strincity_checkbutton->get_active() == true)
	bonus |= Army::ADD2STRINCITY ;
      else
	{
	  if (bonus & Army::ADD2STRINCITY)
	    bonus ^= Army::ADD2STRINCITY;
	}
      a->setArmyBonus(bonus);
    }
}

void ArmySetWindow::on_add1stackinhills_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      guint32 bonus = a->getArmyBonus();
      if (add1stackinhills_checkbutton->get_active() == true)
	bonus |= Army::ADD1STACKINHILLS;
      else
	{
	  if (bonus & Army::ADD1STACKINHILLS)
	    bonus ^= Army::ADD1STACKINHILLS;
	}
      a->setArmyBonus(bonus);
    }
}

void ArmySetWindow::on_suballcitybonus_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      guint32 bonus = a->getArmyBonus();
      if (suballcitybonus_checkbutton->get_active() == true)
	bonus |= Army::SUBALLCITYBONUS;
      else
	{
	  if (bonus & Army::SUBALLCITYBONUS)
	    bonus ^= Army::SUBALLCITYBONUS;
	}
      a->setArmyBonus(bonus);
    }
}

void ArmySetWindow::on_sub1enemystack_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      guint32 bonus = a->getArmyBonus();
      if (sub1enemystack_checkbutton->get_active() == true)
	bonus |= Army::SUB1ENEMYSTACK;
      else
	{
	  if (bonus & Army::SUB1ENEMYSTACK)
	    bonus ^= Army::SUB1ENEMYSTACK;
	}
      a->setArmyBonus(bonus);
    }
}

void ArmySetWindow::on_add1stack_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      guint32 bonus = a->getArmyBonus();
      if (add1stack_checkbutton->get_active() == true)
	bonus |= Army::ADD1STACK;
      else
	{
	  if (bonus & Army::ADD1STACK)
	    bonus ^= Army::ADD1STACK;
	}
      a->setArmyBonus(bonus);
    }
}

void ArmySetWindow::on_add2stack_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      guint32 bonus = a->getArmyBonus();
      if (add2stack_checkbutton->get_active() == true)
	bonus |= Army::ADD2STACK;
      else
	{
	  if (bonus & Army::ADD2STACK)
	    bonus ^= Army::ADD2STACK;
	}
      a->setArmyBonus(bonus);
    }
}

void ArmySetWindow::on_suballnonherobonus_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      guint32 bonus = a->getArmyBonus();
      if (suballnonherobonus_checkbutton->get_active() == true)
	bonus |= Army::SUBALLNONHEROBONUS;
      else
	{
	  if (bonus & Army::SUBALLNONHEROBONUS)
	    bonus ^= Army::SUBALLNONHEROBONUS;
	}
      a->setArmyBonus(bonus);
    }
}

void ArmySetWindow::on_suballherobonus_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      guint32 bonus = a->getArmyBonus();
      if (suballherobonus_checkbutton->get_active() == true)
	bonus |= Army::SUBALLHEROBONUS;
      else
	{
	  if (bonus & Army::SUBALLHEROBONUS)
	    bonus ^= Army::SUBALLHEROBONUS;
	}
      a->setArmyBonus(bonus);
    }
}

void ArmySetWindow::on_add_army_clicked()
{
  //add a new empty army to the armyset
  ArmyProto *a = new ArmyProto();
  //add it to the treeview
  Gtk::TreeIter i = armies_list->append();
  a->setName("Untitled");
  (*i)[armies_columns.name] = a->getName();
  (*i)[armies_columns.army] = a;

}

void ArmySetWindow::on_remove_army_clicked()
{
  //erase the selected row from the treeview
  //remove the army from the armyset
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      armies_list->erase(iterrow);
      d_armyset->remove(a);
    }
}
