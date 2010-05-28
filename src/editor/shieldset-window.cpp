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
#include "recently-edited-file-list.h"

#include "ucompose.hpp"

#include "glade-helpers.h"
#include "masked-image-editor-dialog.h"
#include "recently-edited-file.h"
#include "editor-quit-dialog.h"
#include "editor-recover-dialog.h"


ShieldSetWindow::ShieldSetWindow(std::string load_filename)
{
  autosave = File::getSavePath() + "autosave" + Shieldset::file_extension;
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

    if (load_filename != "")
      current_save_filename = load_filename;
    update_shieldset_menuitems();

    if (File::exists(autosave))
      {
        std::string m;
        std::list<RecentlyEditedFile*> files = RecentlyEditedFileList::getInstance()->getFilesWithExtension(Shieldset::file_extension);
        if (files.size() == 0)
          m = _("Do you want to recover the session?");
        else
          {
            RecentlyEditedShieldsetFile *r = dynamic_cast<RecentlyEditedShieldsetFile*>(files.front());
            if (r->getName() == "")
              m = String::ucompose(_("Do you want to recover %1?"),
                                   File::get_basename(r->getFileName(), true));
            else if (r->getName() != "" & r->getImagesNeeded() == 0)
              m = String::ucompose(_("Do you want to recover %1 (%2)?"),
                                   File::get_basename(r->getFileName(), true),
                                   r->getName());
            else if (r->getName() != "" && r->getImagesNeeded() > 0)
              m = String::ucompose
                (_("Do you want to recover %1 (%2, %3 images needed)?"),
                 File::get_basename(r->getFileName(), true),
                 r->getName(),
                 r->getImagesNeeded());
          }
        EditorRecoverDialog d(m);
        int response = d.run();
        d.hide();
        //ask if we want to recover the autosave.
        if (response == Gtk::RESPONSE_ACCEPT)
          {
            load_shieldset (autosave);
            update_shieldset_menuitems();
            update_shield_panel();
            return;
          }
      }

    if (load_filename.empty() == false)
      {
	bool success = load_shieldset (load_filename);
        if (success)
          {
            update_shieldset_menuitems();
            update_shield_panel();
            update_window_title();
          }
      }
}

void
ShieldSetWindow::update_shieldset_menuitems()
{
  if (d_shieldset == NULL)
    {
      save_shieldset_menuitem->set_sensitive(false);
      save_as_menuitem->set_sensitive(false);
      validate_shieldset_menuitem->set_sensitive(false);
      edit_shieldset_info_menuitem->set_sensitive(false);
    }
  else
    {
      save_shieldset_menuitem->set_sensitive(true);
      save_as_menuitem->set_sensitive(true);
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
  ShieldSetInfoDialog d(shieldset, File::getUserShieldsetDir(), false,
                        _("Make a New Shieldset"));
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
  std::string dir = File::getUserShieldsetDir();
  d_shieldset->setDirectory(dir);
  current_save_filename = d_shieldset->getConfigurationFile();
  RecentlyEditedFileList *refl = RecentlyEditedFileList::getInstance();
  refl->updateEntry(current_save_filename);
  refl->save();
  d_shieldset->setDirectory(File::get_dirname(autosave));
  d_shieldset->setBaseName(File::get_basename(autosave));

  //populate the list with initial entries.
  for (unsigned int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
    addNewShield(Shield::Colour(i), Shield::get_default_color_for_no(i));
  update_shieldset_menuitems();

  d_shieldset->save(autosave, Shieldset::file_extension);
  update_shield_panel();
  needs_saving = true;
  update_window_title();
}

void ShieldSetWindow::on_load_shieldset_activated()
{
  Gtk::FileChooserDialog chooser(*window, 
				 _("Choose an Shieldset to Load"));
  Gtk::FileFilter sav_filter;
  sav_filter.add_pattern("*" + SHIELDSET_EXT);
  chooser.set_filter(sav_filter);
  chooser.set_current_folder(File::getUserShieldsetDir());

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);
      
  chooser.show_all();
  int res = chooser.run();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      bool success = load_shieldset(chooser.get_filename());
      chooser.hide();
      if (success)
        {
          needs_saving = false;
          update_window_title();
        }
    }

  update_shieldset_menuitems();
  update_shield_panel();
}

