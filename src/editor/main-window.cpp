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

#include <config.h>

#include <iomanip>
#include <SDL_video.h>
#include <assert.h>

#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>

#include <gtkmm/widget.h>
#include <gtkmm/menuitem.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/image.h>
#include <gtkmm/dialog.h>
#include <gtkmm/stock.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/menu.h>
#include <gtkmm/menuitem.h>

#include "main-window.h"

#include "../gui/gtksdl.h"
#include "../gui/image-helpers.h"
#include "../gui/input-helpers.h"
#include "../gui/error-utils.h"

#include "../ucompose.hpp"
#include "../TileSet.h"
#include "../GameMap.h"
#include "../defs.h"
#include "../sound.h"
#include "../File.h"
#include "../GraphicsCache.h"
#include "../smallmap.h"
#include "../GameScenario.h"
#include "../armysetlist.h"
#include "../playerlist.h"
#include "../ai_dummy.h"

#include "../stack.h"
#include "../city.h"
#include "../ruin.h"
#include "../signpost.h"
#include "../temple.h"

#include "glade-helpers.h"
#include "editorbigmap.h"
#include "signpost-dialog.h"
#include "temple-dialog.h"
#include "ruin-dialog.h"


MainWindow::MainWindow()
{
    sdl_inited = false;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/main-window.glade");

    Gtk::Window *w = 0;
    xml->get_widget("window", w);
    window.reset(w);

    w->signal_delete_event().connect(
	sigc::mem_fun(*this, &MainWindow::on_delete_event));

    xml->get_widget("sdl_container", sdl_container);

    // the map image
    xml->get_widget("map_image", map_image);
    Gtk::EventBox *map_eventbox;
    xml->get_widget("map_eventbox", map_eventbox);
    map_eventbox->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
			     Gdk::POINTER_MOTION_MASK);
    map_eventbox->signal_button_press_event().connect(
	sigc::mem_fun(*this, &MainWindow::on_map_mouse_button_event));
    map_eventbox->signal_button_release_event().connect(
	sigc::mem_fun(*this, &MainWindow::on_map_mouse_button_event));
    map_eventbox->signal_motion_notify_event().connect(
	sigc::mem_fun(*this, &MainWindow::on_map_mouse_motion_event));

    // setup pointer radiobuttons
    xml->get_widget("terrain_type_table", terrain_type_table);

    setup_pointer_radiobutton(xml, "pointer", "button_selector",
			      EditorBigMap::POINTER, 1);
    setup_pointer_radiobutton(xml, "draw_1", "button_1x1",
			      EditorBigMap::TERRAIN, 1);
    setup_pointer_radiobutton(xml, "draw_3", "button_3x3",
			      EditorBigMap::TERRAIN, 3);
    setup_pointer_radiobutton(xml, "draw_stack", "button_stack",
			      EditorBigMap::STACK, 1);
    setup_pointer_radiobutton(xml, "draw_ruin", "button_ruin",
			      EditorBigMap::RUIN, 1);
    setup_pointer_radiobutton(xml, "draw_signpost", "button_signpost",
			      EditorBigMap::SIGNPOST, 1);
    setup_pointer_radiobutton(xml, "draw_stone", "button_stone",
			      EditorBigMap::STONE, 1);
    setup_pointer_radiobutton(xml, "draw_temple", "button_temple",
			      EditorBigMap::TEMPLE, 1);
    setup_pointer_radiobutton(xml, "draw_road", "button_road",
			      EditorBigMap::ROAD, 1);
    setup_pointer_radiobutton(xml, "draw_city", "button_castle",
			      EditorBigMap::CITY, 1);
    setup_pointer_radiobutton(xml, "erase", "button_erase",
			      EditorBigMap::ERASE, 1);
    on_pointer_radiobutton_toggled();

    
    // connect callbacks for the menu
    xml->connect_clicked("load_map_menuitem",
			 sigc::mem_fun(*this, &MainWindow::on_load_map_activated));
    xml->connect_clicked("save_map_menuitem",
			 sigc::mem_fun(*this, &MainWindow::on_save_map_activated));
    xml->connect_clicked("save_map_as_menuitem", 
			 sigc::mem_fun(*this, &MainWindow::on_save_map_as_activated));
    xml->connect_clicked("quit_menuitem", 
			 sigc::mem_fun(*this, &MainWindow::on_quit_activated));
    
