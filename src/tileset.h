// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2014 Ben Asselstine
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
#include "set.h"
#include "SmallTile.h"
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
 * Tileset objects are often referred to by their base name 
 * (Tileset::d_basename).
 *
 * Tileset objects reside on disk in the tilesets/ directory, each of which is
 * it's own .lwt file.
 */
class Tileset : public sigc::trackable, public std::vector<Tile*>, public Set
{
    public:
	//! The xml tag of this object in a tileset configuration file.
	static std::string d_tag; 

	//! The xml tag of the road smallmap section of the tileset.
	static std::string d_road_smallmap_tag; 

	//! The xml tag of the ruin smallmap section of the tileset.
	static std::string d_ruin_smallmap_tag; 

	//! The xml tag of the temple smallmap section of the tileset.
	static std::string d_temple_smallmap_tag; 

	//! tilesets have this extension. e.g. ".lwt".
	static std::string file_extension; 

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
        Tileset(XML_Helper* helper, std::string directory);

        //! Copy constructor.
        Tileset(const Tileset& tileset);

	//! Destructor.
        ~Tileset();


	// Get Methods

	//! Get the unique identifier for this tileset.
	/**
	 * Analagous to the tileset.d_id XML entity in the tileset 
	 * configuration file.
	 */
        guint32 getId() const {return d_id;}

	//! Return the basename of this Tileset.
        std::string getBaseName() const {return d_basename;}

        //! Returns the name of the tileset.
        std::string getName() const {return _(d_name.c_str());}

        //! Returns the copyright holders of the tileset.
        std::string getCopyright () const {return d_copyright;};

        //! Returns the license of the tileset.
        std::string getLicense() const {return d_license;};

        //! Returns the description of the tileset.
        std::string getInfo() const {return _(d_info.c_str());}

        //! Returns the tilesize of the tileset.
        guint32 getTileSize() const {return d_tileSize;}

	//! Returns the basename of the file containing big selector images.
	std::string getLargeSelectorFilename() {return d_large_selector;};

	//! Returns the basename of the file containing small selector images.
	std::string getSmallSelectorFilename() {return d_small_selector;};

	//! Returns the basename of the file containing the explosion image.
	std::string getExplosionFilename() {return d_explosion;};

	//! Returns the basename of the file containing the road images.
	std::string getRoadsFilename() {return d_roads;};

	//! Returns the basename of the file containing the bridge images.
	std::string getBridgesFilename() {return d_bridges;};

	//! Returns the basename of the file containing the fog images.
	std::string getFogFilename() {return d_fog;};

	//! Returns the basename of the file containing the flag images.
	std::string getFlagsFilename() {return d_flags;};

        //! Get the colour associated with the road on the smallmap.
	Gdk::RGBA getRoadColor() const {return d_road_color;};

        //! Get the colour associated with temples on the smallmap.
	Gdk::RGBA getTempleColor() const {return d_temple_color;};

        //! Get the colour associated with ruins on the smallmap.
	Gdk::RGBA getRuinColor() const {return d_ruin_color;};

	//! Get the explosion image.
	PixMask *getExplosionImage() {return explosion;};

	//! Get a road image.  Pass in the index.
	PixMask *getRoadImage(guint32 i) {return roadpic[i];};

	//! Get a bridge image.  Pass in the index.
	PixMask *getBridgeImage(guint32 i) {return bridgepic[i];};

	//! Get a flag image.  Pass in the index.
	PixMask *getFlagImage(guint32 i) {return flagpic[i];};

	//! Get the flag mask.  Pass in the index.
	PixMask *getFlagMask(guint32 i) {return flagmask[i];};

	//! Get the fog image.  Passin the index.
	PixMask *getFogImage(guint32 i) {return fogpic[i];};

	//! Get the big selector image.  Pass in the index.
	PixMask *getSelectorImage(guint32 i) {return selector[i];};

	//! Get the big selector mask.  Pass in the index.
	PixMask *getSelectorMask(guint32 i) {return selectormask[i];};

	//! Get the small selector image.  Pass in the index.
	PixMask *getSmallSelectorImage(guint32 i) {return smallselector[i];};

