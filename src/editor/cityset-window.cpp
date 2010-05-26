//  Copyright (C) 2009, 2010 Ben Asselstine
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
#include <string.h>

#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>

#include <gtkmm.h>
#include "cityset-window.h"
#include "cityset-info-dialog.h"
#include "masked-image-editor-dialog.h"

#include "image-helpers.h"
#include "input-helpers.h"
#include "error-utils.h"

#include "defs.h"
#include "File.h"

#include "ucompose.hpp"

#include "glade-helpers.h"
#include "image-editor-dialog.h"
#include "GraphicsCache.h"
#include "citysetlist.h"
#include "recently-edited-file.h"
#include "recently-edited-file-list.h"
#include "tile-size-editor-dialog.h"
#include "editor-quit-dialog.h"
#include "editor-recover-dialog.h"


CitySetWindow::CitySetWindow(std::string load_filename)
{
  autosave = File::getSavePath() + "autosave" + Cityset::file_extension;
  needs_saving = false;
  d_cityset = NULL;
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path() + "/cityset-window.ui");

    xml->get_widget("window", window);
    window->set_icon_from_file(File::getMiscFile("various/castle_icon.png"));
    window->signal_delete_event().connect
      (sigc::mem_fun(*this, &CitySetWindow::on_window_closed));

    xml->get_widget("cityset_frame", cityset_frame);
    xml->get_widget("new_cityset_menuitem", new_cityset_menuitem);
    new_cityset_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &CitySetWindow::on_new_cityset_activated));
    xml->get_widget("load_cityset_menuitem", load_cityset_menuitem);
    load_cityset_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &CitySetWindow::on_load_cityset_activated));
    xml->get_widget("save_cityset_menuitem", save_cityset_menuitem);
    save_cityset_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &CitySetWindow::on_save_cityset_activated));
    xml->get_widget("save_as_menuitem", save_as_menuitem);
    save_as_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &CitySetWindow::on_save_as_activated));
    xml->get_widget("validate_cityset_menuitem", validate_cityset_menuitem);
    validate_cityset_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &CitySetWindow::on_validate_cityset_activated));
    xml->get_widget("quit_menuitem", quit_menuitem);
    quit_menuitem->signal_activate().connect
       (sigc::mem_fun(this, &CitySetWindow::on_quit_activated));
    xml->get_widget("edit_cityset_info_menuitem", edit_cityset_info_menuitem);
    edit_cityset_info_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &CitySetWindow::on_edit_cityset_info_activated));
    xml->get_widget ("help_about_menuitem", help_about_menuitem);
    help_about_menuitem->signal_activate().connect
       (sigc::mem_fun(this, &CitySetWindow::on_help_about_activated));
    xml->get_widget("city_tile_width_spinbutton", city_tile_width_spinbutton);
    city_tile_width_spinbutton->set_range (1, 4);
    city_tile_width_spinbutton->signal_changed().connect
      (sigc::mem_fun(this, &CitySetWindow::on_city_tile_width_changed));
    city_tile_width_spinbutton->signal_insert_text().connect
      (sigc::mem_fun(this, &CitySetWindow::on_city_tile_width_text_changed));
    xml->get_widget("ruin_tile_width_spinbutton", ruin_tile_width_spinbutton);
    ruin_tile_width_spinbutton->set_range (1, 4);
    ruin_tile_width_spinbutton->signal_changed().connect
      (sigc::mem_fun(this, &CitySetWindow::on_ruin_tile_width_changed));
    ruin_tile_width_spinbutton->signal_insert_text().connect
      (sigc::mem_fun(this, &CitySetWindow::on_ruin_tile_width_text_changed));
    xml->get_widget("temple_tile_width_spinbutton", 
		    temple_tile_width_spinbutton);
    temple_tile_width_spinbutton->set_range (1, 4);
    temple_tile_width_spinbutton->signal_changed().connect
      (sigc::mem_fun(this, &CitySetWindow::on_temple_tile_width_changed));
    temple_tile_width_spinbutton->signal_insert_text().connect
      (sigc::mem_fun(this, &CitySetWindow::on_temple_tile_width_text_changed));

    window->signal_delete_event().connect
      (sigc::mem_fun(*this, &CitySetWindow::on_delete_event));
    xml->get_widget("change_citypics_button", change_citypics_button);
    change_citypics_button->signal_clicked().connect(
	sigc::mem_fun(this, &CitySetWindow::on_change_citypics_clicked));
    xml->get_widget("change_razedcitypics_button", change_razedcitypics_button);
    change_razedcitypics_button->signal_clicked().connect(
	sigc::mem_fun(this, &CitySetWindow::on_change_razedcitypics_clicked));
    xml->get_widget("change_portpic_button", change_portpic_button);
    change_portpic_button->signal_clicked().connect(
	sigc::mem_fun(this, &CitySetWindow::on_change_portpic_clicked));
    xml->get_widget("change_signpostpic_button", change_signpostpic_button);
    change_signpostpic_button->signal_clicked().connect(
	sigc::mem_fun(this, &CitySetWindow::on_change_signpostpic_clicked));
    xml->get_widget("change_ruinpics_button", change_ruinpics_button);
    change_ruinpics_button->signal_clicked().connect(
	sigc::mem_fun(this, &CitySetWindow::on_change_ruinpics_clicked));
    xml->get_widget("change_templepic_button", change_templepic_button);
    change_templepic_button->signal_clicked().connect(
	sigc::mem_fun(this, &CitySetWindow::on_change_templepic_clicked));
    xml->get_widget("change_towerpics_button", change_towerpics_button);
    change_towerpics_button->signal_clicked().connect(
	sigc::mem_fun(this, &CitySetWindow::on_change_towerpics_clicked));

    if (load_filename != "")
      current_save_filename = load_filename;
    update_cityset_panel();

    if (File::exists(autosave))
      {
        std::string m;
        std::list<RecentlyEditedFile*> files = RecentlyEditedFileList::getInstance()->getFilesWithExtension(Cityset::file_extension);
        if (files.size() == 0)
          m = _("Do you want to recover the session?");
        else
          {
            RecentlyEditedCitysetFile *r = dynamic_cast<RecentlyEditedCitysetFile*>(files.front());
            if (r->getName() == "")
              m = String::ucompose(_("Do you want to recover %1?"),
                                   File::get_basename(r->getFileName(), true));
            else
              m = String::ucompose(_("Do you want to recover %1 (%2)?"),
                                   File::get_basename(r->getFileName(), true),
                                   r->getName());
          }
        EditorRecoverDialog d(m);
        int response = d.run();
        d.hide();
        //ask if we want to recover the autosave.
        if (response == Gtk::RESPONSE_ACCEPT)
          {
            load_cityset (autosave);
            update_cityset_menuitems();
            update_cityset_panel();
            return;
          }
      }

    if (load_filename.empty() == false)
      {
	load_cityset (load_filename);
	update_cityset_panel();
        update_window_title();
      }
    update_cityset_menuitems();
}