#if 0
    xml->connect_clicked("fullscreen_menuitem", 
			 sigc::mem_fun(*this, &MainWindow::on_fullscreen_activated));
    xml->get_widget("fullscreen_menuitem", fullscreen_menuitem);
    xml->connect_clicked("preferences_menuitem", 
			 sigc::mem_fun(*this, &MainWindow::on_preferences_activated));
#endif
}

MainWindow::~MainWindow()
{
}

void MainWindow::setup_pointer_radiobutton(Glib::RefPtr<Gnome::Glade::Xml> xml,
					   std::string prefix,
					   std::string image_file,
					   EditorBigMap::Pointer pointer,
					   int size)
{
    PointerItem item;
    xml->get_widget(prefix + "_radiobutton", item.button);
    item.button->signal_toggled().connect(
	sigc::mem_fun(this, &MainWindow::on_pointer_radiobutton_toggled));
    item.pointer = pointer;
    item.size = size;
    pointer_items.push_back(item);

    Gtk::Image *image;
    xml->get_widget(prefix + "_image", image);
    image->property_file() = File::getEditorFile(image_file);
    item.button->property_draw_indicator() = false;
}

void MainWindow::setup_terrain_radiobuttons()
{
    // get rid of old ones
    terrain_type_table->children().erase(terrain_type_table->children().begin(),
					 terrain_type_table->children().end());

    // then add new ones from the tile set
    TileSet *tset = GameMap::getInstance()->getTileSet();
    Gtk::RadioButton::Group group;
    bool group_set = false;
    const int no_columns = 3;
    for (unsigned int i = 0; i < tset->size(); ++i)
    {
	Tile *tile = (*tset)[i];
	TerrainItem item;
	item.button = manage(new Gtk::RadioButton);
	if (group_set)
	    item.button->set_group(group);
	else
	{
	    group = item.button->get_group();
	    group_set = true;
	}
	item.button->property_draw_indicator() = false;

	int row = i / no_columns, column = i % no_columns;
	
	terrain_type_table->attach(*item.button, column, column + 1,
				   row, row + 1, Gtk::SHRINK);
	item.button->signal_toggled().connect(
	    sigc::mem_fun(this, &MainWindow::on_pointer_radiobutton_toggled));

	Glib::RefPtr<Gdk::Pixbuf> pic = to_pixbuf(tile->getSurface(0));
	item.button->add(*manage(new Gtk::Image(pic)));

	tooltips.set_tip(*item.button, tile->getName());

	item.terrain = tile->getType();
	terrain_items.push_back(item);
    }

    terrain_type_table->show_all();
}

void MainWindow::show()
{
#if 0
    prev_button->show_all();
    next_button->show_all();
    next_movable_button->show_all();
    center_button->show_all();
    defend_button->show_all();
    search_button->show_all();
    move_button->show_all();
    move_all_button->show_all();
    end_turn_button->show_all();
#endif
    sdl_container->show_all();
    window->show();
}

void MainWindow::hide()
{
    window->hide();
}

namespace 
{
    void surface_attached_helper(GtkSDL *gtksdl, gpointer data)
    {
	static_cast<MainWindow *>(data)->on_sdl_surface_changed();
    }
}