	//! Get the small selector mask.  Pass in the index.
	PixMask *getSmallSelectorMask(guint32 i) {return smallselectormask[i];};

	//! Get the number of animation frames in the big selector image.
	guint32 getNumberOfSelectorFrames() {return number_of_selector_frames;};

	//! Get the number of animation frames in the small selector image.
	guint32 getNumberOfSmallSelectorFrames() {return number_of_small_selector_frames;};

        //! Get the first tile that has a certain pattern on the small map.
        Tile *getFirstTile(SmallTile::Pattern pattern) const;

	//! Get filenames in this tileset, excepting the configuration file.
	void getFilenames(std::list<std::string> &files);

	std::string getConfigurationFile() const;

	// Set Methods

	//! Set the basename of where this Tileset resides on disk.
        void setBaseName(std::string dir);

	//! Set the unique identifier for this tileset.
	/**
	 * @note This method is only used in the tileset editor.  
	 */
        void setId(guint32 id) {d_id = id;}

	//! Set the name of the tileset.
	/**
	 * @note This method is only used in the tileset editor.
	 */
        void setName(std::string name) {d_name = name;}

	//! Sets the copyright holders of the tileset.
	void setCopyright(std::string copy) {d_copyright = copy;};

	//! Sets the license of the tileset.
	void setLicense(std::string license) {d_license = license;};

	//! Set the description of the tileset.
	/**
	 * @note This method is only used in the tileset editor.
	 */
        void setInfo(std::string info) {d_info = info;}

	//!  Sets the tilesize of the tileset.
	void setTileSize(guint32 tileSize) {d_tileSize = tileSize;}

	//! Sets the basename of the file containing the big selector images.
	void setLargeSelectorFilename(std::string p){d_large_selector = p;};

	//! Sets the basename of the file containing the small selector images.
	void setSmallSelectorFilename(std::string p){d_small_selector = p;};

	//! Sets the basename of the file containing the explosion image.
	void setExplosionFilename(std::string p){d_explosion = p;};

	//! Sets the basename of the file containing the road images.
	void setRoadsFilename(std::string p){d_roads = p;};

	//! Sets the basename of the file containing the bridge images.
	void setBridgesFilename(std::string p){d_bridges = p;};

	//! Sets the basename of the file containing the fog images.
	void setFogFilename(std::string p){d_fog = p;};

	//! Sets the basename of the file containing the flag images.
	void setFlagsFilename(std::string p){d_flags = p;};

	//! Sets the colour of the road on the smallmap.
	void setRoadColor(Gdk::RGBA color) {d_road_color = color;};

	//! Sets the colour of the ruins on the smallmap.
	void setRuinColor(Gdk::RGBA color) {d_ruin_color = color;};

	//! Sets the colour of the temples on the smallmap.
	void setTempleColor(Gdk::RGBA color) {d_temple_color = color;};

	//! Sets the explosion image.
	void setExplosionImage(PixMask *p) {explosion = p;};

	//! Sets a road image.
	void setRoadImage(guint32 i, PixMask *p) {roadpic[i] = p;};

	//! Sets a bridge image.
	void setBridgeImage(guint32 i, PixMask *p) {bridgepic[i] = p;};

	//! Sets a flag image.
	void setFlagImage(guint32 i, PixMask *p) {flagpic[i] = p;};

	//! Sets a flag mask.
	void setFlagMask(guint32 i, PixMask *p) {flagmask[i] = p;};

	//! Sets a fog image.
	void setFogImage(guint32 i, PixMask *p) {fogpic[i] = p;};

	//! Sets a big selector image.
	void setSelectorImage(guint32 i, PixMask *p) {selector[i] = p;};

	//! Sets a big selector mask.
	void setSelectorMask(guint32 i, PixMask *p) {selectormask[i] = p;};

	//! Sets a small selector image.
	void setSmallSelectorImage(guint32 i, PixMask *p) {smallselector[i] = p;};
	//! Sets a small selector mask.
	void setSmallSelectorMask(guint32 i, PixMask *p) {smallselectormask[i] = p;};

