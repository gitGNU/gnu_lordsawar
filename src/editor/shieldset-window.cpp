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
#include "shieldset-window.h"
#include "shieldset-info-dialog.h"
#include "masked-image-editor-dialog.h"

#include "image-helpers.h"
#include "input-helpers.h"
#include "error-utils.h"

#include "defs.h"
#include "Configuration.h"
#include "GraphicsCache.h"
#include "shieldsetlist.h"
#include "Tile.h"
#include "File.h"
#include "shield.h"
#include "shieldsetlist.h"

#include "ucompose.hpp"

#include "glade-helpers.h"
#include "masked-image-editor-dialog.h"


ShieldSetWindow::ShieldSetWindow(std::string load_filename)
{
  needs_saving = false;
  d_shieldset = NULL;
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path() + "/shieldset-window.ui");

    xml->get_widget("window", window);
    window->set_icon_from_file(File::getMiscFile("various/castle_icon.png"));
    window->signal_delete_event().connect
      (sigc::mem_fun(*this, &ShieldSetWindow::on_window_closed));

    xml->get_widget("shield_frame", shield_frame);
    xml->get_widget("shields_treeview", shields_treeview);
    // connect callbacks for the menu
    xml->get_widget("new_shieldset_menuitem", new_shieldset_menuitem);
    new_shieldset_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &ShieldSetWindow::on_new_shieldset_activated));
    xml->get_widget("load_shieldset_menuitem", load_shieldset_menuitem);
    load_shieldset_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &ShieldSetWindow::on_load_shieldset_activated));
    xml->get_widget("save_as_menuitem", save_as_menuitem);
    save_as_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &ShieldSetWindow::on_save_as_activated));
    xml->get_widget("save_shieldset_menuitem", save_shieldset_menuitem);
    save_shieldset_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &ShieldSetWindow::on_save_shieldset_activated));
    xml->get_widget("validate_shieldset_menuitem", validate_shieldset_menuitem);
    validate_shieldset_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &ShieldSetWindow::on_validate_shieldset_activated));
    xml->get_widget("quit_menuitem", quit_menuitem);
    quit_menuitem->signal_activate().connect
       (sigc::mem_fun(this, &ShieldSetWindow::on_quit_activated));
    xml->get_widget("edit_shieldset_info_menuitem", edit_shieldset_info_menuitem);
    edit_shieldset_info_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &ShieldSetWindow::on_edit_shieldset_info_activated));
    xml->get_widget ("help_about_menuitem", help_about_menuitem);
    help_about_menuitem->signal_activate().connect
       (sigc::mem_fun(this, &ShieldSetWindow::on_help_about_activated));
    xml->get_widget ("change_smallpic_button", change_smallpic_button);
    change_smallpic_button->signal_clicked().connect(
	sigc::mem_fun(this, &ShieldSetWindow::on_change_smallpic_clicked));
    xml->get_widget ("change_mediumpic_button", change_mediumpic_button);
    change_mediumpic_button->signal_clicked().connect(
	sigc::mem_fun(this, &ShieldSetWindow::on_change_mediumpic_clicked));
    xml->get_widget ("change_largepic_button", change_largepic_button);
    change_largepic_button->signal_clicked().connect(
	sigc::mem_fun(this, &ShieldSetWindow::on_change_largepic_clicked));
    xml->get_widget ("player_colorbutton", player_colorbutton);
    player_colorbutton->signal_color_set().connect
      (sigc::mem_fun(this, &ShieldSetWindow::on_player_color_changed));

    window->signal_delete_event().connect(
	sigc::mem_fun(*this, &ShieldSetWindow::on_delete_event));

    shields_list = Gtk::ListStore::create(shields_columns);
    shields_treeview->set_model(shields_list);
    shields_treeview->append_column("", shields_columns.name);
    shields_treeview->set_headers_visible(false);
    shields_treeview->get_selection()->signal_changed().connect
      (sigc::mem_fun(*this, &ShieldSetWindow::on_shield_selected));

    update_shield_panel();

    update_shieldset_menuitems();

    if (load_filename.empty() == false)
      {
	load_shieldset (load_filename);
	update_shieldset_menuitems();
	update_shield_panel();
      }
}