void MainWindow::init(int width, int height)
{
    sdl_widget
	= Gtk::manage(Glib::wrap(gtk_sdl_new(width, height, 0, SDL_SWSURFACE)));

    sdl_widget->set_flags(Gtk::CAN_FOCUS);

    sdl_widget->grab_focus();
    sdl_widget->add_events(Gdk::KEY_PRESS_MASK |
		  Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
	          Gdk::POINTER_MOTION_MASK | Gdk::LEAVE_NOTIFY_MASK);

    sdl_widget->signal_button_press_event().connect(
	sigc::mem_fun(*this, &MainWindow::on_sdl_mouse_button_event));
    sdl_widget->signal_button_release_event().connect(
	sigc::mem_fun(*this, &MainWindow::on_sdl_mouse_button_event));
    sdl_widget->signal_motion_notify_event().connect(
	sigc::mem_fun(*this, &MainWindow::on_sdl_mouse_motion_event));
    sdl_widget->signal_key_press_event().connect(
	sigc::mem_fun(*this, &MainWindow::on_sdl_key_event));
    sdl_widget->signal_leave_notify_event().connect(
	sigc::mem_fun(*this, &MainWindow::on_sdl_leave_event));
    
    // connect to the special signal that signifies that a new surface has been
    // generated and attached to the widget
    g_signal_connect(G_OBJECT(sdl_widget->gobj()), "surface-attached",
		     G_CALLBACK(surface_attached_helper), this);
    
    sdl_container->add(*sdl_widget);
}

bool MainWindow::on_delete_event(GdkEventAny *e)
{
    hide();
    
    return true;
}

void MainWindow::show_initial_map()
{
    GameMap::deleteInstance();
    GameMap::setWidth(112);
    GameMap::setHeight(156);
    GameMap::getInstance("default");

    // sets up the lists
    game_scenario.reset(new GameScenario(_("Untitled"), _("No comments"), true));

    // ...however we need to do some of the setup by hand. We need to create a
    // neutral player to give cities a player upon creation...
    SDL_Color c;
    c.r = c.g = c.b = 180; c.unused = 0;
    Uint32 armyset = Armysetlist::getInstance()->getArmysets()[0];
    Player* neutral = new AI_Dummy("Neutral", armyset, c);
    neutral->setType(Player::AI_DUMMY);
    Playerlist::getInstance()->push_back(neutral);
    Playerlist::getInstance()->setNeutral(neutral);
    Playerlist::getInstance()->nextPlayer();

    // fill the map with grass
    TileSet* tset = GameMap::getInstance()->getTileSet();
    for (unsigned int i = 0; i < tset->size(); ++i)
    {
	if ((*tset)[i]->getType() == Tile::GRASS)
	{
	    GameMap::getInstance()->fill(i);
	    break;
	}
    }

    setup_terrain_radiobuttons();

    init_maps();
}

bool MainWindow::on_sdl_mouse_button_event(GdkEventButton *e)
{
    if (e->type != GDK_BUTTON_PRESS && e->type != GDK_BUTTON_RELEASE)
	return true;	// useless event

    if (bigmap.get())
    {
	button_event = e;	// save it for later use
	bigmap->mouse_button_event(to_input_event(e));
    }
    
    return true;
}

bool MainWindow::on_sdl_mouse_motion_event(GdkEventMotion *e)
{
    if (bigmap.get())
	bigmap->mouse_motion_event(to_input_event(e));
    
    return true;
}

bool MainWindow::on_sdl_key_event(GdkEventKey *e)
{
#if 0
    if (bigmap.get()) {
	KeyPressEvent k;
	bigmap->key_press_event(k);
    }
#endif
    
    return true;
}

bool MainWindow::on_sdl_leave_event(GdkEventCrossing *e)
{
    if (bigmap.get())
    {
	bigmap->mouse_leave_event();
    }
    
    return true;
}

bool MainWindow::on_map_mouse_button_event(GdkEventButton *e)
{
    if (e->type != GDK_BUTTON_PRESS && e->type != GDK_BUTTON_RELEASE)
	return true;	// useless event
    
    if (smallmap.get())
	smallmap->mouse_button_event(to_input_event(e));
    
    return true;
}

