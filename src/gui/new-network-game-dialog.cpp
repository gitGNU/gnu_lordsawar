//  Copyright (C) 2011 Ben Asselstine
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
#include <sigc++/functors/mem_fun.h>

#include "new-network-game-dialog.h"

#include "glade-helpers.h"
#include "input-helpers.h"
#include "defs.h"
#include "File.h"
#include "ucompose.hpp"
#include "profile.h"
#include "profilelist.h"
#include "new-profile-dialog.h"

NewNetworkGameDialog::NewNetworkGameDialog()
{
  Glib::RefPtr<Gtk::Builder> xml = Gtk::Builder::create_from_file
    (get_glade_path() + "/new-network-game-dialog.ui");

  xml->get_widget("dialog", dialog);
  xml->get_widget("client_radiobutton", client_radiobutton);
  client_radiobutton->signal_toggled().connect(sigc::mem_fun(*this, &NewNetworkGameDialog::on_client_radiobutton_toggled));
  xml->get_widget("accept_button", accept_button);
  xml->get_widget("add_button", add_button);
  add_button->signal_clicked().connect(sigc::mem_fun(*this, &NewNetworkGameDialog::on_add_button_clicked));
  xml->get_widget("remove_button", remove_button);
  remove_button->signal_clicked().connect(sigc::mem_fun(*this, &NewNetworkGameDialog::on_remove_button_clicked));
  xml->get_widget("profiles_treeview", profiles_treeview);
  profiles_list = Gtk::ListStore::create(profiles_columns);
  profiles_treeview->set_model(profiles_list);
  profiles_treeview->append_column("", profiles_columns.nickname);

  if (Profilelist::getInstance()->empty() == true)
    Profilelist::getInstance()->push_back(new Profile(Glib::get_user_name()));

  for (Profilelist::iterator i = Profilelist::getInstance()->begin();
       i != Profilelist::getInstance()->end(); i++)
    add_profile(*i);

  xml->get_widget("advertise_checkbutton", advertise_checkbutton);
  decorate(dialog);
  dialog->set_icon_from_file(File::getMiscFile("various/castle_icon.png"));

  advertise_checkbutton->set_label(String::ucompose(_("List the game on %1."),
                                   Configuration::s_gamelist_server_hostname));

  select_preferred_profile(Glib::get_user_name());
  update_buttons();
  profiles_treeview->get_selection()->signal_changed().connect
    (sigc::mem_fun(*this, &NewNetworkGameDialog::on_profile_selected));
}

void NewNetworkGameDialog::select_preferred_profile(Glib::ustring user)
{
  Profile *p = Profilelist::getInstance()->findLastPlayedProfileForUser(user);
  Gtk::TreeModel::Children kids = profiles_list->children();
  for (Gtk::TreeModel::Children::iterator i = kids.begin(); 
       i != kids.end(); i++)
    {
      Gtk::TreeModel::Row row = *i;
      if (row[profiles_columns.profile] == p)
        {
          profiles_treeview->get_selection()->select(row);
          return;
        }
    }

  Gtk::TreeModel::Row row;
  row = profiles_treeview->get_model()->children()[0];
  if(row)
    profiles_treeview->get_selection()->select(row);
}

void NewNetworkGameDialog::add_profile(Profile *profile)
{
    Gtk::TreeIter i = profiles_list->append();
    (*i)[profiles_columns.nickname] = profile->getNickname();
    (*i)[profiles_columns.profile] = profile;
}
	    
NewNetworkGameDialog::~NewNetworkGameDialog()
{
  delete dialog;
}

void NewNetworkGameDialog::update_buttons()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = 
    profiles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();
  if (iterrow) 
    accept_button->set_sensitive(true);
  else
    accept_button->set_sensitive(false);

  if (Configuration::s_gamelist_server_hostname != "" &&
      Configuration::s_gamelist_server_port != 0)
    advertise_checkbutton->set_sensitive(!client_radiobutton->get_active());
  else
    advertise_checkbutton->set_sensitive(false);
}

void NewNetworkGameDialog::set_parent_window(Gtk::Window &parent)
{
  dialog->set_transient_for(parent);
}

void NewNetworkGameDialog::hide()
{
  dialog->hide();
}

bool NewNetworkGameDialog::run()
{
  int response = dialog->run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      Profilelist::getInstance()->save();
      Glib::RefPtr<Gtk::TreeSelection> selection = 
        profiles_treeview->get_selection();
      Gtk::TreeModel::iterator iterrow = selection->get_selected();
      if (iterrow) 
        {
          Gtk::TreeModel::Row row = *iterrow;
          d_profile = row[profiles_columns.profile];
          d_profile->play();
          Profilelist::getInstance()->save();
        }
      return true;
    }
  return false;
}

void NewNetworkGameDialog::on_add_button_clicked()
{
  NewProfileDialog d("");
  d.set_parent_window(*dialog);
  if (d.run())
    {
      Profile *profile = new Profile (d.getNickname());
      Profilelist::getInstance()->push_back(profile);
      add_profile(profile);
    }
  update_buttons();
}

void NewNetworkGameDialog::on_remove_button_clicked()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = 
    profiles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();
  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Profile *profile = row[profiles_columns.profile];
      if (profile)
        Profilelist::getInstance()->remove(profile);
      profiles_list->erase(iterrow);
    }
  update_buttons();
}

void NewNetworkGameDialog::on_profile_selected()
{
  update_buttons();
}
  
void NewNetworkGameDialog::on_client_radiobutton_toggled()
{
  update_buttons();
}