void
ShieldSetWindow::update_shieldset_menuitems()
{
  if (d_shieldset == NULL)
    {
      save_shieldset_menuitem->set_sensitive(false);
      validate_shieldset_menuitem->set_sensitive(false);
      edit_shieldset_info_menuitem->set_sensitive(false);
    }
  else
    {
      std::string file = d_shieldset->getConfigurationFile();
      if (File::exists(file) == false)
	save_shieldset_menuitem->set_sensitive(true);
      else if (File::is_writable(file) == false)
	save_shieldset_menuitem->set_sensitive(false);
      else
	save_shieldset_menuitem->set_sensitive(true);
      edit_shieldset_info_menuitem->set_sensitive(true);
      validate_shieldset_menuitem->set_sensitive(true);
    }
}

void
ShieldSetWindow::update_shield_panel()
{
  std::string none = _("no image set");
  //if nothing selected in the treeview, then we don't show anything in
  //the shield panel
  if (shields_treeview->get_selection()->get_selected() == 0)
    {
      //clear all values
      shield_frame->set_sensitive(false);
      change_smallpic_button->set_label(none);
      change_mediumpic_button->set_label(none);
      change_largepic_button->set_label(none);
      player_colorbutton->set_color(Gdk::Color("black"));
      return;
    }
  shield_frame->set_sensitive(true);
  Glib::RefPtr<Gtk::TreeSelection> selection = shields_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      // Row selected
      Gtk::TreeModel::Row row = *iterrow;

      Shield *s = row[shields_columns.shield];
      fill_shield_info(s);
    }
}
ShieldSetWindow::~ShieldSetWindow()
{
  delete window;
}

void ShieldSetWindow::show()
{
  window->show();
}

void ShieldSetWindow::hide()
{
  window->hide();
}

bool ShieldSetWindow::on_delete_event(GdkEventAny *e)
{
  hide();

  return true;
}

void ShieldSetWindow::on_new_shieldset_activated()
{
  std::string name = "";
  int id = Shieldsetlist::getNextAvailableId(0);
  Shieldset *shieldset = new Shieldset(id, name);
  ShieldSetInfoDialog d(shieldset, File::getUserShieldsetDir() + "<subdir>/", false);
  d.set_parent_window(*window);
  int response = d.run();
  if (response != Gtk::RESPONSE_ACCEPT)
    {
      delete shieldset;
      return;
    }
  if (d_shieldset)
    delete d_shieldset;
  d_shieldset = shieldset;
  shields_list->clear();
  std::string dir = File::getUserShieldsetDir() + "/" + d_shieldset->getSubDir();
  d_shieldset->setDirectory(dir);
  File::create_dir(dir);
  current_save_filename = d_shieldset->getConfigurationFile();

  //populate the list with initial entries.
  for (unsigned int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
    addNewShield(Shield::Colour(i), Shield::get_default_color_for_no(i));
  update_shieldset_menuitems();

  XML_Helper helper(current_save_filename, std::ios::out, false);
  d_shieldset->save(&helper);
  helper.close();
  update_shield_panel();
  needs_saving = true;
  update_window_title();
}

void ShieldSetWindow::on_load_shieldset_activated()
{
  Gtk::FileChooserDialog chooser(*window, 
				 _("Choose an Shieldset to Load"));
  Gtk::FileFilter sav_filter;
  sav_filter.add_pattern("*" + ARMYSET_EXT);
  chooser.set_filter(sav_filter);
  chooser.set_current_folder(File::getUserShieldsetDir());

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);
      
  chooser.show_all();
  int res = chooser.run();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      load_shieldset(chooser.get_filename());
      chooser.hide();
      needs_saving = false;
    }

  update_shieldset_menuitems();
  update_shield_panel();
  update_window_title();
}

void ShieldSetWindow::on_validate_shieldset_activated()
{
  std::list<std::string> msgs;
  if (d_shieldset == NULL)
    return;
  //bool valid;

  std::string msg = "";
  for (std::list<std::string>::iterator it = msgs.begin(); it != msgs.end();
       it++)
    msg += (*it) + "\n";

  if (msg != "")
    {
      Gtk::MessageDialog dialog(*window, msg);
      dialog.run();
      dialog.hide();
    }

  return;
}