bool MainWindow::on_map_mouse_motion_event(GdkEventMotion *e)
{
    if (smallmap.get())
	smallmap->mouse_motion_event(to_input_event(e));
    
    return true;
}

void MainWindow::on_sdl_surface_changed()
{
    if (!sdl_inited) {
	sdl_inited = true;
	sdl_initialized.emit();
    }

    if (bigmap.get()) {
	bigmap->screen_size_changed();
	bigmap->draw();
    }
    
    if (smallmap.get())
	smallmap->draw();
}

void MainWindow::on_new_map_activated()
{
    current_save_filename = "";
}

void MainWindow::on_load_map_activated()
{
    Gtk::FileChooserDialog chooser(*window.get(), _("Choose Map to Load"));
    Gtk::FileFilter sav_filter;
    sav_filter.add_pattern("*.map");
    chooser.set_filter(sav_filter);
    chooser.set_current_folder(Configuration::s_savePath);

    chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
    chooser.set_default_response(Gtk::RESPONSE_ACCEPT);
	
    chooser.show_all();
    int res = chooser.run();
    
    if (res == Gtk::RESPONSE_ACCEPT)
    {
	current_save_filename = chooser.get_filename();
	chooser.hide();

	bigmap.reset();
	smallmap.reset();
	game_scenario.reset();
	GraphicsCache::deleteInstance();

	bool broken;
	game_scenario.reset(new GameScenario(current_save_filename, broken));

	if (broken)
	{
	    show_error(String::ucompose(_("Could not load map %s."),
					current_save_filename));
	    current_save_filename = "";
	    return;
	}

	init_maps();
	bigmap->screen_size_changed();
    }
}

void MainWindow::on_save_map_activated()
{
    if (current_save_filename.empty())
	on_save_map_as_activated();
    else
    {
	bool success = game_scenario->saveGame(current_save_filename, "map");
	if (!success)
	    show_error(_("Map was not saved!"));
    }
}

void MainWindow::on_save_map_as_activated()
{
    Gtk::FileChooserDialog chooser(*window.get(), _("Choose a Name"),
				   Gtk::FILE_CHOOSER_ACTION_SAVE);
    Gtk::FileFilter sav_filter;
    sav_filter.add_pattern("*.map");
    chooser.set_filter(sav_filter);
    chooser.set_current_folder(Configuration::s_savePath);

    chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    chooser.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_ACCEPT);
    chooser.set_default_response(Gtk::RESPONSE_ACCEPT);
    
    chooser.show_all();
    int res = chooser.run();
    
    if (res == Gtk::RESPONSE_ACCEPT)
    {
	current_save_filename = chooser.get_filename();
	chooser.hide();

	bool success = game_scenario->saveGame(current_save_filename, "map");
	if (!success)
	    show_error(_("Map was not saved!"));
    }
}

void MainWindow::on_quit_activated()
{
    // FIXME: ask
    bool end = true;

    if (end) {
    }
    window->hide();
}

void MainWindow::on_fullscreen_activated()
{
    if (fullscreen_menuitem->get_active())
	window->fullscreen();
    else
	window->unfullscreen();
}

void MainWindow::on_pointer_radiobutton_toggled()
{
    EditorBigMap::Pointer pointer = EditorBigMap::POINTER;
    int size = 1;
    
    for (std::vector<PointerItem>::iterator i = pointer_items.begin(),
	     end = pointer_items.end(); i != end; ++i)
    {
	if (i->button->get_active())
	{
	    pointer = i->pointer;
	    size = i->size;
	    break;
	}
    }
    
    if (bigmap.get())
	bigmap->set_pointer(pointer, size, get_terrain());
    terrain_type_table->set_sensitive(pointer == EditorBigMap::TERRAIN);
}