	//! Sets the number of animation frames in the big selector.
	void setNumberOfSelectorFrames(guint32 s) {selector.reserve(s); selectormask.reserve(s); number_of_selector_frames = s;};

	//! Sets the number of animation frames in the small selector.
	void setNumberOfSmallSelectorFrames(guint32 s) {smallselector.reserve(s);smallselectormask.reserve(s); number_of_small_selector_frames = s;};

        std::string getFileFromConfigurationFile(std::string file);
        bool replaceFileInConfigurationFile(std::string file, std::string new_file);
        bool addFileInConfigurationFile(std::string new_file);

        //! Delete the tileset's temporary directory.
        void clean_tmp_dir() const;

	//Methods that operate on class data and modify the class data.

	//! Destroy the images assoicated with this tileset.
	void uninstantiateImages();

	//! Load the images assoicated with this tileset.
	void instantiateImages(bool &broken);

        //! Load the tileset again.
        void reload(bool &broken);
        
        //! make a new tilestyleset from an image and add it to the tile's list.
        bool addTileStyleSet(Tile *tile, std::string filename);

	//Methods that operate on class data and do not modify the class data.

        //! Returns the index to the given terrain type.
        int getIndex(Tile::Type type) const;

	//! Lookup tilestyle by it's id in this tileset.
	TileStyle *getTileStyle(guint32 id) const;

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
	TileStyle *getRandomTileStyle(guint32 index, TileStyle::Type style) const;

	//! Save a Tileset to an opened tile configuration file.
	/**
	 * @param  The opened XML tile configuration file.
	 */
	bool save(XML_Helper *helper) const;

        bool save(std::string filename, std::string extension) const;

	//! Get a unique tile style id among all tile syles in this tileset.
	int getFreeTileStyleId() const;

	//! Get the largest tile style id of all tile styles in this tileset.
	int getLargestTileStyleId() const;

	//! Check to see if this tileset is suitable for use within the game.
	bool validate() const;

        //! Determine the most common tile size in the graphic files.
        guint32 calculate_preferred_tile_size() const;

        //! Where does the given tile style live?
        bool getTileStyle(guint32 id, Tile **tile, TileStyleSet **set, TileStyle ** style) const;
	  
	// Static Methods

	//! Return the default height and width of a tile in the tileset.
	static guint32 getDefaultTileSize();

	//! Create a tileset from the given tileset configuration file.
	static Tileset *create(std::string file, bool &unsupported_version);
        
        static Tileset *copy (const Tileset *orig);

	//! Return a list of tileset basenames in the user's personal collection.
	static std::list<std::string> scanUserCollection();

	//! Return a list of tileset basenames in the system collection.
	static std::list<std::string> scanSystemCollection();

        //! Copy a tileset file from one place to another.
        static bool copy(std::string src, std::string dest);

        //! Rewrite old tileset files.
        static bool upgrade(std::string filename, std::string old_version, std::string new_version);
        static void support_backward_compatibility();
	
    private:
        //! Callback to load Tile objects into the Tileset.
        bool loadTile(std::string, XML_Helper* helper);

	//! Load the various images from the given filenames.
	void instantiateImages(std::string explosion_filename,
			       std::string roads_filename,
			       std::string bridges_filename,
			       std::string fog_filename,
			       std::string flags_filename,
			       std::string selector_filename,
			       std::string small_selector_filename,
                               bool &broken);
        // DATA

	//! The name of the Tileset.
	/**
	 * Equates to the tileset.d_name XML entity in the tileset 
	 * configuration file.
	 * This value appears in dialogs where the user is asked to select
	 * a Tileset among all other Tileset objects available to the game.
	 */
        std::string d_name;

	//! The copyright holders of the tileset.
        std::string d_copyright;

	//! The license of the tileset.
        std::string d_license;

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

	//! The base name of the Tileset.
	/**
	 * This is the base name of the file that the Tileset files are
	 * residing in.  It does not contain a path (e.g. no slashes).
	 * Tileset files sit in the tileset/ directory.
	 */
        std::string d_basename;