void
CitySetWindow::update_cityset_menuitems()
{
  if (d_cityset == NULL)
    {
      save_cityset_menuitem->set_sensitive(false);
      save_as_menuitem->set_sensitive(false);
      validate_cityset_menuitem->set_sensitive(false);
      edit_cityset_info_menuitem->set_sensitive(false);
    }
  else
    {
      save_cityset_menuitem->set_sensitive(true);
      save_as_menuitem->set_sensitive(true);
      edit_cityset_info_menuitem->set_sensitive(true);
      validate_cityset_menuitem->set_sensitive(true);
    }
}

void
CitySetWindow::update_cityset_panel()
{
  cityset_frame->set_sensitive(d_cityset != NULL);
  std::string no_image = _("no image set");
  std::string s;
  if (d_cityset && d_cityset->getCitiesFilename().empty() == false)
    s = d_cityset->getCitiesFilename() + ".png";
  else
    s = no_image;
  change_citypics_button->set_label(s);
  if (d_cityset && d_cityset->getRazedCitiesFilename().empty() == false)
    s = d_cityset->getRazedCitiesFilename() + ".png";
  else
    s = no_image;
  change_razedcitypics_button->set_label(s);
  if (d_cityset && d_cityset->getPortFilename().empty() == false)
    s = d_cityset->getPortFilename() + ".png";
  else
    s = no_image;
  change_portpic_button->set_label(s);
  if (d_cityset && d_cityset->getSignpostFilename().empty() == false)
    s = d_cityset->getSignpostFilename() + ".png";
  else
    s = no_image;
  change_signpostpic_button->set_label(s);
  if (d_cityset && d_cityset->getRuinsFilename().empty() == false)
    s = d_cityset->getRuinsFilename() + ".png";
  else
    s = no_image;
  change_ruinpics_button->set_label(s);
  if (d_cityset && d_cityset->getTemplesFilename().empty() == false)
    s = d_cityset->getTemplesFilename() + ".png";
  else
    s = no_image;
  change_templepic_button->set_label(s);
  if (d_cityset && d_cityset->getTowersFilename().empty() == false)
    s = d_cityset->getTowersFilename() + ".png";
  else
    s = no_image;
  change_towerpics_button->set_label(s);
  if (d_cityset)
    city_tile_width_spinbutton->set_value(d_cityset->getCityTileWidth());
  else
    city_tile_width_spinbutton->set_value(2);
  if (d_cityset)
    ruin_tile_width_spinbutton->set_value(d_cityset->getRuinTileWidth());
  else
    ruin_tile_width_spinbutton->set_value(1);
  if (d_cityset)
    temple_tile_width_spinbutton->set_value(d_cityset->getTempleTileWidth());
  else
    temple_tile_width_spinbutton->set_value(1);
}

