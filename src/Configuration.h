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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>
#include <SDL_types.h>
#include <sigc++/trackable.h>

class XML_Helper;

/** \brief The class which holds all configuration options
  * 
  * Basically, this class is more of a namespace than a real class. It
  * provides all global information about directories, settings etc.
  */

// TODO: do we really want all this static stuff or rather do a singleton or such?

class Configuration : public sigc::trackable
{
    public:
        // CREATORS
        Configuration();
        ~Configuration();

        /** \brief Load a configuration file
          * 
          * @param fileName     the full name of the config file
          *
          * This class loads an xml-style config file and sets the settings
          * appropriately.
          */
        bool loadConfigurationFile(std::string fileName);

        /** \brief Save the configuration file
          * 
          * @param fileName     the full name of the config file
          *
          * This class saves the current config to an xml-style config file.
          */
        static bool saveConfigurationFile(std::string filename);

        static std::string configuration_file_path;
	
        // as the name implies
        static bool s_smoothScrolling;
        static bool s_showNextPlayer;
        static int s_displaySpeedDelay;
        
        //the paths
        static std::string s_dataPath;
        static std::string s_savePath;

        // Language setting
        static std::string s_lang;

	// fullscreen mode or not
        static Uint32 s_flags;

        // if using hardware surfaces for pixmaps or not
        static Uint32 s_surfaceFlags;
        static bool s_hardware;

        //the maximum size of the graphics cache
        static Uint32 s_cacheSize;

        //zip and obfuscate save files
        static bool s_zipfiles;

        //run game in GGZ mode
        static bool s_ggz;

        // if fullscreen is set  
        static bool s_fullScreen;

        // music settings; the cache size is given in pieces instead of memory
        static bool s_musicenable;
        static Uint32 s_musicvolume;
        static Uint32 s_musiccache;

    private:
        /** \brief The callback for the XML_Helper
          * 
          * See the XML_Helper documentation for an explanation what the
          * callback is good for.
          */
        bool parseConfiguration(std::string tag, XML_Helper* helper);

        static std::string s_filename;
};

#endif // CONFIGURATION_H
