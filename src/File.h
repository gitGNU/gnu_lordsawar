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

#ifndef FILE_H
#define FILE_H

#include <string>
#include <list>
#include <SDL.h>

/** \brief Miscellaneous functions for unified file access
  * 
  * These functions should be the sole way to access any files. They will
  * automatically prepend the correct directory, extract the correct file etc.
  * This enables us to install and package LordsAWar (there is no fixed
  * directory structure). To use these functions, you issue e.g. an armyset name
  * and have the full path returned, which is a file you can then load.
  */

class File
{
    public:
        /** Scan the data directories for armysets
          * 
          * @return a list of available armysets
          */
        static std::list<std::string> scanArmysets();


        /** Get the armyset description file
          *
          * @param armysetname      the name of the armyset
          * @return the full name of the description file
          */
        static std::string getArmyset(std::string armysetname);

        /** Get the armyset picture file
          * 
          * @param amysetname       the name of the armyset
          * @return the surface which contains the army pictures of this armyset
          */
        static SDL_Surface* getArmyPicture(std::string armysetname, std::string pic);
        
        

        /** Get the description file for the mapset
          * 
          * @param mapsetname       the name of the mapset
          * @return the full name of the description file
          */
        static std::string getMapset(std::string mapsetname);

        /** Get a mapset picture
          * @param mapsetname       the name of the mapset
          * @param picname          the name of the picture
          * @return the surface which contains the picture
          */
        static SDL_Surface* getMapsetPicture(std::string mapsetname, std::string picname);

	// get a mapset path
	static std::string getMapsetFile(std::string mapsetname, std::string picname);
	
        /** Load a mask needed by various mapset images (e.g. cities)
          * 
          * @param mapsetname       the name of the mapset
          * @param picname          the name of the mask picture
          * @return the surface which contains the surface for the mask
          */
        static SDL_Surface* getMapsetMask(std::string mapsetname, std::string picname);

        
        
        //! load misc file, e.g. hero names 
        static std::string getMiscFile(std::string filename);
        
        /** Load misc pic
          * 
          * @param picname  the name of the image (including the suffix!)
          * @param alpha    set this to false if you encounter transparent
          *                 images that should be opaque. :)
          *                 Especially for background images...
          * @return the surface which contains the image
          */
        static SDL_Surface* getMiscPicture(std::string picname, bool alpha=true);


        
        //! Load the picture of an item
        static SDL_Surface* getItemPicture(std::string picname);

        //! Load the xml file describing the items
        static std::string getItemDescription();
        

        //! Get the path to a border tile
        static SDL_Surface* getBorderPic(std::string filename);

        //! Get the path to an editor image
	static std::string getEditorFile(std::string filename);
    
        //! Load an editor image
        static SDL_Surface* getEditorPic(std::string filename);
        

        // Returns the filename of a music file (description or actual piece)
        static std::string getMusicFile(std::string filename);
        
        // get save game path
        static std::string getSavePath();

        // get the available tilesets
        static std::list<std::string> scanTilesets();

        // get the available maps
        static std::list<std::string> scanMaps();

    private:
        /** Loads an image
          * 
          * This function loads an image, adjusts it to the current resolution etc.
          * to improve blitting performance.
          *
          * @note Some of the images (.jpg??) become transparent if the alpha
          * channel is copied (they don't have transparent bits anyway), so they
          * should be loaded with alpha set to false
          *
          * @param filename     full filename (with path) of the image to load
          * @param alpha        if set to true, copy the alpha channel as well
          *                     (i.e. if the file has transparent pixels, they
          *                     will be marked as transparent in the returned image)
          * @return converted image or 0 if anything failed.
          */
        static SDL_Surface* loadImage(std::string filename, bool alpha = true);
};

#endif //FILE_H

// End of file