CitySetWindow::~CitySetWindow()
{
  delete window;
}

void CitySetWindow::show()
{
  window->show();
}

void CitySetWindow::hide()
{
  window->hide();
}

bool CitySetWindow::on_delete_event(GdkEventAny *e)
{
  hide();

  return true;
}

void CitySetWindow::on_new_cityset_activated()
{
  std::string name = "";
  int id = Citysetlist::getNextAvailableId(0);
  Cityset *cityset = new Cityset(id, name);
  CitySetInfoDialog d(cityset, File::getUserCitysetDir(), false,
                      _("Make a New Cityset"));
  d.set_parent_window(*window);
  int response = d.run();
  if (response != Gtk::RESPONSE_ACCEPT)
    {
      delete cityset;
      return;
    }
  if (d_cityset)
    delete d_cityset;
  d_cityset = cityset;
  std::string dir = File::getUserCitysetDir();
  d_cityset->setDirectory(dir);
  current_save_filename = d_cityset->getConfigurationFile();
  RecentlyEditedFileList *refl = RecentlyEditedFileList::getInstance();
  refl->updateEntry(current_save_filename);
  refl->save();
  d_cityset->setDirectory(File::get_dirname(autosave));
  d_cityset->setBaseName(File::get_basename(autosave));

  d_cityset->save(autosave, Cityset::file_extension);
  update_cityset_panel();
  update_cityset_menuitems();
  needs_saving = true;
  update_window_title();
}

void CitySetWindow::on_load_cityset_activated()
{
  Gtk::FileChooserDialog chooser(*window, 
				 _("Choose a Cityset to Load"));
  Gtk::FileFilter sav_filter;
  sav_filter.add_pattern("*" + CITYSET_EXT);
  chooser.set_filter(sav_filter);
  chooser.set_current_folder(File::getUserCitysetDir());

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);

  chooser.show_all();
  int res = chooser.run();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      load_cityset(chooser.get_filename());
      chooser.hide();
      needs_saving = false;
      update_window_title();
    }

  update_cityset_menuitems();
  update_cityset_panel();
}

