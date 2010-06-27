//  Copyright (C) 2007, 2008, 2009, 2010 Ben Asselstine
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
#include "masked-image-editor-dialog.h"

#include "image-helpers.h"
#include "input-helpers.h"
#include "error-utils.h"

#include "defs.h"
#include "Configuration.h"
#include "GraphicsCache.h"
#include "armysetlist.h"
#include "Tile.h"
#include "File.h"
#include "shield.h"
#include "shieldsetlist.h"
#include "recently-edited-file.h"
#include "recently-edited-file-list.h"
#include "tile-size-editor-dialog.h"
#include "editor-quit-dialog.h"
#include "editor-recover-dialog.h"

#include "ucompose.hpp"

#include "glade-helpers.h"
#include "image-editor-dialog.h"


ArmySetWindow::ArmySetWindow(std::string load_filename)
{
  autosave = File::getSavePath() + "autosave" + Armyset::file_extension;
  needs_saving = false;
  inhibit_needs_saving = false;
  d_armyset = NULL;
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path() + "/armyset-window.ui");

    xml->get_widget("window", window);
    window->set_icon_from_file(File::getMiscFile("various/castle_icon.png"));
    window->signal_delete_event().connect
      (sigc::mem_fun(*this, &ArmySetWindow::on_window_closed));

    xml->get_widget("white_image", white_image);
    xml->get_widget("green_image", green_image);
    xml->get_widget("yellow_image", yellow_image);
    xml->get_widget("light_blue_image", light_blue_image);
    xml->get_widget("red_image", red_image);
    xml->get_widget("dark_blue_image", dark_blue_image);
    xml->get_widget("orange_image", orange_image);
    xml->get_widget("black_image", black_image);
    xml->get_widget("neutral_image", neutral_image);
    xml->get_widget("name_entry", name_entry);
    name_entry->signal_changed().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_name_changed));
    xml->get_widget("armies_treeview", armies_treeview);
    xml->get_widget("description_textview", description_textview);
    description_textview->get_buffer()->signal_changed().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_description_changed));
    xml->get_widget("white_image_button", white_image_button);
    white_image_button->signal_clicked().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_white_image_clicked));
    xml->get_widget("green_image_button", green_image_button);
    green_image_button->signal_clicked().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_green_image_clicked));
    xml->get_widget("yellow_image_button", yellow_image_button);
    yellow_image_button->signal_clicked().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_yellow_image_clicked));
    xml->get_widget("light_blue_image_button", light_blue_image_button);
    light_blue_image_button->signal_clicked().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_light_blue_image_clicked));
    xml->get_widget("red_image_button", red_image_button);
    red_image_button->signal_clicked().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_red_image_clicked));
    xml->get_widget("dark_blue_image_button", 
		    dark_blue_image_button);
    dark_blue_image_button->signal_clicked().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_dark_blue_image_clicked));
    xml->get_widget("orange_image_button", orange_image_button);
    orange_image_button->signal_clicked().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_orange_image_clicked));
    xml->get_widget("black_image_button", black_image_button);
    black_image_button->signal_clicked().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_black_image_clicked));
    xml->get_widget("neutral_image_button", neutral_image_button);
    neutral_image_button->signal_clicked().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_neutral_image_clicked));
    xml->get_widget("production_spinbutton", production_spinbutton);
    production_spinbutton->set_range
      (double(MIN_PRODUCTION_TURNS_FOR_ARMY_UNITS), 
       double(MAX_PRODUCTION_TURNS_FOR_ARMY_UNITS));
    production_spinbutton->signal_changed().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_production_changed));
    production_spinbutton->signal_insert_text().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_production_text_changed));
    xml->get_widget("cost_spinbutton", cost_spinbutton);
    cost_spinbutton->set_range(double(MIN_COST_FOR_ARMY_UNITS), 
			       double(MAX_COST_FOR_ARMY_UNITS));
    cost_spinbutton->signal_changed().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_cost_changed));
    cost_spinbutton->signal_insert_text().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_cost_text_changed));
    xml->get_widget("new_cost_spinbutton", new_cost_spinbutton);
    new_cost_spinbutton->set_range(double(MIN_NEW_COST_FOR_ARMY_UNITS), 
				   double(MAX_NEW_COST_FOR_ARMY_UNITS));
    new_cost_spinbutton->signal_changed().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_new_cost_changed));
    new_cost_spinbutton->signal_insert_text().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_new_cost_text_changed));
    xml->get_widget("upkeep_spinbutton", upkeep_spinbutton);
    upkeep_spinbutton->set_range (double(MIN_UPKEEP_FOR_ARMY_UNITS), 
				  double(MAX_UPKEEP_FOR_ARMY_UNITS));
    upkeep_spinbutton->signal_changed().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_upkeep_changed));
    upkeep_spinbutton->signal_insert_text().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_upkeep_text_changed));
    xml->get_widget("strength_spinbutton", strength_spinbutton);
    strength_spinbutton->set_range (double(MIN_STRENGTH_FOR_ARMY_UNITS), 
				    double(MAX_STRENGTH_FOR_ARMY_UNITS));
    strength_spinbutton->signal_changed().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_strength_changed));
    strength_spinbutton->signal_insert_text().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_strength_text_changed));
    xml->get_widget("moves_spinbutton", moves_spinbutton);
    moves_spinbutton->set_range(double(MIN_MOVES_FOR_ARMY_UNITS), 
				double(MAX_MOVES_FOR_ARMY_UNITS));
    moves_spinbutton->signal_changed().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_moves_changed));
    moves_spinbutton->signal_insert_text().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_moves_text_changed));
    xml->get_widget("exp_spinbutton", exp_spinbutton);
    exp_spinbutton->set_range(double(MIN_EXP_FOR_ARMY_UNITS), 
			      double(MAX_EXP_FOR_ARMY_UNITS));
    exp_spinbutton->signal_changed().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_exp_changed));
    exp_spinbutton->signal_insert_text().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_exp_text_changed));
    xml->get_widget("gender_none_radiobutton", gender_none_radiobutton);
    gender_none_radiobutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_gender_none_toggled));
    xml->get_widget("gender_male_radiobutton", gender_male_radiobutton);
    gender_male_radiobutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_gender_male_toggled));
    xml->get_widget("gender_female_radiobutton", gender_female_radiobutton);
    gender_female_radiobutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_gender_female_toggled));
    xml->get_widget("awardable_checkbutton", awardable_checkbutton);
    awardable_checkbutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_awardable_toggled));
    xml->get_widget("defends_ruins_checkbutton", defends_ruins_checkbutton);
    defends_ruins_checkbutton->signal_toggled().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_defends_ruins_toggled));
    xml->get_widget("sight_spinbutton", sight_spinbutton);
    sight_spinbutton->set_range(double(MIN_SIGHT_FOR_ARMY_UNITS), 
				double(MAX_SIGHT_FOR_ARMY_UNITS));
    sight_spinbutton->signal_changed().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_sight_changed));
    sight_spinbutton->signal_insert_text().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_sight_text_changed));
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
    xml->get_widget("save_as_menuitem", save_as_menuitem);
    save_as_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_save_as_activated));
    xml->get_widget("validate_armyset_menuitem", validate_armyset_menuitem);
    validate_armyset_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_validate_armyset_activated));
    xml->get_widget("quit_menuitem", quit_menuitem);
    quit_menuitem->signal_activate().connect
       (sigc::mem_fun(this, &ArmySetWindow::on_quit_activated));
    xml->get_widget("edit_armyset_info_menuitem", edit_armyset_info_menuitem);
    edit_armyset_info_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_edit_armyset_info_activated));
    xml->get_widget("edit_standard_picture_menuitem", 
		    edit_standard_picture_menuitem);
    edit_standard_picture_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_edit_standard_picture_activated));
    xml->get_widget("edit_bag_picture_menuitem", edit_bag_picture_menuitem);
    edit_bag_picture_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_edit_bag_picture_activated));
    xml->get_widget("edit_ship_picture_menuitem", edit_ship_picture_menuitem);
    edit_ship_picture_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &ArmySetWindow::on_edit_ship_picture_activated));
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

    if (load_filename != "")
      current_save_filename = load_filename;
    update_army_panel();
    update_armyset_buttons();
    update_armyset_menuitems();

    if (File::exists(autosave))
      {
        std::string m;
        std::list<RecentlyEditedFile*> files = RecentlyEditedFileList::getInstance()->getFilesWithExtension(Armyset::file_extension);
        if (files.size() == 0)
          m = _("Do you want to recover the session?");
        else
          {
            RecentlyEditedArmysetFile *r = dynamic_cast<RecentlyEditedArmysetFile*>(files.front());
            if (r->getName() == "")
              m = String::ucompose(_("Do you want to recover %1 (%2 armies)?"),
                                   File::get_basename(r->getFileName(), true),
                                   r->getNumberOfArmies());
            else
              m = String::ucompose
                (_("Do you want to recover %1 (%2, %3 armies)?"),
                 File::get_basename(r->getFileName(), true),
                 r->getName(), r->getNumberOfArmies());
          }
        EditorRecoverDialog d(m);
        int response = d.run();
        d.hide();
        //ask if we want to recover the autosave.
        if (response == Gtk::RESPONSE_ACCEPT)
          {
            load_armyset (autosave);
            update_army_panel();
            update_armyset_buttons();
            update_armyset_menuitems();
            return;
          }
      }
    if (load_filename.empty() == false)
      {
	load_armyset (load_filename);
	update_armyset_buttons();
	update_armyset_menuitems();
	update_army_panel();
      }
}

