//  Copyright (C) 2007 Ole Laursen
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

#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>

#include "signpost-dialog.h"

#include "glade-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "CreateScenarioRandomize.h"
#include "signpost.h"

SignpostDialog::SignpostDialog(Signpost *s, CreateScenarioRandomize *randomizer)
{
    d_randomizer = randomizer;
    signpost = s;
    
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/signpost-dialog.ui");

    xml->get_widget("dialog", dialog);

    xml->get_widget("sign_textview", sign_textview);
    sign_textview->get_buffer()->set_text(s->getName());
    
    xml->get_widget("randomize_button", randomize_button);
    randomize_button->signal_clicked().connect(
	sigc::mem_fun(this, &SignpostDialog::on_randomize_clicked));
}

SignpostDialog::~SignpostDialog()
{
  delete dialog;
}
void SignpostDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void SignpostDialog::run()
{
  dialog->show_all();
  int response = dialog->run();

  if (response == Gtk::RESPONSE_ACCEPT)	// accepted
    signpost->setName(sign_textview->get_buffer()->get_text());
  else
    {
      if (sign_textview->get_buffer()->get_text() != DEFAULT_SIGNPOST)
	d_randomizer->pushRandomSignpost
	  (sign_textview->get_buffer()->get_text());
    }
}

void SignpostDialog::on_randomize_clicked()
{
  std::string existing_name = sign_textview->get_buffer()->get_text();
  bool dynamic = ((rand() % d_randomizer->getNumSignposts()) == 0);
  if (existing_name == DEFAULT_SIGNPOST)
    {
      if (dynamic)
	sign_textview->get_buffer()->set_text
	  (d_randomizer->getDynamicSignpost(signpost));
      else
	sign_textview->get_buffer()->set_text
	  (d_randomizer->popRandomSignpost());
    }
  else
    {
      if (dynamic)
	sign_textview->get_buffer()->set_text
	  (d_randomizer->getDynamicSignpost(signpost));
      else
	{
	  sign_textview->get_buffer()->set_text
	    (d_randomizer->popRandomSignpost());
	  d_randomizer->pushRandomSignpost(existing_name);
	}
    }
}
