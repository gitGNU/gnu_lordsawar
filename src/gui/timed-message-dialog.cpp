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

#include <assert.h>
#include <gtkmm/messagedialog.h>

#include "timed-message-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "timing.h"

TimedMessageDialog::TimedMessageDialog(Gtk::Window &parent, std::string message, int timeout, int grace)
{
  d_timeout = timeout;
  d_timer_count = 0;
  d_grace = grace;
    
  window = new Gtk::MessageDialog(parent, message);
  //Gtk::MessageDialog dialog(parent, message);
  //window.reset(&dialog);
  window->set_message(message);
  window->signal_response().connect
       (sigc::mem_fun(*this, &TimedMessageDialog::on_response));
}

void TimedMessageDialog::on_response(int id)
{
  window->hide();
  main_loop->quit();
}

TimedMessageDialog::~TimedMessageDialog()
{
  delete window;
}

void TimedMessageDialog::show_all()
{
    window->show_all();
}

void TimedMessageDialog::hide()
{
  window->hide();
}

void TimedMessageDialog::run()
{
  if (d_timeout > 0)
    {
      Timing::instance().register_timer
	(sigc::mem_fun(this, &TimedMessageDialog::tick), 1000);
    }
    
    window->show_all();
    main_loop = Glib::MainLoop::create();
    main_loop->run();
}

bool TimedMessageDialog::tick()
{
  d_timer_count++;
  if (d_grace)
    {
      if (d_timer_count >= d_grace)
	{
	  d_grace = 0;
	  d_timer_count = 0;
	  return Timing::CONTINUE;
	}
    }
  else
    {
      int secs = d_timeout - d_timer_count;
      Glib::ustring s;
      s = String::ucompose(ngettext("This message will disappear in %1 second.",
				    "This message will disappear in %1 seconds.",
				    secs), secs);
      window->set_secondary_text(s);
    }

  if (d_timer_count <= d_timeout)
    return Timing::CONTINUE;

  window->hide();
  main_loop->quit();

  return Timing::STOP;
}