void
ArmySetWindow::update_armyset_menuitems()
{
  if (d_armyset == NULL)
    {
      save_armyset_menuitem->set_sensitive(false);
      save_as_menuitem->set_sensitive(false);
      validate_armyset_menuitem->set_sensitive(false);
      edit_armyset_info_menuitem->set_sensitive(false);
      edit_standard_picture_menuitem->set_sensitive(false);
      edit_ship_picture_menuitem->set_sensitive(false);
      edit_bag_picture_menuitem->set_sensitive(false);
    }
  else
    {
      save_armyset_menuitem->set_sensitive(true);
      save_as_menuitem->set_sensitive(true);
      edit_armyset_info_menuitem->set_sensitive(true);
      edit_standard_picture_menuitem->set_sensitive(true);
      edit_ship_picture_menuitem->set_sensitive(true);
      edit_bag_picture_menuitem->set_sensitive(true);
      validate_armyset_menuitem->set_sensitive(true);
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
      inhibit_needs_saving = true;
      name_entry->set_text("");
      description_textview->get_buffer()->set_text("");
      production_spinbutton->set_value(MIN_PRODUCTION_TURNS_FOR_ARMY_UNITS);
      cost_spinbutton->set_value(MIN_COST_FOR_ARMY_UNITS);
      new_cost_spinbutton->set_value(MIN_NEW_COST_FOR_ARMY_UNITS);
      upkeep_spinbutton->set_value(MIN_UPKEEP_FOR_ARMY_UNITS);
      strength_spinbutton->set_value(MIN_STRENGTH_FOR_ARMY_UNITS);
      moves_spinbutton->set_value(MIN_MOVES_FOR_ARMY_UNITS);
      exp_spinbutton->set_value(0);
      gender_none_radiobutton->set_active(true);
      gender_male_radiobutton->set_active(false);
      gender_female_radiobutton->set_active(false);
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
      white_image->clear();
      green_image->clear();
      yellow_image->clear();
      light_blue_image->clear();
      red_image->clear();
      dark_blue_image->clear();
      orange_image->clear();
      black_image->clear();
      neutral_image->clear();
      army_vbox->set_sensitive(false);
      inhibit_needs_saving = false;
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
      inhibit_needs_saving = true;
      fill_army_info(a);
      inhibit_needs_saving = false;
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

void ArmySetWindow::on_new_armyset_activated()
{
  std::string name = "";
  int id = Armysetlist::getNextAvailableId(0);
  Armyset *armyset = new Armyset(id, name);
  ArmySetInfoDialog d(armyset, File::getUserArmysetDir(), "", false,
                      _("Make a New Armyset"));
  d.set_parent_window(*window);
  int response = d.run();
  if (response != Gtk::RESPONSE_ACCEPT)
    {
      delete armyset;
      return;
    }
  if (d_armyset)
    delete d_armyset;
  d_armyset = armyset;
  std::string dir = File::getUserArmysetDir();
  d_armyset->setDirectory(dir);
  current_save_filename = d_armyset->getConfigurationFile();
  RecentlyEditedFileList *refl = RecentlyEditedFileList::getInstance();
  refl->updateEntry(current_save_filename);
  refl->save();
  d_armyset->setDirectory(File::get_dirname(autosave));
  d_armyset->setBaseName(File::get_basename(autosave));

  d_armyset->save(autosave, Armyset::file_extension);
  armies_list->clear();

  update_armyset_buttons();
  update_armyset_menuitems();

  needs_saving = true;
  update_window_title();
}

void ArmySetWindow::on_load_armyset_activated()
{
  Gtk::FileChooserDialog chooser(*window, 
				 _("Choose an Armyset to Load"));
  Gtk::FileFilter sav_filter;
  sav_filter.add_pattern("*" + ARMYSET_EXT);
  chooser.set_filter(sav_filter);
  chooser.set_current_folder(File::getUserArmysetDir());

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);
      
  chooser.show_all();
  int res = chooser.run();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      load_armyset(chooser.get_filename());
      chooser.hide();
      needs_saving = false;
      update_window_title();
    }

  update_armyset_buttons();
  update_armyset_menuitems();
  //update_army_panel();
}