void ShieldSetWindow::on_save_as_activated()
{
  guint32 orig_id = d_shieldset->getId();
  d_shieldset->setId(Shieldsetlist::getNextAvailableId(orig_id));
  ShieldSetInfoDialog d(d_shieldset, File::getUserShieldsetDir() + d_shieldset->getSubDir() +"/", false);
  d.set_parent_window(*window);
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      std::string new_subdir = "";
      guint32 new_id = 0;
      Shieldsetlist::getInstance()->addToPersonalCollection(d_shieldset, new_subdir, new_id);
      save_shieldset_menuitem->set_sensitive(true);
      current_save_filename = d_shieldset->getConfigurationFile();
      needs_saving = false;
      update_window_title();
    }
  else
    {
      d_shieldset->setId(orig_id);
    }
}

void ShieldSetWindow::on_save_shieldset_activated()
{
  if (current_save_filename.empty())
    current_save_filename = d_shieldset->getConfigurationFile();
  
  XML_Helper helper(current_save_filename, std::ios::out, false);
  d_shieldset->save(&helper);
  helper.close();
  needs_saving = false;
  update_window_title();
  shieldset_saved.emit(d_shieldset->getId());
}

void ShieldSetWindow::on_edit_shieldset_info_activated()
{
  ShieldSetInfoDialog d(d_shieldset, File::get_dirname(current_save_filename), true);
  d.set_parent_window(*window);
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      needs_saving = true;
      update_window_title();
    }
}

void ShieldSetWindow::on_help_about_activated()
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

void ShieldSetWindow::on_shield_selected()
{
  update_shield_panel();
}

void ShieldSetWindow::fill_shield_info(Shield*shield)
{
  if (shield)
    {
      std::string none = _("no image set");
      player_colorbutton->set_color(shield->getColor());
      std::string s;
      ShieldStyle* ss = shield->getFirstShieldstyle(ShieldStyle::SMALL);
      if (ss && ss->getImageName().empty() == false)
	s = ss->getImageName() + ".png";
      else
	s = none;
      change_smallpic_button->set_label(s);
      ss = shield->getFirstShieldstyle(ShieldStyle::MEDIUM);
      if (ss && ss->getImageName().empty() == false)
	s = ss->getImageName() + ".png";
      else
	s = none;
      change_mediumpic_button->set_label(s);
      ss = shield->getFirstShieldstyle(ShieldStyle::LARGE);
      if (ss && ss->getImageName().empty() == false)
	s = ss->getImageName() + ".png";
      else
	s = none;
      change_largepic_button->set_label(s);
    }
}

void ShieldSetWindow::load_shieldset(std::string filename)
{
  current_save_filename = filename;

  std::string name = File::get_basename(filename);

  Shieldset *shieldset = Shieldset::create(filename);
  if (shieldset == NULL)
    {
      std::string msg;
      msg = "Error!  Shieldset could not be loaded.";
      Gtk::MessageDialog dialog(*window, msg);
      dialog.run();
      dialog.hide();
      return;
    }
  shields_list->clear();
  if (d_shieldset)
    delete d_shieldset;
  d_shieldset = shieldset;

  d_shieldset->setSubDir(name);
  d_shieldset->instantiateImages();
  for(Shieldset::iterator i = d_shieldset->begin(); 
      i != d_shieldset->end(); i++)
    loadShield(*i);
  if (d_shieldset->size())
    {
      Gtk::TreeModel::Row row;
      row = shields_treeview->get_model()->children()[0];
      if(row)
	shields_treeview->get_selection()->select(row);
    }
  update_shield_panel();
  update_window_title();

}
bool ShieldSetWindow::quit()
{
  if (needs_saving == true)
    {
      Gtk::Dialog* dialog;
      Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path() + 
					 "/editor-quit-dialog.ui");
      xml->get_widget("dialog", dialog);
      Gtk::Button *save_button;
      xml->get_widget("save_button", save_button);
      save_button->set_sensitive(File::is_writable(d_shieldset->getConfigurationFile()));
      dialog->set_transient_for(*window);
      int response = dialog->run();
      dialog->hide();
      delete dialog;
      
      if (response == Gtk::RESPONSE_CANCEL) //we don't want to quit
	return false;

      else if (response == Gtk::RESPONSE_ACCEPT) // save and quit
	on_save_shieldset_activated();
      //else if (Response == Gtk::CLOSE) // don't save just quit
      window->hide();
    }
  else
    window->hide();
  return true;
}
bool ShieldSetWindow::on_window_closed(GdkEventAny*)
{
  return !quit();
}
void ShieldSetWindow::on_quit_activated()
{
  quit();
}
    
