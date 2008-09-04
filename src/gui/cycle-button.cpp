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

#include "cycle-button.h"
#include "image-helpers.h"
#include <gtkmm/image.h>

#include "File.h"

CycleButton::CycleButton(std::list<Gtk::Button*> states)
{
  d_notebook = new Gtk::Notebook();
  d_notebook->property_show_border() = false;
  d_notebook->property_show_tabs() = false;
  d_notebook->property_homogeneous() = true;
  std::string file = File::getMiscFile("various/cycle.png");
  Glib::RefPtr<Gdk::Pixbuf> img = Gdk::Pixbuf::create_from_file(file);
  for (std::list<Gtk::Button*>::iterator it = states.begin(); it != states.end();
       it++)
    {
      d_notebook->append_page(**it);
      (*it)->signal_clicked().connect
	(sigc::mem_fun(this, &CycleButton::on_button_clicked));
      (*it)->property_image() = new Gtk::Image(img);
      (*it)->property_image_position() = Gtk::POS_RIGHT;
      (*it)->set_alignment(1.0, 0.5);
    }
}

void CycleButton::show()
{
  d_notebook->show_all();
}

CycleButton::~CycleButton()
{
  delete d_notebook;
}

    
void CycleButton::set_active(int row_number)
{
  d_notebook->set_current_page(row_number);
}

int CycleButton::get_active()
{
  return d_notebook->get_current_page();
}

Glib::ustring CycleButton::get_active_text()
{
  Gtk::Widget *w = d_notebook->get_nth_page(d_notebook->get_current_page());
  return dynamic_cast<Gtk::Button*>(w)->get_label();
}
    
void CycleButton::on_button_clicked()
{
  if (d_notebook->get_current_page() + 1 == d_notebook->get_n_pages())
    set_active(0);
  else
    d_notebook->next_page();
  signal_changed.emit();
}

