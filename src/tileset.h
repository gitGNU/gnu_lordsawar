// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

#ifndef TILESET_H
#define TILESET_H

#include <string>
#include <vector>
#include <sigc++/trackable.h>

#include "Tile.h"
#include "defs.h"
#include "File.h"
using namespace std;

class XML_Helper;

//! A list of Tile objects in a terrain theme.
/** 
 * Tileset is a list of Tile objects.  It acts as the themeing mechanism for
 * the look (and partially the behaviour) of terrain objects in the game.
 * The Tileset dictates the pixel size of the tiles, and is used to lookup
 * Tile and TileStyle objects.  It is implemented as a singleton because many 
 * classes use it for looking up Tile and TileStyle objects.
 * 
 * Tileset objects are often referred to by their subdirectory (Tileset::d_dir).
 *
 * Tileset objects reside on disk in the tilesets/ directory, each of which is
 * inside it's own directory.
 *
 * The tileset configuration file is a same named XML file inside the Tileset's
 * directory.  E.g. tilesets/${Tileset::d_dir}/${Tileset::d_dir}.xml.
 */
class Tileset : public sigc::trackable, public std::vector<Tile*>
{
    public:
	//! The xml tag of this object in a tileset configuration file.
	static std::string d_tag; 
	static std::string d_road_smallmap_tag; 

	//! Return the default height and width of a tile in the tileset.
	static guint32 getDefaultTileSize();

	//! Default constructor.
	/**
	 * Make a new Tileset.
	 *
	 * @param id    A unique numeric identifier among all tilesets.
	 * @param name  The name of the Tileset.  Analagous to Tileset::d_name.
	 */
	Tileset(guint32 id, std::string name);

	//! Loading constructor.
	/**
	 * Make a new Tileset object by loading the data from a tileset
	 * configuration file.
	 *
	 * @param helper  The opened tileset configuration file to load the
	 *                tileset from.
	 */
        Tileset(XML_Helper* helper, bool private_collection = false);

	static Tileset *create(std::string file, bool private_collection = false);

	//! Destructor.
        ~Tileset();

	//! Return the subdirectory of this Tileset.
        std::string getSubDir() const {return d_dir;}

	//! Set the subdirectory of where this Tileset resides on disk.
        void setSubDir(std::string dir);

	//! Get the unique identifier for this tileset.
	/**
	 * Analagous to the tileset.d_id XML entity in the tileset 
	 * configuration file.
	 */
        guint32 getId() const {return d_id;}

	//! Set the unique identifier for this tileset.
	/**
	 * @note This method is only used in the tileset editor.  
	 */
        void setId(guint32 id) {d_id = id;}

        //! Returns the name of the tileset.
        std::string getName() const {return _(d_name.c_str());}

	//! Set the name of the tileset.
	/**
	 * @note This method is only used in the tileset editor.
	 */
        void setName(std::string name) {d_name = name;}

        //! Returns the description of the tileset.
        std::string getInfo() const {return _(d_info.c_str());}

	//! Set the description of the tileset.
	/**
	 * @note This method is only used in the tileset editor.
	 */
        void setInfo(std::string info) {d_info = info;}

        //! Returns the tilesize of the tileset.
        guint32 getTileSize() const {return d_tileSize;}

	//! Sets the tilesize of the tileset.
	void setTileSize(guint32 tileSize) {d_tileSize = tileSize;}

        //! Returns the index to the given terrain type.
        guint32 getIndex(Tile::Type type) const;

	void setLargeSelectorFilename(std::string p){d_large_selector = p;};
	void setSmallSelectorFilename(std::string p){d_small_selector = p;};
	std::string getLargeSelectorFilename() {return d_large_selector;};
	std::string getSmallSelectorFilename() {return d_small_selector;};
	void setExplosionFilename(std::string p){d_explosion = p;};
	std::string getExplosionFilename() {return d_explosion;};
	void setRoadsFilename(std::string p){d_roads = p;};
	std::string getRoadsFilename() {return d_roads;};
	void setBridgesFilename(std::string p){d_bridges = p;};
	std::string getBridgesFilename() {return d_bridges;};
	void setFogFilename(std::string p){d_fog = p;};
	std::string getFogFilename() {return d_fog;};
	void setFlagsFilename(std::string p){d_flags = p;};
	std::string getFlagsFilename() {return d_flags;};

	//! Lookup tilestyle by it's id in this tileset.
	TileStyle *getTileStyle(guint32 id) {return d_tilestyles[id];}

	//! Lookup a random tile style.
	/**
	 * Scan the TileStyles for the given Tile (given by index) for a
	 * TileStyle that matches the given style.  When there is more than
	 * one TileStyle to choose from, randomly pick one from all of the 
	 * matching TileStyle objects.
	 *
	 * @param index  The index of the Tile in this set to operate on.
	 * @param style  The kind of style we're looking for.
	 *
	 * @return A pointer to the matching TileStyle object, or NULL if no 
	 *         TileStyle could be found with that given style.
	 */
	TileStyle *getRandomTileStyle(guint32 index, TileStyle::Type style);

