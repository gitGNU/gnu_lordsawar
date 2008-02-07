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
#include "../tileset.h"
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
#include "../citylist.h"
#include "../templelist.h"
#include "../ruinlist.h"
#include "../signpostlist.h"
#include "../roadlist.h"
#include "../bridgelist.h"
#include "../portlist.h"
#include "../MapGenerator.h"
#include "../counter.h"

#include "glade-helpers.h"
#include "editorbigmap.h"

#include "signpost-dialog.h"
#include "temple-dialog.h"
#include "ruin-dialog.h"
#include "stack-dialog.h"
#include "players-dialog.h"
#include "city-dialog.h"
#include "map-info-dialog.h"
#include "new-map-dialog.h"


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

    xml->get_widget("terrain_tile_style_hbox", terrain_tile_style_hbox);

    // setup pointer radiobuttons
    xml->get_widget("terrain_type_table", terrain_type_table);

    setup_pointer_radiobutton(xml, "pointer", "button_selector",
			      EditorBigMap::POINTER, 1);
    setup_pointer_radiobutton(xml, "draw_1", "button_1x1",
			      EditorBigMap::TERRAIN, 1);
    setup_pointer_radiobutton(xml, "draw_2", "button_2x2",
			      EditorBigMap::TERRAIN, 2);
    setup_pointer_radiobutton(xml, "draw_3", "button_3x3",
			      EditorBigMap::TERRAIN, 3);
    setup_pointer_radiobutton(xml, "draw_stack", "button_stack",
			      EditorBigMap::STACK, 1);
    setup_pointer_radiobutton(xml, "draw_ruin", "button_ruin",
			      EditorBigMap::RUIN, 1);
    setup_pointer_radiobutton(xml, "draw_signpost", "button_signpost",
			      EditorBigMap::SIGNPOST, 1);
    setup_pointer_radiobutton(xml, "draw_temple", "button_temple",
			      EditorBigMap::TEMPLE, 1);
    setup_pointer_radiobutton(xml, "draw_road", "button_road",
			      EditorBigMap::ROAD, 1);
    setup_pointer_radiobutton(xml, "draw_city", "button_castle",
			      EditorBigMap::CITY, 1);
    setup_pointer_radiobutton(xml, "erase", "button_erase",
			      EditorBigMap::ERASE, 1);
    setup_pointer_radiobutton(xml, "draw_port", "button_port",
			      EditorBigMap::PORT, 1);
    setup_pointer_radiobutton(xml, "draw_bridge", "button_bridge",
			      EditorBigMap::BRIDGE, 1);
    on_pointer_radiobutton_toggled();


    xml->get_widget("mouse_position_label", mouse_position_label);
    
    
    // connect callbacks for the menu
    xml->connect_clicked("new_map_menuitem",
			 sigc::mem_fun(this, &MainWindow::on_new_map_activated));
    xml->connect_clicked("load_map_menuitem",
			 sigc::mem_fun(this, &MainWindow::on_load_map_activated));
    xml->connect_clicked("save_map_menuitem",
			 sigc::mem_fun(this, &MainWindow::on_save_map_activated));
    xml->connect_clicked("save_map_as_menuitem", 
			 sigc::mem_fun(this, &MainWindow::on_save_map_as_activated));
    xml->connect_clicked("quit_menuitem", 
			 sigc::mem_fun(this, &MainWindow::on_quit_activated));

    xml->connect_clicked("edit_players_menuitem", 
			 sigc::mem_fun(this, &MainWindow::on_edit_players_activated));
    xml->connect_clicked("edit_map_info_menuitem", 
			 sigc::mem_fun(this, &MainWindow::on_edit_map_info_activated));
    
    xml->connect_clicked("fullscreen_menuitem", 
			 sigc::mem_fun(this, &MainWindow::on_fullscreen_activated));
    xml->get_widget("fullscreen_menuitem", fullscreen_menuitem);
    xml->connect_clicked("smooth_map_menuitem", 
			 sigc::mem_fun(this, 
				       &MainWindow::on_smooth_map_activated));
    xml->connect_clicked("smooth_screen_menuitem", sigc::mem_fun
			 (this, &MainWindow::on_smooth_screen_activated));
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
    if (prefix == "pointer")
	pointer_radiobutton = item.button;
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
    const int no_columns = 6;
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
	    sigc::mem_fun(this, &MainWindow::on_terrain_radiobutton_toggled));

	SDL_Surface *surf = (*(*tile)[0])[0]->getPixmap();
	Glib::RefPtr<Gdk::Pixbuf> pic = to_pixbuf(surf)->scale_simple(20, 20, Gdk::INTERP_NEAREST);
	item.button->add(*manage(new Gtk::Image(pic)));

	item.terrain = tile->getType();
	terrain_items.push_back(item);
    }

    terrain_type_table->show_all();
}

