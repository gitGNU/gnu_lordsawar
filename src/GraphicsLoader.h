// Copyright (C) 2008, 2009 Ben Asselstine
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

#ifndef GRAPHICS_LOADER_H
#define GRAPHICS_LOADER_H

#include <string>
#include <list>
#include <gtkmm.h>

class Shieldset;
class Shieldsetlist;
class Tileset;
class Tilesetlist;
class TileStyleSet;
class Armyset;
class Armysetlist;
class ArmyProto;

/** \brief Loads images associated with armies, tiles and shields.
  * 
  */

class GraphicsLoader
{
    public:
	static void instantiateImages(Shieldset *shieldset);
	static void instantiateImages(Shieldsetlist *ssl);
	static void instantiateImages(Shieldsetlist *ssl, std::string subdir);
	static void instantiateImages(Armyset *armyset);
	static void instantiateImages(Armysetlist *asl);
	static void instantiateImages(Tilesetlist *tsl);
	static void instantiateImages(Tileset *ts);
	static void instantiateImages(TileStyleSet *tss, guint32 tilesize);

        /** Get the armyset picture file
          * 
          * @param armysetsubdir     the name of the armyset.  this is the
	  *                          subdirectory name of the armyset within
	  *                          the armyset directory.
	  * @param pic               the name of the army picture.
          * @return the surface which contains the army pictures of this armyset
          */
        static Glib::RefPtr<Gdk::Pixbuf> getArmyPicture(std::string armysetsubdir, std::string pic);
        
        /** Get the shield picture file
          * 
          * @param shieldsetsubdir     the name of the shieldset.  this is the
	  *                            subdirectory name of the shieldset
	  *                            within the shieldset directory.
	  * @param pic                 the name of the shield picture.
          * @return the surface which contains the shield picture
          */
        static Glib::RefPtr<Gdk::Pixbuf> getShieldsetPicture(std::string shieldsetsubdir, std::string pic);

        /** Get a tileset picture
          * @param tilesetsubdir     the name of the tileset.  this is the
	  *                          subdirectory name of the tileset within
	  *                          the tileset directory.
          * @param picname          the name of the picture.
          * @return the surface which contains the picture
          */
        static Glib::RefPtr<Gdk::Pixbuf> getTilesetPicture(std::string tilesetsubdir, std::string picname);

        /** Load misc pic
          * 
          * @param picname  the name of the image (including the suffix).
          * @param alpha    set this to false if you encounter transparent
          *                 images that should be opaque. :)
          *                 Especially for background images...
          * @return the surface which contains the image
          */
        static Glib::RefPtr<Gdk::Pixbuf> getMiscPicture(std::string picname, bool alpha=true);

        /** Get a cityset picture
          * @param citysetsubdir     the name of the cityset.  this is the
	  *                          subdirectory name of the cityset within
	  *                          the cityset directory.
          * @param picname          the name of the picture.
          * @return the surface which contains the picture
          */
        static Glib::RefPtr<Gdk::Pixbuf> getCitysetPicture(std::string citysetsubdir, std::string picname);

    private:
	static bool instantiateImages(Armyset *set, ArmyProto *a);
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
        static Glib::RefPtr<Gdk::Pixbuf> loadImage(std::string filename, bool alpha = true);

	static void loadShipPic(Armyset *armyset);
	static void loadStandardPic(Armyset *armyset);
	static void uninstantiateImages(Armysetlist *asl);
	static void uninstantiateImages(Tilesetlist *ssl);
	static void uninstantiateImages(Shieldsetlist *ssl);
	static void uninstantiateImages(Shieldset *shieldset);
	static void uninstantiateImages(Tileset *ts);
	static void uninstantiateImages(Armyset *armyset);
};

#endif //GRAPHICS_LOADER_H

// End of file
