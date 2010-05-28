//  Copyright (C) 2008, 2009, 2010 Ben Asselstine
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

#include <config.h>

#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>
#include <stdlib.h>

#include "tileset-selector-editor-dialog.h"

#include "glade-helpers.h"
#include "gui/image-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "shieldsetlist.h"
#include "GraphicsCache.h"


TilesetSelectorEditorDialog::TilesetSelectorEditorDialog(Tileset *tileset)
{
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/tileset-selector-editor-dialog.ui");

    xml->get_widget("dialog", dialog);
    d_tileset = tileset;
    small_filename = "";
    if (d_tileset->getSmallSelectorFilename().empty() == false)
      {
        small_filename = 
          d_tileset->getFileFromConfigurationFile(d_tileset->getSmallSelectorFilename() + ".png");
        delfiles.push_back(small_filename);
      }
    large_filename = "";
    if (d_tileset->getLargeSelectorFilename().empty() == false)
      {
        large_filename = 
          d_tileset->getFileFromConfigurationFile(d_tileset->getLargeSelectorFilename() + ".png");
        delfiles.push_back(large_filename);
      }

    Gtk::Box *box;
    xml->get_widget("shieldset_box", box);
    setup_shield_theme_combobox(box);
    xml->get_widget("preview_table", preview_table);
    
    xml->get_widget("selector_filechooserbutton", selector_filechooserbutton);
	
    if (large_filename.empty() == false)
      selector_filechooserbutton->set_filename (large_filename);
    selector_filechooserbutton->signal_selection_changed().connect
       (sigc::mem_fun(*this, &TilesetSelectorEditorDialog::on_image_chosen));

    xml->get_widget("large_selector_radiobutton", large_selector_radiobutton);
    large_selector_radiobutton->signal_toggled().connect
      (sigc::mem_fun(*this, &TilesetSelectorEditorDialog::on_large_toggled));
    xml->get_widget("small_selector_radiobutton", small_selector_radiobutton);
    small_selector_radiobutton->signal_toggled().connect
      (sigc::mem_fun(*this, &TilesetSelectorEditorDialog::on_small_toggled));

    if (large_filename.empty() == false)
      show_preview_selectors(large_filename);
}
TilesetSelectorEditorDialog::~TilesetSelectorEditorDialog()
{
  delete dialog;
}

void TilesetSelectorEditorDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

int TilesetSelectorEditorDialog::run()
{
    dialog->show_all();
    int response = dialog->run();

    if (std::find(delfiles.begin(), delfiles.end(), small_filename)
        == delfiles.end() && response == Gtk::RESPONSE_ACCEPT)
      {
        d_tileset->replaceFileInConfigurationFile(d_tileset->getSmallSelectorFilename()+".png", small_filename);
        d_tileset->setSmallSelectorFilename(File::get_basename(small_filename));
      }
    if (std::find(delfiles.begin(), delfiles.end(), large_filename)
        == delfiles.end() && response == Gtk::RESPONSE_ACCEPT)
      {
        d_tileset->replaceFileInConfigurationFile(d_tileset->getLargeSelectorFilename()+".png", large_filename);
        d_tileset->setLargeSelectorFilename(File::get_basename(large_filename));
      }
    for (std::list<std::string>::iterator it = delfiles.begin(); 
         it != delfiles.end(); it++)
      File::erase(*it);
    return response;
}

