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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef GAME_PREFERENCES_DIALOG_H
#define GAME_PREFERENCES_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <gtkmm/window.h>
#include <gtkmm/dialog.h>
#include <gtkmm/combobox.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/cellrenderercombo.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/treeview.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/widget.h>
#include <gtkmm/scale.h>

#include "../game-parameters.h"


// dialog for choosing parameters for starting a new game
class GamePreferencesDialog
{
 public:
    GamePreferencesDialog();
    ~GamePreferencesDialog();

    void set_parent_window(Gtk::Window &parent);

    sigc::signal<void, GameParameters> game_started;
    
    void run();
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;

    Gtk::ComboBoxText *tile_theme_combobox;
    Gtk::ComboBoxText *army_theme_combobox;
    Gtk::ComboBox *process_armies_combobox;
    Gtk::RadioButton *random_map_radio;
    Gtk::FileChooserButton *load_map_filechooser;
    Gtk::Widget *random_map_container;
    Gtk::ComboBox *map_size_combobox;
    Gtk::Scale *grass_scale;
    Gtk::Scale *water_scale;
    Gtk::Scale *swamp_scale;
    Gtk::Scale *forest_scale;
    Gtk::Scale *hills_scale;
    Gtk::Scale *mountains_scale;
    Gtk::Scale *cities_scale;
    Gtk::Scale *ruins_scale;
    Gtk::Scale *temples_scale;

    enum { MAP_SIZE_NORMAL = 0, MAP_SIZE_SMALL, MAP_SIZE_TINY };

    // player settings stuff
    Gtk::TreeView *player_treeview;
    
    class PlayerColumns: public Gtk::TreeModelColumnRecord {
    public:
	PlayerColumns()
	    { add(type); add(name); }
	
	Gtk::TreeModelColumn<Glib::ustring> type;
	Gtk::TreeModelColumn<Glib::ustring> name;
    };
    const PlayerColumns player_columns;
    Glib::RefPtr<Gtk::ListStore> player_list;
    
    Gtk::CellRendererCombo type_renderer;
    Gtk::TreeViewColumn type_column;
    
    class PlayerTypeColumns: public Gtk::TreeModelColumnRecord {
    public:
	PlayerTypeColumns()
	    { add(type); }
	
	Gtk::TreeModelColumn<Glib::ustring> type;
    };
    const PlayerTypeColumns player_type_columns;
    Glib::RefPtr<Gtk::ListStore> player_type_list;
    Gtk::CheckButton *quests_checkbutton;
    Gtk::CheckButton *view_enemies_checkbutton;
    Gtk::CheckButton *view_production_checkbutton;
    Gtk::CheckButton *hidden_map_checkbutton;
    Gtk::ComboBox *neutral_cities_combobox;
    Gtk::CheckButton *diplomacy_checkbutton;
    Gtk::CheckButton *intense_combat_checkbutton;
    Gtk::CheckButton *military_advisor_checkbutton;
    Gtk::CheckButton *random_turns_checkbutton;
    Gtk::CheckButton *quick_start_checkbutton;

    typedef std::vector<Glib::ustring> player_name_seq;
    player_name_seq default_player_names;
    player_name_seq::iterator current_player_name;
    
    void cell_data_type(Gtk::CellRenderer *renderer, const Gtk::TreeIter &i);
    void on_type_edited(const Glib::ustring &path,
			const Glib::ustring &new_text);

    void add_player(const Glib::ustring &type, const Glib::ustring &name);
    void on_add_player_clicked();
    void on_random_map_toggled();
    void on_map_size_changed();
    void on_start_game_clicked();
};

#endif
