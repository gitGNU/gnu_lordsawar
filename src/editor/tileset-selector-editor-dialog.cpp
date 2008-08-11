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

#include <libglademm/xml.h>
#include <sigc++/functors/mem_fun.h>

#include "tileset-selector-editor-dialog.h"

#include "glade-helpers.h"
#include "../gui/image-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../File.h"
#include "../shieldsetlist.h"
#include "../GraphicsCache.h"


TilesetSelectorEditorDialog::TilesetSelectorEditorDialog(Tileset *tileset)
{
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/tileset-selector-editor-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    d_tileset = tileset;

    Gtk::Box *box;
    xml->get_widget("shieldset_box", box);
    setup_shield_theme_combobox(box);
    xml->get_widget("preview_table", preview_table);
    
    xml->get_widget("selector_filechooserbutton", selector_filechooserbutton);
    selector_filechooserbutton->signal_selection_changed().connect
       (sigc::mem_fun(*this, &TilesetSelectorEditorDialog::on_image_chosen));

    xml->get_widget("large_selector_radiobutton", large_selector_radiobutton);
    large_selector_radiobutton->signal_toggled().connect
      (sigc::mem_fun(*this, &TilesetSelectorEditorDialog::on_large_toggled));
    xml->get_widget("small_selector_radiobutton", small_selector_radiobutton);
    small_selector_radiobutton->signal_toggled().connect
      (sigc::mem_fun(*this, &TilesetSelectorEditorDialog::on_small_toggled));

    on_large_toggled();
}

void TilesetSelectorEditorDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void TilesetSelectorEditorDialog::run()
{
    dialog->show_all();
    dialog->run();

    return;
}

void TilesetSelectorEditorDialog::setup_shield_theme_combobox(Gtk::Box *box)
{
  // fill in shield themes combobox
  shield_theme_combobox = manage(new Gtk::ComboBoxText);

  Shieldsetlist *sl = Shieldsetlist::getInstance();
  std::list<std::string> shield_themes = sl->getNames();
  int counter = 0;
  int default_id = 0;
  for (std::list<std::string>::iterator i = shield_themes.begin(),
       end = shield_themes.end(); i != end; ++i)
    {
      if (*i == "Default")
	default_id = counter;
      shield_theme_combobox->append_text(Glib::filename_to_utf8(*i));
      counter++;
    }

  shield_theme_combobox->set_active(default_id);
  shield_theme_combobox->signal_changed().connect
    (sigc::mem_fun(this, &TilesetSelectorEditorDialog::shieldset_changed));

  box->pack_start(*shield_theme_combobox, Gtk::PACK_SHRINK);
}
    
void TilesetSelectorEditorDialog::shieldset_changed()
{
}
void TilesetSelectorEditorDialog::on_image_chosen()
{
  std::string selected_filename = selector_filechooserbutton->get_filename();
  if (selected_filename.empty())
    return;

  std::string str = Configuration::s_dataPath + "/tilesets/" +  
    d_tileset->getSubDir() +"/";
  char mypath[PATH_MAX]; //god i hate path_max.  die die die
  realpath(str.c_str(), mypath);
  std::string path = mypath;
  if (selected_filename.substr(0, path.size()) !=path)
    return;
  std::string filename = &selected_filename.c_str()[path.size() + 1];
  if (large_selector_radiobutton->get_active() == true)
    d_tileset->setLargeSelectorFilename(filename);
  else
    d_tileset->setSmallSelectorFilename(filename);

  show_preview_selectors(filename);
}

void TilesetSelectorEditorDialog::show_preview_selectors(std::string filename)
{
  //load it up and show in the colours of the selected shield theme
  if (heartbeat.connected())
    heartbeat.disconnect();

  clearSelector();
  if (loadSelector(filename) == true)
    {
      heartbeat = Glib::signal_timeout().connect
	(bind_return
	 (sigc::mem_fun (*this, &TilesetSelectorEditorDialog::on_heartbeat), 
	  true), TIMER_BIGMAP_SELECTOR);
    }
}

void TilesetSelectorEditorDialog::clearSelector()
{
  for (std::map< Uint32, std::list<SDL_Surface*>* >::iterator it = selectors.begin();
       it != selectors.end(); it++)
    {
      for (std::list<SDL_Surface *>::iterator lit = (*it).second->begin(); lit != (*it).second->end(); lit++)
	{
	  SDL_FreeSurface(*lit);
	}
      (*it).second->clear();
      delete ((*it).second);
    }
  selectors.clear();
}