void CitySetWindow::on_validate_cityset_activated()
{
  std::list<std::string> msgs;
  if (d_cityset == NULL)
    return;
  if (d_cityset->validateCitiesFilename() == false)
    msgs.push_back(_("The cities picture is not set."));
  if (d_cityset->validateRazedCitiesFilename() == false)
    msgs.push_back(_("The razed cities picture is not set."));
  if (d_cityset->validatePortFilename() == false)
    msgs.push_back(_("The port picture is not set."));
  if (d_cityset->validateSignpostFilename() == false)
    msgs.push_back(_("The signpost picture is not set."));
  if (d_cityset->validateRuinsFilename() == false)
    msgs.push_back(_("The ruins picture is not set."));
  if (d_cityset->validateTemplesFilename() == false)
    msgs.push_back(_("The temple picture is not set."));
  if (d_cityset->validateTowersFilename() == false)
    msgs.push_back(_("The towers picture is not set."));
  if (d_cityset->validateCityTileWidth() == false)
    msgs.push_back(_("The tile width for temples must be over zero."));
  if (d_cityset->validateRuinTileWidth() == false)
    msgs.push_back(_("The tile width for ruins must be over zero."));
  if (d_cityset->validateTempleTileWidth() == false)
    msgs.push_back(_("The tile width for temples must be over zero."));

  std::string msg = "";
  for (std::list<std::string>::iterator it = msgs.begin(); it != msgs.end();
       it++)
    msg += (*it) + "\n";

  if (msg == "")
    msg = _("The cityset is valid.");
      
  Gtk::MessageDialog dialog(*window, msg);
  dialog.run();
  dialog.hide();

  return;
}

void CitySetWindow::on_save_as_activated()
{
  std::string orig_basename = d_cityset->getBaseName();
  guint32 orig_id = d_cityset->getId();
  d_cityset->setId(Citysetlist::getNextAvailableId(orig_id));
  CitySetInfoDialog d(d_cityset, File::getUserCitysetDir() + File::get_basename(current_save_filename, true), false,
                        _("Save a Copy of a Cityset"));
  d.set_parent_window(*window);
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      std::string new_basename = d_cityset->getBaseName();
      guint32 new_id = d_cityset->getId();
      d_cityset->setId(orig_id);
      d_cityset->setBaseName(orig_basename);
      bool success = Citysetlist::getInstance()->addToPersonalCollection(d_cityset, new_basename, new_id);
      if (success)
        {
          save_cityset_menuitem->set_sensitive(true);
          current_save_filename = d_cityset->getConfigurationFile();
          RecentlyEditedFileList *refl = RecentlyEditedFileList::getInstance();
          refl->updateEntry(current_save_filename);
          refl->save();
          load_cityset(current_save_filename);
          needs_saving = false;
          update_window_title();
        }
      else
        {
          std::string msg;
          msg = _("Error!  Cityset could not be saved.");
          Gtk::MessageDialog dialog(*window, msg);
          dialog.run();
          dialog.hide();
        }
    }
  else
    d_cityset->setId(orig_id);
}

void CitySetWindow::on_save_cityset_activated()
{
  if (current_save_filename.empty())
    current_save_filename = d_cityset->getConfigurationFile();

  guint32 suggested_tile_size = d_cityset->calculate_preferred_tile_size();
  if (suggested_tile_size != d_cityset->getTileSize())
    {
      TileSizeEditorDialog d(d_cityset->getTileSize(), suggested_tile_size);
      int response = d.run();
      if (response == Gtk::RESPONSE_ACCEPT)
        d_cityset->setTileSize(d.get_selected_tilesize());
    }
  bool success = d_cityset->save(autosave, Cityset::file_extension);
  if (success)
    {
      RecentlyEditedFileList::getInstance()->updateEntry(current_save_filename);
      RecentlyEditedFileList::getInstance()->save();
      success = Cityset::copy (autosave, current_save_filename);
      if (success == true)
        {
          needs_saving = false;
          update_window_title();
          cityset_saved.emit(d_cityset->getId());
        }
    }
  if (!success)
    {
      std::string msg;
      msg = _("Error!  Cityset could not be saved.");
      Gtk::MessageDialog dialog(*window, msg);
      dialog.run();
      dialog.hide();
    }
}

void CitySetWindow::on_edit_cityset_info_activated()
{
  CitySetInfoDialog d(d_cityset, current_save_filename, true, 
                        _("Edit Cityset Information"));
  d.set_parent_window(*window);
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      needs_saving = true;
      update_window_title();
    }
}

void CitySetWindow::on_help_about_activated()
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