void MainWindow::show()
{
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
    set_filled_map(112, 156, Tile::WATER, "default");
    setup_terrain_radiobuttons();
    remove_tile_style_buttons();
    setup_tile_style_buttons(Tile::GRASS);
}

void MainWindow::set_filled_map(int width, int height, int fill_style, std::string tileset)
{
    clear_map_state();
    d_width = width;
    d_height = height;

    GameMap::deleteInstance();
    GameMap::setWidth(width);
    GameMap::setHeight(height);
    GameMap::getInstance(tileset);

    // sets up the lists
    game_scenario.reset(new GameScenario(_("Untitled"), _("No description"), true));
    //zip past the player IDs (+1 for neutral)
    for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
      fl_counter->getNextId();


    // ...however we need to do some of the setup by hand. We need to create a
    // neutral player to give cities a player upon creation...
    Uint32 armyset = Armysetlist::getInstance()->getArmysets()[0];
    Player* neutral = new AI_Dummy(_("Neutral"), armyset, Player::get_color_for_neutral(), width, height, MAX_PLAYERS);
    neutral->setType(Player::AI_DUMMY);
    Playerlist::getInstance()->push_back(neutral);
    Playerlist::getInstance()->setNeutral(neutral);
    Playerlist::getInstance()->nextPlayer();

    // fill the map with tile type
    TileSet* tset = GameMap::getInstance()->getTileSet();
    for (unsigned int i = 0; i < tset->size(); ++i)
    {
	if ((*tset)[i]->getType() == fill_style)
	{
	    GameMap::getInstance()->fill(i);
	    break;
	}
    }

    init_map_state();
}

void MainWindow::set_random_map(int width, int height,
				int grass, int water, int swamp, int forest,
				int hills, int mountains,
				int cities, int ruins, int temples,
				int signposts, std::string tileset)
{
    clear_map_state();

    GameMap::deleteInstance();
    GameMap::setWidth(width);
    GameMap::setHeight(height);
    GameMap::getInstance(tileset);

    //zip past the player IDs
    if (fl_counter)
	delete fl_counter;
    fl_counter = new FL_Counter(MAX_PLAYERS + 1);
    
    // We need to create a neutral player to give cities a player upon
    // creation...
    Uint32 armyset = Armysetlist::getInstance()->getArmysets()[0];
    Player* neutral = new AI_Dummy(_("Neutral"), armyset, Player::get_color_for_neutral(), width, height, MAX_PLAYERS);
    neutral->setType(Player::AI_DUMMY);
    Playerlist::getInstance()->push_back(neutral);
    Playerlist::getInstance()->setNeutral(neutral);
    Playerlist::getInstance()->nextPlayer();
    
    // create a random map
    MapGenerator gen;
        
    // first, fill the generator with data
    gen.setNoCities(cities);
    gen.setNoRuins(ruins);
    gen.setNoTemples(temples);
    gen.setNoSignposts(signposts);
    
    // if sum > 100 (percent), divide everything by a factor, the numeric error
    // is said to be grass
    int sum = grass + water + forest + swamp + hills + mountains;

    if (sum > 100)
    {
        double factor = 100 / static_cast<double>(sum);
        water = static_cast<int>(water / factor);
        forest = static_cast<int>(forest / factor);
        swamp = static_cast<int>(swamp / factor);
        hills = static_cast<int>(hills / factor);
        mountains = static_cast<int>(mountains / factor);
    }
    
    gen.setPercentages(water, forest, swamp, hills, mountains);
    
    gen.makeMap(width, height, false);
    GameMap::getInstance()->fill(&gen);

    // sets up the lists
    game_scenario.reset(new GameScenario(_("Untitled"), _("No description"), true));
    
    // now fill the lists
    const Maptile::Building* build = gen.getBuildings(width, height);
    for (int j = 0; j < height; j++)
	for (int i = 0; i < width; i++)
	    switch(build[j * width + i])
	    {
	    case Maptile::CITY:
		Citylist::getInstance()->push_back(City(Vector<int>(i,j)));
		(*Citylist::getInstance()->rbegin()).setPlayer(
		    Playerlist::getInstance()->getNeutral());
		break;
	    case Maptile::TEMPLE:
		Templelist::getInstance()->push_back(Temple(Vector<int>(i,j)));
		break;
	    case Maptile::RUIN:
		Ruinlist::getInstance()->push_back(Ruin(Vector<int>(i,j)));
		break;
	    case Maptile::SIGNPOST:
		Signpostlist::getInstance()->push_back(Signpost(Vector<int>(i,j)));
		break;
	    case Maptile::ROAD:
		Roadlist::getInstance()->push_back(Road(Vector<int>(i,j)));
		break;
	    case Maptile::BRIDGE:
		Bridgelist::getInstance()->push_back(Bridge(Vector<int>(i,j)));
		break;
	    case Maptile::PORT:
		Portlist::getInstance()->push_back(Port(Vector<int>(i,j)));
		break;
	    case Maptile::NONE:
		break;
	    }

    init_map_state();
}