void ArmySetWindow::on_validate_armyset_activated()
{
  std::list<std::string> msgs;
  if (d_armyset == NULL)
    return;
  bool valid;
  valid = d_armyset->size() > 0;
  if (!valid)
    msgs.push_back(_("There must be at least one army unit in the armyset."));
  valid = d_armyset->validateHero();
  if (!valid)
    msgs.push_back(_("There must be at least one hero in the armyset."));
  valid = d_armyset->validatePurchasables();
  if (!valid)
    msgs.push_back(_("There must be at least one army unit with a production cost of more than zero."));
  valid = d_armyset->validateRuinDefenders();
  if (!valid)
    msgs.push_back(_("There must be at least one army unit than can defend a ruin."));
  valid = d_armyset->validateAwardables();
  if (!valid)
    msgs.push_back(_("There must be at least one army unit than can be awarded to a hero."));
  valid = d_armyset->validateShip();
  if (!valid)
    msgs.push_back(_("The ship image must be set."));
  valid = d_armyset->validateStandard();
  if (!valid)
    msgs.push_back(_("The hero's standard (the flag) image must be set."));
  valid = d_armyset->validateBag();
  if (!valid)
    msgs.push_back(_("The picture for the bag of items must be set."));

  for (Armyset::iterator it = d_armyset->begin(); it != d_armyset->end(); it++)
    {
      Shield::Colour c;
      bool valid = d_armyset->validateArmyUnitImage(*it, c);
      if (!valid)
	{
	  msgs.push_back(String::ucompose(_("%1 does not have an image set for the %2 player"), (*it)->getName(), Shield::colourToString(c)));
	  break;
	}
    }

  valid = d_armyset->validateArmyUnitNames();
  if (!valid)
    msgs.push_back(_("An army unit does not have a name."));

  std::string msg = "";
  for (std::list<std::string>::iterator it = msgs.begin(); it != msgs.end();
       it++)
    msg += (*it) + "\n";

  if (msg == "")
    msg = _("The armyset is valid.");
      
  Gtk::MessageDialog dialog(*window, msg);
  dialog.run();
  dialog.hide();

  return;
}

