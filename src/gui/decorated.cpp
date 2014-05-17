//  Copyright (C) 2008, 2011, 2014 Ben Asselstine
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

#include "decorated.h"
#include <sigc++/functors/mem_fun.h>
#include <gtkmm.h>

#include "defs.h"
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
  maximized = false;
}

void Decorated::decorate(Gtk::Window *d, GraphicsCache::BackgroundType back, int alpha)
{
  window = d;
  /*
  GraphicsCache *gc = GraphicsCache::getInstance();
  if (Configuration::s_decorated == false)
    return;
  Glib::RefPtr<Gtk::Style> copy;
  RefPtr<Pixmap> pixmap;
  RefPtr<Bitmap> bitmap;
  window->set_decorated(true);
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
  //align->add(*title);

  eventbox->add(*align);
  titlebox->pack_start(*eventbox, true, true, 5);
  Gtk::Button *minimize_button = manage(new Gtk::Button());
  minimize_button->set_label("_");
  minimize_button->set_tooltip_text(_("Minimize"));
  minimize_button->property_relief() = Gtk::RELIEF_NONE;
  minimize_button->property_can_focus() = false;
  minimize_button->signal_clicked().connect(sigc::mem_fun(window, &Gtk::Window::iconify));
  if (window->get_type_hint() == Gdk::WINDOW_TYPE_HINT_NORMAL)
    {
      maximize_button = manage(new Gtk::Button());
      maximize_button->set_label("^");
      maximize_button->set_tooltip_text(_("Maximize"));
      maximize_button->property_relief() = Gtk::RELIEF_NONE;
      maximize_button->property_can_focus() = false;
      maximize_button->signal_clicked().connect(sigc::mem_fun(this, &Decorated::on_maximize));
    }
  Gtk::Button *close_button = manage(new Gtk::Button());
  close_button->set_label("x");
  close_button->set_tooltip_text(_("Close"));
  close_button->property_relief() = Gtk::RELIEF_NONE;
  close_button->property_can_focus() = false;
  close_button->signal_clicked().connect(sigc::mem_fun(this, &Decorated::on_hide));

  windowdecoration->add(*manage(child));
  titlebox->add_events(Gdk::POINTER_MOTION_MASK);
  titlebox->signal_motion_notify_event().connect
    (sigc::mem_fun(*this, &Decorated::on_mouse_motion_event));
  copy = windowdecoration->get_style()->copy();

  Glib::RefPtr<Gdk::Pixbuf> b = gc->getBackgroundPic(back)->to_pixbuf();
  pixmap = Pixmap::create(window->get_window(), b->get_width(), b->get_height());
  copy->set_bg(Gtk::STATE_NORMAL, Gdk::Color("black"));
  window->set_style(copy);
  b->composite_color(b, 0, 0, b->get_width(), b->get_height(), 0.0, 0.0, 1.0, 1.0, Gdk::INTERP_NEAREST, alpha, 0, 0, 64, 0, 0);
  b->render_pixmap_and_mask(pixmap, bitmap, 10);
  copy->set_bg_pixmap(Gtk::STATE_NORMAL, pixmap);
  windowdecoration->show_all();
  window->set_style(copy);
  if (focus)
    window->set_focus(*focus);
    */
}

void Decorated::on_maximize()
{
  maximized = !maximized;
  if (maximized)
    {
      window->maximize();
      maximize_button->set_label("#");
      maximize_button->set_tooltip_text(_("Restore"));
    }
  else
    {
      window->unmaximize();
      maximize_button->set_label("^");
      maximize_button->set_tooltip_text(_("Maximize"));
    }
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
    {
      title->set_text(text);
      window->set_title(text);
    }
  else
    window->set_title(text);
}

void Decorated::on_hide()
{
  window_closed.emit();
}

void Decorated::decorate_border(Gtk::Viewport *container, int alpha)
{
  if (Configuration::s_decorated == false)
    return;
  /*
  Glib::RefPtr<Gtk::Style> copy;
  Glib::RefPtr<Gdk::Pixbuf> back;
  Glib::RefPtr<Gdk::Pixmap> pixmap;
  Glib::RefPtr<Gdk::Bitmap> bitmap;
  copy = container->get_style()->copy();
  back = Gdk::Pixbuf::create_from_file
    (File::getMiscFile("various/back.bmp"));
  pixmap = Gdk::Pixmap::create
    (window->get_window(), back->get_width(), back->get_height());
  back->composite_color(back, 0, 0, 
			back->get_width(), back->get_height(), 
			0.0, 0.0, 1.0, 1.0, Gdk::INTERP_NEAREST, alpha, 
			0, 0, 64, 0, 0);
  back->render_pixmap_and_mask(pixmap, bitmap, 10);
  copy->set_bg_pixmap(Gtk::STATE_NORMAL, pixmap);
  container->set_style(copy);
  */
}