void ShieldSetWindow::on_change_smallpic_clicked()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = shields_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Shield *shield = row[shields_columns.shield];
      std::string filename = "";
      ShieldStyle *ss = shield->getFirstShieldstyle(ShieldStyle::SMALL);
      if (ss->getImageName() != "")
	filename = d_shieldset->getFile(ss->getImageName());
      MaskedImageEditorDialog d(filename);
      d.set_icon_from_file(File::getMiscFile("various/castle_icon.png"));
      d.run();
      if (d.get_selected_filename() != "")
	{
	  std::string file = File::get_basename(d.get_selected_filename());
	  if (d.get_selected_filename() != d_shieldset->getFile(file))
	    {
	      //fixme:warn on overrwite.
	      File::copy (d.get_selected_filename(), 
			  d_shieldset->getFile(file));
	    }
	  ss->setImageName(file);
	  needs_saving = true;
          update_window_title();
	  update_shield_panel();
	}
    }
}

void ShieldSetWindow::on_change_mediumpic_clicked()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = shields_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Shield *shield = row[shields_columns.shield];
      std::string filename = "";
      ShieldStyle *ss = shield->getFirstShieldstyle(ShieldStyle::MEDIUM);
      if (ss->getImageName() != "")
	filename = d_shieldset->getFile(ss->getImageName());
      MaskedImageEditorDialog d(filename);
      d.set_icon_from_file(File::getMiscFile("various/castle_icon.png"));
      d.run();
      if (d.get_selected_filename() != "")
	{
	  std::string file = File::get_basename(d.get_selected_filename());
	  if (d.get_selected_filename() != d_shieldset->getFile(file))
	    {
	      //fixme:warn on overrwite.
	      File::copy (d.get_selected_filename(), 
			  d_shieldset->getFile(file));
	    }
	  ss->setImageName(file);
	  needs_saving = true;
          update_window_title();
	  update_shield_panel();
	}
    }
}

void ShieldSetWindow::on_change_largepic_clicked()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = shields_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Shield *shield = row[shields_columns.shield];
      std::string filename = "";
      ShieldStyle *ss = shield->getFirstShieldstyle(ShieldStyle::LARGE);
      if (ss->getImageName() != "")
	filename = d_shieldset->getFile(ss->getImageName());
      MaskedImageEditorDialog d(filename);
      d.set_icon_from_file(File::getMiscFile("various/castle_icon.png"));
      d.run();
      if (d.get_selected_filename() != "")
	{
	  std::string file = File::get_basename(d.get_selected_filename());
	  if (d.get_selected_filename() != d_shieldset->getFile(file))
	    {
	      //fixme:warn on overrwite.
	      File::copy (d.get_selected_filename(), 
			  d_shieldset->getFile(file));
	    }
	  ss->setImageName(file);
	  needs_saving = true;
          update_window_title();
	  update_shield_panel();
	}

    }
}

void ShieldSetWindow::on_player_color_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = shields_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Shield *s = row[shields_columns.shield];
      s->setColor(player_colorbutton->get_color());
      needs_saving = true;
      update_window_title();
    }
}

void ShieldSetWindow::addNewShield(Shield::Colour owner, Gdk::Color colour)
{
  std::string name = Shield::colourToString(owner);
  Shield *shield = new Shield(owner, colour);
  if (shield)
    {
      shield->push_back(new ShieldStyle(ShieldStyle::SMALL));
      shield->push_back(new ShieldStyle(ShieldStyle::MEDIUM));
      shield->push_back(new ShieldStyle(ShieldStyle::LARGE));
      Gtk::TreeIter i = shields_list->append();
      (*i)[shields_columns.name] = name;
      (*i)[shields_columns.shield] = shield;
    }
}

void ShieldSetWindow::loadShield(Shield *shield)
{
  std::string name = Shield::colourToString(Shield::Colour(shield->getOwner()));
  Gtk::TreeIter i = shields_list->append();
  (*i)[shields_columns.name] = name;
  (*i)[shields_columns.shield] = shield;
}

void ShieldSetWindow::update_window_title()
{
  std::string title = "";
  if (needs_saving)
    title += "*";
  title += File::get_basename(current_save_filename, true);
  title += " - ";
  title += _("LordsAWar! Shieldset Editor");
  window->set_title(title);
}
