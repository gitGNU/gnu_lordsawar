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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.*

#include <config.h>

#include <libglademm/xml.h>
#include <sigc++/functors/mem_fun.h>

#include "preferences-dialog.h"

#include "glade-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../Configuration.h"
#include "../sound.h"


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

void PreferencesDialog::run()
{
    static int width = -1;
    static int height = -1;

    if (width != -1 && height != -1)
	dialog->set_default_size(width, height);
    
    dialog->show();
    dialog->run();
    
    dialog->get_size(width, height);
    Configuration::saveConfigurationFile(Configuration::configuration_file_path);
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