bool CitySetWindow::load_cityset(std::string filename)
{
  std::string old_current_save_filename = current_save_filename;
  current_save_filename = filename;
  if (filename != autosave)
    Cityset::copy(current_save_filename, autosave);
  else
    {
      //go get the real name of the file
      std::list<RecentlyEditedFile*> files = RecentlyEditedFileList::getInstance()->getFilesWithExtension(Cityset::file_extension);
      if (files.size() > 0)
        current_save_filename = files.front()->getFileName();
    }

  std::string name = File::get_basename(filename);

  Cityset *cityset = Cityset::create(autosave);
  if (cityset == NULL)
    {
      std::string msg;
      msg = _("Error!  Cityset could not be loaded.");
      Gtk::MessageDialog dialog(*window, msg);
      current_save_filename = old_current_save_filename;
      dialog.run();
      dialog.hide();
      return false;
    }
  if (File::nameEndsWith(current_save_filename, "/autosave" + Cityset::file_extension) == false)
    {
      RecentlyEditedFileList::getInstance()->addEntry(current_save_filename);
      RecentlyEditedFileList::getInstance()->save();
    }
  if (d_cityset)
    delete d_cityset;
  d_cityset = cityset;

  d_cityset->setBaseName(File::get_basename(autosave));
  d_cityset->instantiateImages();
  update_window_title();
  return true;
}

bool CitySetWindow::quit()
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
	on_save_cityset_activated();
      //else if (Response == Gtk::CLOSE) // don't save just quit
      window->hide();
    }
  else
    window->hide();
  File::erase(autosave);
  return true;
}

bool CitySetWindow::on_window_closed(GdkEventAny*)
{
  return !quit();
}

void CitySetWindow::on_quit_activated()
{
  quit();
}

void CitySetWindow::on_city_tile_width_text_changed(const Glib::ustring &s, int* p)
{
  city_tile_width_spinbutton->set_value(atoi(city_tile_width_spinbutton->get_text().c_str()));
  on_city_tile_width_changed();
}

void CitySetWindow::on_city_tile_width_changed()
{
  if (!d_cityset)
    return;
  d_cityset->setCityTileWidth(city_tile_width_spinbutton->get_value());
  needs_saving = true;
  update_window_title();
}

void CitySetWindow::on_ruin_tile_width_text_changed(const Glib::ustring &s, int* p)
{
  ruin_tile_width_spinbutton->set_value(atoi(ruin_tile_width_spinbutton->get_text().c_str()));
  on_ruin_tile_width_changed();
}

void CitySetWindow::on_ruin_tile_width_changed()
{
  if (!d_cityset)
    return;
  d_cityset->setRuinTileWidth(ruin_tile_width_spinbutton->get_value());
  needs_saving = true;
  update_window_title();
}

void CitySetWindow::on_temple_tile_width_text_changed(const Glib::ustring &s, int* p)
{
  temple_tile_width_spinbutton->set_value(atoi(temple_tile_width_spinbutton->get_text().c_str()));
  on_temple_tile_width_changed();
}

void CitySetWindow::on_temple_tile_width_changed()
{
  if (!d_cityset)
    return;
  d_cityset->setTempleTileWidth(temple_tile_width_spinbutton->get_value());
  needs_saving = true;
  update_window_title();
}

void CitySetWindow::on_change_citypics_clicked()
{
  std::string filename = "";
  if (d_cityset->getCitiesFilename().empty() == false)
    filename = d_cityset->getFileFromConfigurationFile(d_cityset->getCitiesFilename() +".png");
  ImageEditorDialog d(filename, MAX_PLAYERS + 1);
  int response = d.run();
  if (filename != "")
    File::erase(filename);
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_selected_filename() != filename)
        {
          d_cityset->replaceFileInConfigurationFile(d_cityset->getCitiesFilename()+".png", d.get_selected_filename());

          d_cityset->setCitiesFilename
            (File::get_basename(d.get_selected_filename()));
          needs_saving = true;
          update_window_title();
        }
      update_cityset_panel();
    }
}

void CitySetWindow::on_change_razedcitypics_clicked()
{
  std::string filename = "";
  if (d_cityset->getRazedCitiesFilename().empty() == false)
    filename = d_cityset->getFileFromConfigurationFile(d_cityset->getRazedCitiesFilename() +".png");
  ImageEditorDialog d(filename, MAX_PLAYERS);
  int response = d.run();
  if (filename != "")
    File::erase(filename);
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_selected_filename() != filename)
        {
          d_cityset->replaceFileInConfigurationFile(d_cityset->getRazedCitiesFilename()+".png", d.get_selected_filename());
          d_cityset->setRazedCitiesFilename
            (File::get_basename(d.get_selected_filename()));
          needs_saving = true;
          update_window_title();
        }
      update_cityset_panel();
    }
}