bool TilesetSelectorEditorDialog::loadSelector(std::string filename)
{
  int num_frames;
  if (large_selector_radiobutton->get_active() == true)
    num_frames = SELECTOR_FRAMES;
  else
    num_frames = SMALL_SELECTOR_FRAMES;
  std::vector<SDL_Surface *> images;
  std::vector<SDL_Surface *> masks;
  bool success = GraphicsCache::loadSelectorImages(d_tileset->getSubDir(), filename, d_tileset->getTileSize(), images, masks, num_frames);
  if (success)
    {
      std::string subdir = Shieldsetlist::getInstance()->getShieldsetDir 
	(Glib::filename_from_utf8(shield_theme_combobox->get_active_text()));
      Shieldset *shieldset = Shieldsetlist::getInstance()->getShieldset(subdir);

      for (unsigned int i = 0; i < MAX_PLAYERS; i++)
	{
	  std::list<SDL_Surface *> *mylist = new std::list<SDL_Surface*>();
	  selectors[i] = mylist;
	}

      for (std::vector<SDL_Surface*>::iterator it = images.begin(), mit = masks.begin(); it != images.end(); it++, mit++)
	{
	  for (Shieldset::iterator sit = shieldset->begin(); sit != shieldset->end(); sit++)
	    {
	      if ((*sit)->getOwner() == 8) //ignore neutral
		continue;
	      SDL_Surface *image = GraphicsCache::applyMask(*it, *mit, (*sit)->getMaskColor(), false);
		selectors[(*sit)->getOwner()]->push_back(image);
		
		frame[(*sit)->getOwner()] = selectors[(*sit)->getOwner()]->begin();
	    }
	}

      for (std::vector<SDL_Surface*>::iterator it = images.begin(); it != images.end(); it++)
	SDL_FreeSurface(*it);
      images.clear();
      for (std::vector<SDL_Surface*>::iterator it = masks.begin(); it != masks.end(); it++)
	SDL_FreeSurface(*it);
      masks.clear();

    }

  return success;
}

void TilesetSelectorEditorDialog::on_large_toggled()
{
  update_selector_panel();
}

void TilesetSelectorEditorDialog::on_small_toggled()
{
  update_selector_panel();
}


void TilesetSelectorEditorDialog::update_selector_panel()
{
  if (large_selector_radiobutton->get_active() == true)
    {
      std::string filename = d_tileset->getLargeSelectorFilename();
      if (filename.c_str()[0] == '/')
	selector_filechooserbutton->set_filename (filename);
      else
	selector_filechooserbutton->set_filename
	  (File::getTilesetFile(d_tileset->getSubDir(), filename));
    }
  else
    {
      std::string filename = d_tileset->getSmallSelectorFilename();
      if (filename.c_str()[0] == '/')
	selector_filechooserbutton->set_filename (filename);
      else
	selector_filechooserbutton->set_filename
	  (File::getTilesetFile(d_tileset->getSubDir(), filename));
    }

}

void TilesetSelectorEditorDialog::on_heartbeat()
{
  preview_table->foreach(sigc::mem_fun(preview_table, &Gtk::Container::remove));
  preview_table->resize(4, 2);

  int x = 0;
  int y = 0;
  int count = 0;
  for (std::map< Uint32, std::list<SDL_Surface*>* >::iterator it = selectors.begin();
       it != selectors.end(); it++)
    {
      //make a pixbuf and attach it
      switch (count)
	{
	case 0: x = 0; y = 0; break;
	case 1: x = 0; y = 1; break;
	case 2: x = 0; y = 2; break;
	case 3: x = 0; y = 3; break;
	case 4: x = 1; y = 0; break;
	case 5: x = 1; y = 1; break;
	case 6: x = 1; y = 2; break;
	case 7: x = 1; y = 3; break;
	}
      preview_table->attach(*manage(new Gtk::Image(to_pixbuf(*frame[count]))), y, y+1, x, x+1, Gtk::SHRINK, Gtk::SHRINK, 8, 8);
  
      frame[count]++;
      if (frame[count] == selectors[count]->end())
	frame[count] = selectors[count]->begin();
      count++;
    }
  preview_table->show_all();

}