	//! The basename of the small selector image.
	/**
	 * The small selector is the graphic that appears on the bigmap when
	 * a stack is selected that only has one army unit in it.
	 *
	 * The image contains many animation frames, and is masked.
	 *
	 * This basename does not contain any slashes, and it does not contain
	 * a file extension.  It refers to a png file in the directory of 
	 * tileset.
	 */
	std::string d_small_selector;

	//! The basename of the large selector image.
	/**
	 * The large selector is the graphic that appears on the bigmap when
	 * a stack is selected that only has more than one army unit in it.
	 *
	 * The image contains many animation frames, and is masked.
	 *
	 * This basename does not contain any slashes, and it does not contain
	 * a file extension.  It refers to a png file in the directory of 
	 * tileset.
	 */
	std::string d_large_selector;

	//! The basename of the explosion image.
	/**
	 * The explosion image appears on the bigmap when stacks are fighting,
	 * and it also appears in the fight window when an army unit dies.
	 *
	 * This basename does not contain any slashes, and it does not contain
	 * a file extension.  It refers to a png file in the directory of 
	 * tileset.
	 */
	std::string d_explosion;

	//! The basename of the fog image.
	/**
	 * The fog images appear on the bigmap when playing with a hidden map.
	 *
	 * The number and order of frames in the image correlates to the 
	 * FogMap::ShadeType enumeration.
	 *
	 * This basename does not contain any slashes, and it does not contain
	 * a file extension.  It refers to a png file in the directory of 
	 * tileset.
	 */
	std::string d_fog;

	//! The basename of the road image.
	/**
	 * The road images appear on the bigmap overlaid on top of all kinds
	 * of tiles except for water.
	 *
	 * The number and order of frames in the image correlates to the 
	 * Road::Type enumeration.
	 *
	 * This basename does not contain any slashes, and it does not contain
	 * a file extension.  It refers to a png file in the directory of 
	 * tileset.
	 */
	std::string d_roads;

	//! The basename of the bridge image.
	/**
	 * The bridge images appear on the bigmap overlaid on top of certain
	 * water tiles.
	 *
	 * The number and order of frames in the image correlates to the 
	 * Bridge::Type enumeration.
	 *
	 * This basename does not contain any slashes, and it does not contain
	 * a file extension.  It refers to a png file in the directory of 
	 * tileset.
	 */
	std::string d_bridges;

	//! The basename of the flag image.
	/**
	 * The flag images appear on the bigmap beside a stack to indicate the
	 * number of army units in the stack.
	 *
	 * The number of frames in the image corresponds to the maximum number
	 * of army units in a stack.  See the FLAG_TYPES constant in defs.h.
	 *
	 * This basename does not contain any slashes, and it does not contain
	 * a file extension.  It refers to a png file in the directory of 
	 * tileset.
	 */
	std::string d_flags;

        typedef std::map<guint32, TileStyle*> TileStyleIdMap;
	//! A map that provides a TileStyle when supplying a TileStyle id.
        TileStyleIdMap d_tilestyles;

	//! The colour of roads on the smallmap.
	Gdk::RGBA d_road_color;

	//! The colour of ruins on the smallmap.
	Gdk::RGBA d_ruin_color;

	//! The colour of temples on the smallmap.
	Gdk::RGBA d_temple_color;

	//! The road images.
        PixMask* roadpic[ROAD_TYPES];

	//! The bridge images.
        PixMask* bridgepic[BRIDGE_TYPES];

	//! The flag images.
        PixMask* flagpic[FLAG_TYPES];

	//! The flag masks.
        PixMask* flagmask[FLAG_TYPES];

	//! The number of animation frames in the big selector.
	guint32 number_of_selector_frames;

	//! The image frames in the big selector.
	std::vector<PixMask* > selector;

	//! The mask frames of the big selector.
	std::vector<PixMask* > selectormask;

	//! The number of animation frames in the small selector.
	guint32 number_of_small_selector_frames;

	//! The image frames of the small selector.
	std::vector<PixMask* > smallselector;

	//! The mask frames of the small selector.
	std::vector<PixMask* > smallselectormask;

	//! The exposion image.
	PixMask* explosion;

	//! The fog images.
	PixMask*fogpic[FOG_TYPES];
};
#endif // TILESET_H

// End of file