void CitySetWindow::on_change_portpic_clicked()
{
  std::string filename = "";
  if (d_cityset->getPortFilename().empty() == false)
    filename = d_cityset->getFileFromConfigurationFile(d_cityset->getPortFilename() +".png");
  ImageEditorDialog d(filename, 1);
  int response = d.run();
  if (filename != "")
    File::erase(filename);
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_selected_filename() != filename)
        {
          d_cityset->replaceFileInConfigurationFile(d_cityset->getPortFilename()+".png", d.get_selected_filename());
          d_cityset->setPortFilename(File::get_basename(d.get_selected_filename()));
          needs_saving = true;
          update_window_title();
        }
      update_cityset_panel();
    }
}

void CitySetWindow::on_change_signpostpic_clicked()
{
  std::string filename = "";
  if (d_cityset->getSignpostFilename().empty() == false)
    filename = d_cityset->getFileFromConfigurationFile(d_cityset->getSignpostFilename() +".png");
  ImageEditorDialog d(filename, 1);
  int response = d.run();
  if (filename != "")
    File::erase(filename);
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_selected_filename() != filename)
        {
          d_cityset->replaceFileInConfigurationFile(d_cityset->getSignpostFilename()+".png", d.get_selected_filename());

          d_cityset->setSignpostFilename
            (File::get_basename(d.get_selected_filename()));
          needs_saving = true;
          update_window_title();
        }
      update_cityset_panel();
    }
}

void CitySetWindow::on_change_ruinpics_clicked()
{
  std::string filename = "";
  if (d_cityset->getRuinsFilename().empty() == false)
    filename = d_cityset->getFileFromConfigurationFile(d_cityset->getRuinsFilename() +".png");
  ImageEditorDialog d(filename, RUIN_TYPES);
  int response = d.run();
  if (filename != "")
    File::erase(filename);
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_selected_filename() != filename)
        {
          d_cityset->replaceFileInConfigurationFile(d_cityset->getRuinsFilename()+".png", d.get_selected_filename());
          d_cityset->setRuinsFilename
            (File::get_basename(d.get_selected_filename()));
          needs_saving = true;
          update_window_title();
        }
      update_cityset_panel();
    }
}

void CitySetWindow::on_change_templepic_clicked()
{
  std::string filename = "";
  if (d_cityset->getTemplesFilename().empty() == false)
    filename = d_cityset->getFileFromConfigurationFile(d_cityset->getTemplesFilename() +".png");
  ImageEditorDialog d(filename, TEMPLE_TYPES);
  int response = d.run();
  if (filename != "")
    File::erase(filename);
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_selected_filename() != filename)
        {
          d_cityset->replaceFileInConfigurationFile(d_cityset->getTemplesFilename()+".png", d.get_selected_filename());
          d_cityset->setTemplesFilename
            (File::get_basename(d.get_selected_filename()));
          needs_saving = true;
          update_window_title();
        }
      update_cityset_panel();
    }
}

void CitySetWindow::on_change_towerpics_clicked()
{
  std::string filename = "";
  if (d_cityset->getTowersFilename().empty() == false)
    filename = d_cityset->getFileFromConfigurationFile(d_cityset->getTowersFilename() +".png");
  ImageEditorDialog d(filename, MAX_PLAYERS);
  int response = d.run();
  if (filename != "")
    File::erase(filename);
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_selected_filename() != filename)
        {
          d_cityset->replaceFileInConfigurationFile(d_cityset->getTowersFilename()+".png", d.get_selected_filename());
          d_cityset->setTowersFilename
            (File::get_basename(d.get_selected_filename()));
          needs_saving = true;
          update_window_title();
        }
      update_cityset_panel();
    }
}

void CitySetWindow::update_window_title()
{
  std::string title = "";
  if (needs_saving)
    title += "*";
  title += File::get_basename(current_save_filename, true);
  title += " - ";
  title += _("LordsAWar! Cityset Editor");
  window->set_title(title);
}
