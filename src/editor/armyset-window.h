//  Copyright (C) 2007, 2008 Ben Asselstine
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

#ifndef GUI_ARMYLIST_WINDOW_H
#define GUI_ARMYLIST_WINDOW_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <libglademm/xml.h>
#include <gtkmm/window.h>
#include <gtkmm/dialog.h>
#include <gtkmm/container.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treeview.h>
#include <gtkmm/textview.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/image.h>
#include <gtkmm/button.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/table.h>
#include <gtkmm/checkmenuitem.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/tooltips.h>

#include "../army.h"
#include "../armyset.h"

class ArmySetWindow: public sigc::trackable
{
 public:
    ArmySetWindow();
    ~ArmySetWindow();

    void show();
    void hide();

    // initialize the SDL widget 
    void init(int width, int height);

    sigc::signal<void> sdl_initialized;
    Gtk::Window &get_window() { return *window.get(); }

 private:
    std::auto_ptr<Gtk::Window> window;
    Gtk::Container *sdl_container;
    Gtk::Widget *sdl_widget;
    std::string current_save_filename;
    Armyset *d_armyset; //current armyset
    Army *d_army; //current army
    Gtk::Image *army_image;
    Gtk::Entry *name_entry;
    Gtk::TreeView *armies_treeview;
    Gtk::TextView *description_textview;
    Gtk::FileChooserButton *image_filechooserbutton;
    Gtk::SpinButton *production_spinbutton;
    Gtk::SpinButton *cost_spinbutton;
    Gtk::SpinButton *upkeep_spinbutton;
    Gtk::SpinButton *strength_spinbutton;
    Gtk::SpinButton *moves_spinbutton;
    Gtk::SpinButton *exp_spinbutton;
    Gtk::CheckButton *hero_checkbutton;
    Gtk::CheckButton *awardable_checkbutton;
    Gtk::CheckButton *defends_ruins_checkbutton;
    Gtk::SpinButton *sight_spinbutton;
    Gtk::CheckButton *move_forests_checkbutton;
    Gtk::CheckButton *move_marshes_checkbutton;
    Gtk::CheckButton *move_hills_checkbutton;
    Gtk::CheckButton *move_mountains_checkbutton;
    Gtk::CheckButton *can_fly_checkbutton;
    Gtk::CheckButton *add1strinopen_checkbutton;
    Gtk::CheckButton *add2strinopen_checkbutton;
    Gtk::CheckButton *add1strinforest_checkbutton;
    Gtk::CheckButton *add1strinhills_checkbutton;
    Gtk::CheckButton *add1strincity_checkbutton;
    Gtk::CheckButton *add2strincity_checkbutton;
    Gtk::CheckButton *add1stackinhills_checkbutton;
    Gtk::CheckButton *suballcitybonus_checkbutton;
    Gtk::CheckButton *sub1enemystack_checkbutton;
    Gtk::CheckButton *add1stack_checkbutton;
    Gtk::CheckButton *add2stack_checkbutton;
    Gtk::CheckButton *suballnonherobonus_checkbutton;
    Gtk::CheckButton *suballherobonus_checkbutton;
    Gtk::Button *add_army_button;
    Gtk::Button *remove_army_button;
    Gtk::VBox *army_vbox;
    bool sdl_inited;
    Gtk::MenuItem *new_armyset_menuitem;
    Gtk::MenuItem *load_armyset_menuitem;
    Gtk::MenuItem *save_armyset_menuitem;
    Gtk::MenuItem *save_armyset_as_menuitem;
    Gtk::MenuItem *edit_armyset_info_menuitem;

    class ArmiesColumns: public Gtk::TreeModelColumnRecord {
    public:
	ArmiesColumns() 
        { add(name); add(army);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Army *> army;
    };
    const ArmiesColumns armies_columns;
    Glib::RefPtr<Gtk::ListStore> armies_list;

    bool on_delete_event(GdkEventAny *e);

    void addArmyType(Uint32 army_type);
    void update_army_panel();
    void update_armyset_buttons();
    void update_armyset_menuitems();

    void on_new_armyset_activated();
    void on_load_armyset_activated();
    void on_save_armyset_activated();
    void on_save_armyset_as_activated();
    void on_quit_activated();
    void on_edit_armyset_info_activated();
    void on_army_selected();
    void fill_army_info(Army *army);

    bool load(std::string tag, XML_Helper *helper);

    //callbacks
    void on_name_changed();
    void on_description_changed();
    void on_image_changed();
    void on_production_changed();
    void on_cost_changed();
    void on_upkeep_changed();
    void on_strength_changed();
    void on_moves_changed();
    void on_exp_changed();
    void on_sight_changed();
    void on_hero_toggled();
    void on_awardable_toggled();
    void on_defends_ruins_toggled();

    void on_move_forests_toggled();
    void on_move_marshes_toggled();
    void on_move_hills_toggled();
    void on_move_mountains_toggled();
    void on_can_fly_toggled();
    void on_add1strinopen_toggled();
    void on_add2strinopen_toggled();
    void on_add1strinforest_toggled();
    void on_add1strinhills_toggled();
    void on_add1strincity_toggled();
    void on_add2strincity_toggled();
    void on_add1stackinhills_toggled();
    void on_suballcitybonus_toggled();
    void on_sub1enemystack_toggled();
    void on_add1stack_toggled();
    void on_add2stack_toggled();
    void on_suballnonherobonus_toggled();
    void on_suballherobonus_toggled();
    void on_add_army_clicked();
    void on_remove_army_clicked();
public:
    // not part of the API, but for surface_attached_helper
    void on_sdl_surface_changed();
};

#endif