void ArmySetWindow::on_save_as_activated()
{
  std::string orig_basename = d_armyset->getBaseName();
  guint32 orig_id = d_armyset->getId();
  d_armyset->setId(Armysetlist::getNextAvailableId(orig_id));
  ArmySetInfoDialog d(d_armyset, File::getUserArmysetDir(), "", false,
                        _("Save a Copy of a Armyset"));
  d.set_parent_window(*window);
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      std::string new_basename = d_armyset->getBaseName();
      guint32 new_id = d_armyset->getId();
      d_armyset->setId(orig_id);
      d_armyset->setBaseName(orig_basename);
      bool success = Armysetlist::getInstance()->addToPersonalCollection(d_armyset, new_basename, new_id);
      if (success)
        {
          save_armyset_menuitem->set_sensitive(true);
          current_save_filename = d_armyset->getConfigurationFile();
          RecentlyEditedFileList *refl = RecentlyEditedFileList::getInstance();
          refl->updateEntry(current_save_filename);
          refl->save();
          load_armyset(current_save_filename);
          needs_saving = false;
          update_window_title();
        }
      else
        {
          std::string msg;
          msg = _("Error!  Armyset could not be saved.");
          Gtk::MessageDialog dialog(*window, msg);
          dialog.run();
          dialog.hide();
        }
    }
  else
    d_armyset->setId(orig_id);
}