void ShieldSetWindow::on_validate_shieldset_activated()
{
  std::list<std::string> msgs;
  if (d_shieldset == NULL)
    return;
  bool valid = d_shieldset->validateNumberOfShields();
  if (!valid)
    msgs.push_back(_("The shieldset must have 9 shields in it."));

  for (unsigned int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
    {
      valid = d_shieldset->validateShieldImages(Shield::Colour(i));
      if (!valid)
        {
          std::string s;
          s = String::ucompose(_("%1 must have all three images specified."),
                               Shield::colourToString(Shield::Colour(i)));
          msgs.push_back(s);
          break;
        }
    }

  std::string msg = "";
  for (std::list<std::string>::iterator it = msgs.begin(); it != msgs.end();
       it++)
    msg += (*it) + "\n";

  if (msg == "")
    msg = _("The shieldset is valid.");
      
  Gtk::MessageDialog dialog(*window, msg);
  dialog.run();
  dialog.hide();

  return;
}

void ShieldSetWindow::on_save_as_activated()
{
  std::string orig_basename = d_shieldset->getBaseName();
  guint32 orig_id = d_shieldset->getId();
  d_shieldset->setId(Shieldsetlist::getNextAvailableId(orig_id));
  ShieldSetInfoDialog d(d_shieldset, File::getUserShieldsetDir() + File::get_basename(current_save_filename, true), false,
                        _("Save a Copy of a Shieldset"));
  d.set_parent_window(*window);
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      std::string new_basename = d_shieldset->getBaseName();
      guint32 new_id = d_shieldset->getId();
      d_shieldset->setId(orig_id);
      d_shieldset->setBaseName(orig_basename);
      bool success = Shieldsetlist::getInstance()->addToPersonalCollection(d_shieldset, new_basename, new_id);
      if (success)
        {
          save_shieldset_menuitem->set_sensitive(true);
          current_save_filename = d_shieldset->getConfigurationFile();
          RecentlyEditedFileList *refl = RecentlyEditedFileList::getInstance();
          refl->updateEntry(current_save_filename);
          refl->save();
          load_shieldset(current_save_filename);
          needs_saving = false;
          update_window_title();
        }
      else
        {
          std::string msg;
          msg = _("Error!  Shieldset could not be saved.");
          Gtk::MessageDialog dialog(*window, msg);
          dialog.run();
          dialog.hide();
          d_shieldset->setId(orig_id);
        }
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
  
  bool success = d_shieldset->save(autosave, Shieldset::file_extension);
  if (success)
    {
      RecentlyEditedFileList::getInstance()->updateEntry(current_save_filename);
      RecentlyEditedFileList::getInstance()->save();
      success = Shieldset::copy (autosave, current_save_filename);
      if (success == true)
        {
          needs_saving = false;
          update_window_title();
          shieldset_saved.emit(d_shieldset->getId());
        }
    }
  if (!success)
    {
      std::string msg;
      msg = _("Error!  Shieldset could not be saved.");
      Gtk::MessageDialog dialog(*window, msg);
      dialog.run();
      dialog.hide();
    }
}

void ShieldSetWindow::on_edit_shieldset_info_activated()
{
  ShieldSetInfoDialog d(d_shieldset, current_save_filename, true, 
                        _("Edit Shieldset Information"));
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

bool ShieldSetWindow::load_shieldset(std::string filename)
{
  std::string old_current_save_filename = current_save_filename;
  current_save_filename = filename;
  if (filename != autosave)
    Shieldset::copy(current_save_filename, autosave);
  else
    {
      //go get the real name of the file
      std::list<RecentlyEditedFile*> files = RecentlyEditedFileList::getInstance()->getFilesWithExtension(Shieldset::file_extension);
      if (files.size() > 0)
        current_save_filename = files.front()->getFileName();
    }

  std::string name = File::get_basename(filename);

  Shieldset *shieldset = Shieldset::create(autosave);
  if (shieldset == NULL)
    {
      std::string msg;
      msg = _("Error!  Shieldset could not be loaded.");
      Gtk::MessageDialog dialog(*window, msg);
      current_save_filename = old_current_save_filename;
      dialog.run();
      dialog.hide();
      return false;
    }
  if (File::nameEndsWith(current_save_filename, "/autosave" + Shieldset::file_extension) == false)
    {
      RecentlyEditedFileList::getInstance()->addEntry(current_save_filename);
      RecentlyEditedFileList::getInstance()->save();
    }
  shields_list->clear();
  if (d_shieldset)
    delete d_shieldset;
  d_shieldset = shieldset;

  d_shieldset->setBaseName(File::get_basename(autosave));
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
  return true;

}

bool ShieldSetWindow::quit()
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
	on_save_shieldset_activated();
      //else if (Response == Gtk::CLOSE) // don't save just quit
      window->hide();
    }
  else
    window->hide();
  File::erase(autosave);
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
	filename = d_shieldset->getFileFromConfigurationFile(ss->getImageName() +".png");
      MaskedImageEditorDialog d(filename, d_shieldset);
      d.set_icon_from_file(File::getMiscFile("various/castle_icon.png"));
      d.run();
      if (filename != "")
        File::erase(filename);
      if (d.get_selected_filename() != "")
	{
	  std::string file = File::get_basename(d.get_selected_filename());
	  if (d.get_selected_filename() != filename)
	    {
              d_shieldset->replaceFileInConfigurationFile(ss->getImageName()+".png", d.get_selected_filename());
              ss->setImageName(file);
              needs_saving = true;
              update_window_title();
	    }
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
	filename = d_shieldset->getFileFromConfigurationFile(ss->getImageName() +".png");
      MaskedImageEditorDialog d(filename, d_shieldset);
      d.set_icon_from_file(File::getMiscFile("various/castle_icon.png"));
      d.run();
      if (d.get_selected_filename() != "")
	{
	  std::string file = File::get_basename(d.get_selected_filename());
	  if (d.get_selected_filename() != d_shieldset->getFile(file))
            {
              d_shieldset->replaceFileInConfigurationFile(ss->getImageName()+".png", d.get_selected_filename());
              ss->setImageName(file);
              needs_saving = true;
              update_window_title();
            }
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
	filename = d_shieldset->getFileFromConfigurationFile(ss->getImageName() +".png");
      MaskedImageEditorDialog d(filename, d_shieldset);
      d.set_icon_from_file(File::getMiscFile("various/castle_icon.png"));
      d.run();
      if (d.get_selected_filename() != "")
	{
	  std::string file = File::get_basename(d.get_selected_filename());
	  if (d.get_selected_filename() != d_shieldset->getFile(file))
            {
              d_shieldset->replaceFileInConfigurationFile(ss->getImageName()+".png", d.get_selected_filename());
              ss->setImageName(file);
              needs_saving = true;
              update_window_title();
            }
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
      d_shieldset->push_back(shield);
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