void TilesetSelectorEditorDialog::setup_shield_theme_combobox(Gtk::Box *box)
{
  // fill in shield themes combobox
  shield_theme_combobox = manage(new Gtk::ComboBoxText);

  Shieldsetlist *sl = Shieldsetlist::getInstance();
  std::list<std::string> shield_themes = sl->getValidNames();
  int counter = 0;
  int default_id = 0;
  for (std::list<std::string>::iterator i = shield_themes.begin(),
       end = shield_themes.end(); i != end; ++i)
    {
      if (*i == _("Default"))
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

  if (large_selector_radiobutton->get_active() == true)
    large_filename = selected_filename;
  else if (small_selector_radiobutton->get_active() == true)
    small_filename = selected_filename;
  else
    selected_filename = "";
  if (selected_filename.empty() == false)
    show_preview_selectors(selected_filename);
}

void TilesetSelectorEditorDialog::show_preview_selectors(std::string filename)
{
  //load it up and show in the colours of the selected shield theme

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
  if (heartbeat.connected())
    heartbeat.disconnect();

  for (std::map< guint32, std::list<Glib::RefPtr<Gdk::Pixbuf> >* >::iterator it = selectors.begin();
       it != selectors.end(); it++)
    {
      for (std::list<Glib::RefPtr<Gdk::Pixbuf> >::iterator lit = (*it).second->begin(); lit != (*it).second->end(); lit++)
	{
	  (*lit).clear();
	}
      (*it).second->clear();
      delete ((*it).second);
    }
  selectors.clear();
  preview_table->foreach(sigc::mem_fun(preview_table, &Gtk::Container::remove));
}

bool TilesetSelectorEditorDialog::loadSelector(std::string filename)
{
  std::vector<PixMask *> images;
  std::vector<PixMask *> masks;
  bool success = GraphicsCache::loadSelectorImages(filename, d_tileset->getTileSize(), images, masks);
  if (success)
    {
      std::string subdir = Shieldsetlist::getInstance()->getShieldsetDir 
	(Glib::filename_from_utf8(shield_theme_combobox->get_active_text()));
      Shieldset *shieldset = Shieldsetlist::getInstance()->getShieldset(subdir);

      for (unsigned int i = 0; i < MAX_PLAYERS; i++)
	{
	  std::list<Glib::RefPtr<Gdk::Pixbuf> > *mylist = new std::list<Glib::RefPtr<Gdk::Pixbuf> >();
	  selectors[i] = mylist;
	}

      for (std::vector<PixMask*>::iterator it = images.begin(), mit = masks.begin(); it != images.end(); it++, mit++)
	{
	  for (Shieldset::iterator sit = shieldset->begin(); sit != shieldset->end(); sit++)
	    {
	      if ((*sit)->getOwner() == 8) //ignore neutral
		continue;
		selectors[(*sit)->getOwner()]->push_back
		  (GraphicsCache::applyMask(*it, *mit, (*sit)->getColor(), false)->to_pixbuf());
		
		frame[(*sit)->getOwner()] = selectors[(*sit)->getOwner()]->begin();
	    }
	}

      for (std::vector<PixMask*>::iterator it = images.begin(); it != images.end(); it++)
	delete *it;
      for (std::vector<PixMask*>::iterator it = masks.begin(); it != masks.end(); it++)
	delete *it;

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
      if (large_filename.empty() == false)
	selector_filechooserbutton->set_filename (large_filename);
      else
	clearSelector();
    }
  else if (small_selector_radiobutton->get_active() == true)
    {
      if (small_filename.empty() == false)
	selector_filechooserbutton->set_filename (small_filename);
      else
	clearSelector();
    }
}

void TilesetSelectorEditorDialog::on_heartbeat()
{
  preview_table->foreach(sigc::mem_fun(preview_table, &Gtk::Container::remove));
  preview_table->resize(4, 2);

  int x = 0;
  int y = 0;
  int count = 0;
  for (std::map< guint32, std::list<Glib::RefPtr<Gdk::Pixbuf> >* >::iterator it = selectors.begin();
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
      preview_table->attach(*manage(new Gtk::Image(*frame[count])), y, y+1, x, x+1, Gtk::SHRINK, Gtk::SHRINK, 8, 8);
  
      frame[count]++;
      if (frame[count] == selectors[count]->end())
	frame[count] = selectors[count]->begin();
      count++;
    }
  preview_table->show_all();

}
