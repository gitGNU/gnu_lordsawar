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
#include <libglademm/xml.h>

#include "glade-helpers.h"
#include "generation-progress-window.h"

GenerationProgressWindow::GenerationProgressWindow()
{
  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + 
				"/generation-progress-window.glade");

  Gtk::Window *w = 0;
  xml->get_widget("window", w);
  window.reset(w);

  xml->get_widget("statusbar", statusbar);
  xml->get_widget("progressbar", progressbar);

  w->signal_delete_event().connect
    (sigc::mem_fun(*this, &GenerationProgressWindow::on_delete_event));

}

GenerationProgressWindow::~GenerationProgressWindow()
{
}

void GenerationProgressWindow::show_all()
{
  window->show_all();
}

void GenerationProgressWindow::update_progress(double fraction, 
					       std::string status)
{
  progressbar->set_fraction(fraction);
  statusbar->push(status, 0);
}

bool GenerationProgressWindow::on_delete_event(GdkEventAny *e)
{
  return true;
}

