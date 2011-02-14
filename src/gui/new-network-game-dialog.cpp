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

NewNetworkGameDialog::NewNetworkGameDialog(Glib::ustring network_game_nickname)
{
  Glib::RefPtr<Gtk::Builder> xml = Gtk::Builder::create_from_file
    (get_glade_path() + "/new-network-game-dialog.ui");

  xml->get_widget("dialog", dialog);
  xml->get_widget("client_radiobutton", client_radiobutton);
  xml->get_widget("accept_button", accept_button);

  decorate(dialog);
  dialog->set_icon_from_file(File::getMiscFile("various/castle_icon.png"));
  xml->get_widget("nick_entry", nick_entry);
  std::string nick;
  if (getenv("USER"))
    nick = getenv("USER");
  else if (network_game_nickname != "")
    nick = network_game_nickname;
  else
    nick = "guest";
  nick_entry->set_text(nick);
  nick_entry->set_activates_default(true);
  nick_entry->signal_changed().connect 
    (sigc::mem_fun(*this, &NewNetworkGameDialog::on_nickname_changed));
  update_buttons();
}
	    
NewNetworkGameDialog::~NewNetworkGameDialog()
{
  delete dialog;
}

void NewNetworkGameDialog::on_nickname_changed()
{
  update_buttons();
}

void NewNetworkGameDialog::update_buttons()
{
  if (String::utrim(nick_entry->get_text()) == "")
    accept_button->set_sensitive(false);
  else
    {
      accept_button->set_sensitive(true);
      accept_button->property_can_focus() = true;
      accept_button->property_can_default() = true;
      accept_button->property_has_default() = true;
      nick_entry->property_activates_default() = true;
      accept_button->property_receives_default() = true;
    }
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
    return true;
  return false;
}
