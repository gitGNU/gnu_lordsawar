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

#include "decorated.h"
#include <sigc++/functors/mem_fun.h>
#include <gtkmm/window.h>
#include <gtkmm/label.h>
#include <gtkmm/frame.h>
#include <gtkmm/box.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/alignment.h>
#include <gtkmm/button.h>

#include "File.h"
#include "Configuration.h"
using namespace Gtk;
using namespace Gdk;
using namespace Glib;
Decorated::Decorated()
{
}

Decorated::~Decorated()
{
}

void Decorated::decorate(Gtk::Window *d, std::string filename, int alpha)
{
  window = d;
  if (Configuration::s_decorated == false)
    return;
  Glib::RefPtr<Gtk::Style> copy;
  RefPtr<Pixbuf> back;
  RefPtr<Pixmap> pixmap;
  RefPtr<Bitmap> bitmap;
  window->set_decorated(false);
  Gtk::Widget *focus = window->get_focus();
  Gtk::Widget *child = window->get_child();
  window->remove();
  Gtk::Frame *windowdecoration =manage(new Gtk::Frame());
  window->add(*windowdecoration);

  windowdecoration->property_shadow_type() = Gtk::SHADOW_ETCHED_OUT;
  Gtk::HBox *titlebox= new Gtk::HBox();
  Gtk::EventBox *eventbox = manage(new Gtk::EventBox());

  title = manage(new Gtk::Label(window->get_title()));
  Gtk::Alignment *align= manage(new Gtk::Alignment());
  align->set_padding(0, 0, 10, 10);
  title->unset_bg(Gtk::STATE_NORMAL);
  align->add(*title);

  eventbox->add(*align);
  titlebox->pack_start(*eventbox, true, true, 5);
  Gtk::Button *minimize_button = manage(new Gtk::Button());
  minimize_button->set_label("_");
  minimize_button->property_relief() = Gtk::RELIEF_NONE;
  minimize_button->property_can_focus() = false;
  minimize_button->signal_clicked().connect(sigc::mem_fun(window, &Gtk::Window::iconify));
  Gtk::Button *close_button = manage(new Gtk::Button());
  close_button->set_label("x");
  close_button->property_relief() = Gtk::RELIEF_NONE;
  close_button->property_can_focus() = false;
  close_button->signal_clicked().connect(sigc::mem_fun(this, &Decorated::on_hide));
  titlebox->pack_end(*close_button, false, false, 0);
  titlebox->pack_end(*minimize_button, false, false, 0);
  windowdecoration->set_label_align(Gtk::ALIGN_RIGHT);
  windowdecoration->set_label_widget(*titlebox);


  windowdecoration->add(*manage(child));
  titlebox->add_events(Gdk::POINTER_MOTION_MASK);
  titlebox->signal_motion_notify_event().connect
    (sigc::mem_fun(*this, &Decorated::on_mouse_motion_event));
  copy = windowdecoration->get_style()->copy();

  if (filename == "")
    filename = File::getMiscFile("various/background.png");
  back = Pixbuf::create_from_file(filename);
  pixmap = Pixmap::create(window->get_window(), back->get_width(), back->get_height());
  copy->set_bg(Gtk::STATE_NORMAL, Gdk::Color("black"));
  window->set_style(copy);
  back->composite_color(back, 0, 0, back->get_width(), back->get_height(), 0.0, 0.0, 1.0, 1.0, Gdk::INTERP_NEAREST, alpha, 0, 0, 64, 0, 0);
  back->render_pixmap_and_mask(pixmap, bitmap, 10);
  copy->set_bg_pixmap(Gtk::STATE_NORMAL, pixmap);
  windowdecoration->show_all();
  window->set_style(copy);
  if (focus)
  window->set_focus(*focus);
}

bool Decorated::on_mouse_motion_event(GdkEventMotion *e)
{
  int x, y;
  window->get_position(x, y);
  window->move(x + int(e->x), y + int(e->y));
  return true;
}

    
void Decorated::set_title(std::string text)
{
  if (Configuration::s_decorated)
    title->set_text(text);
  else
    window->set_title(text);
}
  
void Decorated::on_hide()
{
  window_closed.emit();
}
