//  Copyright (C) 2007, Ole Laursen
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

#include <config.h>

#include <libglademm/xml.h>
#include <sigc++/functors/mem_fun.h>

#include "preferences-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../Configuration.h"
#include "../sound.h"
#include "../game.h"
#include "../playerlist.h"
#include "../citylist.h"
#include "../ruinlist.h"
#include "../ai_fast.h"
#include "../GraphicsCache.h"


PreferencesDialog::PreferencesDialog()
{
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/preferences-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    xml->get_widget("show_turn_popup_checkbutton", show_turn_popup_checkbutton);
    xml->get_widget("play_music_checkbutton", play_music_checkbutton);
    xml->get_widget("music_volume_scale", music_volume_scale);
    xml->get_widget("music_volume_hbox", music_volume_hbox);
    xml->get_widget("players_vbox", players_vbox);
    
    GraphicsCache *gc = GraphicsCache::getInstance();
    Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::iterator i = pl->begin(); i != pl->end(); ++i)
      {
	if (*i == Playerlist::getInstance()->getNeutral())
	  continue;
	Gtk::HBox *player_hbox = new Gtk::HBox();
	Gtk::Image *image = new Gtk::Image(to_pixbuf(gc->getShieldPic(2, *i)));
	Gtk::ComboBoxText *type = new Gtk::ComboBoxText();
	type->append_text(_("Human"));
	type->append_text(_("Computer"));
	if ((*i)->getType() == Player::HUMAN)
	  type->set_active(0);
	else
	  type->set_active(1);
	if ((*i)->isDead())
	  type->set_sensitive(false);
	player_hbox->pack_start(*manage(image), Gtk::PACK_SHRINK, 
				Gtk::PACK_SHRINK);
	player_hbox->pack_start(*manage(type), Gtk::PACK_SHRINK, 10);
	player_types.push_back(type);
	Gtk::Label *player_name = new Gtk::Label((*i)->getName());
	player_hbox->pack_start(*manage(player_name), Gtk::PACK_SHRINK, 10);
	players_vbox->pack_start(*manage(player_hbox));
      }
    players_vbox->show_all_children();
    show_turn_popup_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &PreferencesDialog::on_show_turn_popup_toggled));
    play_music_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &PreferencesDialog::on_play_music_toggled));
    music_volume_scale->signal_value_changed().connect(
	sigc::mem_fun(this, &PreferencesDialog::on_music_volume_changed));

    show_turn_popup_checkbutton->set_active(Configuration::s_showNextPlayer);
    play_music_checkbutton->set_active(Configuration::s_musicenable);
    music_volume_hbox->set_sensitive(Configuration::s_musicenable);
    music_volume_scale->set_value(Configuration::s_musicvolume * 100.0 / 128);
    
}

void PreferencesDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void PreferencesDialog::run(Game *game)
{
    static int width = -1;
    static int height = -1;

    if (width != -1 && height != -1)
	dialog->set_default_size(width, height);
    
    dialog->show();
    dialog->run();
    
    dialog->get_size(width, height);
    Configuration::saveConfigurationFile(Configuration::configuration_file_path);
    Playerlist* pl = Playerlist::getInstance();
    std::list<Gtk::ComboBoxText *>::iterator j = player_types.begin();
    Playerlist::iterator i = pl->begin();
    for (; j != player_types.end(); ++i, ++j)
      {
	if ((*i) == Playerlist::getInstance()->getNeutral())
	  i++;
	if ((*i)->getType() == Player::HUMAN) //changing human to:
	  {
	    if ((*j)->get_active_text() == _("Human")) //human, no change
	      ;
	    else //computer, change to easy
	      {
		AI_Fast *new_player = new AI_Fast(**i);
		Player *old_player = *i;
		Playerlist::getInstance()->swap(old_player, new_player);
		//disconnect and connect game signals
		game->addPlayer(new_player);
	      }
	  }
	else //changing computer to:
	  {
	    if ((*j)->get_active_text() == _("Human")) //human, change it
	      {
		RealPlayer *new_player = new RealPlayer(**i);
		Player *old_player = *i;
		Playerlist::getInstance()->swap(old_player, new_player);
		//disconnect and connect game signals
		game->addPlayer(new_player);
		
	      }
	    else //computer, no change
	      ;
	  }
      }
    dialog->hide();
}

void PreferencesDialog::on_show_turn_popup_toggled()
{
    Configuration::s_showNextPlayer = show_turn_popup_checkbutton->get_active();
}

void PreferencesDialog::on_play_music_toggled()
{
    bool play_music = play_music_checkbutton->get_active();

    Configuration::s_musicenable = play_music;

    if (play_music)
    {
        Sound::getInstance()->enableBackground();
    }
    else
    {
        Sound::getInstance()->haltMusic();
        Sound::getInstance()->disableBackground();
    }
    music_volume_hbox->set_sensitive(Configuration::s_musicenable);
}

void PreferencesDialog::on_music_volume_changed()
{
    int volume = int(music_volume_scale->get_value() / 100 * 128);
    
#ifdef FL_SOUND
    Mix_VolumeMusic(volume);
#endif

    Configuration::s_musicvolume = volume;
}