void ArmySetWindow::on_save_armyset_activated()
{
  if (current_save_filename.empty())
    current_save_filename = d_armyset->getConfigurationFile();
  
  guint32 suggested_tile_size = d_armyset->calculate_preferred_tile_size();
  if (suggested_tile_size != d_armyset->getTileSize())
    {
      TileSizeEditorDialog d(d_armyset->getTileSize(), suggested_tile_size);
      int response = d.run();
      if (response == Gtk::RESPONSE_ACCEPT)
        d_armyset->setTileSize(d.get_selected_tilesize());
    }
  //Reorder the armyset according to the treeview
  d_armyset->clear();
  for (Gtk::TreeIter i = armies_list->children().begin(),
       end = armies_list->children().end(); i != end; ++i) 
    d_armyset->push_back((*i)[armies_columns.army]);

  bool success = d_armyset->save(autosave, Armyset::file_extension);
  if (success)
    {
      RecentlyEditedFileList::getInstance()->updateEntry(current_save_filename);
      RecentlyEditedFileList::getInstance()->save();
      bool success = Armyset::copy (autosave, current_save_filename);
      if (success == true)
        {
          needs_saving = false;
          update_window_title();
          armyset_saved.emit(d_armyset->getId());
        }
    }
  if (!success)
    {
      std::string msg;
      msg = _("Error!  Armyset could not be saved.");
      Gtk::MessageDialog dialog(*window, msg);
      dialog.run();
      dialog.hide();
    }
}