	//! Save a Tileset to an opened tile configuration file.
	/**
	 * @param  The opened XML tile configuration file.
	 */
	bool save(XML_Helper *helper);

	Tile *lookupTileByName(std::string name);

	int getFreeTileStyleId();

	int getLargestTileStyleId();

	bool validate();

        //! Get the colour associated with the road on the smallmap.
	Gdk::Color getRoadColor() const {return d_road_color;};
	void setRoadColor(Gdk::Color color) {d_road_color = color;};

	//! Return whether this is an tileset in the user's personal collection.
	bool fromPrivateCollection() {return private_collection;};

	void setExplosionImage(PixMask *p) {explosion = p;};
	PixMask *getExplosionImage() {return explosion;};
	void setRoadImage(guint32 i, PixMask *p) {roadpic[i] = p;};
	PixMask *getRoadImage(guint32 i) {return roadpic[i];};
	void setBridgeImage(guint32 i, PixMask *p) {bridgepic[i] = p;};
	PixMask *getBridgeImage(guint32 i) {return bridgepic[i];};
	void setFlagImage(guint32 i, PixMask *p) {flagpic[i] = p;};
	PixMask *getFlagImage(guint32 i) {return flagpic[i];};
	void setFlagMask(guint32 i, PixMask *p) {flagmask[i] = p;};
	PixMask *getFlagMask(guint32 i) {return flagmask[i];};
	void setFogImage(guint32 i, PixMask *p) {fogpic[i] = p;};
	PixMask *getFogImage(guint32 i) {return fogpic[i];};
	void setSelectorImage(guint32 i, PixMask *p) {selector[i] = p;};
	PixMask *getSelectorImage(guint32 i) {return selector[i];};
	void setSelectorMask(guint32 i, PixMask *p) {selectormask[i] = p;};
	PixMask *getSelectorMask(guint32 i) {return selectormask[i];};
	void setSmallSelectorImage(guint32 i, PixMask *p) {smallselector[i] = p;};
	PixMask *getSmallSelectorImage(guint32 i) {return smallselector[i];};
	void setSmallSelectorMask(guint32 i, PixMask *p) {smallselectormask[i] = p;};
	PixMask *getSmallSelectorMask(guint32 i) {return smallselectormask[i];};
	guint32 getNumberOfSelectorFrames() {return number_of_selector_frames;};
	guint32 getNumberOfSmallSelectorFrames() {return number_of_small_selector_frames;};
	void setNumberOfSelectorFrames(guint32 s) {selector.reserve(s); selectormask.reserve(s); number_of_selector_frames = s;};
	void setNumberOfSmallSelectorFrames(guint32 s) {smallselector.reserve(s);smallselectormask.reserve(s); number_of_small_selector_frames = s;};
    private:
        //! Callback to load Tile objects into the Tileset.
        bool loadTile(std::string, XML_Helper* helper);

        // DATA
	//! The name of the Tileset.
	/**
	 * Equates to the tileset.d_name XML entity in the tileset 
	 * configuration file.
	 * This value appears in dialogs where the user is asked to select
	 * a Tileset among all other Tileset objects available to the game.
	 */
        std::string d_name;

	//! A unique numeric identifier among all tilesets.
	guint32 d_id;

	//! The description of the Tileset.
	/**
	 * Equates to the tileset.d_info XML entity in the tileset
	 * configuration file.
	 * This value is not used.
	 */
        std::string d_info;

	//! The size of the graphic tiles in the Tileset.
	/**
	 * Equates to the tileset.d_tilesize XML entity in the tileset
	 * configuration file.
	 * It represents the size in pixels of the width and height of tile
	 * imagery onscreen.
	 */
        guint32 d_tileSize;

	//! The subdirectory of the Tileset.
	/**
	 * This is the name of the subdirectory that the Tileset files are
	 * residing in.  It does not contain a path (e.g. no slashes).
	 * Tileset directories sit in the tileset/ directory.
	 */
        std::string d_dir;

	std::string d_small_selector;
	std::string d_large_selector;
	std::string d_explosion;
	std::string d_fog;
	std::string d_roads;
	std::string d_bridges;
	std::string d_flags;

        typedef std::map<guint32, TileStyle*> TileStyleIdMap;
	//! A map that provides a TileStyle when supplying a TileStyle id.
        TileStyleIdMap d_tilestyles;

	Gdk::Color d_road_color;

	//! Whether this is a system tileset, or one that the user made.
	bool private_collection;


        PixMask* roadpic[ROAD_TYPES];
        PixMask* bridgepic[BRIDGE_TYPES];
        PixMask* flagpic[MAX_STACK_SIZE];
        PixMask* flagmask[MAX_STACK_SIZE];
	guint32 number_of_selector_frames;
	std::vector<PixMask* > selector;
	std::vector<PixMask* > selectormask;
	guint32 number_of_small_selector_frames;
	std::vector<PixMask* > smallselector;
	std::vector<PixMask* > smallselectormask;
	PixMask* explosion;
	PixMask*fogpic[FOG_TYPES];
};
#endif // TILESET_H

// End of file
