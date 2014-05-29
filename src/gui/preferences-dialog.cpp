//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014 Ben Asselstine
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

#include "preferences-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "Configuration.h"
#include "sound.h"
#include "game.h"
#include "playerlist.h"
#include "citylist.h"
#include "ruinlist.h"
#include "city.h"
#include "AI_Diplomacy.h"
#include "AI_Analysis.h"
#include "ai_fast.h"
#include "GraphicsCache.h"


PreferencesDialog::PreferencesDialog(Gtk::Window &parent, bool readonly)
{
  d_readonly = readonly;
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/preferences-dialog.ui");

    xml->get_widget("dialog", dialog);
    dialog->set_transient_for(parent);
    xml->get_widget("commentator_checkbutton", commentator_checkbutton);
    xml->get_widget("speed_scale", speed_scale);
    xml->get_widget("ui_combobox", ui_combobox);
    ui_combobox->signal_changed().connect(
	sigc::mem_fun(this, &PreferencesDialog::on_ui_form_factor_changed));
    xml->get_widget("play_music_checkbutton", play_music_checkbutton);
    xml->get_widget("music_volume_scale", music_volume_scale);
    xml->get_widget("music_volume_hbox", music_volume_hbox);
    xml->get_widget("players_vbox", players_vbox);
    
    GraphicsCache *gc = GraphicsCache::getInstance();
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
      {
	Player *p = Playerlist::getInstance()->getPlayer(i);
	if (p == NULL)
	  continue;
	if (p == Playerlist::getInstance()->getNeutral())
	  continue;
	Gtk::HBox *player_hbox = new Gtk::HBox();
	Gtk::Image *image = new Gtk::Image();
	image->property_pixbuf() = gc->getShieldPic(2, p)->to_pixbuf();
	Gtk::ComboBoxText *type = new Gtk::ComboBoxText();
	type->signal_changed().connect
	  (sigc::bind(sigc::mem_fun
		      (this, &PreferencesDialog::on_type_changed), type));
	Gtk::CheckButton *observe = new Gtk::CheckButton(_("Observe"));
	observe->signal_toggled().connect
	  (sigc::bind(sigc::mem_fun
		      (this, &PreferencesDialog::on_observe_toggled), observe));

	observe->set_active(p->isObservable());
	type->append(_("Human"));
	type->append(_("Computer"));
        if (readonly)
          type->append(_("Networked"));
	if (p->getType() == Player::HUMAN)
	  {
	    observe->set_sensitive(false);
	    type->set_active(0);
	  }
        else if (p->getType() == Player::NETWORKED)
	  {
	    observe->set_sensitive(false);
	    type->set_active(2);
	  }
	else
	  type->set_active(1);
	if (p->isDead() || (Playerlist::getActiveplayer() == p && 
			    p->getType() != Player::HUMAN))
	  {
	    type->set_sensitive(false);
	    observe->set_sensitive(false);
	  }
	if (readonly)
	  type->set_sensitive(false);
	player_hbox->pack_start(*manage(image), Gtk::PACK_SHRINK, 
				Gtk::PACK_SHRINK);
	player_hbox->pack_start(*manage(type), Gtk::PACK_SHRINK, 10);
	player_hbox->pack_start(*manage(observe), Gtk::PACK_SHRINK, 10);
	//player_types.push_back(type);
	player_types[p] = type;
	player_observed[p] = observe;
	//player_observed.push_back(observe);
	Gtk::Label *player_name = new Gtk::Label(p->getName());
	player_hbox->pack_start(*manage(player_name), Gtk::PACK_SHRINK, 10);
	players_vbox->pack_start(*manage(player_hbox));
      }
    players_vbox->show_all_children();
    commentator_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &PreferencesDialog::on_show_commentator_toggled));
    speed_scale->set_value(Configuration::s_displaySpeedDelay);
    speed_scale->signal_value_changed().connect(
	sigc::mem_fun(this, &PreferencesDialog::on_speed_changed));
    play_music_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &PreferencesDialog::on_play_music_toggled));
    music_volume_scale->signal_value_changed().connect(
	sigc::mem_fun(this, &PreferencesDialog::on_music_volume_changed));

    commentator_checkbutton->set_active(Configuration::s_displayCommentator);
    play_music_checkbutton->set_active(Configuration::s_musicenable);
    music_volume_hbox->set_sensitive(Configuration::s_musicenable);
    music_volume_scale->set_value(Configuration::s_musicvolume * 100.0 / 128);
    ui_combobox->set_active(Configuration::s_ui_form_factor);
    
}

PreferencesDialog::~PreferencesDialog()
{
  delete dialog;
}

void PreferencesDialog::on_type_changed(Gtk::ComboBoxText *combo)
{
  for (PlayerTypeMap::iterator it = player_types.begin();
       it != player_types.end(); it++)
    {
      if (combo == (*it).second)
	{
	  /**
	   * if we're turning this player into a human,
	   * then we desensitize the associated checkbutton
	   * otherwise, we presume that we want to observe it
	   */
	  if (combo->get_active_text() == _("Human"))
	    {
	      player_observed[(*it).first]->set_active(true);
	      player_observed[(*it).first]->set_sensitive(false);
	    }
	  else
	    {
	      player_observed[(*it).first]->set_sensitive(true);
	      player_observed[(*it).first]->set_active(true);
	    }
	}
    }
}

void PreferencesDialog::on_observe_toggled(Gtk::CheckButton *button)
{
  for (PlayerObserveMap::iterator it = player_observed.begin();
       it != player_observed.end(); it++)
    {
      if (button == (*it).second)
	(*it).first->setObservable(button->get_active());
    }
}

void PreferencesDialog::hide()
{
  dialog->hide();
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
    PlayerTypeMap::iterator j = player_types.begin();
    for (; j != player_types.end(); ++j)
      {
	Player *p = (*j).first;
	if (p == NULL)
	  continue;
	if (p == Playerlist::getInstance()->getNeutral())
	  continue;
	if (p->getType() == Player::HUMAN) //changing human to:
	  {
	    if ((*j).second->get_active_text() == _("Human")) //human, no change
	      ;
	    else //computer, change to easy
	      {
		AI_Fast *new_player = new AI_Fast(*p);
		Player *old_player = p;
		Playerlist::getInstance()->swap(old_player, new_player);
		//disconnect and connect game signals
		game->addPlayer(new_player);
		delete old_player;
	      }
	  }
	else //changing computer to:
	  {
	    if ((*j).second->get_active_text() == _("Human")) //human, change it
	      {
		RealPlayer *new_player = new RealPlayer(*p);
		Player *old_player = p;
		Playerlist::getInstance()->swap(old_player, new_player);
		//disconnect and connect game signals
		game->addPlayer(new_player);
		delete old_player;
	      }
	    else //computer, no change
	      ;
	  }
      }
    dialog->hide();
}

void PreferencesDialog::on_show_commentator_toggled()
{
    Configuration::s_displayCommentator = commentator_checkbutton->get_active();
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

void PreferencesDialog::on_speed_changed()
{
  Configuration::s_displaySpeedDelay = int(speed_scale->get_value());
}

void PreferencesDialog::on_music_volume_changed()
{
    int volume = int(music_volume_scale->get_value() / 100 * 128);
    
#ifdef FL_SOUND
    Mix_VolumeMusic(volume);
#endif

    Configuration::s_musicvolume = volume;
}

void PreferencesDialog::on_ui_form_factor_changed()
{
  Configuration::s_ui_form_factor = 
    Configuration::UiFormFactor (ui_combobox->get_active_row_number());
  ui_form_factor_changed.emit(Configuration::s_ui_form_factor);
}