Tile::Type MainWindow::get_terrain()
{
    Tile::Type terrain = Tile::GRASS;
    for (std::vector<TerrainItem>::iterator i = terrain_items.begin(),
	     end = terrain_items.end(); i != end; ++i)
    {
	if (i->button->get_active())
	{
	    terrain = i->terrain;
	    break;
	}
    }
    
    return terrain;
}

void MainWindow::on_smallmap_changed(SDL_Surface *map)
{
    map_image->property_pixbuf() = to_pixbuf(map);
}

void MainWindow::init_maps()
{
    // init the bigmap
    bigmap.reset(new EditorBigMap);
    bigmap->objects_selected.connect(
	sigc::mem_fun(this, &MainWindow::on_objects_selected));
#if 0
    bigmap->city_selected.connect(
	sigc::mem_fun(this, &MainWindow::on_city_selected));
    bigmap->ruin_selected.connect(
	sigc::mem_fun(this, &MainWindow::on_ruin_selected));
    bigmap->signpost_selected.connect(
	sigc::mem_fun(this, &MainWindow::on_signpost_selected));
    bigmap->temple_selected.connect(
	sigc::mem_fun(this, &MainWindow::on_temple_selected));
#endif
    
    // init the smallmap
    smallmap.reset(new SmallMap);
    smallmap->map_changed.connect(
	sigc::mem_fun(this, &MainWindow::on_smallmap_changed));

    // connect the two maps
    bigmap->view_changed.connect(
	sigc::mem_fun(smallmap.get(), &SmallMap::set_view));
    bigmap->map_changed.connect(
	sigc::mem_fun(smallmap.get(), &SmallMap::draw));
    smallmap->view_changed.connect(
	sigc::mem_fun(bigmap.get(), &EditorBigMap::set_view));

    smallmap->resize(GameMap::get_dim() * 2);
}

void MainWindow::on_objects_selected(std::vector<Object *> objects)
{
    assert(!objects.empty());

    if (objects.size() == 1)
    {
	popup_dialog_for_object(objects.front());
    }
    else
    {
	// show a popup
	Gtk::Menu *menu = manage(new Gtk::Menu);
	for (std::vector<Object *>::iterator i = objects.begin(), end = objects.end();
	     i != end; ++i)
	{
	    Glib::ustring s;
	    if (dynamic_cast<Stack *>(*i))
		s = _("Stack");
	    else if (dynamic_cast<City *>(*i))
		s = _("City");
	    else if (dynamic_cast<Ruin *>(*i))
		s = _("Ruin");
	    else if (dynamic_cast<Signpost *>(*i))
		s = _("Signpost");
	    else if (dynamic_cast<Temple *>(*i))
		s = _("Temple");
	    
	    Gtk::MenuItem *item = manage(new Gtk::MenuItem(s));
	    item->signal_activate().connect(
		sigc::bind(sigc::mem_fun(this, &MainWindow::popup_dialog_for_object), *i));
	    item->show();
	    menu->add(*item);
	}
	menu->popup(button_event->button, button_event->time);
    }
}

void MainWindow::popup_dialog_for_object(Object *object)
{
    if (Stack *o = dynamic_cast<Stack *>(object))
    {
#if 0
	StackDialog d(o);
	d.set_parent_window(*window.get());
	d.run();
#endif
    }
    else if (City *o = dynamic_cast<City *>(object))
    {
    }
    else if (Ruin *o = dynamic_cast<Ruin *>(object))
    {
	RuinDialog d(o);
	d.set_parent_window(*window.get());
	d.run();
    }
    else if (Signpost *o = dynamic_cast<Signpost *>(object))
    {
	SignpostDialog d(o);
	d.set_parent_window(*window.get());
	d.run();
    }
    else if (Temple *o = dynamic_cast<Temple *>(object))
    {
	TempleDialog d(o);
	d.set_parent_window(*window.get());
	d.run();

	// we might have changed something visible
	bigmap->draw();
    }
}