void ArmySetWindow::on_edit_ship_picture_activated()
{
  std::string filename = "";
  if (d_armyset->getShipImageName() != "")
    filename = d_armyset->getFileFromConfigurationFile(d_armyset->getShipImageName() +".png");
  MaskedImageEditorDialog d(filename);
  d.set_icon_from_file(File::getMiscFile("various/castle_icon.png"));
  int response = d.run();
  if (filename != "")
    File::erase(filename);
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_selected_filename() != filename)
        {
          d_armyset->replaceFileInConfigurationFile(d_armyset->getShipImageName()+".png", d.get_selected_filename());
          d_armyset->setShipImageName(File::get_basename(d.get_selected_filename()));
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_edit_standard_picture_activated()
{
  std::string filename = "";
  if (d_armyset->getStandardImageName() != "")
    filename = d_armyset->getFileFromConfigurationFile(d_armyset->getStandardImageName() +".png");
  MaskedImageEditorDialog d(filename);
  d.set_icon_from_file(File::getMiscFile("various/castle_icon.png"));
  int response = d.run();
  if (filename != "")
    File::erase(filename);
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_selected_filename() != filename)
        {
          d_armyset->replaceFileInConfigurationFile(d_armyset->getStandardImageName()+".png", d.get_selected_filename());
          d_armyset->setStandardImageName(File::get_basename(d.get_selected_filename()));
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_edit_bag_picture_activated()
{
  std::string filename = "";
  if (d_armyset->getBagImageName().empty() == false)
    filename = d_armyset->getFileFromConfigurationFile(d_armyset->getBagImageName() +".png");
  ImageEditorDialog d(filename, 1);
  d.set_icon_from_file(File::getMiscFile("various/tileset_icon.png"));
  d.set_parent_window(*window);
  int response = d.run();
  if (filename != "")
    File::erase(filename);
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_selected_filename() != filename)
        {
          d_armyset->replaceFileInConfigurationFile(d_armyset->getBagImageName()+".png", d.get_selected_filename());
          d_armyset->setBagImageName(File::get_basename(d.get_selected_filename()));
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_edit_armyset_info_activated()
{
  ArmySetInfoDialog d(d_armyset, File::get_dirname(current_save_filename), 
                      File::get_basename(current_save_filename), true, 
                      _("Edit Armyset Information"));
  d.set_parent_window(*window);
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      needs_saving = true;
      update_window_title();
    }
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
  dialog->set_logo(GraphicsCache::getMiscPicture("castle_icon.png")->to_pixbuf());
  dialog->show_all();
  dialog->run();
  delete dialog;

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
  update_armyset_buttons();
  update_army_panel();
}

void ArmySetWindow::fill_army_image(Gtk::Button *button, Gtk::Image *image, Shield::Colour c, ArmyProto *army)
{
  if (army->getImageName(c) != "")
    {
      std::string filename = "";
      filename = d_armyset->getFileFromConfigurationFile(army->getImageName(c) +".png");
      if (filename != "")
        {
          Gdk::Color colour = Shieldsetlist::getInstance()->getColor(1, c);
          army->instantiateImages(d_armyset->getTileSize(), c, filename);
          PixMask *army_image = GraphicsCache::applyMask(army->getImage(c), 
                                                         army->getMask(c), 
                                                         colour, false);
          image->property_pixbuf() = army_image->to_pixbuf();
          delete army_image;
          File::erase(filename);

          button->set_label(army->getImageName(c) + ".png");
        }
    }
  else
    {
      button->set_label(_("no image set"));
      image->clear();
    }
}

void ArmySetWindow::fill_army_info(ArmyProto *army)
{
  fill_army_image(white_image_button, white_image, Shield::WHITE, army);
  fill_army_image(green_image_button, green_image, Shield::GREEN, army);
  fill_army_image(yellow_image_button, yellow_image, Shield::YELLOW, army);
  fill_army_image(light_blue_image_button, light_blue_image, 
                  Shield::LIGHT_BLUE, army);
  fill_army_image(red_image_button, red_image, Shield::RED, army);
  fill_army_image(dark_blue_image_button, dark_blue_image, Shield::DARK_BLUE, 
                  army);
  fill_army_image(orange_image_button, orange_image, Shield::ORANGE, army);
  fill_army_image(black_image_button, black_image, Shield::BLACK, army);
  fill_army_image(neutral_image_button, neutral_image, Shield::NEUTRAL, army);
  name_entry->set_text(army->getName());
  description_textview->get_buffer()->set_text(army->getDescription());
  double turns = army->getProduction();
  production_spinbutton->set_value(turns);
  cost_spinbutton->set_value(army->getProductionCost());
  new_cost_spinbutton->set_value(army->getNewProductionCost());
  upkeep_spinbutton->set_value(army->getUpkeep());
  strength_spinbutton->set_value(army->getStrength());
  moves_spinbutton->set_value(army->getMaxMoves());
  exp_spinbutton->set_value(int(army->getXpReward()));
  switch (army->getGender())
    {
    case Hero::NONE: gender_none_radiobutton->set_active(true); break;
    case Hero::MALE: gender_male_radiobutton->set_active(true); break;
    case Hero::FEMALE: gender_female_radiobutton->set_active(true); break;
    }
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_image_changed(Gtk::Button *button, Gtk::Image *image, Shield::Colour c)
{

  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      std::string filename = "";
      if (a->getImageName(c) != "")
        filename = d_armyset->getFileFromConfigurationFile(a->getImageName(c) + ".png");
      MaskedImageEditorDialog d(filename);
      d.set_parent_window(*window);
      int response = d.run();
      if (filename != "")
        File::erase(filename);
      if (response == Gtk::RESPONSE_ACCEPT)
        {
          if (d.get_selected_filename() != "" && d.get_selected_filename() != filename)
            {
              d_armyset->replaceFileInConfigurationFile(a->getImageName(c)+".png", d.get_selected_filename());
              a->setImageName(c, File::get_basename(d.get_selected_filename()));
              Gdk::Color colour = Shieldsetlist::getInstance()->getColor(1, c);
              a->instantiateImages(d_armyset->getTileSize(), c, d.get_selected_filename());

              PixMask *army_image = GraphicsCache::applyMask(a->getImage(c), 
                                                             a->getMask(c), 
                                                             colour, false);
              image->property_pixbuf() = army_image->to_pixbuf();
              delete army_image;

              if (inhibit_needs_saving == false)
                {
                  needs_saving = true;
                  update_window_title();
                }
            }
        }
      if (a->getImageName(c) == "")
        button->set_label(_("no image set"));
      else
        button->set_label(a->getImageName(c) + ".png");
    }
}

void ArmySetWindow::on_white_image_clicked()
{
  on_image_changed(white_image_button, white_image, Shield::WHITE);
}
void ArmySetWindow::on_green_image_clicked()
{
  on_image_changed(green_image_button, green_image, Shield::GREEN);
}
void ArmySetWindow::on_yellow_image_clicked()
{
  on_image_changed(yellow_image_button, yellow_image, Shield::YELLOW);
}
void ArmySetWindow::on_light_blue_image_clicked()
{
  on_image_changed(light_blue_image_button, light_blue_image, 
                   Shield::LIGHT_BLUE);
}
void ArmySetWindow::on_red_image_clicked()
{
  on_image_changed(red_image_button, red_image, Shield::RED);
}
void ArmySetWindow::on_dark_blue_image_clicked()
{
  on_image_changed(dark_blue_image_button, dark_blue_image, Shield::DARK_BLUE);
}
void ArmySetWindow::on_orange_image_clicked()
{
  on_image_changed(orange_image_button, orange_image, Shield::ORANGE);
}
void ArmySetWindow::on_black_image_clicked()
{
  on_image_changed(black_image_button, black_image, Shield::BLACK);
}
void ArmySetWindow::on_neutral_image_clicked()
{
  on_image_changed(neutral_image_button, neutral_image, Shield::NEUTRAL);
}

void ArmySetWindow::on_production_text_changed(const Glib::ustring &s, int* p)
{
  production_spinbutton->set_value(atoi(production_spinbutton->get_text().c_str()));
  on_production_changed();
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_cost_text_changed(const Glib::ustring &s, int* p)
{
  cost_spinbutton->set_value(atoi(cost_spinbutton->get_text().c_str()));
  on_cost_changed();
}
void ArmySetWindow::on_cost_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      if (cost_spinbutton->get_value() < MIN_COST_FOR_ARMY_UNITS)
	cost_spinbutton->set_value(MIN_COST_FOR_ARMY_UNITS);
      else if (strength_spinbutton->get_value() > MAX_COST_FOR_ARMY_UNITS)
	cost_spinbutton->set_value(MAX_COST_FOR_ARMY_UNITS);
      else
	a->setProductionCost(int(cost_spinbutton->get_value()));
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}


void ArmySetWindow::on_new_cost_text_changed(const Glib::ustring &s, int* p)
{
  new_cost_spinbutton->set_value(atoi(new_cost_spinbutton->get_text().c_str()));
  on_new_cost_changed();
}
void ArmySetWindow::on_new_cost_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      a->setNewProductionCost(int(new_cost_spinbutton->get_value()));
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_upkeep_text_changed(const Glib::ustring &s, int* p)
{
  upkeep_spinbutton->set_value(atoi(upkeep_spinbutton->get_text().c_str()));
  on_upkeep_changed();
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_strength_text_changed(const Glib::ustring &s, int* p)
{
  strength_spinbutton->set_value(atoi(strength_spinbutton->get_text().c_str()));
  on_strength_changed();
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_moves_text_changed(const Glib::ustring &s, int* p)
{
  moves_spinbutton->set_value(atoi(moves_spinbutton->get_text().c_str()));
  on_moves_changed();
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_exp_text_changed(const Glib::ustring &s, int* p)
{
  exp_spinbutton->set_value(atoi(exp_spinbutton->get_text().c_str()));
  on_exp_changed();
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_sight_text_changed(const Glib::ustring &s, int* p)
{
  sight_spinbutton->set_value(atoi(sight_spinbutton->get_text().c_str()));
  on_sight_changed();
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_gender_none_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      a->setGender(Hero::NONE);
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_gender_male_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      a->setGender(Hero::MALE);
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_gender_female_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      a->setGender(Hero::FEMALE);
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }

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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
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
  d_armyset->push_back(a);
  needs_saving = true;
  update_window_title();

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
      needs_saving = true;
      update_window_title();
    }
}

bool ArmySetWindow::load_armyset(std::string filename)
{
  std::string old_current_save_filename = current_save_filename;
  current_save_filename = filename;
  if (filename != autosave)
    Armyset::copy(current_save_filename, autosave);
  else
    {
      //go get the real name of the file
      std::list<RecentlyEditedFile*> files = RecentlyEditedFileList::getInstance()->getFilesWithExtension(Armyset::file_extension);
      if (files.size() > 0)
        current_save_filename = files.front()->getFileName();
    }

  std::string name = File::get_basename(filename);

  bool unsupported_version;
  Armyset *armyset = Armyset::create(autosave, unsupported_version);
  if (armyset == NULL)
    {
      std::string msg;
      if (unsupported_version)
        msg = _("Error!  The version of the armyset is unsupported.");
      else
        msg = _("Error!  Armyset could not be loaded.");
      Gtk::MessageDialog dialog(*window, msg);
      current_save_filename = old_current_save_filename;
      dialog.run();
      dialog.hide();
      return false;
    }
  if (File::nameEndsWith(current_save_filename, "/autosave" + Armyset::file_extension) == false)
    {
      RecentlyEditedFileList::getInstance()->addEntry(current_save_filename);
      RecentlyEditedFileList::getInstance()->save();
    }
  if (d_armyset)
    delete d_armyset;
  d_armyset = armyset;

  d_armyset->setBaseName(File::get_basename(autosave));
  guint32 max = d_armyset->getSize();
  armies_list->clear();
  for (unsigned int i = 0; i < max; i++)
    addArmyType(i);
  if (max)
    {
      Gtk::TreeModel::Row row;
      row = armies_treeview->get_model()->children()[0];
      if(row)
	armies_treeview->get_selection()->select(row);
    }

  d_armyset->instantiateImages();
  needs_saving = false;
  update_window_title();
  return true;
}

bool ArmySetWindow::quit()
{
  if (needs_saving == true)
    {
      EditorQuitDialog d;
      int response = d.run();
      d.set_parent_window(*window);
      d.hide();
      
      if (response == Gtk::RESPONSE_CANCEL) //we don't want to quit
	return false;

      else if (response == Gtk::RESPONSE_ACCEPT) // save and quit
	on_save_armyset_activated();
      //else if (Response == Gtk::CLOSE) // don't save just quit
      window->hide();
    }
  else
    window->hide();
  File::erase(autosave);
  if (d_armyset)
    delete d_armyset;
  return true;
}

bool ArmySetWindow::on_window_closed(GdkEventAny*)
{
  return !quit();
}

void ArmySetWindow::on_quit_activated()
{
  quit();
}

void ArmySetWindow::update_window_title()
{
  std::string title = "";
  if (needs_saving)
    title += "*";
  title += File::get_basename(current_save_filename, true);
  title += " - ";
  title += _("LordsAWar! Armyset Editor");
  window->set_title(title);
}
