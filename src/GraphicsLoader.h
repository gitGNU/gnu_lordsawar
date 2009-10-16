// Copyright (C) 2008, 2009 Ben Asselstine
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

#ifndef GRAPHICS_LOADER_H
#define GRAPHICS_LOADER_H

#include <string>
#include <list>
#include <gtkmm.h>
#include "PixMask.h"
#include "shield.h"

class Shieldset;
class Shieldsetlist;
class Tileset;
class Tilesetlist;
class TileStyleSet;
class Armyset;
class Armysetlist;
class ArmyProto;
class Cityset;
class Citysetlist;

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
	static void instantiateImages(Tileset *ts, TileStyleSet *tss, guint32 tilesize);
	static void instantiateImages(Citysetlist *tsl);
	static void instantiateImages(Cityset *ts);

        /** Load misc pic
          * 
          * @param picname  the name of the image (including the suffix).
          * @param alpha    set this to false if you encounter transparent
          *                 images that should be opaque. :)
          *                 Especially for background images...
          * @return the surface which contains the image
          */
        static PixMask* getMiscPicture(std::string picname, bool alpha=true);


	static bool instantiateImages(Armyset *set, ArmyProto *a, Shield::Colour c);
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
        static PixMask* loadImage(std::string filename, bool alpha = true);

	static void loadShipPic(Armyset *armyset);
	static void loadStandardPic(Armyset *armyset);
	static void uninstantiateImages(Armysetlist *asl);
	static void uninstantiateImages(Tilesetlist *ssl);
	static void uninstantiateImages(Shieldsetlist *ssl);
	static void uninstantiateImages(Shieldset *shieldset);
	static void uninstantiateImages(Tileset *ts);
	static void uninstantiateImages(Armyset *armyset);
	static void uninstantiateImages(Citysetlist *csl);
	static void uninstantiateImages(Cityset *cs);
};

#endif //GRAPHICS_LOADER_H

// End of file