void MainWindow::clear_map_state()
{
    bigmap.reset();
    smallmap.reset();
    game_scenario.reset();
    GraphicsCache::deleteInstance();
}

void MainWindow::init_map_state()
{
    pointer_radiobutton->set_active();
    init_maps();
    bigmap->screen_size_changed();
}


bool MainWindow::on_sdl_mouse_button_event(GdkEventButton *e)
{
    if (e->type != GDK_BUTTON_PRESS && e->type != GDK_BUTTON_RELEASE)
	return true;	// useless event

    if (bigmap.get())
    {
	button_event = e;	// save it for later use
	bigmap->mouse_button_event(to_input_event(e));
	if (smallmap.get())
	  smallmap->draw();
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

    NewMapDialog d;
    d.set_parent_window(*window.get());
    d.run();

    if (d.map_set)
    {
	if (d.map.fill_style == -1)
	    set_random_map(d.map.width, d.map.height,
			   d.map.grass, d.map.water, d.map.swamp, d.map.forest,
			   d.map.hills, d.map.mountains,
			   d.map.cities, d.map.ruins, d.map.temples, 
			   d.map.signposts, d.map.tileset);
	else
	    set_filled_map(d.map.width, d.map.height, d.map.fill_style, 
			   d.map.tileset);
    }
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

	clear_map_state();

	bool broken;
	game_scenario.reset(new GameScenario(current_save_filename, broken));

	if (broken)
	{
	    show_error(String::ucompose(_("Could not load map %1."),
					current_save_filename));
	    current_save_filename = "";
	    return;
	}

	init_map_state();
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

void MainWindow::on_edit_players_activated()
{
    PlayersDialog d(d_width, d_height);
    d.set_parent_window(*window.get());
    d.run();
}

void MainWindow::on_edit_map_info_activated()
{
    MapInfoDialog d(game_scenario.get());
    d.set_parent_window(*window.get());
    d.run();
}

void MainWindow::on_fullscreen_activated()
{
    if (fullscreen_menuitem->get_active())
	window->fullscreen();
    else
	window->unfullscreen();
}
void MainWindow::remove_tile_style_buttons()
{
  Glib::ListHandle<Gtk::Widget*> children = 
    terrain_tile_style_hbox->get_children();
  if (!children.empty()) 
    {
      Glib::ListHandle<Gtk::Widget*>::iterator child = children.begin();
      for (; child != children.end(); child++)
	terrain_tile_style_hbox->remove(**child);
    }
  tile_style_items.clear();
}
void MainWindow::setup_tile_style_buttons(Tile::Type terrain)
{
  Gtk::RadioButton::Group group;
  //iterate through tilestyles of a certain TERRAIN tile
  TileSet *tileset = GameMap::getInstance()->getTileSet();
  Uint32 index = tileset->getIndex(terrain);
  Tile *tile = (*tileset)[index];
  if (tile == NULL)
    return;
	  
  TileStyleItem auto_item;
  auto_item.button = manage(new Gtk::RadioButton);
  terrain_tile_style_hbox->pack_start(*manage(auto_item.button), 
				      Gtk::PACK_SHRINK, 0, 0);
  auto_item.button->set_label(_("Auto"));
  auto_item.button->property_draw_indicator() = false;
	  
  auto_item.button->signal_toggled().connect
    (sigc::mem_fun(this, &MainWindow::on_tile_style_radiobutton_toggled));

  auto_item.tile_style_id = -1;
  group = auto_item.button->get_group();
  tile_style_items.push_back(auto_item);

  for (unsigned int i = 0; i < tile->size(); i++)
    {
      TileStyleSet *tilestyleset = (*tile)[i];
      //loop through tile style sets
      for (unsigned int j = 0; j < tilestyleset->size(); j++)
	{
	  //now loop through the tile styles
	  TileStyle *tilestyle = (*tilestyleset)[j];

	  //let's make a button
	  TileStyleItem item;
	  item.button = manage(new Gtk::RadioButton);
	  item.button->set_group(group);
	  item.button->property_draw_indicator() = false;

	  terrain_tile_style_hbox->pack_start(*manage(item.button), 
					      Gtk::PACK_SHRINK, 0, 0);
	  item.button->signal_toggled().connect
	    (sigc::mem_fun(this, 
			   &MainWindow::on_tile_style_radiobutton_toggled));

	  SDL_Surface *surf = tilestyle->getPixmap();
	  Glib::RefPtr<Gdk::Pixbuf> pic = to_pixbuf(surf);
	  item.button->add(*manage(new Gtk::Image(pic)));
	  item.tile_style_id = tilestyle->getId();

	  tile_style_items.push_back(item);
	}
    }

  terrain_tile_style_hbox->show_all();
}
void MainWindow::on_tile_style_radiobutton_toggled()
{
  on_pointer_radiobutton_toggled();
}
void MainWindow::on_terrain_radiobutton_toggled()
{
  remove_tile_style_buttons();
  setup_tile_style_buttons(get_terrain());
  on_pointer_radiobutton_toggled();
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
	bigmap->set_pointer(pointer, size, get_terrain(),
			    get_tile_style_id());
    terrain_type_table->set_sensitive(pointer == EditorBigMap::TERRAIN);
    terrain_tile_style_hbox->set_sensitive(pointer == EditorBigMap::TERRAIN);
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

int MainWindow::get_tile_style_id()
{
  int tile_style_id = -1;
  for (std::vector<TileStyleItem>::iterator i = tile_style_items.begin(),
       end = tile_style_items.end(); i != end; ++i)
    {
	if (i->button->get_active())
	  {
	    tile_style_id = i->tile_style_id;
	    break;
	  }
    }

    return tile_style_id;
}

void MainWindow::on_smallmap_changed(SDL_Surface *map)
{
    map_image->property_pixbuf() = to_pixbuf(map);
}

void MainWindow::init_maps()
{
    // init the bigmap
    bigmap.reset(new EditorBigMap);
    bigmap->mouse_on_tile.connect(
	sigc::mem_fun(this, &MainWindow::on_mouse_on_tile));
    bigmap->objects_selected.connect(
	sigc::mem_fun(this, &MainWindow::on_objects_selected));
    
    // init the smallmap
    smallmap.reset(new SmallMap);
    smallmap->map_changed.connect(
	sigc::mem_fun(this, &MainWindow::on_smallmap_changed));

    // connect the two maps
    bigmap->view_changed.connect(
	sigc::mem_fun(smallmap.get(), &SmallMap::set_view));
    bigmap->map_changed.connect(
	sigc::mem_fun(smallmap.get(), &SmallMap::redraw_tiles));
    smallmap->view_changed.connect(
	sigc::mem_fun(bigmap.get(), &EditorBigMap::set_view));

    smallmap->resize(GameMap::get_dim() * 2);
}

void MainWindow::on_mouse_on_tile(Vector<int> tile)
{
    Glib::ustring str;
    if (tile.x > 0 && tile.y > 0)
	// note to translators: this is a coordinate pair (x, y)
	str = "<i>" + String::ucompose(_("(%1, %2)"), tile.x, tile.y) + "</i>";
    
    mouse_position_label->set_markup(str);
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
	StackDialog d(o);
	d.set_parent_window(*window.get());
	d.run();

	// we might have changed something visible
	bigmap->draw();
	smallmap->draw();
    }
    else if (City *o = dynamic_cast<City *>(object))
    {
	CityDialog d(o);
	d.set_parent_window(*window.get());
	d.run();

	// we might have changed something visible
	bigmap->draw();
	smallmap->draw();
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
void MainWindow::on_smooth_map_activated()
{
  GameMap::getInstance()->applyTileStyles(0, 0, GameMap::getHeight(), 
					  GameMap::getWidth(), true);
  bigmap->draw();
}
void MainWindow::on_smooth_screen_activated()
{
  bigmap->smooth_view();
}
