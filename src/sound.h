// Copyright (C) 2006 Ulf Lorenz
// Copyright (C) 2006 Andrea Paternesi
// Copyright (C) 2007, 2009, 2014 Ben Asselstine
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

#ifndef SOUND_H
#define SOUND_H

#include "xmlhelper.h"

#include <map>
#include <vector>
#include <sigc++/trackable.h>
#ifdef FL_SOUND
#include <SDL_mixer.h>
#endif

/** Sound class
  * 
  * The purpose of putting the sound code into one class is (besides hiding the
  * internals and supplying a friendly interface) to put all these ugly ifdefs
  * in one place (sound can be disabled for fewer dependencies).
  *
  * @note: As this becomes too complicated, I throw away the caching stuff.
  *
  * Sound and music are treated a bit differently since the mixer calls are others.
  * However, both are referenced by strings (I think in this case, id's aren't
  * very readable). There are basically two types of music. Background music
  * is enabled by enableBackground(). It will always play until disabled again
  * by looking through the databse of available background music pieces. On top
  * of that, it is possible to play other music pieces. In this case, the
  * background music fades out (maybe), the other music fades or pops in
  * and goes away again with the backgroun dmusic taking its place again
  * afterwards.
  */

struct MusicItem
{
    // The file where the sound piece can be loaded from
    Glib::ustring file;
    // Can it be played in the background?
    bool background;
    // If loading this file fails, we can define an alias to load instead.
    Glib::ustring alias;
};


//! This class manages sound within the game.
class Sound : public sigc::trackable
{
    public:
	// Get Methods
        
        //! Returns whether music is enabled
        bool isMusicEnabled();

        //! Returns the music volume in the range 0..128
        int getMusicVolume();


	// Set Methods

        /** Enables/disables music and sets volume. If the sound is disabled,
          * subsequent calls to play sounds will be silently ignored.
          *
          * @param enable       enable/disable sound
          * @param volume       set the sound volume in the range from 0 to 128
          *
          * @return false for wrong volume data, otherwise true
          */
        bool setMusic(bool enable, int volume);


	// Methods that operate on class data and modify the class.

        /** Plays a given music piece.
          *
          * The current (background) track will be stopped (faded out) if
          * neccessary and the new piece will be faded in. Each call to
          * playMusic should be accompanied by a call to haltMusic(), otherwise the
          * background music wil not continue.
          * 
          * @param piece        the identifier(name) of the music track to play.
          * @param nloops       the amount of time the piece should be played
          *                     (-1: infinitely often)
          * @param fade         if set to true, fade out a playing music piece 
          * @return false if any error occurred.
          */
        bool playMusic(Glib::ustring piece, int nloops = -1, bool fade = true);
        
        /** Stops the current (event) music. Note that the background music might
          * continue with playing.
          *
          * @param fade         if set to true, fade out a playing piece.
          *
          * @return false on error.
          */
        bool haltMusic(bool fade = true);

        /** Enables background music.
          * 
          * Starts playing background music. Picks a random piece that has
          * the background tag enabled and starts playing it, then picks the
          * next etc.
          */
        void enableBackground();

        /** Stops playing of background music
          * 
          * @param fade     if set to true, fade out.
          */
        void disableBackground(bool fade=true);

        //! Activates the next background piece
        void nextPiece();


	// Static Methods

        //! Singleton getter
        static Sound* getInstance();

        //! Explicitely delete the singleton
        static void deleteInstance();

    private:
        //! Constructor.  Initializes the sound and loads the music data
        Sound();

        //! Destructor.  Deinitializes sound
        ~Sound();

        //! Callback for the music data, see XML_Helper
        bool loadMusic(Glib::ustring tag, XML_Helper* helper);

	// DATA
        
        // music is stored here, access by d_musicMap[name]
        std::map<Glib::ustring, MusicItem*> d_musicMap;
        std::vector<Glib::ustring> d_bgMap;  // shallow copy of background pieces

        // currently playing background and foreground piece
#ifdef FL_SOUND
        Mix_Music* d_music;
#endif

        // if initialization failed, set this to true => no music/sound played
        bool d_broken;

        // if set to true, play background music
        bool d_background;

        // if set to true, don't continue with next piece
        bool d_block;

        // static instanton pointer
        static Sound* s_instance;
};

#endif //SOUND_H
