//  Copyright (C) 2009 Ben Asselstine
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

#ifndef GUI_CITYSET_WINDOW_H
#define GUI_CITYSET_WINDOW_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <gtkmm.h>

#include "cityset.h"

//! Cityset Editor.  Edit an cityset.
class CitySetWindow: public sigc::trackable
{
 public:
    CitySetWindow(std::string load_filename = "");
    ~CitySetWindow();

    void show();
    void hide();

    Gtk::Window &get_window() { return *window; }

    sigc::signal<void, guint32> cityset_saved;

 private:
    Gtk::Window* window;
    std::string current_save_filename;
    Cityset *d_cityset; //current cityset
    bool needs_saving;
    Gtk::MenuItem *new_cityset_menuitem;
    Gtk::MenuItem *load_cityset_menuitem;
    Gtk::MenuItem *save_cityset_menuitem;
    Gtk::MenuItem *validate_cityset_menuitem;
    Gtk::MenuItem *edit_cityset_info_menuitem;
    Gtk::MenuItem *quit_menuitem;
    Gtk::MenuItem *help_about_menuitem;
    Gtk::Button *change_citypics_button;
    Gtk::Button *change_razedcitypics_button;
    Gtk::Button *change_portpic_button;
    Gtk::Button *change_signpostpic_button;
    Gtk::Button *change_ruinpics_button;
    Gtk::Button *change_templepic_button;
    Gtk::Button *change_towerpics_button;
    Gtk::SpinButton *city_tile_width_spinbutton;
    Gtk::SpinButton *ruin_tile_width_spinbutton;
    Gtk::SpinButton *temple_tile_width_spinbutton;
    Gtk::Frame *cityset_frame;

    bool on_delete_event(GdkEventAny *e);

    void update_cityset_panel();
    void update_cityset_menuitems();

    void load_cityset(std::string filename);

    //callbacks
    void on_new_cityset_activated();
    void on_load_cityset_activated();
    void on_save_cityset_activated();
    void on_validate_cityset_activated();
    void on_quit_activated();
    bool on_window_closed(GdkEventAny*);
    bool quit();
    void on_edit_cityset_info_activated();
    void on_help_about_activated();
    void on_city_tile_width_changed();
    void on_city_tile_width_text_changed(const Glib::ustring &s, int* p);
    void on_ruin_tile_width_changed();
    void on_ruin_tile_width_text_changed(const Glib::ustring &s, int* p);
    void on_temple_tile_width_changed();
    void on_temple_tile_width_text_changed(const Glib::ustring &s, int* p);
    void on_change_citypics_clicked();
    void on_change_razedcitypics_clicked();
    void on_change_portpic_clicked();
    void on_change_signpostpic_clicked();
    void on_change_ruinpics_clicked();
    void on_change_templepic_clicked();
    void on_change_towerpics_clicked();
};

#endif
